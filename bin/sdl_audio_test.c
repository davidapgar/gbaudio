#if 0
# To build and run, execute: sh ./thisfile.c <args>)>
# For c++11 on macs
# clang++ -std=c++0x --stdlib=libc++ (source)
source=$0
output=${source%.*}

CFLAGS="-g"
LFLAGS="-lSDL2 -lSDL2_ttf"

gcc ${CFLAGS} ${LFLAGS} -o ${output} ${source} && ./${output} "$@"
exit
#endif

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_ttf.h>

#include <gbaudio/freq_gen.h>
#include <gbaudio/graphics.h>
#include <gbaudio/lfsr_gen.h>


/// Returns not zero if there was a key pressed in the alphabet.
char downkey(SDL_Event *event)
{
    switch (event->type) {
    case SDL_KEYDOWN:
        if (event->key.keysym.sym >= 'a' && event->key.keysym.sym <= 'z') {
            return (char)event->key.keysym.sym;
        }
    }
    return '\0';
}

static int const freq = 44100;

static int const amplitude = 72;
static int const note_freq = 440;

static freq_gen_t gen_real;
static lfsr_gen_t lfsr_real;

static int const abuf_len = 8192;
static uint16_t abuf[abuf_len];

void audio_callback(void *userdata, Uint8* stream, int len)
{
    freq_gen_t *gen = &gen_real;
    lfsr_gen_t *lfsr = &lfsr_real;

    // Silence the base stream.
    SDL_memset(stream, 0, len);
    if (len > abuf_len) {
        printf("stream buffer too large\n");
    }

    for (int i = 0; i < len/2; ++i) {
        if (true) {
            abuf[i] = freq_gen_next(gen, freq);
        } else {
            abuf[i] = lfsr_gen_next(lfsr, freq);
        }
    }

    //SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, SDL_MIX_MAXVOLUME / 4);
//    SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, 32);
    memcpy(stream, abuf, len);
}

void adjust_freq_gen(freq_gen_t *gen, char key)
{
    switch (key) {
    case 'u':
        freq_gen_adjust_amplitude(gen, 16);
        break;
    case 'd':
        freq_gen_adjust_amplitude(gen, -16);
        break;
    case 'l':
        freq_gen_adjust_frequency(gen, -10);
        break;
    case 'r':
        freq_gen_adjust_frequency(gen, 10);
        break;
    case 'w':
        freq_gen_cycle_duty(gen);
        break;
    default:
        return;
    }
    printf("Frequence Gen adjust: Freq %d Amp %d Duty %d\n",
        gen->frequency, gen->amplitude, gen->duty);
}

int const width = 1024;
int const height = 144;

int main(int argc, char* argv[])
{
    if (argc < 1) {
        printf("Usage: %s\n", argv[0]);
        return 1;
    }
    printf("Hello World\n");

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init");
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Audio Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_SHOWN);
    if (!win) {
        logSDLError(stderr, "Window create");
        SDL_Quit();
        return 2;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        logSDLError(stderr, "SDL_CreateRenderer");
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 3;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);

    // AUDIO setup
    SDL_AudioSpec desired = {
        .freq = freq,
        .format = AUDIO_U16SYS,
        .channels = 1,
        .silence = 0,
        .samples = 4096,
        .size = 0,
        .callback = audio_callback,
        .userdata = NULL,
    };
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

    freq_gen_init(&gen_real, amplitude, note_freq, duty_50);
    lfsr_gen_init(&lfsr_real, amplitude, false, 8);

    Uint32 last = SDL_GetTicks();

    SDL_Event event;
    while (true) {
        bool quit = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            char key = downkey(&event);
            if (key != '\0') {
                printf("Key pressed: %c\n", key);
                switch (key) {
                case 'q':
                    quit = true;
                    break;
                case 'c':
                    SDL_PauseAudioDevice(dev, 0);
                    break;
                case 's':
                    SDL_PauseAudioDevice(dev, 1);
                    break;
                case 'u':
                case 'd':
                case 'l':
                case 'r':
                case 'w':
                    adjust_freq_gen(&gen_real, key);
                    break;
                }
            }
        }
        if (quit) {
            break;
        }
        // White background
        //SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0);
        // Gameboy "white" background (screen off color)
        SDL_SetRenderDrawColor(renderer, 0xCA, 0xDC, 0x9F, 0xFF);
        SDL_RenderClear(renderer);

        draw_audio(texture, abuf, abuf_len);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Present
        SDL_RenderPresent(renderer);

        Uint32 cur = SDL_GetTicks();
        if (cur - last < 16) {
            SDL_Delay(cur-last);
        }
    }
    SDL_CloseAudioDevice(dev);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
