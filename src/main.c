#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <stdbool.h>
#include <math.h>

#include "map_data.h"
#include "map_io.h"
#include "noise_generator.h"
#include "map_shaping.h"
#include "hydrology.h"

#define MAP_WIDTH 512
#define MAP_HEIGHT 256
#define OUTPUT_PNG_FILENAME "world_map_fix.png" // New filename


#define CONTINENT_LAND_THRESHOLD 0.52 // Increased this value
#define REDISTRIBUTION_EXPONENT 1.8
#define APPLY_TERRACING false
#define NUM_TERRACE_LEVELS 12


#define NUM_RIVERS 100
#define MIN_RIVER_LENGTH 15
#define MAX_RIVER_LENGTH 500
#define RIVER_START_ELEV_MIN 0.5


#define OCEAN_LEVEL_FOR_LAKES 0.18


#define LATITUDE_TEMP_EFFECT_STRENGTH 0.0

#define ENABLE_CONSOLE_OUTPUT false // Set to true to print ANSI map, false to skip


void free_temp_map(double** temp_map, int height) {
    if (!temp_map) return;
    for (int y = 0; y < height; y++) {
        free(temp_map[y]);
    }
    free(temp_map);
}


int main() {
    printf("Procedural Map Generator - Fix Attempt\n");

    srand((unsigned int)time(NULL));
    int seed1 = rand();
    int seed2 = rand();
    int seed3 = rand();
    printf("Using seeds: Elev=%d, Moist=%d, Cont=%d\n", seed1, seed2, seed3);


    NoiseParams elev_params = {
        .octaves = 6, .persistence = 0.5, .lacunarity = 2.0,
        .base_frequency = 0.02, .use_ridged = false
    };
    NoiseParams moist_params = {
        .octaves = 4, .persistence = 0.45, .lacunarity = 2.1,
        .base_frequency = 0.06, .use_ridged = false
    };
    NoiseParams cont_params = {
        .octaves = 2, .persistence = 0.5, .lacunarity = 2.0,
        .base_frequency = 0.008, .use_ridged = false
    };


    NoiseState* noise_gen_elev = init_noise_generator(seed1);
    NoiseState* noise_gen_moist = init_noise_generator(seed2);
    NoiseState* noise_gen_cont = init_noise_generator(seed3);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);


    double** continent_map = malloc(MAP_HEIGHT * sizeof(double*));
    bool cont_map_ok = (continent_map != NULL);
    if(cont_map_ok) {
        for(int y = 0; y < MAP_HEIGHT; ++y) {
            continent_map[y] = malloc(MAP_WIDTH * sizeof(double));
            if (!continent_map[y]) {
                cont_map_ok = false;
                free_temp_map(continent_map, y);
                break;
            }
        }
    }


    if (!noise_gen_elev || !noise_gen_moist || !noise_gen_cont || !map || !cont_map_ok) {
        fprintf(stderr, "Initialization or temp map allocation failed.\n");
        cleanup_noise_generator(noise_gen_elev);
        cleanup_noise_generator(noise_gen_moist);
        cleanup_noise_generator(noise_gen_cont);
        destroy_map(map);
        if(cont_map_ok) free_temp_map(continent_map, MAP_HEIGHT);
        return EXIT_FAILURE;
    }


    printf("Generating Base Elevation Map...\n");
    generate_octave_noise_to_layer(noise_gen_elev, map->width, map->height, map->elevation, &elev_params);
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);
    printf("Generating Continent Noise Map...\n");
    generate_octave_noise_to_layer(noise_gen_cont, map->width, map->height, continent_map, &cont_params);


    printf("Applying Continent Mask...\n");
    apply_continent_mask(map, continent_map, map->width, map->height, CONTINENT_LAND_THRESHOLD);
    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);
    if (APPLY_TERRACING) {
        printf("Applying Terraces...\n");
        apply_terraces(map, NUM_TERRACE_LEVELS);
    }


    printf("Filling Lakes...\n");
    fill_lakes(map, OCEAN_LEVEL_FOR_LAKES);


    printf("Generating Rivers...\n");
    generate_rivers(map, NUM_RIVERS, MIN_RIVER_LENGTH, MAX_RIVER_LENGTH, RIVER_START_ELEV_MIN);


    if (ENABLE_CONSOLE_OUTPUT) {
    	printf("Printing text map to console...\n");
    	print_map_text(map, LATITUDE_TEMP_EFFECT_STRENGTH);
	}

    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME, LATITUDE_TEMP_EFFECT_STRENGTH) != 0) {
        fprintf(stderr, "Error writing PNG file.\n");
    }


    destroy_map(map);
    cleanup_noise_generator(noise_gen_elev);
    cleanup_noise_generator(noise_gen_moist);
    cleanup_noise_generator(noise_gen_cont);
    free_temp_map(continent_map, MAP_HEIGHT);

    return EXIT_SUCCESS;
}
