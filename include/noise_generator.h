#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include "map_data.h" // To fill the map data

// Opaque pointer type for noise state to hide implementation details
typedef struct NoiseState NoiseState;

// Initializes the noise generator with a seed
NoiseState* init_noise_generator(int seed);

// Frees resources associated with the noise generator
void cleanup_noise_generator(NoiseState* state);

// Fills the map's elevation data using the initialized noise generator
void generate_noise_map(NoiseState* state, MapData* map);

// Gets a single noise value (useful for later features maybe)
// Note: FastNoiseLite returns -1 to 1, we'll rescale in generate_noise_map
// FN_DECIMAL is float by default in FastNoiseLite.h unless changed
float get_noise_value(NoiseState* state, float x, float y);

#endif // NOISE_GENERATOR_H
