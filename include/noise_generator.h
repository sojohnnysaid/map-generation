#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include "map_data.h" // MapData needed only if funcs return it or take it
#include <stdbool.h> // <-- Include for bool type

// --- NEW Struct for Noise Parameters ---
typedef struct {
    int octaves;
    double persistence;
    double lacunarity;
    double base_frequency;
    bool use_ridged;
    // Could add seed here too if desired
} NoiseParams;
// --------------------------------------

typedef struct NoiseState NoiseState;

NoiseState* init_noise_generator(int seed);
void cleanup_noise_generator(NoiseState* state);

// Modified signature: takes NoiseParams struct
void generate_octave_noise_to_layer(NoiseState* state,
                                    int width, int height,
                                    double** target_layer,
                                    const NoiseParams* params); // Pass struct by const pointer

float get_noise_value(NoiseState* state, float x, float y);

#endif // NOISE_GENERATOR_H
