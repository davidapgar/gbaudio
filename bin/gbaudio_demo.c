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
#include <fcntl.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_ttf.h>

#include <gbaudio/freq_gen.h>
#include <gbaudio/freq_mod.h>
#include <gbaudio/gbaudio_channel.h>
#include <gbaudio/gbaudio_mixer.h>
#include <gbaudio/gbaudio_to_gen.h>
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
static int raw_file = 0;

void audio_callback(void *userdata, Uint8* stream, int len)
{
    audio_gen_t **audio_gen_ref = (audio_gen_t **)userdata;
    audio_gen_t *audio_gen = *audio_gen_ref;

    // Silence the base stream.
    SDL_memset(stream, 0, len);
    if (!audio_gen) {
        return;
    }

    if (len > abuf_len) {
        printf("stream buffer too large\n");
    }

    for (int i = 0; i < len/2; ++i) {
        abuf[i] = audio_gen_next(audio_gen, frequency);
    }

    //SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, SDL_MIX_MAXVOLUME / 4);
//    SDL_MixAudioFormat(stream, abuf, AUDIO_S8, len, 32);
    memcpy(stream, abuf, len);
    if (raw_file != 0) {
        size_t written = 0;
        while (written < len) {
            ssize_t result = write(raw_file, abuf, len);
            if (result < 0) {
                close(raw_file);
                raw_file = 0;
                break;
            }
            written += result;
        }
    }
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

void main_loop(SDL_AudioDeviceID dev,
    audio_gen_t **audio_gen,
    SDL_Renderer *renderer,
    audioview_t *audioview,
    lineview_t *lineview,
    TTF_Font *font,
    SDL_Color textcolor
)
{
    freq_gen_init(&gen_real, amplitude, note_freq, duty_50);
    audio_gen_t freq_audio = freq_to_audio_gen(&gen_real);
    lfsr_gen_init(&lfsr_real, amplitude, false, 8);
    audio_gen_t lfsr_audio = lfsr_to_audio_gen(&lfsr_real);

    sweep_gen_t sweep_gen;
    sweep_gen_init(&sweep_gen, &freq_audio, true, 2, 7, 4);
    audio_gen_t sweep_audio = sweep_to_audio_gen(&sweep_gen);

    *audio_gen = &freq_audio;

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

    *audio_gen = &saw_audio;

    freq_mod_t freq_mod;
    //freq_mod_init(&freq_mod, &carrier_a, &modulator_a);
    freq_mod_init(&freq_mod, &saw_audio, &saw_audio2);
    audio_gen_t freq_mod_audio = freq_mod_to_audio_gen(&freq_mod);

    *audio_gen = &freq_mod_audio;

    gbaudio_channel_t channel1;
    gbaudio_channel_init(&channel1);
    gbaudio_channel_freq(&channel1, 440);
    gbaudio_channel_volume_envelope(&channel1, 0x0f, true, 0);
    gbaudio_channel_set_amplitude(&channel1, 200);
    gbaudio_channel_length_duty(&channel1, 0, wave_duty_50);
    gbaudio_channel_sweep(&channel1, 1, true, 7);
    gbaudio_channel_trigger(&channel1, true, false);

    audio_gen_t channel1_a = channel_to_audio_gen(&channel1);
    (void)channel1_a;

    *audio_gen = &channel1_a;

    gbaudio_mixer_t mixer;
    gbaudio_mixer_init(&mixer);
    gbaudio_mixer_set_output(&mixer, output_terminal_both, output_terminal_both);
    gbaudio_mixer_set_volume(&mixer, 0x0f, 0x0f);
    gbaudio_mixer_enable(&mixer, true);

    gbaudio_channel_freq(&mixer.ch1, 440);
    gbaudio_channel_volume_envelope(&mixer.ch1, 0x0f, true, 0);
    gbaudio_channel_length_duty(&mixer.ch1, 0, wave_duty_50);
    gbaudio_channel_trigger(&mixer.ch1, true, false);

    gbaudio_channel_freq(&mixer.ch2, 260);
    gbaudio_channel_volume_envelope(&mixer.ch2, 0x0f, true, 0);
    gbaudio_channel_length_duty(&mixer.ch2, 0, wave_duty_50);
    gbaudio_channel_trigger(&mixer.ch2, true, false);

    audio_gen_t mixer_a = mixer_to_audio_gen(&mixer, 200);
    (void)mixer_a;

    *audio_gen = &mixer_a;
    *audio_gen = &freq_audio;

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
                    if (*audio_gen == &freq_audio) {
                        *audio_gen = &lfsr_audio;
                    } else if (*audio_gen == &lfsr_audio) {
                        *audio_gen = &sweep_audio;
                        sweep_gen_reset(&sweep_gen);
                    } else if (*audio_gen == &sweep_audio) {
                        *audio_gen = &freq_mod_audio;
                    } else {
                        *audio_gen = &freq_audio;
                    }
                    break;
                case 'u':
                case 'd':
                case 'l':
                case 'r':
                case 'w':
                    if (*audio_gen == &freq_audio) {
                        adjust_freq_gen(&gen_real, key);
                    } else if (*audio_gen == &lfsr_audio) {
                        adjust_lfsr_gen(&lfsr_real, key);
                    } else if (*audio_gen == &sweep_audio) {
                        adjust_audio_gen(*audio_gen, key);
                        if (key == 'w') {
                            sweep_gen.change = !sweep_gen.change;
                        }
                    } else {
                        adjust_audio_gen(*audio_gen, key);
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
        lineview_display(lineview, renderer, font, textcolor);

        // Present
        SDL_RenderPresent(renderer);

        Uint32 cur = SDL_GetTicks();
        if (cur - last < 16) {
            SDL_Delay(cur-last);
        }
    }
}

typedef struct {
    uint32_t tick;
    uint16_t addr;
    uint8_t val;
} replay_log_t;

typedef enum {
    apu_reg_nr10 = 0xFF10,
    apu_reg_nr11 = 0xFF11,
    apu_reg_nr12 = 0xFF12,
    apu_reg_nr13 = 0xFF13,
    apu_reg_nr14 = 0xFF14,

    apu_reg_nr21 = 0xFF16,
    apu_reg_nr22 = 0xFF17,
    apu_reg_nr23 = 0xFF18,
    apu_reg_nr24 = 0xFF19,

    apu_reg_nr30 = 0xFF1A,
    apu_reg_nr31 = 0xFF1B,
    apu_reg_nr32 = 0xFF1C,
    apu_reg_nr33 = 0xFF1D,
    apu_reg_nr34 = 0xFF1E,

    apu_reg_nr41 = 0xFF20,
    apu_reg_nr42 = 0xFF21,
    apu_reg_nr43 = 0xFF22,
    apu_reg_nr44 = 0xFF23,

    apu_reg_nr50 = 0xFF24,
    apu_reg_nr51 = 0xFF25,
    apu_reg_nr52 = 0xFF26,

    apu_reg_wave_start = 0xFF30,
    apu_reg_wave_end = 0xFF3F,
} apu_reg;

void apu_reg_write(gbaudio_mixer_t *mixer, uint16_t reg, uint8_t value)
{
    switch (reg) {
    case apu_reg_nr10: {
        uint8_t time = (value >> 4) & 0x07;
        bool addition = (value & 0x08) == 0x08 ? false : true;
        uint8_t shift = (value >> 0) & 0x07;
        gbaudio_channel_sweep(&mixer->ch1, time, addition, shift);
        break;
        }
    case apu_reg_nr11: {
        uint8_t length = (value >> 0) & 0x3F;
        wave_duty_t duty = (value >> 6) & 0x03;
        gbaudio_channel_length_duty(&mixer->ch1, length, duty);
        break;
        }
    case apu_reg_nr12: {
        uint8_t initial = (value >> 4) & 0x0F;
        bool increase = (value & 0x08) == 0x08 ? true : false;
        uint8_t n_envelope = (value >> 0) & 0x07;
        gbaudio_channel_volume_envelope(&mixer->ch1, initial, increase, n_envelope);
        break;
        }
    case apu_reg_nr13: {
        gbaudio_channel_gbfreq_low(&mixer->ch1, value);
        break;
        }
    case apu_reg_nr14: {
        bool trigger = (value & 0x80) == 0x80 ? true : false;
        bool single = (value & 0x40) == 0x40 ? true : false;
        uint8_t freq_high = (value >> 0) & 0x07;
        gbaudio_channel_trigger_freq_high(&mixer->ch1, trigger, single, freq_high);
        break;
        }

    case apu_reg_nr21: {
        uint8_t length = (value >> 0) & 0x3F;
        wave_duty_t duty = (value >> 6) & 0x03;
        gbaudio_channel_length_duty(&mixer->ch2, length, duty);
        break;
        }
    case apu_reg_nr22: {
        uint8_t initial = (value >> 4) & 0x0F;
        bool increase = (value & 0x08) == 0x08 ? true : false;
        uint8_t n_envelope = (value >> 0) & 0x07;
        gbaudio_channel_volume_envelope(&mixer->ch2, initial, increase, n_envelope);
        break;
        }
    case apu_reg_nr23: {
        gbaudio_channel_gbfreq_low(&mixer->ch2, value);
        break;
        }
    case apu_reg_nr24: {
        bool trigger = (value & 0x80) == 0x80 ? true : false;
        bool single = (value & 0x40) == 0x40 ? true : false;
        uint8_t freq_high = (value >> 0) & 0x07;
        gbaudio_channel_trigger_freq_high(&mixer->ch2, trigger, single, freq_high);
        break;
        }

    case apu_reg_nr41:
        break;
    case apu_reg_nr42:
        break;
    case apu_reg_nr43:
        break;
    case apu_reg_nr44:
        break;

    case apu_reg_nr50: {
        uint8_t right = (value >> 4) & 0x07;
        uint8_t left = (value >> 0) & 0x07;
        gbaudio_mixer_set_volume(mixer, right, left);
        break;
        }
    case apu_reg_nr51: {
        output_terminal_t ch1 =
            ((value >> 0) & 0x01) | ((value >> 3) & 0x02);
        output_terminal_t ch2 =
            ((value >> 1) & 0x01) | ((value >> 4) & 0x02);
        gbaudio_mixer_set_output(mixer, ch1, ch2);
        break;
        }
    case apu_reg_nr52: {
        bool enable = (value & 0x80) == 0x80 ? true : false;
        gbaudio_mixer_enable(mixer, enable);
        break;
        }

    case apu_reg_nr30:
    case apu_reg_nr31:
    case apu_reg_nr32:
    case apu_reg_nr33:
    case apu_reg_nr34:

    default:
        break;
    }
}

bool load_replay(char const *fname, replay_log_t *replay, size_t *len)
{
    size_t idx = 0;
    FILE *fp = fopen(fname, "r");
    if (!fp) {
        return false;
    }
    while (idx < *len) {
        unsigned int tick;
        unsigned int addr;
        unsigned int val;
        if (fscanf(fp, "%x %x %x ", &tick, &addr, &val) < 3) {
            break;
        }
        replay[idx] = (replay_log_t){
            .tick = tick,
            .addr = addr,
            .val = val,
        };
        ++idx;
    }
    fclose(fp);
    *len = idx;
    return true;
}

void replay_loop(SDL_AudioDeviceID dev,
    audio_gen_t **audio_gen,
    SDL_Renderer *renderer,
    audioview_t *audioview,
    lineview_t *lineview,
    TTF_Font *font,
    SDL_Color textcolor,
    replay_log_t *replay_log,
    size_t len
)
{
    gbaudio_mixer_t mixer;
    gbaudio_mixer_init(&mixer);
    audio_gen_t mixer_audio = mixer_to_audio_gen(&mixer, 200);
    *audio_gen = &mixer_audio;

    //raw_file = open("audio.raw", O_WRONLY|O_CREAT|O_TRUNC, 0644);

    size_t idx = 0;

    Uint32 last = SDL_GetTicks();
    SDL_Event event;
    SDL_PauseAudioDevice(dev, 0);

    // 67k ticks per frame (~16ms)
    size_t frames = 0;
    while (idx < len) {
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
                case 'u':
                case 'd':
                case 'l':
                case 'r':
                case 'w':
                    adjust_audio_gen(*audio_gen, key);
                    break;
                }
            }
        }
        if (quit) {
            break;
        }

        do {
            replay_log_t log = replay_log[idx];
            // Divide by 64k to approximate ticks in frames
            size_t frame_ticks = log.tick >> 16;
            if (frame_ticks <= frames) {
                apu_reg_write(&mixer, log.addr, log.val);
                frames = 0;
                ++idx;
            } else {
                break;
            }
        } while (idx < len);
        ++frames;

        SDL_SetRenderDrawColor(renderer, 0xCA, 0xDC, 0x9F, 0xFF);
        SDL_RenderClear(renderer);

        SDL_LockAudioDevice(dev);
        draw_audio(audioview->texture, abuf, abuf_len);
        SDL_UnlockAudioDevice(dev);
        audioview_display(audioview, renderer);
        lineview_display(lineview, renderer, font, textcolor);

        // Present
        SDL_RenderPresent(renderer);

        Uint32 cur = SDL_GetTicks();
        if (cur - last < 16) {
            SDL_Delay(cur-last);
        }
        last = cur;
    }
    close(raw_file);
}

static size_t const replay_log_size = 1<<20;
static replay_log_t replay_log[replay_log_size];

int const width = 1024;
int const height = 144 + 16;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Usage: %s <font> <replay log?>\n", argv[0]);
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

    audio_gen_t *audio_gen = NULL;

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

    if (argc == 3) {
        do {
            size_t len = replay_log_size;
            if (!load_replay(argv[2], replay_log, &len)) {
                printf("Error loading replay log %s\n", argv[2]);
                break;
            }

            printf("Loaded %lu log entries\n", len);
            replay_loop(dev,
                &audio_gen,
                renderer,
                audioview,
                &lineview,
                font,
                textcolor,
                replay_log,
                len
            );
        } while (0);
    } else {
        main_loop(dev,
            &audio_gen,
            renderer,
            audioview,
            &lineview,
            font,
            textcolor
            );
    }

    SDL_CloseAudioDevice(dev);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
