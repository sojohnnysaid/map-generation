#include "map_data.h"
#include <stdlib.h>
#include <stdio.h> // For error messages

MapData* create_map(int width, int height) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Error: Map dimensions must be positive.\n");
        return NULL;
    }

    MapData* map = malloc(sizeof(MapData));
    if (!map) {
        perror("Error allocating MapData structure");
        return NULL;
    }

    map->width = width;
    map->height = height;
    map->elevation = NULL; // Initialize to NULL

    // Allocate rows (array of pointers to double)
    map->elevation = malloc(height * sizeof(double*));
    if (!map->elevation) {
        perror("Error allocating map elevation rows");
        free(map); // Clean up MapData struct
        return NULL;
    }

    // Allocate columns for each row (actual double values)
    for (int y = 0; y < height; y++) {
        map->elevation[y] = malloc(width * sizeof(double));
        if (!map->elevation[y]) {
            perror("Error allocating map elevation columns");
            // Clean up previously allocated rows and the row pointers array
            for (int i = 0; i < y; i++) {
                free(map->elevation[i]);
            }
            free(map->elevation);
            free(map); // Clean up MapData struct
            return NULL;
        }
    }

    printf("Created map (%dx%d)\n", width, height);
    fill_map_constant(map, 0.0); // Initialize to 0.0
    return map;
}

void destroy_map(MapData* map) {
    if (!map) {
        return; // Nothing to destroy
    }

    if (map->elevation) {
        // Free columns for each row first
        for (int y = 0; y < map->height; y++) {
            free(map->elevation[y]); // Frees the double array for row y
        }
        // Free the array of row pointers
        free(map->elevation);
    }

    // Free the main struct
    free(map);
    printf("Destroyed map\n");
}

// Helper function to initialize the map grid
void fill_map_constant(MapData* map, double value) {
    if (!map || !map->elevation) return;

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            map->elevation[y][x] = value;
        }
    }
}
