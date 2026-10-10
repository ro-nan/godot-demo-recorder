# Demo Recorder
This is a game-agnostic demo recorder for Godot. It allows your builds to record demos and upload them to a server for later playback and analysis.

Watching players play your game rather than relying on general gameplay statistics is usually optimal, but watching those players over Discord is time consuming. 

This addon seeks to solve this issue by recording gameplay sessions, uploading them to a server, and sending them to you to watch at whatever time you please. In addition to this, you can pick out specific sections to watch, skip around in the replays, and gather general data from them as well. 

## Applications 
- Instant replay systems
- Cheating detection 
- Playtesting session recording
- Debugging

## Usage 
Set recording mode using `RecordedScene.set_recording(bool)` and check if the scene is recording using `RecordedScene.is_recording()`.

Make sure to disable scripts that may interfere with replaying by returning during their process when `RecordedScene.is_recording()`. Eg.

Instead of
```
func _physics_process(delta):
    global_rotation = Vector3.ZERO # On some frames this will lead to framefighting
```
Do
```
func _physics_process(delta):
    if RecordedScene.is_recording(): return # RecordedScene being the recorded scene root node
    global_rotation = Vector3.ZERO # On some frames this will lead to framefighting

```

Pause the recording using `RecordedScene.process_mode = Node.PROCESS_MODE_PAUSED` and unpause it using `main.process_mode = Node.PROCESS_MODE_INHERIT`.

Scrub thru the recording using `main.scrub_to_frame(int)`

## Advantages
- No external software needed
- Can any session any time
- Bundled with every release, automatically collecting demos for you
- Perfectly lossless quality at any resolution (60 FPS, no bitrate smearing)
- Multiple camera angles
- Record custom debugging variables (aids in fixing bugs)
- Can also be used for other application than playtesting like instant replays

**Additional advantages compared to similar methods:**
| Advantage | Compared to Streaming | Compared to Video Recordings |
| --- | --- | --- |
| **Asynchronous Review** | Review anytime without scheduling live sessions | — |
| **Playback Control** | Scrub around in the recording | — |
| **Bandwidth / File Size** | Takes up far less bandwidth | Smaller file size in most cases |

## Limitations 
While the file size of a simple recording (~380kb for a 12s recording) is much less than a similar quality video (High quality 1080p video @ 60fps is ~9MB, ~25x bigger), the file size of recordings with many moving objects may be larger than video. 

## Building
Run `./build_all.sh` to build debug and release libraries for the platforms and architectures declared in `demos/demo-simple/bin/demo-recorder.gdextension`. Select a platform, build type, and architecture with `./build_all.sh linux debug x86_64`; use `all` for any filter to include every configured value. Set `JOBS` to control SCons parallelism.

Cross builds need a protobuf library and headers built for each target. Set `PROTOBUF_LIB_DIR` and `PROTOBUF_INCLUDE_DIR` as defaults, or use variables such as `PROTOBUF_LIB_DIR_ANDROID_ARM64` and `PROTOBUF_INCLUDE_DIR_ANDROID_ARM64` for target-specific paths. Platform SDKs/toolchains must also be installed. iOS builds require macOS with Xcode and package device/simulator outputs as XCFrameworks.

### Example: Building on Linux
Install host-side dependencies
```
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config git \
  protobuf-compiler libprotobuf-dev \
  gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 \
  gcc-mingw-w64-i686 g++-mingw-w64-i686 \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
  gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
```

**Build to Linux x86_64**
```
git clone --branch v3.21.12 --depth 1 https://github.com/protocolbuffers/protobuf.git ~/src/protobuf
cd ~/src/protobuf

rm -rf build-linux-x86_64
cmake -S cmake -B build-linux-x86_64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/linux-x86_64

cmake --build build-linux-x86_64
cmake --install build-linux-x86_64
```
then 
```
export PROTOBUF_INCLUDE_DIR=$HOME/protobuf-cross/linux-x86_64/include
export PROTOBUF_LIB_DIR=$HOME/protobuf-cross/linux-x86_64/lib
```

**Build to Linux arm64**
```
cd ~/src/protobuf

cmake -S cmake -B build-linux-arm64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DCMAKE_FIND_ROOT_PATH=/usr/aarch64-linux-gnu \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/linux-arm64

cmake --build build-linux-arm64 --target install
```
then
```
export PROTOBUF_LIB_DIR_LINUX_ARM64=$HOME/protobuf-cross/linux-arm64/lib
export PROTOBUF_INCLUDE_DIR_LINUX_ARM64=$HOME/protobuf-cross/linux-arm64/include
```

