#ifndef NOISE_GENERATOR_H
#define NOISE_GENERATOR_H

#include "map_data.h" // MapData needed only if funcs return it or take it
#include <stdbool.h> // <-- Include for bool type

// FastNoise noise type constants
typedef enum {
    NOISE_TYPE_OPENSIMPLEX2,
    NOISE_TYPE_CELLULAR,
    NOISE_TYPE_PERLIN,
    NOISE_TYPE_VALUE_CUBIC
} NoiseType;

// FastNoise cellular distance function
typedef enum {
    CELLULAR_DISTANCE_EUCLIDEAN,
    CELLULAR_DISTANCE_MANHATTAN,
    CELLULAR_DISTANCE_HYBRID
} CellularDistanceFunction;

// FastNoise cellular return type
typedef enum {
    CELLULAR_RETURN_CELL_VALUE,
    CELLULAR_RETURN_DISTANCE,
    CELLULAR_RETURN_DISTANCE2,
    CELLULAR_RETURN_DISTANCE2_ADD,
    CELLULAR_RETURN_DISTANCE2_SUB,
    CELLULAR_RETURN_DISTANCE2_MUL,
    CELLULAR_RETURN_DISTANCE2_DIV
} CellularReturnType;

// FastNoise fractal type
typedef enum {
    FRACTAL_TYPE_NONE,
    FRACTAL_TYPE_FBM,
    FRACTAL_TYPE_RIDGED,
    FRACTAL_TYPE_PING_PONG,
    FRACTAL_TYPE_DOMAIN_WARP_PROGRESSIVE,
    FRACTAL_TYPE_DOMAIN_WARP_INDEPENDENT
} FractalType;

// --- Enhanced Noise Parameters ---
typedef struct {
    // Basic settings
    NoiseType noise_type;
    int seed;
    double frequency;
    
    // Fractal settings
    FractalType fractal_type;
    int octaves;
    double lacunarity;
    double gain; // Same as persistence but more standard name
    double weighted_strength;
    double ping_pong_strength;
    
    // Cellular settings
    CellularDistanceFunction distance_function;
    CellularReturnType return_type;
    double jitter;
    
    // Domain warp settings
    bool use_domain_warp;
    NoiseType domain_warp_type;
    double domain_warp_amplitude;
    double domain_warp_frequency;
    FractalType domain_warp_fractal_type;
    int domain_warp_octaves;
    double domain_warp_lacunarity;
    double domain_warp_gain;
    int domain_warp_seed;
} NoiseParams;
// --------------------------------------

typedef struct NoiseState NoiseState;

NoiseState* init_noise_generator(int seed);
void cleanup_noise_generator(NoiseState* state);

// Create a noise generator with specific parameters
NoiseState* init_noise_generator_with_params(const NoiseParams* params);

// Modified signature: takes NoiseParams struct
void generate_octave_noise_to_layer(NoiseState* state,
                                    int width, int height,
                                    double** target_layer,
                                    const NoiseParams* params); // Pass struct by const pointer

// Generate cellular noise layer with full control
void generate_cellular_noise_to_layer(NoiseState* state,
                                     int width, int height,
                                     double** target_layer,
                                     const NoiseParams* params);

float get_noise_value(NoiseState* state, float x, float y);

// Create default cellular noise parameters based on the JSON settings
NoiseParams create_cellular_default_params();

#endif // NOISE_GENERATOR_H
