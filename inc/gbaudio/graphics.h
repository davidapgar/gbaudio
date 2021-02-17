#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <SDL2/SDL.h>


void logSDLError(FILE* fileno, const char *message);
void draw_audio(SDL_Texture *texture, uint16_t *buf, int len);

#endif
