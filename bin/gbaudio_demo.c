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
#include <gbaudio/freq_mod.h>
#include <gbaudio/graphics.h>
#include <gbaudio/lfsr_gen.h>
#include <gbaudio/saw_gen.h>
#include <gbaudio/sweep_gen.h>


/// Bottom of window display/log line.
static lineview_t lineview;

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

static int const frequency = 44100;

static int const amplitude = 72;
static int const note_freq = 440;

static freq_gen_t gen_real;
static lfsr_gen_t lfsr_real;

static int const abuf_len = 8192;
static uint16_t abuf[abuf_len];

void audio_callback(void *userdata, Uint8* stream, int len)
{
    audio_gen_t **audio_gen_ref = (audio_gen_t **)userdata;
    audio_gen_t *audio_gen = *audio_gen_ref;

    // Silence the base stream.
    SDL_memset(stream, 0, len);
    if (len > abuf_len) {
        printf("stream buffer too large\n");
    }

    for (int i = 0; i < len/2; ++i) {
        abuf[i] = audio_gen_next(audio_gen, frequency);
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
    char buf[128];
    snprintf(buf, 128, "Frequence Gen adjust: Freq %d Amp %d Duty %d\n",
        gen->frequency, gen->amplitude, gen->duty);
    line_update(&lineview.line, buf);
}

void adjust_lfsr_gen(lfsr_gen_t *lfsr, char key)
{
    switch (key) {
    case 'u':
        lfsr_gen_adjust_amplitude(lfsr, 16);
        break;
    case 'd':
        lfsr_gen_adjust_amplitude(lfsr, -16);
        break;
    case 'l':
        lfsr_gen_adjust_period(lfsr, -1);
        break;
    case 'r':
        lfsr_gen_adjust_period(lfsr, 1);
        break;
    case 'w':
        lfsr_gen_cycle_width(lfsr);
        break;
    default:
        return;
    }

    char buf[128];
    snprintf(buf, 128, "LFSR Gen adjust: Amp %d Period %d width %d\n",
        lfsr->amplitude, lfsr->update_period, lfsr->width);
    line_update(&lineview.line, buf);
}

void adjust_audio_gen(audio_gen_t *audio_gen, char key)
{
    switch (key) {
    case 'u':
        audio_gen_adjust_amplitude(audio_gen, 16);
        break;
    case 'd':
        audio_gen_adjust_amplitude(audio_gen, -16);
        break;
    case 'l':
        audio_gen_adjust_frequency(audio_gen, -10);
        break;
    case 'r':
        audio_gen_adjust_frequency(audio_gen, 10);
        break;
    default:
        return;
    }
    char buf[128];
    snprintf(buf, 128, "Audio Gen adjust: Freq %d Amp %d \n",
        audio_gen_get_frequency(audio_gen),
        audio_gen_get_amplitude(audio_gen));
    line_update(&lineview.line, buf);
}

int const width = 1024;
int const height = 144 + 16;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <font>\n", argv[0]);
        return 1;
    }

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

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
    }

    TTF_Font *font = TTF_OpenFont(argv[1], 16);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
    }

    audioview_t audioview_real;
    audioview_t *audioview = &audioview_real;
    audioview_init(audioview, renderer, width, 144);

    SDL_Color textcolor = {
        .r = 0x20,
        .g = 0x60,
        .b = 0x20,
        .a = 0xff,
    };
    lineview_init(&lineview, width, 16);
    lineview.view.frame.x = 0;
    lineview.view.frame.y = 144;


    freq_gen_init(&gen_real, amplitude, note_freq, duty_50);
    audio_gen_t freq_audio = freq_to_audio_gen(&gen_real);
    lfsr_gen_init(&lfsr_real, amplitude, false, 8);
    audio_gen_t lfsr_audio = lfsr_to_audio_gen(&lfsr_real);

    sweep_gen_t sweep_gen;
    sweep_gen_init(&sweep_gen, &freq_audio, true, 2, 7, 4);
    audio_gen_t sweep_audio = sweep_to_audio_gen(&sweep_gen);

    audio_gen_t *audio_gen = &freq_audio;

    freq_gen_t carrier;
    freq_gen_init(&carrier, amplitude, note_freq, duty_50);
    audio_gen_t carrier_a = freq_to_audio_gen(&carrier);
    (void)carrier_a;

    freq_gen_t modulator;
    freq_gen_init(&modulator, amplitude, 20, duty_50);
    audio_gen_t modulator_a = freq_to_audio_gen(&modulator);
    (void)modulator_a;

    saw_gen_t saw;
    saw_gen_init(&saw, amplitude, note_freq);
    audio_gen_t saw_audio = saw_to_audio_gen(&saw);

    saw_gen_t saw2;
    saw_gen_init(&saw2, amplitude, 20);
    audio_gen_t saw_audio2 = saw_to_audio_gen(&saw2);

    audio_gen = &saw_audio;

    freq_mod_t freq_mod;
    //freq_mod_init(&freq_mod, &carrier_a, &modulator_a);
    freq_mod_init(&freq_mod, &saw_audio, &saw_audio2);
    audio_gen_t freq_mod_audio = freq_mod_to_audio_gen(&freq_mod);

    audio_gen = &freq_mod_audio;

    // AUDIO setup
    SDL_AudioSpec desired = {
        .freq = frequency,
        .format = AUDIO_U16SYS,
        .channels = 1,
        .silence = 0,
        .samples = 4096,
        .size = 0,
        .callback = audio_callback,
        .userdata = &audio_gen,
    };
    SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

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
                case 'a':
                    if (audio_gen == &freq_audio) {
                        audio_gen = &lfsr_audio;
                    } else if (audio_gen == &lfsr_audio) {
                        audio_gen = &sweep_audio;
                        sweep_gen_reset(&sweep_gen);
                    } else if (audio_gen == &sweep_audio) {
                        audio_gen = &freq_mod_audio;
                    } else {
                        audio_gen = &freq_audio;
                    }
                    break;
                case 'u':
                case 'd':
                case 'l':
                case 'r':
                case 'w':
                    if (audio_gen == &freq_audio) {
                        adjust_freq_gen(&gen_real, key);
                    } else if (audio_gen == &lfsr_audio) {
                        adjust_lfsr_gen(&lfsr_real, key);
                    } else if (audio_gen == &sweep_audio) {
                        adjust_audio_gen(audio_gen, key);
                        if (key == 'w') {
                            sweep_gen.change = !sweep_gen.change;
                        }
                    } else {
                        adjust_audio_gen(audio_gen, key);
                    }
                    break;
                case 'p':
                    sweep_gen_reset(&sweep_gen);
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

        SDL_LockAudioDevice(dev);
        draw_audio(audioview->texture, abuf, abuf_len);
        SDL_UnlockAudioDevice(dev);
        audioview_display(audioview, renderer);
        lineview_display(&lineview, renderer, font, textcolor);

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
