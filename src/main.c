#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#include "map_data.h"
#include "map_io.h" // Includes PNG function declaration now
#include "noise_generator.h"
#include "map_shaping.h"


#define MAP_WIDTH 1024
#define MAP_HEIGHT 512 // Can increase this for better PNGs later

// --- Define output filename (CHANGED) ---
#define OUTPUT_PNG_FILENAME "world_map.png"
// --------------------------------------

#define REDISTRIBUTION_EXPONENT 2.0
#define ISLAND_MIX_FACTOR 0.7
#define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE

int main() {
    printf("Procedural Map Generator - Milestone 11 (PNG)\n"); // Updated title

    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);

    NoiseParams elev_params = { /* ... */ .base_frequency = 0.04 };
    NoiseParams moist_params = { /* ... */ .base_frequency = 0.06 };
    elev_params.octaves = 5; elev_params.persistence = 0.5; elev_params.lacunarity = 2.0;
    moist_params.octaves = 4; moist_params.persistence = 0.45; moist_params.lacunarity = 2.1;


    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);
    if (!noise_gen_elev || !noise_gen_moist || !map) { /* ... error handling ... */ return EXIT_FAILURE; }


    printf("Generating Elevation Map...\n");
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation, &elev_params);
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);


    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);
    printf("Applying Island Shape...\n");
    shape_island(map, ISLAND_MIX_FACTOR, ISLAND_SHAPE_TYPE);


    printf("Printing text map to console...\n");
    print_map_text(map);

    // --- Updated Call to write PNG ---
    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME) != 0) { // <-- Call PNG write function
        fprintf(stderr, "Error writing PNG file.\n");
    }
    // ---------------------------------


    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
