#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h" // Includes terrace function declaration

#define MAP_WIDTH 512
#define MAP_HEIGHT 256
#define OUTPUT_PNG_FILENAME "world_map_terraced.png" // New filename

// --- Noise Params (Ridged for Elevation) ---
NoiseParams elev_params = {
    .octaves = 6, .persistence = 0.55, .lacunarity = 2.0,
    .base_frequency = 0.03, .use_ridged = true
};
NoiseParams moist_params = {
    .octaves = 4, .persistence = 0.45, .lacunarity = 2.1,
    .base_frequency = 0.06, .use_ridged = false
};
// ------------------------------------------

// --- Transformation Params ---
#define REDISTRIBUTION_EXPONENT 2.5
#define ISLAND_MIX_FACTOR 0.8
#define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE
#define NUM_TERRACE_LEVELS 12 // How many distinct steps (e.g., 8-20 often works well)
// ---------------------------


int main() {
    printf("Procedural Map Generator - Milestone 13 (Terraces)\n");

    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);

    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!noise_gen_elev || !noise_gen_moist || !map) { /* ... error handling ... */ return EXIT_FAILURE; }

    // --- Generation ---
    printf("Generating Elevation Map (Ridged)...\n");
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation, &elev_params);
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);

    // --- Transformations ---
    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);
    printf("Applying Island Shape...\n");
    shape_island(map, ISLAND_MIX_FACTOR, ISLAND_SHAPE_TYPE);
    printf("Applying Terraces...\n"); // Add message
    apply_terraces(map, NUM_TERRACE_LEVELS); // <-- Call the terracing function

    // --- Output ---
    printf("Printing text map to console...\n");
    print_map_text(map); // ANSI output will show the stepped colors
    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME) != 0) { /* ... error handling ... */ }

    // --- Cleanup ---
    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
