#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

PLATFORM_FILTER="${1:-all}"
TARGET_FILTER="${2:-all}"
ARCH_FILTER="${3:-all}"
JOBS="${JOBS:-$(nproc)}"
shift "$(( $# < 3 ? $# : 3 ))"
SCONS_EXTRA_ARGS=("$@")

declare -A ARCHES=(
  [linux]="x86_64 arm64 rv64"
  [windows]="x86_32 x86_64"
  [macos]="universal"
  [ios]="arm64"
  [android]="x86_64 arm64"
)

platforms=(linux windows macos ios android)
targets=(template_debug template_release)
valid_arches=(x86_32 x86_64 arm64 rv64 universal)

if [[ "$PLATFORM_FILTER" != all ]]; then
  if [[ ! " ${platforms[*]} " =~ " ${PLATFORM_FILTER} " ]]; then
    echo "Unknown platform '$PLATFORM_FILTER'. Choose: ${platforms[*]} or all." >&2
    exit 2
  fi
  platforms=("$PLATFORM_FILTER")
fi

if [[ "$ARCH_FILTER" != all && ! " ${valid_arches[*]} " =~ " ${ARCH_FILTER} " ]]; then
  echo "Unknown architecture '$ARCH_FILTER'. Choose: ${valid_arches[*]} or all." >&2
  exit 2
fi

case "$TARGET_FILTER" in
  all) ;;
  debug) targets=(template_debug) ;;
  release) targets=(template_release) ;;
  *)
    echo "Unknown target '$TARGET_FILTER'. Choose debug, release, or all." >&2
    exit 2
    ;;
esac

toolchain_env_for() {
  local platform="$1" arch="$2"
  local -a env_vars=()

  case "$platform:$arch" in
    linux:x86_64)
      env_vars+=("CC=${CC:-gcc}" "CXX=${CXX:-g++}" "AR=${AR:-ar}" "RANLIB=${RANLIB:-ranlib}" "LINK=${LINK:-g++}")
      ;;
    linux:arm64)
      env_vars+=("CC=${CC:-aarch64-linux-gnu-gcc}" "CXX=${CXX:-aarch64-linux-gnu-g++}" "AR=${AR:-aarch64-linux-gnu-ar}" "RANLIB=${RANLIB:-aarch64-linux-gnu-ranlib}" "LINK=${LINK:-aarch64-linux-gnu-g++}")
      ;;
    linux:rv64)
      env_vars+=("CC=${CC:-riscv64-linux-gnu-gcc}" "CXX=${CXX:-riscv64-linux-gnu-g++}" "AR=${AR:-riscv64-linux-gnu-ar}" "RANLIB=${RANLIB:-riscv64-linux-gnu-ranlib}" "LINK=${LINK:-riscv64-linux-gnu-g++}")
      ;;
    windows:x86_64)
      env_vars+=("CC=${CC:-x86_64-w64-mingw32-gcc}" "CXX=${CXX:-x86_64-w64-mingw32-g++}" "AR=${AR:-x86_64-w64-mingw32-ar}" "RANLIB=${RANLIB:-x86_64-w64-mingw32-ranlib}" "LINK=${LINK:-x86_64-w64-mingw32-g++}")
      ;;
    windows:x86_32)
      env_vars+=("CC=${CC:-i686-w64-mingw32-gcc}" "CXX=${CXX:-i686-w64-mingw32-g++}" "AR=${AR:-i686-w64-mingw32-ar}" "RANLIB=${RANLIB:-i686-w64-mingw32-ranlib}" "LINK=${LINK:-i686-w64-mingw32-g++}")
      ;;
    *)
      ;;
  esac

  printf '%s\n' "${env_vars[@]}"
}

