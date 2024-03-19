// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT
#ifndef USE_LIBUSB

#include "audio.h"
#include "adapters/SDL.h"
#include <stdint.h>

static SDL_AudioDeviceID devid_in = 0;
static SDL_AudioDeviceID devid_out = 0;

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

void audio_cb_in(void *userdata, uint8_t *stream, int len) {
//    SDL_QueueAudio(devid_out, stream, len);

    /* Mix as much data as possible */
    len = (len > audio_len ? audio_len : len);
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;

}

void audio_cb_out(void *userdata, uint8_t *stream, int len) {
//    SDL_QueueAudio(devid_out, stream, len);

    /* Mix as much data as possible */
    len = (len > audio_len ? audio_len : len);
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;

}

int audio_init(int audio_buffer_size, const char *output_device_name, SDL_mutex *mutex) {

//    int i = 0;
//    int m8_device_id = -1;
//    int devcount_in = 0; // audio input device count

    // wait for system to initialize possible new audio devices
    SDL_Delay(500);

//    devcount_in = SDL_GetNumAudioDevices(SDL_TRUE);

//    if (devcount_in < 1) {
//        SDL_Log("No audio capture devices, SDL Error: %s", SDL_GetError());
//        return 0;
//    } else {
//        for (i = 0; i < devcount_in; i++) {
//            // Check if input device exists before doing anything else
//            SDL_LogDebug(SDL_LOG_CATEGORY_AUDIO, "%s", SDL_GetAudioDeviceName(i, SDL_TRUE));
//            if (SDL_strstr(SDL_GetAudioDeviceName(i, SDL_TRUE), "M8") != NULL) {
//                SDL_Log("M8 Audio Input device found: %s",
//                        SDL_GetAudioDeviceName(i, SDL_TRUE));
//                m8_device_id = i;
//            }
//        }
//        if (m8_device_id == -1) {
//            // forget about it
//            SDL_Log("Cannot find M8 audio input device");
//            return 0;
//        }

        SDL_AudioSpec want_in, have_in, want_out, have_out;

        // Open output device first to avoid possible Directsound errors
        SDL_zero(want_out);
        want_out.freq = 44100;
        want_out.format = AUDIO_S16;
        want_out.channels = 2;
        want_out.samples = audio_buffer_size;
        want_out.callback = audio_cb_out;
        devid_out = SDL_OpenAudio(&want_out, &have_out);
        if (devid_out == 0) {
            SDL_Log("Failed to open output: %s", SDL_GetError());
            return 0;
        }

//        SDL_zero(want_in);
//        want_in.freq = 44100;
//        want_in.format = AUDIO_S16;
//        want_in.channels = 2;
//        want_in.samples = audio_buffer_size;
//        want_in.callback = audio_cb_in;
//        devid_in = SDL_OpenAudio(&want_in, &have_in);
//        if (devid_in == 0) {
//            SDL_Log("Failed to open M8 audio device, SDL Error: %s", SDL_GetError());
//            return 0;
//        }
//    }

    // Start audio processing
    SDL_Log("Opening audio devices");
    SDL_PauseAudio(0);
//    SDL_PauseAudioDevice(devid_in, 0);
//    SDL_PauseAudioDevice(devid_out, 0);

    return 1;
}

void audio_destroy() {
    SDL_Log("Closing audio devices");
//    SDL_PauseAudioDevice(devid_in, 1);
//    SDL_PauseAudioDevice(devid_out, 1);
//    SDL_CloseAudioDevice(devid_in);
//    SDL_CloseAudioDevice(devid_out);
    SDL_CloseAudio();
}

#endif
