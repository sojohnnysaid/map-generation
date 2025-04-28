#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include "map_data.h"

typedef struct NoiseState NoiseState;

NoiseState* init_noise_generator(int seed);
void cleanup_noise_generator(NoiseState* state);

// Modified signature: takes target layer
void generate_octave_noise_to_layer(NoiseState* state,
                                    int width, int height, // Pass dimensions
                                    double** target_layer, // Target layer (e.g., map->elevation)
                                    int octaves, double persistence,
                                    double lacunarity, double base_frequency);

// get_noise_value can remain, but its utility is low now
float get_noise_value(NoiseState* state, float x, float y);

#endif // NOISE_GENERATOR_H
