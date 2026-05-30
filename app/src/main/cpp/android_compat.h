#pragma once

#ifdef __ANDROID__

#include <android/log.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <limits.h>
#include <dirent.h>
#include <stdint.h>

typedef unsigned int uint;

#define ANDROID_LOG_TAG "NCam"
#define cs_android_log(...) __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)

/* ── fork / daemon / do_daemon ─────────────────────────────────────────────
 * NCam uses fork() in do_daemon() and restart_daemon(). On Android these
 * calls are either forbidden or cause SELinux denials.
 * - fork()      → always return 0  (child side, no parent exits)
 * - daemon()    → no-op, return 0
 * - do_daemon() → no-op macro (ncam.c defines a static do_daemon that calls
 *                 fork()+setsid(); even with fork()→0, setsid() can fail
 *                 with EPERM under strict SELinux → exit=1)
 * We must define do_daemon BEFORE ncam.c is compiled so our macro wins.
 */
#undef fork
#undef daemon
#define fork()         0
#define daemon(a,b)    0
#define do_daemon(a,b) 0    /* skip daemon mode entirely on Android */

/* ── system / popen ─────────────────────────────────────────────────────── */
static inline int _ncam_system(const char *cmd)
{
    (void)cmd;
    cs_android_log("system() blocked: %s", cmd ? cmd : "(null)");
    errno = ENOSYS;
    return -1;
}
static inline FILE *_ncam_popen(const char *cmd, const char *mode)
{
    (void)cmd; (void)mode;
    cs_android_log("popen() blocked: %s", cmd ? cmd : "(null)");
    errno = ENOSYS;
    return NULL;
}
static inline int _ncam_pclose(FILE *f) { (void)f; return 0; }
#undef system
#undef popen
#undef pclose
#define system(x)   _ncam_system(x)
#define popen(x,y)  _ncam_popen(x,y)
#define pclose(x)   _ncam_pclose(x)

#define HAVE_STATIC_CRASH_HANDLER 1
#define SKIP_WEBIF_LSUSB 1

/* Disable unsupported hardware subsystems */
#undef LCDSUPPORT
#undef LEDSUPPORT
#undef HAVE_DVBAPI
#undef WITH_CARDREADER
#undef WITH_STAPI
#undef WITH_STAPI5
#undef WITH_COOLAPI
#undef WITH_COOLAPI2
#undef WITH_AZBOX
#undef WITH_GXAPI
#undef WITH_WI

#ifndef CS_CONFDIR
#define CS_CONFDIR "/data/data/com.ncam.app/files/ncam"
#endif

#endif /* __ANDROID__ */
