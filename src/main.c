#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"


#define MAP_WIDTH 80
#define MAP_HEIGHT 25


#define ELEV_OCTAVES 5
#define ELEV_PERSISTENCE 0.5
#define ELEV_LACUNARITY 2.0
#define ELEV_BASE_FREQUENCY 0.04


#define MOIST_OCTAVES 4
#define MOIST_PERSISTENCE 0.45
#define MOIST_LACUNARITY 2.1
#define MOIST_BASE_FREQUENCY 0.06


#define REDISTRIBUTION_EXPONENT 2.0

int main() {
    printf("Procedural Map Generator - Milestone 6 (Refactored)\n"); // Updated title slightly

    int seed1 = time(NULL);
    int seed2 = seed1 + 1;

    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);

    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    if (!noise_gen_elev || !noise_gen_moist || !map) {
        fprintf(stderr, "Initialization failed.\n");
        cleanup_noise_generator(noise_gen_elev);
        cleanup_noise_generator(noise_gen_moist);
        destroy_map(map);
        return EXIT_FAILURE;
    }

    // --- Generate Elevation ---
    printf("Generating Elevation Map...\n");
    generate_octave_noise_to_layer(noise_gen_elev,             // Noise generator
                                   map->width, map->height,    // Dimensions
                                   map->elevation,             // Target layer
                                   ELEV_OCTAVES, ELEV_PERSISTENCE,
                                   ELEV_LACUNARITY, ELEV_BASE_FREQUENCY); // Parameters

    // --- Generate Moisture ---
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist,            // Noise generator
                                   map->width, map->height,    // Dimensions
                                   map->moisture,              // Target layer
                                   MOIST_OCTAVES, MOIST_PERSISTENCE,
                                   MOIST_LACUNARITY, MOIST_BASE_FREQUENCY); // Parameters

    // --- Remove the HACK block ---

    // --- Redistribute Elevation ONLY ---
    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);

    // --- Output ---
    print_map_text(map);

    // --- Cleanup ---
    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
