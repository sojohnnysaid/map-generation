#include "map_io.h"
#include <stdio.h>

#define BIOME_OCEAN       '~'
#define BIOME_BEACH       '.'
#define BIOME_GRASSLAND   ','
#define BIOME_DRY_GRASS   '"'
#define BIOME_FOREST      '%'
#define BIOME_ROCK        '^'
#define BIOME_SNOW        '#'

#define ELEV_OCEAN_MAX    0.10
#define ELEV_BEACH_MAX    0.15
#define ELEV_MOUNTAIN_MIN 0.80

#define MOIST_DRY_MAX     0.30
#define MOIST_FOREST_MIN  0.65

void print_map_text(const MapData* map) {
    if (!map || !map->elevation || !map->moisture) {
        printf("Cannot print NULL or incomplete map.\n");
        return;
    }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            double e = map->elevation[y][x];
            double m = map->moisture[y][x];

            char biome_char;

            if (e < ELEV_OCEAN_MAX) {
                biome_char = BIOME_OCEAN;
            } else if (e < ELEV_BEACH_MAX) {
                biome_char = BIOME_BEACH;
            } else if (e > ELEV_MOUNTAIN_MIN) {
                if (m < MOIST_DRY_MAX) {
                    biome_char = BIOME_ROCK;
                } else {
                    biome_char = BIOME_SNOW;
                }
            } else {
                if (m < MOIST_DRY_MAX) {
                    biome_char = BIOME_DRY_GRASS;
                } else if (m < MOIST_FOREST_MIN) {
                    biome_char = BIOME_GRASSLAND;
                } else {
                    biome_char = BIOME_FOREST;
                }
            }
            putchar(biome_char);
        }
        putchar('\n');
    }
    printf("--------------------\n");
}
