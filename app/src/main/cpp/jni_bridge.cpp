#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#define TAG  "NCam-JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

extern "C" {
    extern int32_t main(int32_t argc, char *argv[]);
    extern volatile int exit_oscam;
}

static pthread_t  g_ncam_thread   = 0;
static JavaVM    *g_jvm           = nullptr;
static jobject    g_callback      = nullptr;

/* ── stdout → logcat bridge ─────────────────────────────────────────────── */
static int  g_pipe_fds[2]   = {-1, -1};
static int  g_saved_stdout  = -1;
static int  g_saved_stderr  = -1;

static void *stdout_reader_func(void *)
{
    char line[4096];
    int  pos = 0;
    char ch;
    while (true) {
        ssize_t n = read(g_pipe_fds[0], &ch, 1);
        if (n <= 0) break;
        if (ch == '\n' || pos >= (int)sizeof(line) - 1) {
            line[pos] = '\0';
            if (pos > 0)
                __android_log_print(ANDROID_LOG_DEBUG, "NCam", "%s", line);
            pos = 0;
        } else {
            line[pos++] = ch;
        }
    }
    return nullptr;
}

static void redirect_stdout()
{
    if (pipe(g_pipe_fds) != 0) { LOGE("pipe() failed: %s", strerror(errno)); return; }
    g_saved_stdout = dup(STDOUT_FILENO);
    g_saved_stderr = dup(STDERR_FILENO);
    dup2(g_pipe_fds[1], STDOUT_FILENO);
    dup2(g_pipe_fds[1], STDERR_FILENO);
    close(g_pipe_fds[1]);
    g_pipe_fds[1] = -1;
    pthread_t t;
    pthread_create(&t, nullptr, stdout_reader_func, nullptr);
    pthread_detach(t);
}

static void restore_stdout()
{
    if (g_saved_stdout >= 0) { dup2(g_saved_stdout, STDOUT_FILENO); close(g_saved_stdout); g_saved_stdout = -1; }
    if (g_saved_stderr >= 0) { dup2(g_saved_stderr, STDERR_FILENO); close(g_saved_stderr); g_saved_stderr = -1; }
    if (g_pipe_fds[0] >= 0) { close(g_pipe_fds[0]); g_pipe_fds[0] = -1; }
}

/* ── NCam argument block ─────────────────────────────────────────────────── */
struct NcamArgs {
    int   argc;
    char *argv[32];
    char  buf[2048];
};
static NcamArgs g_args;

/* ── NCam thread ─────────────────────────────────────────────────────────── */
static void *ncam_thread_func(void *arg)
{
    NcamArgs *a = (NcamArgs *)arg;
    LOGI("NCam thread starting (argc=%d)", a->argc);
    for (int i = 0; i < a->argc; i++)
        LOGD("  argv[%d] = %s", i, a->argv[i]);

    int ret = main(a->argc, a->argv);
    LOGI("NCam main() returned %d", ret);

    restore_stdout();

    if (g_jvm && g_callback) {
        JNIEnv *env = nullptr;
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) {
            jclass    cls = env->GetObjectClass(g_callback);
            jmethodID m   = env->GetMethodID(cls, "onNcamStopped", "(I)V");
            if (m) env->CallVoidMethod(g_callback, m, (jint)ret);
            g_jvm->DetachCurrentThread();
        }
    }
    g_ncam_thread = 0;
    return nullptr;
}

/* ── JNI_OnLoad ─────────────────────────────────────────────────────────── */
extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
    g_jvm = vm;
    return JNI_VERSION_1_6;
}

/* ── startNCam ───────────────────────────────────────────────────────────── */
extern "C"
JNIEXPORT jint JNICALL
Java_com_ncam_app_NCamJNI_startNCam(JNIEnv *env, jobject,
                                     jstring jConfigDir,
                                     jobject jCallback)
{
    if (g_ncam_thread) { LOGE("NCam already running"); return -1; }

    const char *confdir = env->GetStringUTFChars(jConfigDir, nullptr);
    LOGI("startNCam: confdir=%s", confdir);

    /* Ensure directories exist */
    mkdir(confdir, 0700);

    /* Build tmp dir inside confdir */
    char tmpdir[512];
    snprintf(tmpdir, sizeof(tmpdir), "%s/tmp", confdir);
    mkdir(tmpdir, 0700);
    LOGI("tmpdir=%s", tmpdir);

    /* Build argv */
    memset(&g_args, 0, sizeof(g_args));
    char *p = g_args.buf;

    auto push = [&](const char *s) {
        g_args.argv[g_args.argc++] = p;
        while (*s) *p++ = *s++;
        *p++ = '\0';
    };

    push("ncam");
    push("-f");           /* foreground: skip daemon/fork entirely */
    push("-c"); push(confdir);
    push("-t"); push(tmpdir);
    push("-r"); push("0"); /* disable restart mode */

    env->ReleaseStringUTFChars(jConfigDir, confdir);

    if (jCallback) {
        if (g_callback) env->DeleteGlobalRef(g_callback);
        g_callback = env->NewGlobalRef(jCallback);
    }

    redirect_stdout();
    exit_oscam = 0;

    int rc = pthread_create(&g_ncam_thread, nullptr, ncam_thread_func, &g_args);
    if (rc != 0) {
        LOGE("pthread_create failed: %d", rc);
        restore_stdout();
        g_ncam_thread = 0;
        return -1;
    }
    pthread_detach(g_ncam_thread);
    LOGI("NCam thread launched");
    return 0;
}

/* ── stopNCam ────────────────────────────────────────────────────────────── */
extern "C"
JNIEXPORT void JNICALL
Java_com_ncam_app_NCamJNI_stopNCam(JNIEnv *, jobject)
{
    if (!g_ncam_thread) return;
    LOGI("Requesting NCam stop");
    exit_oscam = 1;
    usleep(500000);
    g_ncam_thread = 0;
}

/* ── isRunning ───────────────────────────────────────────────────────────── */
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_ncam_app_NCamJNI_isRunning(JNIEnv *, jobject)
{
    return (g_ncam_thread != 0) ? JNI_TRUE : JNI_FALSE;
}

/* ── getVersion ──────────────────────────────────────────────────────────── */
extern "C"
JNIEXPORT jstring JNICALL
Java_com_ncam_app_NCamJNI_getVersion(JNIEnv *env, jobject)
{
    extern const char *ncam_version;
    return env->NewStringUTF(ncam_version ? ncam_version : "unknown");
}
