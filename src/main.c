#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h" // <-- Include new header

// --- Map Dimensions ---
#define MAP_WIDTH 80
#define MAP_HEIGHT 25

// --- Noise Parameters ---
#define ELEV_OCTAVES 5
#define ELEV_PERSISTENCE 0.5
#define ELEV_LACUNARITY 2.0
#define ELEV_BASE_FREQUENCY 0.04

#define MOIST_OCTAVES 4
#define MOIST_PERSISTENCE 0.45
#define MOIST_LACUNARITY 2.1
#define MOIST_BASE_FREQUENCY 0.06

// --- Redistribution ---
#define REDISTRIBUTION_EXPONENT 2.0

// --- Island Shaping Parameters ---
#define ISLAND_MIX_FACTOR 0.7 // 0.0 = none, 1.0 = full shape. Try 0.5 to 0.9
#define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE // Or ISLAND_SHAPE_RADIAL

int main() {
    printf("Procedural Map Generator - Milestone 8\n"); // Updated title

    // --- Init (Seeds, Noise Generators, Map) ---
    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);
    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!noise_gen_elev || !noise_gen_moist || !map) { /* ... error handling ... */ return EXIT_FAILURE; }

    // --- Generate Base Noise Layers ---
    printf("Generating Elevation Map...\n");
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation,
                                   ELEV_OCTAVES, ELEV_PERSISTENCE, ELEV_LACUNARITY, ELEV_BASE_FREQUENCY);
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture,
                                   MOIST_OCTAVES, MOIST_PERSISTENCE, MOIST_LACUNARITY, MOIST_BASE_FREQUENCY);

    // --- Apply Transformations ---
    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);

    printf("Applying Island Shape...\n"); // Add message
    shape_island(map, ISLAND_MIX_FACTOR, ISLAND_SHAPE_TYPE); // <-- Call the shaping function

    // --- Output ---
    print_map_text(map); // Print the final shaped map

    // --- Cleanup ---
    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
