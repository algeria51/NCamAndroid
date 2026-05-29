#pragma once

#ifdef __ANDROID__

#include <android/log.h>
#include <errno.h>
#include <stdio.h>
// Pull in unistd.h BEFORE defining any macros that shadow its symbols.
// NDK 25+ declares fork() and daemon() in unistd.h; if our macros are
// defined first the compiler sees a function-like macro with the wrong
// arity when it parses the header declarations.
#include <unistd.h>

#define ANDROID_LOG_TAG "NCam"
#define cs_android_log(...) __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)

// ── fork / daemon ─────────────────────────────────────────────────────────
// Undefine the real symbols first so our zero-returning macros don't clash
// with the already-parsed declarations in unistd.h.
#undef fork
#undef daemon
#define fork()          0
#define do_daemon(a,b)  0
#define daemon(a,b)     0

// ── system / popen ────────────────────────────────────────────────────────
static inline int ncam_android_system(const char *cmd)
{
    (void)cmd;
    cs_android_log("system() blocked: %s", cmd ? cmd : "(null)");
    errno = ENOSYS;
    return -1;
}
static inline FILE *ncam_android_popen(const char *cmd, const char *mode)
{
    (void)cmd; (void)mode;
    cs_android_log("popen() blocked: %s", cmd ? cmd : "(null)");
    errno = ENOSYS;
    return NULL;
}
static inline int ncam_android_pclose(FILE *f)
{
    (void)f;
    return 0;
}
#undef system
#undef popen
#undef pclose
#define system(x)   ncam_android_system(x)
#define popen(x,y)  ncam_android_popen(x,y)
#define pclose(x)   ncam_android_pclose(x)

#define HAVE_STATIC_CRASH_HANDLER 1

#ifdef LCDSUPPORT
#undef LCDSUPPORT
#endif
#ifdef LEDSUPPORT
#undef LEDSUPPORT
#endif
#ifdef HAVE_DVBAPI
#undef HAVE_DVBAPI
#endif
#ifdef WITH_CARDREADER
#undef WITH_CARDREADER
#endif

#define CARDREADER_PHOENIX    0
#define CARDREADER_INTERNAL   0
#define CARDREADER_STINGER    0
#define CARDREADER_PCSC       0
#define CARDREADER_SMART      0
#define CARDREADER_DB2COM     0
#define CARDREADER_SC8IN1     0
#define CARDREADER_SMARGO     0
#define CARDREADER_MP35       0

#ifdef MODULE_SERIAL
#undef MODULE_SERIAL
#endif
#ifdef WITH_STAPI
#undef WITH_STAPI
#endif
#ifdef WITH_STAPI5
#undef WITH_STAPI5
#endif
#ifdef WITH_COOLAPI
#undef WITH_COOLAPI
#endif
#ifdef WITH_COOLAPI2
#undef WITH_COOLAPI2
#endif
#ifdef WITH_AZBOX
#undef WITH_AZBOX
#endif
#ifdef WITH_GXAPI
#undef WITH_GXAPI
#endif

#define SKIP_WEBIF_LSUSB 1

#ifndef CS_CONFDIR
#define CS_CONFDIR "/data/data/com.ncam.app/files"
#endif

#endif // __ANDROID__
