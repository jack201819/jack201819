# Shipwreck Chart

A single-page map of Minecraft shipwrecks, ocean ruins and buried treasure for any
**Java** or **Bedrock** seed.
Open `index.html` in a browser, type your seed, and pan/zoom around.

- Wreck positions and biomes come from [cubiomes](https://github.com/Cubitect/cubiomes),
  compiled to WebAssembly and embedded in the page, so it works offline.
- Sites that fail the biome check (nothing actually spawns) are hidden by default.
- Buried treasure only shows when zoomed in. Java treasure coordinates are the exact chest block.
- Beached wrecks (on beach biomes) are marked separately from ocean wrecks.
- Java: 1.13 through 1.21+. Bedrock: 1.18 through 1.21+.
- Bedrock placement uses a Mersenne Twister seeded with
  `low32(seed) + rx*2570712328 + rz*4048968661 + salt`. Shipwrecks: salt 165745295,
  24-chunk regions, 20-chunk spread. Ocean ruins: salt 14357621, 20-chunk regions,
  12-chunk spread. Buried treasure: salt 16842397, 4-chunk regions, triangular
  2-chunk spread. It was checked against the open-source
  [SeedFinder](https://github.com/zebedelu/SeedFinder) Bedrock finder, and the constants match
  [MCBE-seedcracker](https://github.com/Alist2930/MCBE-seedcracker). Since 1.18, Bedrock biomes
  match Java biomes for the same seed, so the same biome check applies.

## Rebuilding

`src/build.sh` clones cubiomes, downloads the WASI sysroot, compiles
`src/wrap.c` with clang's wasm32 target and inlines the result into
`src/template.html` to produce `index.html`.
