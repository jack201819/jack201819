// Thin WebAssembly wrapper around cubiomes (https://github.com/Cubitect/cubiomes)
// for the shipwreck finder page.
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include <stdlib.h>

static Generator g;
static int cur_mc = -1;
static uint64_t cur_seed;
static int bedrock = 0;
static int out[4 * 20000];
static int *bcache = NULL;
static size_t bcache_len = 0;

static void ensure(int mc, uint64_t seed)
{
    if (cur_mc != mc || cur_seed != seed) {
        setupGenerator(&g, mc, 0);
        applySeed(&g, DIM_OVERWORLD, seed);
        cur_mc = mc;
        cur_seed = seed;
    }
}

__attribute__((export_name("setWorld")))
void setWorld(int mc, uint32_t lo, uint32_t hi, int isBedrock)
{
    ensure(mc, ((uint64_t)hi << 32) | lo);
    bedrock = isBedrock;
}

// First two outputs of a standard MT19937 seeded with s.
static void mt2(uint32_t s, uint32_t *a, uint32_t *b)
{
    uint32_t m[399];
    m[0] = s;
    for (int i = 1; i < 399; i++)
        m[i] = 1812433253u * (m[i-1] ^ (m[i-1] >> 30)) + i;
    for (int i = 0; i < 2; i++) {
        uint32_t y = (m[i] & 0x80000000u) | (m[i+1] & 0x7fffffffu);
        y = m[i+397] ^ (y >> 1) ^ ((y & 1) ? 0x9908b0dfu : 0);
        y ^= y >> 11;
        y ^= (y << 7) & 0x9d2c5680u;
        y ^= (y << 15) & 0xefc60000u;
        y ^= y >> 18;
        if (i == 0) *a = y; else *b = y;
    }
}

// Bedrock Edition 1.18+: shipwrecks use 24-chunk regions with a 20-chunk
// spread, placed by a Mersenne Twister seeded from the low 32 bits of the
// world seed.
static int bedrockShipwreckPos(uint64_t seed, int rx, int rz, Pos *p)
{
    uint32_t s = (uint32_t)seed + (uint32_t)rx * 2570712328u
               + (uint32_t)rz * 4048968661u + 165745295u;
    uint32_t a, b;
    mt2(s, &a, &b);
    p->x = (rx * 24 + (int)(a % 20)) * 16;
    p->z = (rz * 24 + (int)(b % 20)) * 16;
    return 1;
}

// Shipwreck attempts in regions [rx0..rx1] x [rz0..rz1].
// Writes x, z, viable, biome per shipwreck into out[]; returns the count.
__attribute__((export_name("find")))
int find(int rx0, int rz0, int rx1, int rz1)
{
    int n = 0;
    for (int rz = rz0; rz <= rz1; rz++)
    for (int rx = rx0; rx <= rx1; rx++) {
        Pos p;
        if (bedrock ? !bedrockShipwreckPos(cur_seed, rx, rz, &p)
                    : !getStructurePos(Shipwreck, cur_mc, cur_seed, rx, rz, &p))
            continue;
        if (n >= 20000)
            return n;
        int *o = out + 4 * n++;
        o[0] = p.x;
        o[1] = p.z;
        o[2] = isViableStructurePos(Shipwreck, &g, p.x, p.z, 0);
        o[3] = getBiomeAt(&g, 4, (p.x + 8) >> 2, 16, (p.z + 8) >> 2);
    }
    return n;
}

__attribute__((export_name("outPtr")))
int *outPtr(void) { return out; }

// Biome ids for an sx*sz grid at the given scale (1, 4, 16, 64 or 256).
__attribute__((export_name("biomes")))
int *biomes(int scale, int x, int z, int sx, int sz)
{
    int y = scale == 1 ? 64 : scale == 4 ? 16 : scale == 16 ? 4 : scale == 64 ? 1 : 0;
    Range r = {scale, x, z, sx, sz, y, 1};
    size_t len = getMinCacheSize(&g, scale, sx, 1, sz);
    if (len > bcache_len) {
        free(bcache);
        bcache = malloc(len * sizeof(int));
        bcache_len = len;
    }
    if (genBiomes(&g, bcache, r))
        return NULL;
    return bcache;
}

__attribute__((export_name("mcVersion")))
int mcVersion(int i)
{
    static const int v[] = {
        MC_1_13, MC_1_14, MC_1_15, MC_1_16_1, MC_1_17,
        MC_1_18, MC_1_19, MC_1_20, MC_NEWEST,
    };
    return v[i];
}
