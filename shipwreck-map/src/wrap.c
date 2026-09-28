// Thin WebAssembly wrapper around cubiomes (https://github.com/Cubitect/cubiomes)
// for the shipwreck finder page.
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include <stdlib.h>

static Generator g;
static int cur_mc = -1;
static uint64_t cur_seed;
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
void setWorld(int mc, uint32_t lo, uint32_t hi)
{
    ensure(mc, ((uint64_t)hi << 32) | lo);
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
        if (!getStructurePos(Shipwreck, cur_mc, cur_seed, rx, rz, &p))
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
