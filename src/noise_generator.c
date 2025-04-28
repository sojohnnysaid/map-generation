#include "noise_generator.h"
#include <stdlib.h>
#include <stdio.h> // Included multiple times in original, ensure only once needed
#include <math.h> // Needed for pow() if used later, currently not
#include <float.h> // For DBL_MAX, DBL_MIN (or FLT_MAX/MIN)

// --- FastNoiseLite Integration ---
// Define the implementation macro *before* including the header
#define FNL_IMPL
#include "FastNoiseLite.h"
// ---------------------------------

// Define the actual structure for our opaque pointer
struct NoiseState {
    fnl_state noise;
    // Can add other persistent state here if needed later
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
    // We don't set a default frequency here; it will be set per-octave
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

// Static helper: Gets raw noise [-1, 1] using the current state settings
// Assumes state->noise.frequency has been set appropriately before calling.
static inline float get_raw_noise(NoiseState* state, float x, float y) {
     // Make sure state is valid if this were public, but it's static inline
     return fnlGetNoise2D(&(state->noise), x, y);
}

// Octave generation function using world coordinates and internal frequency setting
void generate_octave_noise_map(NoiseState* state, MapData* map,
                               int octaves, double persistence,
                               double lacunarity, double base_frequency) // Note: Parameter renamed
{
    if (!state || !map || !map->elevation) {
        fprintf(stderr, "Error: Invalid state or map provided to generate_octave_noise_map.\n");
        return;
    }
    if (octaves < 1) octaves = 1;

    printf("Generating octave noise map (%d octaves, persist=%.2f, lacun=%.2f, freq=%.4f)...\n", // Label updated
           octaves, persistence, lacunarity, base_frequency);

    // --- Keep offsets defined but unused for now ---
    float octave_offsets_x[] = { 0.0f, 17.8f, 31.1f, 47.5f, 59.3f, 71.9f, 83.1f, 97.7f };
    float octave_offsets_y[] = { 0.0f, 23.5f, 41.7f, 53.3f, 67.3f, 79.1f, 89.9f, 101.3f };
    int max_supported_octaves = sizeof(octave_offsets_x) / sizeof(octave_offsets_x[0]);
    if (octaves > max_supported_octaves) {
        fprintf(stderr, "Warning: Requested %d octaves, but only have offsets for %d. Clamping.\n",
                octaves, max_supported_octaves);
        octaves = max_supported_octaves;
    }
    // --- ---

    // Variables to track min/max output
    double min_elevation = DBL_MAX;
    double max_elevation = -DBL_MAX; // Correct initialization

    // Pre-calculate max amplitude for normalization
    double max_possible_amplitude = 0.0;
    double current_amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        max_possible_amplitude += current_amplitude;
        current_amplitude *= persistence;
    }
    printf("--> Calculated max_possible_amplitude: %.4f\n", max_possible_amplitude);

    // Handle potential division by zero
    if (max_possible_amplitude <= 1e-6) {
        max_possible_amplitude = 1.0;
        fprintf(stderr, "Warning: Max possible amplitude is near zero. Normalization may be inaccurate.\n");
    }

    // Main generation loop
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {

            // --- Use Raw Map Coordinates (x, y) ---
            float world_x = (float)x;
            float world_y = (float)y;

            double total_noise = 0.0;
            double amplitude = 1.0;
            double frequency = base_frequency; // Start with base frequency

            for (int i = 0; i < octaves; i++) {
                // Set the frequency for the current octave in the noise state
                state->noise.frequency = (float)frequency;

                // Coordinates for this octave (offsets still commented out)
                float noise_x = world_x; // + octave_offsets_x[i]; // Can add later if needed
                float noise_y = world_y; // + octave_offsets_y[i];

                // Get raw noise [-1, 1] using the current state settings
                float noise_val = get_raw_noise(state, noise_x, noise_y);

                // Accumulate weighted noise
                total_noise += noise_val * amplitude;

                // Update amplitude and frequency for the next octave
                amplitude *= persistence;
                frequency *= lacunarity; // Frequency increases
            }

            // Normalize the total noise from ~[-max_amp, +max_amp] to [0, 1]
            double normalized_noise = (total_noise / max_possible_amplitude) * 0.5 + 0.5;

            // Clamp values just in case they slightly exceed bounds
            if (normalized_noise < 0.0) normalized_noise = 0.0;
            if (normalized_noise > 1.0) normalized_noise = 1.0;

            map->elevation[y][x] = normalized_noise;

            // Update min/max tracking
            if (normalized_noise < min_elevation) {
                min_elevation = normalized_noise;
            }
            if (normalized_noise > max_elevation) {
                max_elevation = normalized_noise; // Corrected update
            }
        }
    }
    printf("Octave noise map generation complete.\n");

    // Print the actual range found
    printf("--> Actual elevation range generated: [%.4f, %.4f]\n",
           min_elevation, max_elevation);
}

// Gets a single noise value using specified coordinates and the CURRENT state settings
// Be cautious if frequency isn't explicitly set before calling externally.
float get_noise_value(NoiseState* state, float x, float y) {
     if (!state) return 0.0f;
     // Note: Uses whatever frequency is currently set in state->noise!
     // Might be better to add a frequency parameter here if called externally.
     return fnlGetNoise2D(&(state->noise), x, y);
}
