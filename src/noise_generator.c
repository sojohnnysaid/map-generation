#include "noise_generator.h"
#include <stdlib.h>
#include <stdio.h>

// --- FastNoiseLite Integration ---
// Define the implementation macro *before* including the header
// This tells the header to include the actual function bodies here.
#define FNL_IMPL
#include "FastNoiseLite.h"
// ---------------------------------

// Define the actual structure for our opaque pointer
// We just need to hold the fnl_state
struct NoiseState {
    fnl_state noise;
};

NoiseState* init_noise_generator(int seed) {
    NoiseState* state = malloc(sizeof(NoiseState));
    if (!state) {
        perror("Error allocating NoiseState");
        return NULL;
    }

    state->noise = fnlCreateState(); // Get default state from the library
    state->noise.noise_type = FNL_NOISE_OPENSIMPLEX2; // Or FNL_NOISE_PERLIN etc.
    state->noise.seed = seed;
    // state->noise.frequency = 0.01f; // Can set defaults here if desired

    printf("Initialized noise generator with seed %d\n", seed);
    return state;
}

void cleanup_noise_generator(NoiseState* state) {
    if (state) {
        // fnlCreateState doesn't allocate dynamic memory for the state itself
        // in the default setup, so we just free our wrapper struct.
        free(state);
        printf("Cleaned up noise generator state.\n");
    }
}

// Helper to get noise and rescale it to 0.0 - 1.0
// FastNoiseLite typically returns -1.0 to 1.0
static inline double get_scaled_noise(NoiseState* state, float x, float y) {
     // Use library's 2D noise function
    float noise_val = fnlGetNoise2D(&(state->noise), x, y);
    // Rescale from [-1, 1] to [0, 1]
    return (double)(noise_val * 0.5f + 0.5f);
}


void generate_noise_map(NoiseState* state, MapData* map) {
    if (!state || !map || !map->elevation) {
        fprintf(stderr, "Error: Invalid state or map provided to generate_noise_map.\n");
        return;
    }

    printf("Generating noise map...\n");
    // Note: The tutorial normalizes coordinates to [-0.5, 0.5] and then scales by frequency.
    // FastNoiseLite often works well with larger coordinate ranges directly, scaled by frequency.
    // Let's try the tutorial's approach first for consistency.

    // Set a base frequency for the noise calculation
    state->noise.frequency = 0.1f; // Lower frequency = larger features. Adjust as needed.

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            // Normalize coordinates to [-0.5, 0.5] range
            // Use float for noise function inputs
            float nx = (float)x / map->width - 0.5f;
            float ny = (float)y / map->height - 0.5f;

            // Important: Noise functions often take coordinates directly.
            // The frequency setting in FastNoiseLite handles the scaling.
            // So, we multiply normalized coords by a factor related to frequency,
            // OR we can skip normalization and use world coords (x,y) directly
            // and rely solely on the frequency setting.
            // Let's use world coords and frequency setting for simplicity with FNL.
            // We might need to adjust frequency significantly.

            // --- Method 1: Using world coordinates and FNL frequency ---
            // float noise_x = (float)x;
            // float noise_y = (float)y;
            // state->noise.frequency = 0.02f; // Adjust this! Try 0.01 to 0.1

            // --- Method 2: Using normalized coordinates like tutorial ---
            // We still need scaling even with normalized coords. Let's use a base scale factor.
            // This factor effectively *is* the frequency/wavelength control.
            float scale = 5.0f; // Similar to frequency=5 in tutorial example. Adjust this!
            float noise_x = nx * scale;
            float noise_y = ny * scale;

            // Let's stick with Method 2 for now to follow the tutorial structure closely.
            map->elevation[y][x] = get_scaled_noise(state, noise_x, noise_y);
        }
    }
    printf("Noise map generation complete.\n");
}

// Implementation for the direct value getter (may not be used initially)
float get_noise_value(NoiseState* state, float x, float y) {
     if (!state) return 0.0f;
     return fnlGetNoise2D(&(state->noise), x, y);
}
