#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h" // Includes continent mask function declaration

#define MAP_WIDTH 512
#define MAP_HEIGHT 256
#define OUTPUT_PNG_FILENAME "world_map_continents.png" // New filename

// --- Base Elevation Noise Params (Ridged) ---
NoiseParams elev_params = {
    .octaves = 6, .persistence = 0.55, .lacunarity = 2.0,
    .base_frequency = 0.03, .use_ridged = true // Ridged optional now
};
// --- Moisture Noise Params ---
NoiseParams moist_params = {
    .octaves = 4, .persistence = 0.45, .lacunarity = 2.1,
    .base_frequency = 0.06, .use_ridged = false
};
// --- Continent Noise Params ---
NoiseParams cont_params = {
    .octaves = 2,           // Few octaves for large blobs
    .persistence = 0.5,     // Less important for few octaves
    .lacunarity = 2.0,
    .base_frequency = 0.008, // VERY low frequency for large shapes
    .use_ridged = false
};
// ---------------------------

// --- Transformation Params ---
#define CONTINENT_LAND_THRESHOLD 0.45 // Value in cont_map above which is land (Adjust!)
#define REDISTRIBUTION_EXPONENT 1.5
// #define ISLAND_MIX_FACTOR 0.8 // Removed
// #define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE // Removed
#define NUM_TERRACE_LEVELS 12 // Keep terracing for now
// ---------------------------

// --- Helper to free 2D array --- (Could move to map_data)
void free_temp_map(double** temp_map, int height) {
    if (!temp_map) return;
    for (int y = 0; y < height; y++) {
        free(temp_map[y]);
    }
    free(temp_map);
}
// -----------------------------

int main() {
    printf("Procedural Map Generator - Milestone 14 (Continents)\n");

    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    int seed3 = seed2 + 1; // Third seed
    printf("Using seeds: Elev=%d, Moist=%d, Cont=%d\n", seed1, seed2, seed3);

    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    NoiseState* noise_gen_cont = init_noise_generator(seed3); // Init third generator
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    // --- Allocate temporary continent map ---
    double** continent_map = malloc(MAP_HEIGHT * sizeof(double*));
    bool cont_map_ok = (continent_map != NULL);
    if(cont_map_ok) {
        for(int y = 0; y < MAP_HEIGHT; ++y) {
            continent_map[y] = malloc(MAP_WIDTH * sizeof(double));
            if (!continent_map[y]) {
                cont_map_ok = false;
                free_temp_map(continent_map, y); // Free partially allocated rows
                break;
            }
        }
    }
    // ------------------------------------

    if (!noise_gen_elev || !noise_gen_moist || !noise_gen_cont || !map || !cont_map_ok) {
        fprintf(stderr, "Initialization or temp map allocation failed.\n");
        cleanup_noise_generator(noise_gen_elev);
        cleanup_noise_generator(noise_gen_moist);
        cleanup_noise_generator(noise_gen_cont);
        destroy_map(map);
        if(cont_map_ok) free_temp_map(continent_map, MAP_HEIGHT); // Free if fully alloc'd but others failed
        return EXIT_FAILURE;
    }

    // --- Generation ---
    printf("Generating Base Elevation Map...\n"); // Renamed message
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation, &elev_params);
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);
    printf("Generating Continent Noise Map...\n");
    generate_octave_noise_to_layer(noise_gen_cont, map->width, map->height, continent_map, &cont_params);

    // --- Transformations ---
    printf("Applying Continent Mask...\n"); // Apply mask BEFORE redistribution
    apply_continent_mask(map, continent_map, map->width, map->height, CONTINENT_LAND_THRESHOLD);

    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);

    // Removed Island Shape call

    printf("Applying Terraces...\n");
    apply_terraces(map, NUM_TERRACE_LEVELS); // Terracing still optional

    // --- Output ---
    printf("Printing text map to console...\n");
    print_map_text(map);
    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME) != 0) { /* ... error handling ... */ }

    // --- Cleanup ---
    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);
    cleanup_noise_generator(noise_gen_cont); // Cleanup third generator
    free_temp_map(continent_map, MAP_HEIGHT); // Free the temporary map

    return EXIT_SUCCESS;
}
