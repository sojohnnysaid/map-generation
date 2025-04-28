#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include "map_data.h"

typedef struct NoiseState NoiseState;

NoiseState* init_noise_generator(int seed);
void cleanup_noise_generator(NoiseState* state);

// Updated function signature:
void generate_octave_noise_map(NoiseState* state, MapData* map,
                               int octaves,       // Number of noise layers
                               double persistence, // Amplitude multiplier per octave (<1)
                               double lacunarity,  // Frequency multiplier per octave (>1)
                               double base_scale); // Initial scale/frequency

// Keep the single value getter if needed, though less relevant now
float get_noise_value(NoiseState* state, float x, float y);

#endif // NOISE_GENERATOR_H
