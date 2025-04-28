#include "map_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct { unsigned char r, g, b; } RGBColor;

const RGBColor COLOR_LAKE_WATER      = { 93, 173, 226};
const RGBColor COLOR_OCEAN           = { 68, 108, 179};
const RGBColor COLOR_BEACH           = {247, 220, 111};
const RGBColor COLOR_FOREST_GREEN    = { 39, 174,  96};
const RGBColor COLOR_JUNGLE_GREEN    = { 46, 204, 113};
const RGBColor COLOR_GRASS           = { 88, 214, 141};
const RGBColor COLOR_SAVANNAH        = {212, 172,  13};
const RGBColor COLOR_SHRUBLAND       = {241, 196,  15};
const RGBColor COLOR_TAIGA           = { 93, 173, 226};
const RGBColor COLOR_TUNDRA          = {169, 204, 227};
const RGBColor COLOR_SNOW            = {236, 240, 241};
const RGBColor COLOR_SCORCHED        = {192,  57,  43};
const RGBColor COLOR_BARE_ROCK       = {149, 165, 166};
const RGBColor COLOR_DESERT_SAND     = {210, 180, 140};
const RGBColor COLOR_SHALLOW_OCEAN   = { 77, 136, 209}; // Defined but not used now

#define ELEV_OCEAN           0.15
#define ELEV_LAKE_MAX        0.18
#define ELEV_BEACH           0.18
#define ELEV_TROPICAL_MAX    0.40
#define ELEV_TEMPERATE_MAX   0.65
#define ELEV_BOREAL_MAX      0.85

#define MOIST_DESERT         0.20
#define MOIST_GRASS_SAVANNAH 0.40
#define MOIST_WOODLAND_SHRUB 0.70

#define ANSI_BG_LAKE_WATER   "\x1b[46m"
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
#define ANSI_BG_SHALLOW_OCEAN "\x1b[104m" // Defined but not used now
#define ANSI_RESET           "\x1b[0m"

static inline double clamp_io(double value, double min_val, double max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

void print_map_text(const MapData* map, double latitude_temp_factor) {
    if (!map || !map->elevation || !map->moisture) { return; }

    printf("--- Map (%dx%d) ---\n", map->width, map->height);
    int width = map->width;
    int height = map->height;

    for (int y = 0; y < height; y++) {
        double latitude_norm = (double)y / (height > 1 ? height - 1 : 1);
        double dist_from_equator = fabs(latitude_norm - 0.5) * 2.0;

        for (int x = 0; x < width; x++) {
            double e_orig = map->elevation[y][x];
            double m = map->moisture[y][x];
            double e_effective = clamp_io(e_orig + latitude_temp_factor * dist_from_equator, 0.0, 1.0);
            const char* bg_color_code;

            if (e_effective < ELEV_OCEAN) {
                bg_color_code = ANSI_BG_DEEP_BLUE;
            }
            else if (e_effective < ELEV_BEACH) {
                 bg_color_code = ANSI_BG_LAKE_WATER;
            }
            else if (e_effective > ELEV_BOREAL_MAX) { if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SCORCHED; else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_BARE_ROCK; else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TUNDRA; else bg_color_code = ANSI_BG_SNOW;}
            else if (e_effective > ELEV_TEMPERATE_MAX) { if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_SHRUBLAND; else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_TAIGA; else bg_color_code = ANSI_BG_TAIGA; }
            else if (e_effective > ELEV_TROPICAL_MAX) { if (m < MOIST_DESERT) bg_color_code = ANSI_BG_SAVANNAH; else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_FOREST_GREEN; else bg_color_code = ANSI_BG_FOREST_GREEN; }
            else { if (m < MOIST_DESERT) bg_color_code = ANSI_BG_DESERT_SAND; else if (m < MOIST_GRASS_SAVANNAH) bg_color_code = ANSI_BG_GRASS; else if (m < MOIST_WOODLAND_SHRUB) bg_color_code = ANSI_BG_JUNGLE_GREEN; else bg_color_code = ANSI_BG_JUNGLE_GREEN; }

            printf("%s %s", bg_color_code, ANSI_RESET);
         }
         putchar('\n');
     }
     printf("--------------------\n");
}


int write_map_png(const MapData* map, const char* filename, double latitude_temp_factor) {
     if (!map || !map->elevation || !map->moisture) { return 1; }
     if (!filename) { return 1; }

     int width = map->width;
     int height = map->height;
     int channels = 3;
     unsigned char *pixel_data = malloc(width * height * channels * sizeof(unsigned char));
     if (!pixel_data) { return 1; }

     printf("Preparing pixel data for PNG file: %s\n", filename);

     for (int y = 0; y < height; y++) {
         double latitude_norm = (double)y / (height > 1 ? height - 1 : 1);
         double dist_from_equator = fabs(latitude_norm - 0.5) * 2.0;

         for (int x = 0; x < width; x++) {
             double e_orig = map->elevation[y][x];
             double m = map->moisture[y][x];
             double e_effective = clamp_io(e_orig + latitude_temp_factor * dist_from_equator, 0.0, 1.0);
             RGBColor color;

             if (e_effective < ELEV_OCEAN) {
                 color = COLOR_OCEAN;
             }
             else if (e_effective < ELEV_BEACH) {
                 color = COLOR_LAKE_WATER;
             }
             else if (e_effective > ELEV_BOREAL_MAX) { if (m < MOIST_DESERT) color = COLOR_SCORCHED; else if (m < MOIST_GRASS_SAVANNAH) color = COLOR_BARE_ROCK; else if (m < MOIST_WOODLAND_SHRUB) color = COLOR_TUNDRA; else color = COLOR_SNOW;}
             else if (e_effective > ELEV_TEMPERATE_MAX) { if (m < MOIST_DESERT) color = COLOR_SAVANNAH; else if (m < MOIST_GRASS_SAVANNAH) color = COLOR_SHRUBLAND; else if (m < MOIST_WOODLAND_SHRUB) color = COLOR_TAIGA; else color = COLOR_TAIGA; }
             else if (e_effective > ELEV_TROPICAL_MAX) { if (m < MOIST_DESERT) color = COLOR_SAVANNAH; else if (m < MOIST_GRASS_SAVANNAH) color = COLOR_GRASS; else if (m < MOIST_WOODLAND_SHRUB) color = COLOR_FOREST_GREEN; else color = COLOR_FOREST_GREEN; }
             else { if (m < MOIST_DESERT) color = COLOR_DESERT_SAND; else if (m < MOIST_GRASS_SAVANNAH) color = COLOR_GRASS; else if (m < MOIST_WOODLAND_SHRUB) color = COLOR_JUNGLE_GREEN; else color = COLOR_JUNGLE_GREEN; }

             int index = (y * width + x) * channels;
             pixel_data[index + 0] = color.r;
             pixel_data[index + 1] = color.g;
             pixel_data[index + 2] = color.b;
         }
     }

     printf("Writing map to PNG file: %s\n", filename);
     int success = stbi_write_png(filename, width, height, channels, pixel_data, width * channels);
     free(pixel_data);

     if (success) { printf("PNG file write complete.\n"); return 0; }
     else { fprintf(stderr, "Error writing PNG file using stb_image_write.\n"); return 1; }
}
