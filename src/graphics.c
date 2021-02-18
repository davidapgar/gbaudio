#include <gbaudio/graphics.h>

#include <stdio.h>
#include <string.h>


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

void audioview_init(audioview_t *audioview, SDL_Renderer *renderer, int width, int height)
{
    view_init(&audioview->view, width, height);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
    audioview->texture = texture;
}

void audioview_display(audioview_t *audioview, SDL_Renderer *renderer)
{
    SDL_RenderCopy(renderer, audioview->texture, NULL, &audioview->view.frame);
}

void view_init(view_t *view, int width, int height)
{
    SDL_Rect frame = {
        .x = 0,
        .y = 0,
        .w = width,
        .h = height,
    };
    view->frame = frame;
}

void lineview_init(lineview_t *lineview, int width, int height)
{
    view_init(&lineview->view, width, height);
    line_init(&lineview->line);
}

void line_init(line_t *line)
{
    memset(line, 0, sizeof(*line));
}

void line_update(line_t *line, char *str)
{
    strlcpy(line->str, str, linelen);
    line->dirty = true;
}

#define MIN(x, y) x < y ? x : y

int line_display(line_t *line, SDL_Renderer *renderer, TTF_Font* font, SDL_Color color, SDL_Rect const *rect)
{
    int w, h;

    if (line->texture == NULL && line->dirty == false) {
        return 0;
    }

    if (line->dirty) {
        line->dirty = false;

        if (line->texture) {
            SDL_DestroyTexture(line->texture);
            line->texture = NULL;
        }

        SDL_Surface *display_str = TTF_RenderText_Solid(
            font,
            line->str,
            color);
        if (!display_str) {
            fprintf(stderr, "display str %s\n", TTF_GetError());
            return 0;
        }
        line->texture = SDL_CreateTextureFromSurface(
            renderer,
            display_str);
        SDL_FreeSurface(display_str);
    }

    SDL_QueryTexture(line->texture, NULL, NULL, &w, &h);
    SDL_Rect dest = {
        .x = rect->x,
        .y = rect->y,
        .w = MIN(w, rect->w),
        .h = MIN(h, rect->h),
    };
    SDL_RenderCopy(renderer, line->texture, NULL, &dest);
    return MIN(h, rect->h);
}

int lineview_display(lineview_t *lineview, SDL_Renderer *renderer, TTF_Font *font, SDL_Color color)
{
    return line_display(
        &lineview->line,
        renderer,
        font,
        color,
        &lineview->view.frame);
}
