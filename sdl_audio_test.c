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
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>


void logSDLError(FILE* fileno, const char *message)
{
    fprintf(fileno, "%s Error: %s\n", message, SDL_GetError());
}

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

typedef struct freq_gen_s {
    int amplitude;
    int note_hz;
    int duty;
    int ticks;
    // Generate period during sound generation
} freq_gen_t;

int16_t gen_next(freq_gen_t *gen)
{
    int period = freq / gen->note_hz;
    int ticks = gen->ticks;
    if (ticks >= period) {
        ticks -= period;
    }
    int16_t ret;

    int duty = period / 8;
    switch (gen->duty) {
    case 0:
        // 12.5% down duty cycle
        break;
    case 1:
        // 25%
        duty = duty * 2;
        break;
    case 2:
        // 50%
        duty = duty * 4;
        break;
    case 3:
        // 75%
        duty = duty * 6;
        break;
    }
    if (ticks < duty) {
        ret = gen->amplitude * -1;
    } else {
        ret = gen->amplitude;
    }
    gen->ticks = (ticks + 1);
    return ret;
}

static int const amplitude = 720;
static int const note_freq = 440;
static int const sample_period = (48000/440); // 109

static freq_gen_t gen_real = {
    .amplitude = amplitude,
    .note_hz = 440,
    .duty = 2,
    .ticks = 0,
};

void audio_callback(void *userdata, Uint8* stream, int len)
{
    freq_gen_t *gen = &gen_real;

    int const abuf_len = 8192;
    static uint16_t abuf[8192];

    // Silence the base stream.
    SDL_memset(stream, 0, len);
    if (len > abuf_len) {
        printf("stream buffer too large\n");
    }

    for (int i = 0; i < len/2; ++i) {
        abuf[i] = gen_next(gen);
    }

    //SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, SDL_MIX_MAXVOLUME / 4);
//    SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, 32);
    memcpy(stream, abuf, len);
}

void adjust_volume(char key)
{
    int amplitude = gen_real.amplitude;
    switch (key) {
    case 'u':
        amplitude += 16;
        if (amplitude > 32768) {
            amplitude = 32768;
        }
        break;
    case 'd':
        amplitude -= 16;
        if (amplitude < 0) {
            amplitude = 0;
        }
        break;

    default:
        return;
    }
    gen_real.amplitude = amplitude;
    printf("Amplitude changed to %d\n", amplitude);
}

void adjust_freq(char key)
{
    int freq = gen_real.note_hz;
    switch (key) {
    case 'l':
        freq = freq - 10;
        if (freq < 220) {
            freq = 220;
        }
        break;
    case 'r':
        freq = freq + 10;
        if (freq > 880) {
            freq = 880;
        }
        break;
    default:
        return;
    }
    gen_real.note_hz = freq;
    printf("Frequence changed to %d\n", freq);
}

void adjust_duty()
{
    int duty = gen_real.duty;
    duty += 1;
    if (duty > 3) {
        duty = 0;
    }
    gen_real.duty = duty;
    printf("Duty adjusted to %d\n", duty);
}

int const width = 160;
int const height = 144;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <anything>\n", argv[0]);
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
                if (key == 'q') {
                    quit = true;
                } else if (key == 'c') {
                    SDL_PauseAudioDevice(dev, 0);
                } else if (key == 's') {
                    SDL_PauseAudioDevice(dev, 1);
                } else if (key == 'u' || key == 'd') {
                    adjust_volume(key);
                } else if (key == 'l' || key == 'r') {
                    adjust_freq(key);
                } else if (key == 'w') {
                    adjust_duty();
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

        // Present
        SDL_RenderPresent(renderer);
    }
shutdown_all:
    SDL_CloseAudioDevice(dev);

shutdown_renderer:
    SDL_DestroyRenderer(renderer);
shutdown_window:
    SDL_DestroyWindow(win);
shutdown_quit:
    SDL_Quit();

    return 0;
}
