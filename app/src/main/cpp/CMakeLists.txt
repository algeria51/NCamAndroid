cmake_minimum_required(VERSION 3.22)
project(ncam_android C CXX)

set(NCAM_SRC ${CMAKE_SOURCE_DIR}/ncam)

# Force-include android_compat.h في كل ملف مترجم
set(COMPAT_HEADER ${CMAKE_SOURCE_DIR}/android_compat.h)
add_compile_options(-include ${COMPAT_HEADER})

# ملاحظة: جميع خيارات التفعيل/التعطيل (WITH_EMU, WEBIF, MODULE_*, ...) 
# تُدار من ncam/config.h مباشرة — لا نُعيد تعريفها هنا لتجنب تعارض الماكرو.
# CMakeLists يُعرِّف فقط ما لا يوجد في config.h:

string(TIMESTAMP CS_BUILD_DATE "%d.%m.%Y %H:%M:%S")

add_compile_definitions(
    __ANDROID__=1
    ANDROID_BUILD=1
    "CS_BUILD_DATE=\"${CS_BUILD_DATE}\""
    CS_CONFDIR="/data/data/com.ncam.app/files/ncam"
    NEED_DAEMON=1
)

include_directories(
    ${NCAM_SRC}
    ${NCAM_SRC}/csctapi
    ${NCAM_SRC}/cscrypt
    ${NCAM_SRC}/extapi
    ${NCAM_SRC}/minilzo
    ${NCAM_SRC}/tommyDS_hashlin
    ${NCAM_SRC}/webif
    ${CMAKE_SOURCE_DIR}
)

file(GLOB NCAM_CORE    ${NCAM_SRC}/ncam*.c)
file(GLOB NCAM_MODULES ${NCAM_SRC}/module-*.c)
file(GLOB NCAM_READERS ${NCAM_SRC}/reader-*.c)
file(GLOB NCAM_CSCRYPT ${NCAM_SRC}/cscrypt/*.c)
file(GLOB NCAM_CSCTAPI ${NCAM_SRC}/csctapi/*.c)
# tommy*.c مُضمَّنة مباشرة داخل ncam-hashtable.c بـ #include
# لذا لا تُضاف هنا — تعريفها في add_library يسبب duplicate symbols
# file(GLOB NCAM_TOMMY ...) — intentionally excluded

set(NCAM_EXTRA
    ${NCAM_SRC}/cardlist.c
    ${NCAM_SRC}/minilzo/minilzo.c
    ${NCAM_SRC}/webif/pages.c
)

# ncam-signing.c يحتاج OpenSSL — استُبدل بـ android_signing_stub.c
list(FILTER NCAM_CORE EXCLUDE REGEX "ncam-signing\\.c$")

# وحدات DVB API لا تعمل على Android
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-stapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-coolapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-azbox.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-gxapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-mca.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi\\.c$")

# Card reader drivers لا تعمل على Android (لا USB/serial)
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_stapi.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_cool.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_azbox.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_amsmc.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_gx.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_smartreader.*\\.c$")
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_pcsc.*\\.c$")

add_library(ncam SHARED
    ${CMAKE_SOURCE_DIR}/jni_bridge.cpp
    ${CMAKE_SOURCE_DIR}/android_stubs.c
    ${CMAKE_SOURCE_DIR}/android_signing_stub.c
    ${NCAM_CORE}
    ${NCAM_MODULES}
    ${NCAM_READERS}
    ${NCAM_CSCTAPI}
    ${NCAM_CSCRYPT}
    ${NCAM_EXTRA}
)

target_link_libraries(ncam
    android
    log
    z
)
