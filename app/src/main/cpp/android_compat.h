#pragma once

#ifdef __ANDROID__

/* Unlock POSIX/GNU extensions before any system header */
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

#endif /* __ANDROID__ */
