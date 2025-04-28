#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 25

// --- Noise Parameters ---
#define NUM_OCTAVES 4        // How many layers of noise
#define PERSISTENCE 0.5    // How much detail is added or removed at each octave (amplitude multiplier)
#define LACUNARITY 2.0     // How much detail increases or decreases for each octave (frequency multiplier)
#define BASE_FREQUENCY 0.05     // Initial frequency/scale (adjust for overall zoom)


int main() {
    printf("Procedural Map Generator - Milestone 4\n");

    // --- Initialize ---
    int seed = time(NULL);
    // int seed = 12345; // Use fixed seed for testing consistency
    NoiseState* noise_gen = init_noise_generator(seed);
    if (!noise_gen) {
         fprintf(stderr, "Failed to initialize noise generator.\n");
         return EXIT_FAILURE;
    }

    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!map) {
        fprintf(stderr, "Failed to create map.\n");
        cleanup_noise_generator(noise_gen);
        return EXIT_FAILURE;
    }

    // --- Generate ---
    // Call the new octave generation function
    generate_octave_noise_map(noise_gen, map,
                              NUM_OCTAVES, PERSISTENCE, LACUNARITY, BASE_FREQUENCY);

    // --- Output ---
    print_map_text(map); // Use the same text output for now

    // --- Cleanup ---
    destroy_map(map);
    map = NULL;
    cleanup_noise_generator(noise_gen);
    noise_gen = NULL;

    return EXIT_SUCCESS;
}
