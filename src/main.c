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

#define MAP_WIDTH 1024    // High resolution for better detail
#define MAP_HEIGHT 512    // Height is half of width (standard for world maps)
#define OUTPUT_PNG_FILENAME "world_map_cellular.png"  // New filename for cellular output

// Improved land shaping parameters
#define CONTINENT_LAND_THRESHOLD 0.40  // Adjusted for cellular distance values
#define REDISTRIBUTION_EXPONENT 1.9    // Higher exponent for more dramatic elevation
#define APPLY_TERRACING false         // Leave off for smoother mountains
#define NUM_TERRACE_LEVELS 12

// River parameters
#define NUM_RIVERS 200               // More rivers for added detail
#define MIN_RIVER_LENGTH 20
#define MAX_RIVER_LENGTH 800         // Longer max rivers
#define RIVER_START_ELEV_MIN 0.5     // Start rivers at higher elevation

// Lake parameters
#define OCEAN_LEVEL_FOR_LAKES 0.18

// Climate parameters
#define LATITUDE_TEMP_EFFECT_STRENGTH 0.05  // Add slight latitude effect

// Visualization
#define ENABLE_CONSOLE_OUTPUT false  // Set to true to print ANSI map, false to skip

// Detail noise parameters
#define DETAIL_STRENGTH 0.2  // Controls how strong the detail noise effect is

// Use fixed seed from JSON
#define CELLULAR_SEED 1337


void free_temp_map(double** temp_map, int height) {
    if (!temp_map) return;
    for (int y = 0; y < height; y++) {
        free(temp_map[y]);
    }
    free(temp_map);
}


int main() {
    printf("Procedural Map Generator - Cellular Terrain Edition\n");

    srand((unsigned int)time(NULL));
    
    // Use the fixed seed for the cellular noise (from the JSON)
    int cellular_seed = CELLULAR_SEED;
    int seed2 = rand();
    int seed4 = rand();  // New seed for detail map
    printf("Using seeds: Cellular=%d, Moist=%d, Detail=%d\n", cellular_seed, seed2, seed4);

    // Initialize moisture and detail noise with standard params
    NoiseParams moist_params = {
        .noise_type = NOISE_TYPE_OPENSIMPLEX2,
        .seed = seed2,
        .frequency = 0.06,
        .fractal_type = FRACTAL_TYPE_FBM,
        .octaves = 4,
        .lacunarity = 2.1,
        .gain = 0.45,
        .weighted_strength = 0.0,
        .ping_pong_strength = 0.0,
        // Cellular settings not used for this noise
        .distance_function = CELLULAR_DISTANCE_EUCLIDEAN,
        .return_type = CELLULAR_RETURN_CELL_VALUE,
        .jitter = 0.0,
        // No domain warp for this noise
        .use_domain_warp = false
    };
    
    // Create the cellular params as per the JSON settings
    NoiseParams cellular_params = create_cellular_default_params();
    
    // Detail noise params
    NoiseParams detail_params = {
        .noise_type = NOISE_TYPE_OPENSIMPLEX2,
        .seed = seed4,
        .frequency = 0.08,
        .fractal_type = FRACTAL_TYPE_RIDGED,
        .octaves = 5,
        .lacunarity = 2.5,
        .gain = 0.6,
        .weighted_strength = 0.0,
        .ping_pong_strength = 0.0,
        // Cellular settings not used for this noise
        .distance_function = CELLULAR_DISTANCE_EUCLIDEAN,
        .return_type = CELLULAR_RETURN_CELL_VALUE,
        .jitter = 0.0,
        // No domain warp for this noise
        .use_domain_warp = false
    };

    // Initialize noise generators
    NoiseState* noise_gen_cellular = init_noise_generator_with_params(&cellular_params);
    NoiseState* noise_gen_moist = init_noise_generator_with_params(&moist_params);
    NoiseState* noise_gen_detail = init_noise_generator_with_params(&detail_params);
    MapData* map = create_map(MAP_WIDTH, MAP_HEIGHT);

    // Allocate continent map
    double** cellular_map = malloc(MAP_HEIGHT * sizeof(double*));
    bool cellular_map_ok = (cellular_map != NULL);
    if(cellular_map_ok) {
        for(int y = 0; y < MAP_HEIGHT; ++y) {
            cellular_map[y] = malloc(MAP_WIDTH * sizeof(double));
            if (!cellular_map[y]) {
                cellular_map_ok = false;
                free_temp_map(cellular_map, y);
                break;
            }
        }
    }

    // Check for initialization failures
    if (!noise_gen_cellular || !noise_gen_moist || !noise_gen_detail || 
        !map || !cellular_map_ok) {
        fprintf(stderr, "Initialization or temp map allocation failed.\n");
        cleanup_noise_generator(noise_gen_cellular);
        cleanup_noise_generator(noise_gen_moist);
        cleanup_noise_generator(noise_gen_detail);
        destroy_map(map);
        if(cellular_map_ok) free_temp_map(cellular_map, MAP_HEIGHT);
        return EXIT_FAILURE;
    }

    // Generate base elevation using cellular noise
    printf("Generating Cellular Continent Map...\n");
    generate_cellular_noise_to_layer(noise_gen_cellular, map->width, map->height, cellular_map, &cellular_params);
    
    // Generate moisture map with standard noise
    printf("Generating Moisture Map...\n");
    generate_octave_noise_to_layer(noise_gen_moist, map->width, map->height, map->moisture, &moist_params);

    // Apply cellular terrain to elevation map
    printf("Applying Cellular Terrain to Elevation Map...\n");
    apply_continent_mask(map, cellular_map, map->width, map->height, CONTINENT_LAND_THRESHOLD);
    
    // Redistribute to enhance terrain
    printf("Redistributing Elevation Map...\n");
    redistribute_map(map, REDISTRIBUTION_EXPONENT);
    
    // Generate high-frequency detail map
    printf("Generating Detail Map...\n");
    generate_detail_map(map, noise_gen_detail, seed4);
    
    // Optional: Apply terracing for step-like mountains
    if (APPLY_TERRACING) {
        printf("Applying Terraces...\n");
        apply_terraces(map, NUM_TERRACE_LEVELS);
    }

    // Generate hydrology features
    printf("Filling Lakes...\n");
    fill_lakes(map, OCEAN_LEVEL_FOR_LAKES);
    printf("Generating Rivers...\n");
    generate_rivers(map, NUM_RIVERS, MIN_RIVER_LENGTH, MAX_RIVER_LENGTH, RIVER_START_ELEV_MIN);

    // Calculate normals for 3D lighting effect
    printf("Calculating Surface Normals...\n");
    calculate_normals(map);

    // Output the map
    if (ENABLE_CONSOLE_OUTPUT) {
        printf("Printing text map to console...\n");
        print_map_text(map, LATITUDE_TEMP_EFFECT_STRENGTH);
    }

    printf("Writing map to PNG image file...\n");
    if (write_map_png(map, OUTPUT_PNG_FILENAME, LATITUDE_TEMP_EFFECT_STRENGTH) != 0) {
        fprintf(stderr, "Error writing PNG file.\n");
    }

    // Clean up
    destroy_map(map);
    cleanup_noise_generator(noise_gen_cellular);
    cleanup_noise_generator(noise_gen_moist);
    cleanup_noise_generator(noise_gen_detail);
    free_temp_map(cellular_map, MAP_HEIGHT);

    printf("Map generation complete.\n");
    return EXIT_SUCCESS;
}
