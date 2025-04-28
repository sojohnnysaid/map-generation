#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h"


#define MAP_WIDTH 80
#define MAP_HEIGHT 25


#define REDISTRIBUTION_EXPONENT 2.0
#define ISLAND_MIX_FACTOR 0.7
#define ISLAND_SHAPE_TYPE ISLAND_SHAPE_SQUARE

int main() {
    printf("Procedural Map Generator - Milestone 10\n");


    int seed1 = time(NULL);
    int seed2 = seed1 + 1;
    printf("Using seeds: Elev=%d, Moist=%d\n", seed1, seed2);


    NoiseParams elev_params = {
        .octaves = 5,
        .persistence = 0.5,
        .lacunarity = 2.0,
        .base_frequency = 0.04
    };

    NoiseParams moist_params = {
        .octaves = 4,
        .persistence = 0.45,
        .lacunarity = 2.1,
        .base_frequency = 0.06
    };


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


    printf("Generating Elevation Map...\n");
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation,
                                   &elev_params);

    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture,
                                   &moist_params);


    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);

    printf("Applying Island Shape...\n");
    shape_island(map, ISLAND_MIX_FACTOR, ISLAND_SHAPE_TYPE);


    print_map_text(map);


    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);

    return EXIT_SUCCESS;
}
