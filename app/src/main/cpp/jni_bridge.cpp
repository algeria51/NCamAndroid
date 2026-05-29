#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define TAG "NCam-JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" {
    extern int32_t main(int32_t argc, char *argv[]);
    extern volatile int exit_oscam;
}

static pthread_t  g_ncam_thread = 0;
static JavaVM    *g_jvm         = nullptr;
static jobject    g_callback    = nullptr;

struct NcamArgs {
    int   argc;
    char *argv[32];
    char  buf[2048];
};

static NcamArgs g_args;

static void *ncam_thread_func(void *arg)
{
    NcamArgs *a = (NcamArgs *)arg;
    LOGI("NCam thread starting");
    int ret = main(a->argc, a->argv);
    LOGI("NCam thread exited with %d", ret);

    if (g_jvm && g_callback) {
        JNIEnv *env = nullptr;
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) {
            jclass   cls = env->GetObjectClass(g_callback);
            jmethodID m  = env->GetMethodID(cls, "onNcamStopped", "(I)V");
            if (m) env->CallVoidMethod(g_callback, m, (jint)ret);
            g_jvm->DetachCurrentThread();
        }
    }
    return nullptr;
}

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    g_jvm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_ncam_app_NCamJNI_startNCam(JNIEnv *env, jobject,
                                     jstring jConfigDir,
                                     jobject jCallback)
{
    if (g_ncam_thread) {
        LOGE("NCam already running");
        return -1;
    }

    const char *confdir = env->GetStringUTFChars(jConfigDir, nullptr);

    memset(&g_args, 0, sizeof(g_args));
    char *p = g_args.buf;

    auto push = [&](const char *s) {
        g_args.argv[g_args.argc++] = p;
        while (*s) *p++ = *s++;
        *p++ = '\0';
    };

    push("ncam");
    push("-c"); push(confdir);
    push("-t"); push(confdir);

    env->ReleaseStringUTFChars(jConfigDir, confdir);

    if (jCallback) {
        if (g_callback) env->DeleteGlobalRef(g_callback);
        g_callback = env->NewGlobalRef(jCallback);
    }

    exit_oscam = 0;

    int rc = pthread_create(&g_ncam_thread, nullptr, ncam_thread_func, &g_args);
    if (rc != 0) {
        LOGE("pthread_create failed: %d", rc);
        g_ncam_thread = 0;
        return -1;
    }
    pthread_detach(g_ncam_thread);
    LOGI("NCam thread launched");
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ncam_app_NCamJNI_stopNCam(JNIEnv *, jobject)
{
    if (!g_ncam_thread) return;
    LOGI("Requesting NCam stop");
    exit_oscam = 1;
    // give ncam a moment to exit cleanly
    usleep(500000);
    g_ncam_thread = 0;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_ncam_app_NCamJNI_isRunning(JNIEnv *, jobject)
{
    return (g_ncam_thread != 0) ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_ncam_app_NCamJNI_getVersion(JNIEnv *env, jobject)
{
    extern const char *ncam_version;
    return env->NewStringUTF(ncam_version ? ncam_version : "unknown");
}