**Build to Linux riscv64**
```
cd ~/src/protobuf
cmake -S cmake -B build-linux-riscv64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=riscv64 \
  -DCMAKE_C_COMPILER=riscv64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=riscv64-linux-gnu-g++ \
  -DCMAKE_FIND_ROOT_PATH=/usr/riscv64-linux-gnu \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/linux-riscv64

cmake --build build-linux-riscv64 --target install
```
then
```
export PROTOBUF_LIB_DIR_LINUX_RV64=$HOME/protobuf-cross/linux-riscv64/lib
export PROTOBUF_INCLUDE_DIR_LINUX_RV64=$HOME/protobuf-cross/linux-riscv64/include
```

**Build to Windows x86_32**
```
cd ~/src/protobuf

cmake -S cmake -B build-win-x86_32 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_SYSTEM_PROCESSOR=x86 \
  -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++ \
  -DCMAKE_RC_COMPILER=i686-w64-mingw32-windres \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/windows-x86_32

cmake --build build-win-x86_32 --target install
```
then
```
export PROTOBUF_LIB_DIR_WINDOWS_X86_32=$HOME/protobuf-cross/windows-x86_32/lib
export PROTOBUF_INCLUDE_DIR_WINDOWS_X86_32=$HOME/protobuf-cross/windows-x86_32/include
```

**Build to Windows x86_64**
```
cd ~/src/protobuf

cmake -S cmake -B build-win-x86_64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
  -DCMAKE_RC_COMPILER=x86_64-w64-mingw32-windres \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/windows-x86_64

cmake --build build-win-x86_64 --target install
```
then
```
export PROTOBUF_LIB_DIR_WINDOWS_X86_64=$HOME/protobuf-cross/windows-x86_64/lib
export PROTOBUF_INCLUDE_DIR_WINDOWS_X86_64=$HOME/protobuf-cross/windows-x86_64/include
```

**Build to Android arm64-v8a**
You need to install android command-line tools or Android Studio, then set:
```
export ANDROID_HOME=$HOME/Android/Sdk
export ANDROID_NDK_ROOT=$ANDROID_HOME/ndk/26.3.11579264
```

```
cmake -S cmake -B build-android-arm64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_ANDROID_NDK=$ANDROID_NDK_ROOT \
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
  -DCMAKE_ANDROID_API=24 \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/android-arm64

cmake --build build-android-arm64 --target install
```
then 
```
export PROTOBUF_LIB_DIR_ANDROID_ARM64=$HOME/protobuf-cross/android-arm64/lib
export PROTOBUF_INCLUDE_DIR_ANDROID_ARM64=$HOME/protobuf-cross/android-arm64/include
```

**Build to Android arm x86_64**
You need to install android command-line tools or Android Studio, then set:
```
export ANDROID_HOME=$HOME/Android/Sdk
export ANDROID_NDK_ROOT=$ANDROID_HOME/ndk/26.3.11579264
```

Note: untested
```
cmake -S cmake -B build-android-x86_64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -DCMAKE_SYSTEM_NAME=Android \
  -DCMAKE_ANDROID_NDK=$ANDROID_NDK_ROOT \
  -DCMAKE_ANDROID_ARCH_ABI=x86_64 \
  -DCMAKE_ANDROID_API=24 \
  -DCMAKE_INSTALL_PREFIX=$HOME/protobuf-cross/android-x86_64 \ 
  -DANDROID_ABI=x86_64

cmake --build build-android-x86_64 --target install
```
then 
```
export PROTOBUF_LIB_DIR_ANDROID_X86_64=$HOME/protobuf-cross/android-x86_64/lib
export PROTOBUF_INCLUDE_DIR_ANDROID_X86_64=$HOME/protobuf-cross/android-x86_64/include
```

**iOS/macOS**
Unfortunately you need to spin up a macOS machine or VM and build the project there. 

This is currently out of the scope of this project's README

**Web**
This is also out of the scope of this project's README.

See https://docs.godotengine.org/en/stable/engine_details/development/compiling/compiling_for_web.html for instructions on compiling godot cpp to the web.

You will need to compile your own export templates with your GDExtension included then build your project.

This is currently untested and I'm not really sure how this will work with protobuf. 

## AI Use Disclosure
AI is/was used for code cleaning and project set up purposes ONLY. 

Code design is mine.