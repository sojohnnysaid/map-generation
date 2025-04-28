#include <stdio.h>
#include <stdlib.h>
#include <time.h> // For seeding random number generator

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h" // Include the noise generator header

#define MAP_WIDTH 80
#define MAP_HEIGHT 25

int main() {
    printf("Procedural Map Generator - Milestone 3\n");

    // --- Initialize ---
    // Use current time for a different map each run
    // For reproducibility during development, use a fixed seed like 12345
    int seed = time(NULL);
    // int seed = 12345;
    NoiseState* noise_gen = init_noise_generator(seed);
    if (!noise_gen) {
         fprintf(stderr, "Failed to initialize noise generator.\n");
         return EXIT_FAILURE;
    }

    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!map) {
        fprintf(stderr, "Failed to create map.\n");
        cleanup_noise_generator(noise_gen); // Clean up noise gen before exiting
        return EXIT_FAILURE;
    }

    // --- Generate ---
    // Replace the test pattern fill with noise generation
    // fill_map_constant(map, 0.5); // Remove or comment out previous fill
    generate_noise_map(noise_gen, map);

    // --- Output ---
    print_map_text(map);

    // --- Cleanup ---
    destroy_map(map);
    map = NULL;
    cleanup_noise_generator(noise_gen);
    noise_gen = NULL;

    return EXIT_SUCCESS;
}
