#include "noise_generator.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#define FNL_IMPL
#include "FastNoiseLite.h"

struct NoiseState {
    fnl_state noise;
};

NoiseState* init_noise_generator(int seed) {
    NoiseState* state = malloc(sizeof(NoiseState));
    if (!state) {
        perror("Error allocating NoiseState");
        return NULL;
    }
    state->noise = fnlCreateState();
    state->noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    state->noise.seed = seed;
    printf("Initialized noise generator with seed %d\n", seed);
    return state;
}

void cleanup_noise_generator(NoiseState* state) {
    if (state) {
        free(state);
        printf("Cleaned up noise generator state.\n");
    }
}

// Static helper function remains unchanged
static inline float get_raw_noise(NoiseState* state, float x, float y) {
     return fnlGetNoise2D(&(state->noise), x, y);
}

// Updated function definition and implementation
void generate_octave_noise_to_layer(NoiseState* state,
                                    int width, int height, // Use passed dimensions
                                    double** target_layer, // Use target layer
                                    int octaves, double persistence,
                                    double lacunarity, double base_frequency)
{
    // Check target_layer validity
    if (!state || !target_layer) {
        fprintf(stderr, "Error: Invalid state or target_layer provided.\n");
        return;
    }
    // Check dimensions validity (optional but good)
    if (width <= 0 || height <= 0) {
         fprintf(stderr, "Error: Invalid dimensions provided.\n");
         return;
    }

    if (octaves < 1) octaves = 1;

    printf("Generating octave noise (%d octaves, persist=%.2f, lacun=%.2f, freq=%.4f)...\n",
           octaves, persistence, lacunarity, base_frequency);

    double min_val = DBL_MAX; // Use generic names now
    double max_val = -DBL_MAX;

    double max_possible_amplitude = 0.0;
    double current_amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        max_possible_amplitude += current_amplitude;
        current_amplitude *= persistence;
    }
    printf("--> Calculated max_possible_amplitude: %.4f\n", max_possible_amplitude);

    if (max_possible_amplitude <= 1e-6) {
        max_possible_amplitude = 1.0;
        fprintf(stderr, "Warning: Max possible amplitude is near zero. Normalization may be inaccurate.\n");
    }

    for (int y = 0; y < height; y++) { // Use passed height
        // Check if row pointer is valid (optional but safer)
        if (!target_layer[y]) {
             fprintf(stderr, "Error: Target layer row %d is NULL.\n", y);
             continue; // Skip this row
        }
        for (int x = 0; x < width; x++) { // Use passed width

            float world_x = (float)x;
            float world_y = (float)y;

            double total_noise = 0.0;
            double amplitude = 1.0;
            double frequency = base_frequency;

            for (int i = 0; i < octaves; i++) {
                state->noise.frequency = (float)frequency;
                float noise_x = world_x;
                float noise_y = world_y;
                float noise_val = get_raw_noise(state, noise_x, noise_y);
                total_noise += noise_val * amplitude;
                amplitude *= persistence;
                frequency *= lacunarity;
            }

            double normalized_noise = (total_noise / max_possible_amplitude) * 0.5 + 0.5;

            if (normalized_noise < 0.0) normalized_noise = 0.0;
            if (normalized_noise > 1.0) normalized_noise = 1.0;

            target_layer[y][x] = normalized_noise; // Write to target layer

            if (normalized_noise < min_val) {
                min_val = normalized_noise;
            }
            if (normalized_noise > max_val) {
                max_val = normalized_noise;
            }
        }
    }
    printf("Octave noise generation complete.\n");
    printf("--> Actual value range generated: [%.4f, %.4f]\n", min_val, max_val);
}

// get_noise_value implementation remains the same
float get_noise_value(NoiseState* state, float x, float y) {
     if (!state) return 0.0f;
     return fnlGetNoise2D(&(state->noise), x, y);
}
