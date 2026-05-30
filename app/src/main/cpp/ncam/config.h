#ifndef CONFIG_H_
#define CONFIG_H_

/* ============================================================
 * NCam Android build configuration
 * يُعدَّل هذا الملف مباشرة — لا نستخدم -D flags في CMake
 * لتجنب تعارض الماكرو مع globals.h
 * ============================================================ */

/* --- Core features --- */
#define WITH_EMU            1
#define WITH_SOFTCAM        1
#define WITH_DEBUG          1
#define WITH_LB             1
#define READ_SDT_CHARSETS   1
#define CS_ANTICASC         1
#define CS_CACHEEX          1
#define CS_CACHEEX_AIO      1
#define CW_CYCLE_CHECK      1

/* --- WebIF --- */
#define WEBIF               1
#define WEBIF_LIVELOG       1
/* #define WEBIF_JQUERY       1 */   /* يحتاج curl */
/* #define WITH_COMPRESS_WEBIF 1 */  /* يحتاج zlib webif */

/* --- Protocols --- */
#define MODULE_MONITOR      1
#define MODULE_CAMD35       1
#define MODULE_CAMD35_TCP   1
#define MODULE_NEWCAMD      1
#define MODULE_CCCAM        1
#define MODULE_CCCSHARE     1
#define MODULE_GBOX         1
#define MODULE_CONSTCW      1
#define MODULE_SCAM         1

/* --- معطّل على Android --- */
/* #define HAVE_DVBAPI        1 */   /* DVB API غير موجود */
/* #define MODULE_SERIAL      1 */
/* #define LCDSUPPORT         1 */
/* #define LEDSUPPORT         1 */
/* #define WITH_SIGNING       1 */   /* يحتاج OpenSSL */
/* #define WITH_SSL           1 */
/* #define MODULE_CAMD33      1 */
/* #define MODULE_RADEGAST    1 */
/* #define MODULE_PANDORA     1 */
/* #define MODULE_GHTTP       1 */
/* #define MODULE_STREAMRELAY 1 */

/* --- WITH_CARDREADER مُفعَّل لكن drivers الفعلية stub-only ---
 * android_stubs.c يوفر no-op implementations لكل ICC_Async_* functions
 * بينما رمز الإعداد (ncam-config-reader.c) يحتاج struct s_reader كاملاً */
#define WITH_CARDREADER     1

#ifdef WITH_CARDREADER

/* Readers: مُفعَّلة فقط لأغراض الـ parsing — لا hardware فعلي */
#define READER_NAGRA        1
#define READER_NAGRA_MERLIN 1
#define READER_IRDETO       1
#define READER_CONAX        1
#define READER_CRYPTOWORKS  1
#define READER_SECA         1
#define READER_VIACCESS     1
#define READER_VIDEOGUARD   1
#define READER_DRE          1
#define READER_TONGFANG     1
#define READER_STREAMGUARD  1
#define READER_JET          1
#define READER_BULCRYPT     1
#define READER_GRIFFIN      1
#define READER_DGCRYPT      1

/* Physical card reader drivers: كلها معطلة على Android */
/* #define CARDREADER_PHOENIX  1 */
/* #define CARDREADER_INTERNAL 1 */
/* #define CARDREADER_STINGER  1 */
/* #define CARDREADER_MP35     1 */
/* #define CARDREADER_SC8IN1   1 */
/* #define CARDREADER_SMARGO   1 */
/* #define CARDREADER_DB2COM   1 */
/* #define CARDREADER_DRECAS   1 */
/* #define CARDREADER_PCSC     1 */
/* #define CARDREADER_SMART    1 */

#ifdef READER_DRE
#define READER_DRECAS       1
#endif

#endif /* WITH_CARDREADER */

#endif /* CONFIG_H_ */
