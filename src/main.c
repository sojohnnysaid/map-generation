#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"

// --- Map Dimensions ---
#define MAP_WIDTH 80
#define MAP_HEIGHT 25

// --- Noise Parameters ---
#define NUM_OCTAVES 4
#define PERSISTENCE 0.5
#define LACUNARITY 2.0
#define BASE_FREQUENCY 0.05

// --- Redistribution Parameter ---
#define REDISTRIBUTION_EXPONENT 2.0 // Exponent > 1 pushes mid-values down (flatter valleys)
                                   // Exponent < 1 pulls mid-values up (sharper peaks)
                                   // Exponent = 1 does nothing

int main() {
    printf("Procedural Map Generator - Milestone 5\n");

    // --- Initialize ---
    int seed = time(NULL);
    // int seed = 12345; // Fixed seed for testing consistency
    NoiseState* noise_gen = init_noise_generator(seed);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    if (!noise_gen || !map) {
        fprintf(stderr, "Initialization failed.\n");
        cleanup_noise_generator(noise_gen); // Safe to call even if NULL
        destroy_map(map);                   // Safe to call even if NULL
        return EXIT_FAILURE;
    }

    // --- Generate Noise ---
    generate_octave_noise_map(noise_gen, map,
                              NUM_OCTAVES, PERSISTENCE, LACUNARITY, BASE_FREQUENCY);

    // --- Redistribute ---
    redistribute_map(map, REDISTRIBUTION_EXPONENT); // Call the new function

    // --- Output ---
    print_map_text(map); // Prints the map *after* redistribution

    // --- Cleanup ---
    destroy_map(map);
    cleanup_noise_generator(noise_gen);

    return EXIT_SUCCESS;
}
