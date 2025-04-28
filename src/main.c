#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <stdbool.h> // Include for bool type

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h"

#define MAP_WIDTH 512 // Increased size for better visual
#define MAP_HEIGHT 256
#define OUTPUT_PNG_FILENAME "world_map_ridged.png" // New filename

#define REDISTRIBUTION_EXPONENT 1.5 // May need different exponent for ridged
#define ISLAND_MIX_FACTOR 0.8       // Slightly stronger island effect maybe
#define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE

int main() {
    printf("Procedural Map Generator - Milestone 12 (Ridged)\n");

    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);

    // --- Noise Parameters ---
    NoiseParams elev_params = {
        .octaves = 6,           // More octaves might look good with ridged
        .persistence = 0.55,    // Adjust persistence
        .lacunarity = 2.0,
        .base_frequency = 0.03, // Adjust frequency
        .use_ridged = true      // <-- ENABLE RIDGED FOR ELEVATION
    };

    NoiseParams moist_params = {
        .octaves = 4,
        .persistence = 0.45,
        .lacunarity = 2.1,
        .base_frequency = 0.06,
        .use_ridged = false     // Standard noise for moisture
    };
    // -----------------------

    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!noise_gen_elev || !noise_gen_moist || !map) { /* ... error handling ... */ return EXIT_FAILURE; }


    printf("Generating Elevation Map (Ridged)...\n"); // Updated message
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation, &elev_params);

    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);


    printf("Redistributing Elevation Map...\n");
    // NOTE: Redistribution might behave differently with ridged noise.
    // Ridged noise naturally pushes values away from 0.5.
    // An exponent > 1 might make valleys *very* flat. Experiment needed.
    redistribute_map(map, REDISTRIBUTION_EXPONENT);

    printf("Applying Island Shape...\n");
    shape_island(map, ISLAND_MIX_FACTOR, ISLAND_SHAPE_TYPE);


    printf("Printing text map to console...\n");
    print_map_text(map);

    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME) != 0) { /* ... error handling ... */ }


    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
