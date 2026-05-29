# NCam Android

Run [NCam](https://github.com/ncam-project/NCam) as a native Android application using the Android NDK.
NCam C source is compiled directly into a JNI shared library (`libncam.so`) — no binary execution, no root required.

---

## How It Works

```
NCam C source  (app/src/main/cpp/ncam/)
      ↓  NDK clang
  libncam.so  (JNI)
      ↓
  Kotlin / Java
      ↓
  Android Device  (no root)
```

`android_compat.h` is force-included into every translation unit via CMake.
It disables hardware-dependent code at compile time without modifying NCam source.

---

## Requirements

| Tool | Version |
|------|---------|
| Android Studio | Hedgehog 2023.1+ |
| Android NDK | r26d (`26.3.11579264`) |
| CMake | 3.22+ |
| Android SDK | API 21 – 34 |

---

## Supported Devices

- **Android 5.0 (API 21)** minimum — no root required
- **Architectures:** `arm64-v8a`, `armeabi-v7a`, `x86_64`
- Phones, tablets, Android TV boxes

---

## Build Locally

```bash
git clone https://github.com/YOUR_USERNAME/NCamAndroid.git
cd NCamAndroid
./gradlew assembleDebug
```

APK: `app/build/outputs/apk/debug/app-debug.apk`

Or open in Android Studio and click **Run**.

---

## GitHub Actions (CI)

Every push builds Debug + Release APKs automatically.
Find them under: **Actions → latest run → Artifacts**

---

## What Is Enabled / Disabled

| Feature | Status |
|---------|--------|
| CCCAM | ✅ |
| NewCAMD | ✅ |
| CAMD35 / TCP | ✅ |
| GBOX | ✅ |
| Emulator (WITH_EMU) | ✅ |
| CacheEx / CacheEx AIO | ✅ |
| WebIF (port 8888) | ✅ |
| Anti-cascade | ✅ |
| CW Cycle Check | ✅ |
| Physical card readers | ❌ |
| DVB-API | ❌ |
| Serial module | ❌ |
| STAPI / CoolAPI / AZBox | ❌ |

---

## Configuration

Config files are stored on the device at:
```
/data/data/com.ncam.app/files/ncam/
```

Default `ncam.conf` is copied from assets on first launch.

WebIF access from any browser on the same network:
```
http://<device-ip>:8888
```
Default login: `admin` / `admin`

---

## Project Structure

```
NCamAndroid/
├── .github/workflows/build.yml
├── app/src/main/
│   ├── assets/
│   │   └── ncam.conf                default config
│   ├── cpp/
│   │   ├── android_compat.h         override layer
│   │   ├── android_stubs.c          linker stubs
│   │   ├── jni_bridge.cpp           Java ↔ NCam entry
│   │   ├── CMakeLists.txt
│   │   └── ncam/                    NCam source (bundled)
│   ├── java/com/ncam/app/
│   │   ├── NCamJNI.kt
│   │   ├── NCamService.kt
│   │   ├── MainActivity.kt
│   │   └── BootReceiver.kt
│   └── res/
├── COMPILE_ERRORS.md
└── README.md
```

---

## Updating NCam Source

```bash
rm -rf app/src/main/cpp/ncam
cp -r /path/to/new/NCam-master app/src/main/cpp/ncam
./gradlew assembleDebug
```

If a new NCam version introduces incompatibilities, update `android_compat.h` only.

---

## License

Android wrapper — MIT License.
NCam — see `app/src/main/cpp/ncam/COPYING`.
