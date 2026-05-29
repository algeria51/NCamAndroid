cmake_minimum_required(VERSION 3.22)
project(ncam_android C CXX)

set(NCAM_SRC ${CMAKE_SOURCE_DIR}/ncam)

set(COMPAT_HEADER ${CMAKE_SOURCE_DIR}/android_compat.h)
add_compile_options(-include ${COMPAT_HEADER})

# Generate build date string (ncam.c references CS_BUILD_DATE)
string(TIMESTAMP CS_BUILD_DATE "%d.%m.%Y %H:%M:%S")

add_compile_definitions(
    __ANDROID__=1
    ANDROID_BUILD=1
    "CS_BUILD_DATE=\"${CS_BUILD_DATE}\""
    CS_CONFDIR="/data/data/com.ncam.app/files"
    HAVE_DVBAPI=0
    WITH_CARDREADER=0
    MODULE_SERIAL=0
    LCDSUPPORT=0
    LEDSUPPORT=0
    WITH_STAPI=0
    WITH_STAPI5=0
    WITH_COOLAPI=0
    WITH_COOLAPI2=0
    WITH_AZBOX=0
    WITH_GXAPI=0
    WITH_EMU=1
    WITH_SOFTCAM=1
    MODULE_CAMD35=1
    MODULE_CAMD35_TCP=1
    MODULE_NEWCAMD=1
    MODULE_CCCAM=1
    MODULE_CCCSHARE=1
    MODULE_GBOX=1
    MODULE_MONITOR=1
    MODULE_CONSTCW=1
    MODULE_SCAM=1
    CS_ANTICASC=1
    CS_CACHEEX=1
    CS_CACHEEX_AIO=1
    CW_CYCLE_CHECK=1
    WEBIF=1
    WEBIF_LIVELOG=1
    WITH_DEBUG=1
    WITH_LB=1
    READ_SDT_CHARSETS=1
    READER_NAGRA=1
    READER_IRDETO=1
    READER_CONAX=1
    READER_CRYPTOWORKS=1
    READER_SECA=1
    READER_VIACCESS=1
    READER_VIDEOGUARD=1
    READER_DRE=1
    READER_BULCRYPT=1
    READER_GRIFFIN=1
    READER_DGCRYPT=1
    READER_STREAMGUARD=1
    READER_JET=1
    READER_TONGFANG=1
)

include_directories(
    ${NCAM_SRC}
    ${NCAM_SRC}/csctapi
    ${NCAM_SRC}/cscrypt
    ${NCAM_SRC}/extapi
    ${NCAM_SRC}/minilzo
    ${NCAM_SRC}/tommyDS_hashlin
    ${CMAKE_SOURCE_DIR}
)

file(GLOB NCAM_CORE    ${NCAM_SRC}/ncam*.c)
file(GLOB NCAM_MODULES ${NCAM_SRC}/module-*.c)
file(GLOB NCAM_READERS ${NCAM_SRC}/reader-*.c)
file(GLOB NCAM_CSCRYPT ${NCAM_SRC}/cscrypt/*.c)
file(GLOB NCAM_TOMMY   ${NCAM_SRC}/tommyDS_hashlin/*.c)
file(GLOB NCAM_CSCTAPI ${NCAM_SRC}/csctapi/*.c)

set(NCAM_EXTRA
    ${NCAM_SRC}/cardlist.c
    ${NCAM_SRC}/minilzo/minilzo.c
    ${NCAM_SRC}/ncam.c
)

# ncam-signing.c requires OpenSSL (X509, BIO, etc.) — excluded and replaced
# by android_signing_stub.c which provides safe no-op implementations.
list(FILTER NCAM_CORE   EXCLUDE REGEX "ncam-signing\\.c$")

list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-stapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-coolapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-azbox.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-gxapi.*\\.c$")
list(FILTER NCAM_MODULES EXCLUDE REGEX "module-dvbapi-mca.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_stapi.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_cool.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_azbox.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_amsmc.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_gx.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_smartreader.*\\.c$")
list(FILTER NCAM_CSCTAPI  EXCLUDE REGEX "ifd_pcsc.*\\.c$")

add_library(ncam SHARED
    ${CMAKE_SOURCE_DIR}/jni_bridge.cpp
    ${CMAKE_SOURCE_DIR}/android_stubs.c
    ${CMAKE_SOURCE_DIR}/android_signing_stub.c
    ${NCAM_CORE}
    ${NCAM_MODULES}
    ${NCAM_READERS}
    ${NCAM_CSCTAPI}
    ${NCAM_CSCRYPT}
    ${NCAM_TOMMY}
    ${NCAM_EXTRA}
)

target_link_libraries(ncam
    android
    log
    z
)
