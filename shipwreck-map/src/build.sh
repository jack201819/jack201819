#!/usr/bin/env bash
# Builds ../index.html: compiles cubiomes + src/wrap.c to WebAssembly with
# clang's wasm32 target, gzips it, and inlines it into src/template.html.
#
# Needs: clang + wasm-ld (LLVM 18+), curl, gzip, base64, python3.
set -euo pipefail
cd "$(dirname "$0")"
WORK=${WORK:-.build}
mkdir -p "$WORK"

[ -d "$WORK/cubiomes" ] || git clone -q --depth 1 https://github.com/Cubitect/cubiomes.git "$WORK/cubiomes"
if [ ! -d "$WORK/wasi-sysroot-24.0" ]; then
  curl -sSL https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-24/wasi-sysroot-24.0.tar.gz | tar xz -C "$WORK"
fi
if [ ! -d "$WORK/libclang_rt.builtins-wasm32-wasi-24.0" ]; then
  curl -sSL https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-24/libclang_rt.builtins-wasm32-wasi-24.0.tar.gz | tar xz -C "$WORK"
fi

C="$WORK/cubiomes"
clang --target=wasm32-wasi --sysroot="$WORK/wasi-sysroot-24.0" -O3 \
  -mexec-model=reactor -nodefaultlibs -Wl,--no-entry -I"$WORK" -w \
  -o "$WORK/ship.wasm" wrap.c \
  "$C"/finders.c "$C"/generator.c "$C"/biomes.c "$C"/biomenoise.c \
  "$C"/layers.c "$C"/noise.c "$C"/util.c \
  -lc -lm "$WORK/libclang_rt.builtins-wasm32-wasi-24.0/libclang_rt.builtins-wasm32.a"

gzip -9 -c "$WORK/ship.wasm" | base64 -w0 > "$WORK/ship.wasm.gz.b64"
python3 - "$WORK/ship.wasm.gz.b64" <<'EOF'
import sys
b64 = open(sys.argv[1]).read().strip()
html = open("template.html").read().replace("__WASM_GZ_BASE64__", b64)
open("../index.html", "w").write(
    '<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
    '<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">\n'
    '</head>\n<body>\n' + html + '</body>\n</html>\n')
EOF
echo "Wrote $(cd .. && pwd)/index.html"