build_one() (
  local platform="$1" target="$2" arch="$3"
  shift 3
  local args=("platform=$platform" "target=$target" "arch=$arch" "api_version=4.6" "custom_api_file=extension_api.json" "-j$JOBS")
  local protobuf_key="PROTOBUF_LIB_DIR_${platform^^}_${arch^^}"
  local protobuf_lib_dir="${!protobuf_key:-${PROTOBUF_LIB_DIR:-}}"
  if [[ "$platform" == ios && "$arch" == universal ]]; then
    protobuf_lib_dir="${PROTOBUF_LIB_DIR_IOS_SIMULATOR:-${protobuf_lib_dir}}"
  fi
  local protobuf_include_key="PROTOBUF_INCLUDE_DIR_${platform^^}_${arch^^}"
  local protobuf_include_dir="${!protobuf_include_key:-${PROTOBUF_INCLUDE_DIR:-}}"

  if [[ "$platform" == windows ]]; then
    if [[ -z "${!protobuf_key:-}" || -z "${!protobuf_include_key:-}" ]]; then
      echo "Windows $arch requires ${protobuf_key} and ${protobuf_include_key} in this shell." >&2
      exit 1
    fi
    if [[ ! -f "$protobuf_lib_dir/libprotobuf.a" || ! -d "$protobuf_include_dir/google/protobuf" ]]; then
      echo "Windows $arch protobuf files were not found under the configured library/include paths." >&2
      exit 1
    fi
  fi

  args+=("$@")
  args+=("${SCONS_EXTRA_ARGS[@]}")

  # Avoid inheriting a stale cross-compiler from a previous target build in the same shell.
  unset CC CXX AR AS RANLIB STRIP LINK

  local -a toolchain_env=()
  while IFS= read -r entry; do
    [[ -n "$entry" ]] && toolchain_env+=("$entry")
  done < <(toolchain_env_for "$platform" "$arch")

  echo "Building $platform $arch $target"
  export PROTOBUF_LIB_DIR="$protobuf_lib_dir"
  export PROTOBUF_INCLUDE_DIR="$protobuf_include_dir"
  for entry in "${toolchain_env[@]}"; do
    export "${entry%%=*}"="${entry#*=}"
  done
  scons "${args[@]}"
)

for platform in "${platforms[@]}"; do
  for target in "${targets[@]}"; do
    for arch in ${ARCHES[$platform]}; do
      if [[ "$ARCH_FILTER" != all && "$ARCH_FILTER" != "$arch" ]]; then
        continue
      fi

      if [[ "$platform" == ios ]]; then
        build_one ios "$target" arm64
        build_one ios "$target" universal ios_simulator=yes
        if [[ " ${SCONS_EXTRA_ARGS[*]} " == *" -n "* ]]; then
          continue
        fi
        if ! command -v xcodebuild >/dev/null 2>&1; then
          echo "xcodebuild is required to package iOS device and simulator libraries." >&2
          exit 1
        fi
        for arch in arm64 universal; do
          if [[ "$arch" == universal ]]; then
            protobuf_key="PROTOBUF_LIB_DIR_IOS_SIMULATOR"
            protobuf_lib_dir="${PROTOBUF_LIB_DIR_IOS_SIMULATOR:-${PROTOBUF_LIB_DIR:-}}"
          else
            protobuf_key="PROTOBUF_LIB_DIR_IOS_ARM64"
            protobuf_lib_dir="${PROTOBUF_LIB_DIR_IOS_ARM64:-${PROTOBUF_LIB_DIR:-}}"
          fi
          if [[ -z "$protobuf_lib_dir" ]]; then
            echo "Set $protobuf_key (or PROTOBUF_LIB_DIR) to the matching iOS protobuf library directory." >&2
            exit 1
          fi
          if [[ "$arch" == universal ]]; then
            archive="demos/demo-simple/bin/libdemo-recorder.ios.$target.universal.simulator.a"
          else
            archive="demos/demo-simple/bin/libdemo-recorder.ios.$target.arm64.a"
          fi
          libtool -static -o "$archive.packaged.a" "$archive" "$protobuf_lib_dir/libprotobuf.a"
        done
        rm -rf "demos/demo-simple/bin/libdemo-recorder.ios.$target.xcframework"
        rm -rf "demos/demo-simple/bin/libgodot-cpp.ios.$target.xcframework"
        xcodebuild -create-xcframework \
          -library "demos/demo-simple/bin/libdemo-recorder.ios.$target.arm64.a.packaged.a" \
          -library "demos/demo-simple/bin/libdemo-recorder.ios.$target.universal.simulator.a.packaged.a" \
          -output "demos/demo-simple/bin/libdemo-recorder.ios.$target.xcframework"
        xcodebuild -create-xcframework \
          -library "godot-cpp/bin/libgodot-cpp.ios.$target.arm64.a" \
          -library "godot-cpp/bin/libgodot-cpp.ios.$target.universal.simulator.a" \
          -output "demos/demo-simple/bin/libgodot-cpp.ios.$target.xcframework"
      else
        build_one "$platform" "$target" "$arch"
      fi
    done
  done
done

echo "Build complete. Artifacts are under demos/demo-simple/bin/."