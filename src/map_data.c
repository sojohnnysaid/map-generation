#include "map_data.h"
#include "noise_generator.h"
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
    map->normals = NULL;
    map->detail = NULL;

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

    // Allocate normals
    map->normals = malloc(height * sizeof(Vector3*));
    if (!map->normals) {
        perror("Error allocating map normals rows");
        for(int y=0; y<height; y++) free(map->is_river[y]);
        free(map->is_river);
        for(int y=0; y<height; y++) free(map->moisture[y]);
        free(map->moisture);
        for(int y=0; y<height; y++) free(map->elevation[y]);
        free(map->elevation);
        free(map);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        map->normals[y] = malloc(width * sizeof(Vector3));
        if (!map->normals[y]) {
            perror("Error allocating map normals columns");
            for(int i=0; i<y; i++) free(map->normals[i]);
            free(map->normals);
            for(int i=0; i<height; i++) free(map->is_river[i]);
            free(map->is_river);
            for(int i=0; i<height; i++) free(map->moisture[i]);
            free(map->moisture);
            for(int i=0; i<height; i++) free(map->elevation[i]);
            free(map->elevation);
            free(map);
            return NULL;
        }
        for (int x = 0; x < width; x++) {
            map->normals[y][x].x = 0.0;
            map->normals[y][x].y = 0.0;
            map->normals[y][x].z = 1.0; // Default normal pointing up
        }
    }

    // Allocate detail map
    map->detail = malloc(height * sizeof(double*));
    if (!map->detail) {
        perror("Error allocating map detail rows");
        for(int y=0; y<height; y++) free(map->normals[y]);
        free(map->normals);
        for(int y=0; y<height; y++) free(map->is_river[y]);
        free(map->is_river);
        for(int y=0; y<height; y++) free(map->moisture[y]);
        free(map->moisture);
        for(int y=0; y<height; y++) free(map->elevation[y]);
        free(map->elevation);
        free(map);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        map->detail[y] = malloc(width * sizeof(double));
        if (!map->detail[y]) {
            perror("Error allocating map detail columns");
            for(int i=0; i<y; i++) free(map->detail[i]);
            free(map->detail);
            for(int i=0; i<height; i++) free(map->normals[i]);
            free(map->normals);
            for(int i=0; i<height; i++) free(map->is_river[i]);
            free(map->is_river);
            for(int i=0; i<height; i++) free(map->moisture[i]);
            free(map->moisture);
            for(int i=0; i<height; i++) free(map->elevation[i]);
            free(map->elevation);
            free(map);
            return NULL;
        }
        for (int x = 0; x < width; x++) {
            map->detail[y][x] = 0.0;
        }
    }

    printf("Created map (%dx%d) with elevation, moisture, river, normals, and detail layers\n", width, height);

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

    if (map->normals) {
        for (int y = 0; y < map->height; y++) {
            free(map->normals[y]);
        }
        free(map->normals);
    }

    if (map->detail) {
        for (int y = 0; y < map->height; y++) {
            free(map->detail[y]);
        }
        free(map->detail);
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

// Calculate surface normals based on elevation differences
void calculate_normals(MapData* map) {
    if (!map || !map->elevation || !map->normals) {
        fprintf(stderr, "Error: Cannot calculate normals for NULL map.\n");
        return;
    }

    printf("Calculating surface normals...\n");
    
    // Heightfield normal calculation factor - controls how steep the normals appear
    const double height_scale = 5.0; 
    
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            // Sample neighboring heights with wrapping for x (longitude)
            int x_prev = (x > 0) ? (x - 1) : (map->width - 1); 
            int x_next = (x < map->width - 1) ? (x + 1) : 0;
            int y_prev = (y > 0) ? (y - 1) : 0; // Clamp at poles
            int y_next = (y < map->height - 1) ? (y + 1) : (map->height - 1);
            
            // Calculate the slopes in x and y direction
            double dx = (map->elevation[y][x_next] - map->elevation[y][x_prev]) * height_scale;
            double dy = (map->elevation[y_next][x] - map->elevation[y_prev][x]) * height_scale;
            
            // Generate normal from slopes (-dx, -dy, 1) and normalize
            double len = sqrt(dx*dx + dy*dy + 1.0);
            map->normals[y][x].x = -dx / len;
            map->normals[y][x].y = -dy / len;
            map->normals[y][x].z = 1.0 / len;
        }
    }
    
    printf("Normal calculation complete.\n");
}

// Generate a high-detail noise layer for fine terrain details
void generate_detail_map(MapData* map, NoiseState* noise, int seed __attribute__((unused))) {
    if (!map || !map->detail || !noise) {
        fprintf(stderr, "Error: Cannot generate detail map for NULL map or noise state.\n");
        return;
    }

    printf("Generating high-frequency detail map...\n");
    
    // Set up noise parameters for fine details
    NoiseParams detail_params = {
        .noise_type = NOISE_TYPE_OPENSIMPLEX2,
        .seed = seed,
        .frequency = 0.1,          // Higher frequency than base terrain
        .fractal_type = FRACTAL_TYPE_RIDGED,
        .octaves = 4,
        .lacunarity = 3.0,
        .gain = 0.6,               // Same as persistence in old structure
        .weighted_strength = 0.0,
        .ping_pong_strength = 0.0,
        .distance_function = CELLULAR_DISTANCE_EUCLIDEAN,
        .return_type = CELLULAR_RETURN_CELL_VALUE,
        .jitter = 0.0,
        .use_domain_warp = false
    };
    
    // Generate high-frequency noise
    generate_octave_noise_to_layer(noise, map->width, map->height, map->detail, &detail_params);
    
    // Scale down the intensity of the details
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            // Only apply detail to land areas
            if (map->elevation[y][x] > 0.18) { // Above ocean level
                // Scale down the detail intensity
                map->detail[y][x] = map->detail[y][x] * 0.1;
                
                // Apply the detail to the elevation
                map->elevation[y][x] += map->detail[y][x];
                
                // Ensure elevation stays in valid range
                if (map->elevation[y][x] > 1.0) {
                    map->elevation[y][x] = 1.0;
                }
            }
        }
    }
    
    printf("Detail map generation complete.\n");
}
