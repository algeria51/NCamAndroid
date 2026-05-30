#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#define TAG "NCam-JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" {
    extern int32_t main(int32_t argc, char *argv[]);
    extern volatile int exit_oscam;
}

static volatile pthread_t  g_thread   = 0;
static JavaVM    *g_jvm      = nullptr;
static jobject    g_callback = nullptr;

/* Args passed to NCam main() */
struct NcamArgs {
    int   argc;
    char *argv[16];
    char  buf[1024];
};
static NcamArgs g_args;

static void *ncam_thread(void *arg)
{
    NcamArgs *a = (NcamArgs *)arg;
    LOGI("Starting NCam (argc=%d)", a->argc);
    for(int i = 0; i < a->argc; i++)
        LOGI("  argv[%d]=%s", i, a->argv[i]);

    int ret = main(a->argc, a->argv);
    LOGI("NCam exited with code %d", ret);

    if(g_jvm && g_callback) {
        JNIEnv *env = nullptr;
        if(g_jvm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            jclass    cls = env->GetObjectClass(g_callback);
            jmethodID mid = env->GetMethodID(cls, "onNcamStopped", "(I)V");
            if(mid) env->CallVoidMethod(g_callback, mid, (jint)ret);
            env->DeleteGlobalRef(g_callback);
            g_callback = nullptr;
            g_jvm->DetachCurrentThread();
        }
    }
    g_thread = 0;
    return nullptr;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    g_jvm = vm;
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_ncam_app_NCamJNI_startNCam(JNIEnv *env, jobject,
                                     jstring jConfDir, jobject jCallback)
{
    if(g_thread) { LOGE("Already running"); return -1; }

    const char *confdir = env->GetStringUTFChars(jConfDir, nullptr);
    LOGI("confdir=%s", confdir);

    /* Ensure dirs exist */
    mkdir(confdir, 0700);
    char tmpdir[512];
    snprintf(tmpdir, sizeof(tmpdir), "%s/tmp", confdir);
    mkdir(tmpdir, 0700);

    /* Build argv */
    memset(&g_args, 0, sizeof(g_args));
    char *p = g_args.buf;
    auto push = [&](const char *s) {
        g_args.argv[g_args.argc++] = p;
        while(*s) *p++ = *s++;
        *p++ = '\0';
    };

    push("ncam");
    push("-f");           /* foreground — daemon code already disabled in source */
    push("-c"); push(confdir);
    push("-t"); push(tmpdir);
    push("-r"); push("0"); /* no restart daemon */

    env->ReleaseStringUTFChars(jConfDir, confdir);

    if(jCallback) {
        if(g_callback) env->DeleteGlobalRef(g_callback);
        g_callback = env->NewGlobalRef(jCallback);
    }

    exit_oscam = 0;
    pthread_t tid = 0;
    int rc = pthread_create(&tid, nullptr, ncam_thread, &g_args);
    if(rc) { LOGE("pthread_create failed: %d", rc); return -1; }
    pthread_detach(tid);
    g_thread = tid;
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_ncam_app_NCamJNI_stopNCam(JNIEnv *, jobject)
{
    if(!g_thread) return;
    LOGI("Stopping NCam");
    exit_oscam = 1;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_ncam_app_NCamJNI_isRunning(JNIEnv *, jobject)
{
    return g_thread ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_ncam_app_NCamJNI_getVersion(JNIEnv *env, jobject)
{
    /* ncam_version is defined in android_signing_stub.c as "Android".
     * CS_VERSION is the build-time version from CMake defines. */
    extern const char *ncam_version;
#ifdef CS_VERSION
    return env->NewStringUTF(CS_VERSION);
#else
    return env->NewStringUTF(ncam_version ? ncam_version : "NCam-Android");
#endif
}
