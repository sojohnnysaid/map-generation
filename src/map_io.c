#include "map_io.h"
#include <stdio.h>

// Simple threshold for demo purposes
#define WATER_LEVEL 0.5

void print_map_text(const MapData* map) {
    if (!map || !map->elevation) {
        printf("Cannot print NULL map.\n");
        return;
    }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            if (map->elevation[y][x] < WATER_LEVEL) {
                putchar('~'); // Water
            } else {
                putchar('#'); // Land
            }
        }
        putchar('\n'); // Newline after each row
    }
    printf("--------------------\n");
}
