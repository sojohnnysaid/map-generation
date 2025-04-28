#include "map_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>

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
    map->elevation = NULL;
    map->moisture = NULL;
    map->is_river = NULL;

    // Allocate elevation
    map->elevation = malloc(height * sizeof(double*));
    if (!map->elevation) {
        perror("Error allocating map elevation rows");
        free(map);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        map->elevation[y] = malloc(width * sizeof(double));
        if (!map->elevation[y]) {
            perror("Error allocating map elevation columns");
            for (int i = 0; i < y; i++) free(map->elevation[i]);
            free(map->elevation);
            free(map);
            return NULL;
        }
    }

    // Allocate moisture
    map->moisture = malloc(height * sizeof(double*));
    if (!map->moisture) {
        perror("Error allocating map moisture rows");
        for (int y = 0; y < height; y++) free(map->elevation[y]);
        free(map->elevation);
        free(map);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        map->moisture[y] = malloc(width * sizeof(double));
        if (!map->moisture[y]) {
            perror("Error allocating map moisture columns");
            for (int i = 0; i < y; i++) free(map->moisture[i]);
            free(map->moisture);
            for (int i = 0; i < height; i++) free(map->elevation[i]);
            free(map->elevation);
            free(map);
            return NULL;
        }
    }

    // Allocate is_river
    map->is_river = malloc(height * sizeof(bool*));
    if (!map->is_river) {
        perror("Error allocating map is_river rows");
        for(int y=0; y<height; y++) free(map->moisture[y]);
        free(map->moisture);
        for(int y=0; y<height; y++) free(map->elevation[y]);
        free(map->elevation);
        free(map);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        map->is_river[y] = malloc(width * sizeof(bool));
        if (!map->is_river[y]) {
            perror("Error allocating map is_river columns");
            for(int i=0; i<y; i++) free(map->is_river[i]);
            free(map->is_river);
            for(int i=0; i<height; i++) free(map->moisture[i]);
            free(map->moisture);
            for(int i=0; i<height; i++) free(map->elevation[i]);
            free(map->elevation);
            free(map);
            return NULL;
        }
        for (int x = 0; x < width; x++) {
            map->is_river[y][x] = false;
        }
    }

    printf("Created map (%dx%d) with elevation, moisture, and river layers\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            map->elevation[y][x] = 0.0;
            map->moisture[y][x] = 0.0;
        }
    }

    return map;
}

void destroy_map(MapData* map) {
    if (!map) return;

    if (map->elevation) {
        for (int y = 0; y < map->height; y++) {
            free(map->elevation[y]);
        }
        free(map->elevation);
    }

    if (map->moisture) {
        for (int y = 0; y < map->height; y++) {
            free(map->moisture[y]);
        }
        free(map->moisture);
    }

    if (map->is_river) {
        for (int y = 0; y < map->height; y++) {
            free(map->is_river[y]);
        }
        free(map->is_river);
    }

    free(map);
    printf("Destroyed map\n");
}

void redistribute_map(MapData* map, double exponent) {
    if (!map || !map->elevation) {
        fprintf(stderr, "Error: Cannot redistribute NULL map.\n");
        return;
    }
    if (exponent <= 0) {
        fprintf(stderr, "Warning: Using non-positive exponent (%.2f) in redistribution might lead to unexpected results. Applying anyway.\n", exponent);
        if (exponent == 0.0) exponent = 1e-9;
    }

    printf("Applying redistribution with exponent %.2f...\n", exponent);

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            double original_elevation = map->elevation[y][x];
            map->elevation[y][x] = pow(original_elevation, exponent);
        }
    }

    printf("Redistribution complete.\n");
}
