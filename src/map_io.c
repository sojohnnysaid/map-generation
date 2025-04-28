#include "map_io.h"
#include <stdio.h>

// --- ANSI Color Codes (Backgrounds) ---
// Reference: https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit
#define ANSI_BG_BLACK   "\x1b[40m"
#define ANSI_BG_RED     "\x1b[41m"
#define ANSI_BG_GREEN   "\x1b[42m"
#define ANSI_BG_YELLOW  "\x1b[43m"
#define ANSI_BG_BLUE    "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN    "\x1b[46m"
#define ANSI_BG_WHITE   "\x1b[47m"
// Bright versions (optional)
#define ANSI_BG_BRIGHT_BLACK   "\x1b[100m" // Gray
#define ANSI_BG_BRIGHT_RED     "\x1b[101m"
#define ANSI_BG_BRIGHT_GREEN   "\x1b[102m"
#define ANSI_BG_BRIGHT_YELLOW  "\x1b[103m"
#define ANSI_BG_BRIGHT_BLUE    "\x1b[104m"
#define ANSI_BG_BRIGHT_MAGENTA "\x1b[105m"
#define ANSI_BG_BRIGHT_CYAN    "\x1b[106m"
#define ANSI_BG_BRIGHT_WHITE   "\x1b[107m"

#define ANSI_RESET      "\x1b[0m" // Resets all attributes
                                  //
#define BIOME_OCEAN       '~'
#define BIOME_BEACH       '.'
#define BIOME_GRASSLAND   ','
#define BIOME_DRY_GRASS   '"'
#define BIOME_FOREST      '%'
#define BIOME_ROCK        '^'
#define BIOME_SNOW        '#'

#define ELEV_OCEAN_MAX    0.10
#define ELEV_BEACH_MAX    0.15
#define ELEV_MOUNTAIN_MIN 0.80

#define MOIST_DRY_MAX     0.30
#define MOIST_FOREST_MIN  0.65

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

            const char* bg_color_code; // Store the ANSI code string

            // Determine biome and set the corresponding background color
            if (e < ELEV_OCEAN_MAX) {
                bg_color_code = ANSI_BG_BLUE;          // Ocean = Blue
            } else if (e < ELEV_BEACH_MAX) {
                bg_color_code = ANSI_BG_BRIGHT_YELLOW; // Beach = Light Yellow
            } else if (e > ELEV_MOUNTAIN_MIN) {
                if (m < MOIST_DRY_MAX) {
                    bg_color_code = ANSI_BG_BRIGHT_BLACK; // Rock = Gray
                } else {
                    bg_color_code = ANSI_BG_WHITE;        // Snow = White
                }
            } else { // Mid-elevation lands
                if (m < MOIST_DRY_MAX) {
                    bg_color_code = ANSI_BG_YELLOW;       // Dry Grass/Savannah = Dark Yellow/Ochre
                } else if (m < MOIST_FOREST_MIN) {
                    bg_color_code = ANSI_BG_GREEN;        // Grassland = Green
                } else {
                    bg_color_code = ANSI_BG_BRIGHT_GREEN; // Forest = Bright Green
                }
            }

            // Print the color, a space to show it, and reset
            printf("%s %s", bg_color_code, ANSI_RESET); // Print color, space, reset

            // Alternatively, use a block character for potentially better visuals:
            // printf("%s█%s", bg_color_code, ANSI_RESET); // Requires terminal support for block chars
            // Note: Using block char might double the perceived width in some terminals. Let's stick with space.

        }
        putchar('\n'); // Newline after each row (outside the color codes)
    }
    printf("--------------------\n");
}
