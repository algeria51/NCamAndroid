#pragma once

#ifdef __ANDROID__

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
#define cs_android_log(...) \
    __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, __VA_ARGS__)

#ifndef CS_CONFDIR
#define CS_CONFDIR "/data/data/com.ncam.app/files/ncam"
#endif

/*
 * Bionic (Android libc) on ARM 32-bit aborts with "invalid pthread_t" when
 * pthread_join() is called on a thread that has already exited and been
 * cleaned up (detached or previously joined).  The POSIX-correct return value
 * would be ESRCH, but Bionic calls abort() instead.
 *
 * We replace pthread_join with a wrapper that catches the signal and returns
 * ESRCH safely, so NCam's SAFE_THREAD_JOIN / direct pthread_join calls never
 * crash the process.
 */
#include <setjmp.h>

static sigjmp_buf  _android_pthread_join_jmp;
static volatile sig_atomic_t _android_pthread_join_guard = 0;

static void _android_sigabrt_handler(int sig)
{
    (void)sig;
    if (_android_pthread_join_guard)
        siglongjmp(_android_pthread_join_jmp, 1);
}

static inline int android_safe_pthread_join(pthread_t t, void **retval)
{
    struct sigaction sa_new, sa_old;
    sa_new.sa_handler = _android_sigabrt_handler;
    sigemptyset(&sa_new.sa_mask);
    sa_new.sa_flags = SA_RESETHAND;
    sigaction(SIGABRT, &sa_new, &sa_old);

    _android_pthread_join_guard = 1;
    int rc;
    if (sigsetjmp(_android_pthread_join_jmp, 1) == 0) {
        rc = pthread_join(t, retval);
    } else {
        rc = ESRCH;
    }
    _android_pthread_join_guard = 0;
    sigaction(SIGABRT, &sa_old, NULL);
    return rc;
}

#define pthread_join(t, r)  android_safe_pthread_join((t), (r))

#endif /* __ANDROID__ */
