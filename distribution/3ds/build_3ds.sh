#!/bin/bash
set -e

export DEVKITPRO="/Users/pdrtorres/.gemini/antigravity/scratch/tools/dkp_root/opt/devkitpro"
export DEVKITARM="${DEVKITPRO}/devkitARM"
export PATH="/Users/pdrtorres/.gemini/antigravity/scratch/tools/cmake-4.4.4-macos-universal/CMake.app/Contents/bin:${DEVKITPRO}/tools/bin:${DEVKITPRO}/portlibs/3ds/bin:${DEVKITARM}/bin:${PATH}"

ROOT_DIR="/Users/pdrtorres/.gemini/antigravity/scratch/openrct2-3ds"
BUILD_DIR="${ROOT_DIR}/build_3ds"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${ROOT_DIR}/engine" \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/3ds.toolchain.cmake" \
    -DNINTENDO_3DS=ON \
    -DDISABLE_NETWORK=ON \
    -DDISABLE_HTTP_TWITCH=ON \
    -DDISABLE_TTF=ON \
    -DDISABLE_OPENGL=ON \
    -DENABLE_LIGHTFX=OFF \
    -DWITH_TESTS=OFF \
    -DDOWNLOAD_TITLE_SEQUENCES=OFF

make -j$(sysctl -n hw.ncpu)

echo "Generating SMDH and 3DSX..."
smdhtool --create "OpenRCT2 (RCT1+2)" "RollerCoaster Tycoon 1 & 2 Dual-Screen Edition" "OpenRCT2 Team / N3DS Port" "${ROOT_DIR}/engine/resources/logo/icon_x48.png" "${BUILD_DIR}/openrct2.smdh"
3dsxtool "${BUILD_DIR}/openrct2" "${BUILD_DIR}/openrct2.3dsx" --smdh="${BUILD_DIR}/openrct2.smdh"

echo "Generating CIA..."
if [ ! -f "${ROOT_DIR}/banner/banner.bnr" ]; then
    mkdir -p "${ROOT_DIR}/banner"
    sips -z 128 128 "${ROOT_DIR}/engine/resources/logo/icon_x128.png" --out "${ROOT_DIR}/banner/icon128.png"
    sips -p 128 256 "${ROOT_DIR}/banner/icon128.png" --padColor 1b4d2e --out "${ROOT_DIR}/banner/banner.png"
    python3 -c "
import wave, struct, math
sample_rate = 22050
duration = 1.0
samples = int(sample_rate * duration)
with wave.open('${ROOT_DIR}/banner/sound.wav', 'w') as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(sample_rate)
    for i in range(samples):
        t = i / sample_rate
        decay = math.exp(-3.5 * t)
        val = 0.3 * math.sin(2 * math.pi * 523.25 * t) + 0.3 * math.sin(2 * math.pi * 659.25 * t) + 0.3 * math.sin(2 * math.pi * 783.99 * t)
        val = int(val * decay * 32767)
        w.writeframes(struct.pack('<h', val))
"
    bannertool makebanner -i "${ROOT_DIR}/banner/banner.png" -a "${ROOT_DIR}/banner/sound.wav" -o "${ROOT_DIR}/banner/banner.bnr"
fi

makerom \
    -f cia \
    -o "${BUILD_DIR}/openrct2.cia" \
    -rsf "${ROOT_DIR}/openrct2.rsf" \
    -target t \
    -elf "${BUILD_DIR}/openrct2" \
    -icon "${BUILD_DIR}/openrct2.smdh" \
    -banner "${ROOT_DIR}/banner/banner.bnr"

echo "Build successful! Generated outputs:"
ls -lh "${BUILD_DIR}/openrct2.3dsx" "${BUILD_DIR}/openrct2.smdh" "${BUILD_DIR}/openrct2.cia"
