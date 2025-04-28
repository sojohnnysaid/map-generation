#include <stdio.h>
#include <stdlib.h>
#include "map_data.h"
#include "map_io.h" // Include the I/O header

#define MAP_WIDTH 80
#define MAP_HEIGHT 25 // Adjusted height for better console view

int main() {
    printf("Procedural Map Generator - Milestone 2\n");

    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    if (!map) {
        fprintf(stderr, "Failed to create map.\n");
        return EXIT_FAILURE;
    }

    // --- Fill with a test pattern for now ---
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            // Simple gradient for testing output
             map->elevation[y][x] = (double)x / (map->width -1);
        }
    }
    printf("Filled map with test pattern.\n");

    // --- Print the map ---
    print_map_text(map);

    destroy_map(map);
    map = NULL;

    return EXIT_SUCCESS;
}
