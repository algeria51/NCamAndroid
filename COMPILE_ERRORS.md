# Common Compile Errors & Fixes

All fixes go into `android_compat.h` or `CMakeLists.txt` only.
Never modify NCam source files.

---

## 1. `getpwuid` / `getpwnam` undefined
Already handled in `globals.h` via `#ifndef __ANDROID__` guards.
No action needed.

## 2. `termios` / `ioctl` serial errors
These come from `csctapi/io_serial.c`.
Since `WITH_CARDREADER` is off, the code is `#ifdef`-guarded.
If you see stray includes, add to `android_compat.h`:
```c
#define TIOCMGET 0
#define TIOCMSET 0
#define TIOCM_RTS 0
#define TIOCM_DTR 0
```

## 3. `sys/io.h` not found (x86 only)
Add to CMakeLists.txt filter:
```cmake
list(FILTER NCAM_CSCTAPI EXCLUDE REGEX "ifd_db2com.*\\.c$")
```

## 4. `openssl/ssl.h` not found
Either install OpenSSL via prefab or keep `WITH_SSL` undefined (default).
NCam works fine without SSL for basic protocols.

## 5. `iconv` not found
Add to `android_compat.h`:
```c
#define iconv_t            void*
#define iconv_open(a,b)    NULL
#define iconv(a,b,c,d,e)   ((size_t)-1)
#define iconv_close(a)     0
```

## 6. `gdb` crash handler — `system()` call in `ncam.c:735`
Blocked by `system()` macro in `android_compat.h`. Returns -1 silently.

## 7. `restart_daemon` / `fork()` in `ncam.c`
`fork()` is already `#define fork() 0` in `globals.h` for `__uClinux__`.
Our `android_compat.h` adds the same for `__ANDROID__`.
`restart_daemon()` will loop on `fork()` returning 0 (child path),
which means it enters the main init path directly — correct behavior.

## 8. `WEBIF_JQUERY` / compressed pages
Keep enabled. The webif serves on a local socket; works fine on Android.
Access via browser: `http://localhost:8888`

## 9. Module-specific `popen` (gbox-sms, webif, dvbapi)
All blocked by `popen()` macro. Functions return early with NULL check.

## 10. `linker: undefined symbol` for disabled cardreaders
If linker complains about `cardreader_internal_sci` etc., add stubs:
```c
// in android_stubs.c (new file, add to CMakeLists NCAM_EXTRA)
#include "globals.h"
#ifdef __ANDROID__
const struct s_cardreader cardreader_internal_sci = {0};
const struct s_cardreader cardreader_mouse        = {0};
const struct s_cardreader cardreader_stinger      = {0};
#endif
```
