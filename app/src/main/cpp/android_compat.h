#pragma once

#ifdef __ANDROID__

/* Must be defined before any system header to unlock POSIX/GNU extensions:
 * pthread_rwlock_t, struct sigaction, SA_RESTART, struct addrinfo, PATH_MAX */
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#undef  _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <android/log.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <limits.h>
#include <dirent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef unsigned int uint;

#define ANDROID_LOG_TAG "NCam"
#define cs_android_log(...) __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)

/* ── fork / daemon / do_daemon ─────────────────────────────────────────────
 * fork() and daemon() are problematic on Android (SELinux EPERM).
 * do_daemon() in ncam.c calls fork()+setsid() — we replace it entirely.
 */
#undef fork
#undef daemon
#define fork()         0
#define daemon(a,b)    0
#define do_daemon(a,b) 0

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
#define system(x)    _ncam_system(x)
#define popen(x,y)   _ncam_popen(x,y)
#define pclose(x)    _ncam_pclose(x)

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
