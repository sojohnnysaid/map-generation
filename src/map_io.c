#include "map_io.h"
#include <stdio.h>

#define ANSI_BG_DEEP_BLUE    "\x1b[44m"
#define ANSI_BG_SAND         "\x1b[103m"
#define ANSI_BG_FOREST_GREEN "\x1b[42m"
#define ANSI_BG_JUNGLE_GREEN "\x1b[102m"
#define ANSI_BG_GRASS        "\x1b[42m"
#define ANSI_BG_SAVANNAH     "\x1b[43m"
#define ANSI_BG_SHRUBLAND    "\x1b[103m"
#define ANSI_BG_TAIGA        "\x1b[46m"
#define ANSI_BG_TUNDRA       "\x1b[106m"
#define ANSI_BG_SNOW         "\x1b[107m"
#define ANSI_BG_SCORCHED     "\x1b[41m"
#define ANSI_BG_BARE_ROCK    "\x1b[100m"
#define ANSI_BG_DESERT_SAND  "\x1b[43m"

#define ANSI_RESET           "\x1b[0m"


#define ELEV_OCEAN           0.15
#define ELEV_BEACH           0.18
#define ELEV_TROPICAL_MAX    0.40
#define ELEV_TEMPERATE_MAX   0.65
#define ELEV_BOREAL_MAX      0.85


#define MOIST_DESERT         0.20
#define MOIST_GRASS_SAVANNAH 0.40
#define MOIST_WOODLAND_SHRUB 0.70


void print_map_text(const MapData* map) {
    if (!map || !map->elevation || !map->moisture) {
        printf("Cannot print NULL or incomplete map.\n");
        return;
    }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            double e = map->elevation[y][x];
            double m = map->moisture[y][x];

            const char* bg_color_code;

            if (e < ELEV_OCEAN) {
                bg_color_code = ANSI_BG_DEEP_BLUE;
            } else if (e < ELEV_BEACH) {
                bg_color_code = ANSI_BG_SAND;
            }
            else if (e > ELEV_BOREAL_MAX) {
                if (m < 0.1) bg_color_code = ANSI_BG_SCORCHED; // Keep original finer moisture bands for arctic? Or use new ones? Let's use new ones for consistency
                else if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SCORCHED;      // Scorched (<0.2)
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_BARE_ROCK;// Bare (0.2-0.4)
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TUNDRA;   // Tundra (0.4-0.7)
                else bg_color_code = ANSI_BG_SNOW;                  // Snow (>0.7)
            }
            else if (e > ELEV_TEMPERATE_MAX) { // Boreal (0.65 - 0.85)
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; // Temperate Desert
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_SHRUBLAND; // Shrubland (maybe use SAVANNAH color?)
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TAIGA; // Taiga Forest (sparse)
                else bg_color_code = ANSI_BG_TAIGA; // Taiga Forest (dense) - Using same color
            }
            else if (e > ELEV_TROPICAL_MAX) { // Temperate (0.40 - 0.65)
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; // Temperate Desert
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; // Grassland
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_FOREST_GREEN; // Deciduous Forest
                else bg_color_code = ANSI_BG_FOREST_GREEN; // Temperate Rainforest - Using same color
            }
            else { // Tropical (<= 0.40)
                if (m < MOIST_DESERT) bg_color_code = ANSI_BG_DESERT_SAND; // Subtropical Desert
                else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; // Grassland (Tropical version?) - Or maybe SAVANNAH color?
                else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_JUNGLE_GREEN; // Seasonal Forest
                else bg_color_code = ANSI_BG_JUNGLE_GREEN; // Rainforest - Using same color
            }

            printf("%s %s", bg_color_code, ANSI_RESET);

        }
        putchar('\n');
    }
    printf("--------------------\n");
}
