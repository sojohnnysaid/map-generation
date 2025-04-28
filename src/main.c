#include <stdio.h>
#include <stdlib.h> // For EXIT_SUCCESS/FAILURE
#include "map_data.h" // Include our map data header

// Define map dimensions (can be made dynamic later)
#define MAP_WIDTH 80
#define MAP_HEIGHT 40

int main() {
    printf("Procedural Map Generator - Milestone 1\n");

    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    if (!map) {
        fprintf(stderr, "Failed to create map.\n");
        return EXIT_FAILURE;
    }

    // --- We will add generation and output here later ---
    printf("Map data structure created successfully.\n");
    printf("Map dimensions: %d x %d\n", map->width, map->height);
    // Example: Accessing an element (just for demo, it's 0.0 now)
    printf("Elevation at (0,0): %f\n", map->elevation[0][0]);

    destroy_map(map);
    map = NULL; // Good practice to NULL dangling pointers

    return EXIT_SUCCESS;
}
