#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_ttf.h>


enum {
    linelen = 128,
};

typedef struct line_s {
    char str[linelen];
    SDL_Texture *texture;
    bool dirty;
} line_t;

typedef struct view_s {
    SDL_Rect frame;
} view_t;

typedef struct lineview_s {
    line_t line;
    view_t view;
} lineview_t;

typedef struct audioview_s {
    SDL_Texture *texture;
    view_t view;
} audioview_t;

void logSDLError(FILE* fileno, const char *message);

void draw_audio(SDL_Texture *texture, uint16_t *buf, int len);
void audioview_init(audioview_t *audioview, SDL_Renderer *renderer, int width, int height);
void audioview_display(audioview_t *audioview, SDL_Renderer *renderer);

void view_init(view_t *view, int width, int height);
void lineview_init(lineview_t *lineview, int width, int height);
void line_init(line_t *line);
void line_update(line_t *line, char *str);
// Returns the height of the line
int line_display(line_t *line, SDL_Renderer *renderer, TTF_Font* font, SDL_Color color, SDL_Rect const *rect);
int lineview_display(lineview_t *lineview, SDL_Renderer *renderer, TTF_Font *font, SDL_Color color);

#endif
