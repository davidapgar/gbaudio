#include <gbaudio/graphics.h>

#include <stdio.h>


void logSDLError(FILE* fileno, const char *message)
{
    fprintf(fileno, "%s Error: %s\n", message, SDL_GetError());
}

void draw_audio(SDL_Texture *texture, uint16_t *buf, int len)
{
    if (len > 1024) {
        len = 1024;
    }
    SDL_Color background = {
        .r = 0x55,
        .g = 0x55,
        .b = 0x55,
        .a = 0xff,
    };

    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    uint8_t *pixels = NULL;
    int pitch;
    int const bpp = 4;

    if (SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch)) {
        logSDLError(stderr, "Lock texture...");
        return;
    }

    if (pitch != width * bpp) {
        fprintf(stderr, "Unexpected pitch %d\n", pitch);
        SDL_UnlockTexture(texture);
        return;
    }

    // Set to single color
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            int idx = (h * pitch) + (w * bpp);
            SDL_Color *dest = (SDL_Color *)&pixels[idx];
            *dest = background;
        }
    }

    // Find min and max amplitude in the sample
    int16_t max = 0, min = 0;
    for (int i = 0; i < len; ++i) {
        int16_t sample = buf[i];
        if (sample > max) max = sample;
        if (sample < min) min = sample;
    }

    // set center line based on the ratio of min/max
    // (abs(min) + max) / height is the ratio of position
    int range = max + (-min);

    SDL_Color solid = {
        .r = 0x10,
        .g = 0x10,
        .b = 0x10,
        .a = 0xff,
    };
    int center = height / 2;
    for (int w = 0; w < len; ++w) {
        int idx = (center * pitch) + (w * bpp);
        SDL_Color *dest = (SDL_Color *)&pixels[idx];
        *dest = solid;

        if (!range) {
            continue;
        }
        int16_t sample = buf[w];
        float s = (float)sample / range;
        int h = (s * (center-2)) + center;
        idx = (h * pitch) + (w * bpp);
        dest = (SDL_Color *)&pixels[idx];
        *dest = solid;
    }

    SDL_UnlockTexture(texture);
}
