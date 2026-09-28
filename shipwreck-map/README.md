# Shipwreck Chart

A single-page map of Minecraft **Java Edition** shipwrecks for any seed.
Open `index.html` in a browser, type your seed, and pan/zoom around.

- Wreck positions and biomes come from [cubiomes](https://github.com/Cubitect/cubiomes),
  compiled to WebAssembly and embedded in the page, so it works offline.
- Sites that fail the biome check (no wreck actually spawns) are hidden by default.
- Beached wrecks (on beach biomes) are marked separately from ocean wrecks.
- Supports 1.13 through 1.21+. Bedrock Edition is not supported.

## Rebuilding

`src/build.sh` clones cubiomes, downloads the WASI sysroot, compiles
`src/wrap.c` with clang's wasm32 target and inlines the result into
`src/template.html` to produce `index.html`.
