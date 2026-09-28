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

// First n (at most 4) outputs of a standard MT19937 seeded with s.
static void mtFirst(uint32_t s, uint32_t *out, int n)
{
    uint32_t m[401];
    m[0] = s;
    for (int i = 1; i < 397 + n; i++)
        m[i] = 1812433253u * (m[i-1] ^ (m[i-1] >> 30)) + i;
    for (int i = 0; i < n; i++) {
        uint32_t y = (m[i] & 0x80000000u) | (m[i+1] & 0x7fffffffu);
        y = m[i+397] ^ (y >> 1) ^ ((y & 1) ? 0x9908b0dfu : 0);
        y ^= y >> 11;
        y ^= (y << 7) & 0x9d2c5680u;
        y ^= (y << 15) & 0xefc60000u;
        y ^= y >> 18;
        out[i] = y;
    }
}

enum { SHIPWRECK, OCEAN_RUIN, TREASURE };
static const int CUBIOMES_TYPE[] = { Shipwreck, Ocean_Ruin, Treasure };

// Bedrock Edition 1.18+ placement. Each structure has one attempt per region,
// placed by a Mersenne Twister seeded from the low 32 bits of the world seed.
static const struct { uint32_t salt; int spacing, range, triangular; } BEDROCK[] = {
    [SHIPWRECK]  = { 165745295, 24, 20, 0 },
    [OCEAN_RUIN] = {  14357621, 20, 12, 0 },
    [TREASURE]   = {  16842397,  4,  2, 1 },
};

static void bedrockPos(int type, uint64_t seed, int rx, int rz, Pos *p)
{
    uint32_t s = (uint32_t)seed + (uint32_t)rx * 2570712328u
               + (uint32_t)rz * 4048968661u + BEDROCK[type].salt;
    uint32_t r[4];
    int range = BEDROCK[type].range, x, z;
    if (BEDROCK[type].triangular) {
        mtFirst(s, r, 4);
        x = (r[0] % range + r[1] % range) / 2;
        z = (r[2] % range + r[3] % range) / 2;
    } else {
        mtFirst(s, r, 2);
        x = r[0] % range;
        z = r[1] % range;
    }
    p->x = (rx * BEDROCK[type].spacing + x) * 16 + 8;
    p->z = (rz * BEDROCK[type].spacing + z) * 16 + 8;
}

// Structure attempts of the given type in regions [rx0..rx1] x [rz0..rz1].
// Writes x, z, viable, biome per structure into out[]; returns the count.
// x and z are the block the structure centres on (chunk middle, or the exact
// chest block for Java buried treasure).
__attribute__((export_name("find")))
int find(int type, int rx0, int rz0, int rx1, int rz1)
{
    int st = CUBIOMES_TYPE[type];
    int n = 0;
    for (int rz = rz0; rz <= rz1; rz++)
    for (int rx = rx0; rx <= rx1; rx++) {
        Pos p;
        if (bedrock) {
            bedrockPos(type, cur_seed, rx, rz, &p);
        } else {
            if (!getStructurePos(st, cur_mc, cur_seed, rx, rz, &p))
                continue;
            if (type != TREASURE) {
                p.x += 8;
                p.z += 8;
            }
        }
        if (n >= 20000)
            return n;
        int *o = out + 4 * n++;
        o[0] = p.x;
        o[1] = p.z;
        o[2] = isViableStructurePos(st, &g, p.x & ~15, p.z & ~15, 0);
        o[3] = getBiomeAt(&g, 4, p.x >> 2, 16, p.z >> 2);
    }
    return n;
}

__attribute__((export_name("regionChunks")))
int regionChunks(int type)
{
    if (bedrock)
        return BEDROCK[type].spacing;
    StructureConfig sc;
    getStructureConfig(CUBIOMES_TYPE[type], cur_mc, &sc);
    return sc.regionSize;
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
