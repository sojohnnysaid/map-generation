#include "map_io.h"
#include <stdio.h>

// --- ANSI Color Codes (Backgrounds) ---
#define ANSI_BG_DEEP_BLUE    "\x1b[44m" // OCEAN
#define ANSI_BG_SAND         "\x1b[103m"// BEACH (Bright Yellow)
#define ANSI_BG_FOREST_GREEN "\x1b[42m" // TEMPERATE_DECIDUOUS_FOREST, TEMPERATE_RAIN_FOREST (Dark Green)
#define ANSI_BG_JUNGLE_GREEN "\x1b[102m"// TROPICAL_RAIN_FOREST, TROPICAL_SEASONAL_FOREST (Bright Green)
#define ANSI_BG_GRASS        "\x1b[42m" // GRASSLAND (Using Dark Green for now)
#define ANSI_BG_SAVANNAH     "\x1b[43m" // SAVANNAH / TEMPERATE_DESERT (Dark Yellow)
#define ANSI_BG_SHRUBLAND    "\x1b[103m"// SHRUBLAND (Using Bright Yellow for now)
#define ANSI_BG_TAIGA        "\x1b[46m" // TAIGA (Cyan)
#define ANSI_BG_TUNDRA       "\x1b[106m"// TUNDRA (Bright Cyan)
#define ANSI_BG_SNOW         "\x1b[107m"// SNOW (Bright White)
#define ANSI_BG_SCORCHED     "\x1b[41m" // SCORCHED (Red)
#define ANSI_BG_BARE_ROCK    "\x1b[100m"// BARE rock (Gray)
#define ANSI_BG_DESERT_SAND  "\x1b[43m" // SUBTROPICAL_DESERT (Using Dark Yellow)

#define ANSI_RESET           "\x1b[0m"

// --- Biome Thresholds (Inspired by Tutorial Diagram - NEEDS TUNING!) ---
// Elevation Bands
#define ELEV_OCEAN           0.15 // Deep water
#define ELEV_BEACH           0.18 // Shallow water / beach sand

#define ELEV_TROPICAL_MAX    0.40 // Upper limit for tropical band
#define ELEV_TEMPERATE_MAX   0.65 // Upper limit for temperate band
#define ELEV_BOREAL_MAX      0.85 // Upper limit for boreal/taiga band
// Mountain/Arctic band is above BOREAL_MAX

// Moisture Bands (Approximate divisions of 0.0 to 1.0)
#define MOIST_DESERT         0.20 // Driest
#define MOIST_GRASS_SAVANNAH 0.40 // Dryish
#define MOIST_WOODLAND_SHRUB 0.70 // Medium
// Wettest is above WOODLAND_SHRUB

void print_map_text(const MapData* map) {
    if (!map || !map->elevation || !map->moisture) {
        printf("Cannot print NULL or incomplete map.\n");
        return;
    }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            double e = map->elevation[y][x]; // Elevation [0, 1]
            double m = map->moisture[y][x];  // Moisture [0, 1]

            const char* bg_color_code; // Store the ANSI code string

            // Determine biome based on elevation and moisture thresholds (complex version)
            if (e < ELEV_OCEAN) {
                bg_color_code = ANSI_BG_DEEP_BLUE; // OCEAN
            } else if (e < ELEV_BEACH) {
                bg_color_code = ANSI_BG_SAND; // BEACH
            }
            // High Elevation (Arctic / Alpine)
            else if (e > ELEV_BOREAL_MAX) { // Tutorial used > 0.8
                if (m < 0.1) bg_color_code = ANSI_BG_SCORCHED;      // SCORCHED (Dryest high elev)
                else if (m < 0.2) bg_color_code = ANSI_BG_BARE_ROCK;// BARE (Dry high elev)
                else if (m < 0.5) bg_color_code = ANSI_BG_TUNDRA;   // TUNDRA (Mid moisture high elev)
                else bg_color_code = ANSI_BG_SNOW;                  // SNOW (Wet high elev)
            }
            // Mid-High Elevation (Boreal / Taiga)
            else if (e > ELEV_TEMPERATE_MAX) { // Tutorial used > 0.6
                if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_SAVANNAH; // TEMPERATE_DESERT (Treating as Savannah color)
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_SHRUBLAND; // SHRUBLAND
                else bg_color_code = ANSI_BG_TAIGA; // TAIGA
            }
            // Mid-Low Elevation (Temperate)
            else if (e > ELEV_TROPICAL_MAX) { // Tutorial used > 0.3
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; // TEMPERATE_DESERT (Treating as Savannah color)
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_GRASS; // GRASSLAND
                else if (m < 0.83) bg_color_code = ANSI_BG_FOREST_GREEN; // TEMPERATE_DECIDUOUS_FOREST
                else bg_color_code = ANSI_BG_FOREST_GREEN; // TEMPERATE_RAIN_FOREST (Using same color)
            }
            // Low Elevation (Tropical)
            else { // Tutorial used e <= 0.3
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_DESERT_SAND; // SUBTROPICAL_DESERT
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; // GRASSLAND (Tropical version)
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_JUNGLE_GREEN; // TROPICAL_SEASONAL_FOREST
                else bg_color_code = ANSI_BG_JUNGLE_GREEN; // TROPICAL_RAIN_FOREST (Using same color)
            }

            // Print the color, a space to show it, and reset
            printf("%s %s", bg_color_code, ANSI_RESET);

        }
        putchar('\n'); // Newline after each row
    }
    printf("--------------------\n");
}
