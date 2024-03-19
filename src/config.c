// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#include "config.h"
#include "ini.h"
#include "SDL2_compat.h"
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <SDL_keysym.h>
#include <SDL_rwops.h>
#include <unistd.h>

/* Case insensitive string compare from ini.h library */
static int strcmpci(const char *a, const char *b) {
    for (;;) {
        int d = tolower(*a) - tolower(*b);
        if (d != 0 || !*a) {
            return d;
        }
        a++, b++;
    }
}

config_params_s init_config() {
    config_params_s c;

    c.filename = "config.ini"; // default config file to load

    c.init_fullscreen = 0; // default fullscreen state at load
    c.init_use_gpu = 1;    // default to use hardware acceleration
    c.idle_ms = 10;        // default to high performance
    c.wait_for_device = 1; // default to exit if device disconnected
    c.wait_packets = 1024; // default zero-byte attempts to disconnect (about 2
    // sec for default idle_ms)
    c.audio_enabled = 0;   // route M8 audio to default output
    c.audio_buffer_size = 1024; // requested audio buffer size in samples
    c.audio_device_name = NULL; // Use this device, leave NULL to use the default output device

    c.key_up = SDLK_w;
    c.key_left = SDLK_q;
    c.key_down = SDLK_s;
    c.key_right = SDLK_d;
    c.key_select = SDLK_n;
    c.key_select_alt = SDLK_n;
    c.key_start = SDLK_m;
    c.key_start_alt = SDLK_m;
    c.key_opt = SDLK_b;
    c.key_opt_alt = SDLK_y;
    c.key_edit = SDLK_a;
    c.key_edit_alt = SDLK_x;
    c.key_delete = SDLK_DELETE;
    c.key_reset = SDLK_u;

    return c;
}

// Write config to file
void write_config(config_params_s *conf) {

    // Open the default config file for writing
    char config_path[1024] = {0};
    getcwd(config_path,  sizeof(config_path));
    snprintf(config_path, sizeof(config_path), "%s%s", "m8c", conf->filename);
    SDL_RWops *rw = SDL_RWFromFile(config_path, "w");

    SDL_Log("Writing config file to %s", config_path);

    const unsigned int INI_LINE_COUNT = 25;
    const unsigned int LINELEN = 50;

    // Entries for the config file
    char ini_values[INI_LINE_COUNT][LINELEN];
    int initPointer = 0;
    snprintf(ini_values[initPointer++], LINELEN, "[graphics]\n");
    snprintf(ini_values[initPointer++], LINELEN, "fullscreen=%s\n",
             conf->init_fullscreen ? "true" : "false");
    snprintf(ini_values[initPointer++], LINELEN, "use_gpu=%s\n",
             conf->init_use_gpu ? "true" : "false");
    snprintf(ini_values[initPointer++], LINELEN, "idle_ms=%d\n", conf->idle_ms);
    snprintf(ini_values[initPointer++], LINELEN, "wait_for_device=%s\n",
             conf->wait_for_device ? "true" : "false");
    snprintf(ini_values[initPointer++], LINELEN, "wait_packets=%d\n",
             conf->wait_packets);
    snprintf(ini_values[initPointer++], LINELEN, "[audio]\n");
    snprintf(ini_values[initPointer++], LINELEN, "audio_enabled=%s\n",
             conf->audio_enabled ? "true" : "false");
    snprintf(ini_values[initPointer++], LINELEN, "audio_buffer_size=%d\n",
             conf->audio_buffer_size);
    snprintf(ini_values[initPointer++], LINELEN, "audio_device_name=%s\n",
             conf->audio_device_name ? conf->audio_device_name : "Default");
    snprintf(ini_values[initPointer++], LINELEN, "[keyboard]\n");
    snprintf(ini_values[initPointer++], LINELEN, "key_up=%d\n", conf->key_up);
    snprintf(ini_values[initPointer++], LINELEN, "key_left=%d\n", conf->key_left);
    snprintf(ini_values[initPointer++], LINELEN, "key_down=%d\n", conf->key_down);
    snprintf(ini_values[initPointer++], LINELEN, "key_right=%d\n",
             conf->key_right);
    snprintf(ini_values[initPointer++], LINELEN, "key_select=%d\n",
             conf->key_select);
    snprintf(ini_values[initPointer++], LINELEN, "key_select_alt=%d\n",
             conf->key_select_alt);
    snprintf(ini_values[initPointer++], LINELEN, "key_start=%d\n",
             conf->key_start);
    snprintf(ini_values[initPointer++], LINELEN, "key_start_alt=%d\n",
             conf->key_start_alt);
    snprintf(ini_values[initPointer++], LINELEN, "key_opt=%d\n", conf->key_opt);
    snprintf(ini_values[initPointer++], LINELEN, "key_opt_alt=%d\n",
             conf->key_opt_alt);
    snprintf(ini_values[initPointer++], LINELEN, "key_edit=%d\n", conf->key_edit);
    snprintf(ini_values[initPointer++], LINELEN, "key_edit_alt=%d\n",
             conf->key_edit_alt);
    snprintf(ini_values[initPointer++], LINELEN, "key_delete=%d\n",
             conf->key_delete);
    snprintf(ini_values[initPointer++], LINELEN, "key_reset=%d\n",
             conf->key_reset);

    // Ensure we aren't writing off the end of the array
    assert(initPointer == INI_LINE_COUNT);

    if (rw != NULL) {
        // Write ini_values array to config file
        for (int i = 0; i < INI_LINE_COUNT; i++) {
            size_t len = SDL_strlen(ini_values[i]);
            if (SDL_RWwrite(rw, ini_values[i], 1, len) != len) {
                SDL_LogDebug(SDL_LOG_CATEGORY_SYSTEM,
                             "Couldn't write line into config file.");
            } else {
                SDL_LogDebug(SDL_LOG_CATEGORY_SYSTEM, "Wrote to config: %s\n",
                             ini_values[i]);
            }
        }
        SDL_RWclose(rw);
    } else {
        SDL_Log("Couldn't write into config file.");
    }
}

// Read config
void read_config(config_params_s *conf) {

    char config_path[1024] = {0};
    getcwd(config_path,  sizeof(config_path));
    snprintf(config_path, sizeof(config_path), "%s%s", "m8c", conf->filename);

    SDL_Log("Reading config %s", config_path);
    ini_t *ini = ini_load(config_path);
    if (ini == NULL) {
        SDL_Log("Could not load config.");
        write_config(conf);
        return;
    }

    read_audio_config(ini, conf);
    read_graphics_config(ini, conf);
    read_key_config(ini, conf);

    // Frees the mem used for the config
    ini_free(ini);

    // Write any new default options after loading
    write_config(conf);
}

void read_audio_config(ini_t *ini, config_params_s *conf) {
    const char *param_audio_enabled = ini_get(ini, "audio", "audio_enabled");
    const char *param_audio_buffer_size =
            ini_get(ini, "audio", "audio_buffer_size");
    const char *param_audio_device_name =
            ini_get(ini, "audio", "audio_device_name");

    if (param_audio_enabled != NULL) {
        if (strcmpci(param_audio_enabled, "true") == 0) {
            conf->audio_enabled = 1;
        } else {
            conf->audio_enabled = 1;
        }
    }

    if (param_audio_device_name != NULL && SDL_strcmp(param_audio_device_name, "Default") != 0) {
        conf->audio_device_name = SDL_strdup(param_audio_device_name);
    }

    if (param_audio_buffer_size != NULL) {
        conf->audio_buffer_size = SDL_atoi(param_audio_buffer_size);
    }
}

void read_graphics_config(ini_t *ini, config_params_s *conf) {
    const char *param_fs = ini_get(ini, "graphics", "fullscreen");
    const char *param_gpu = ini_get(ini, "graphics", "use_gpu");
    const char *idle_ms = ini_get(ini, "graphics", "idle_ms");
    const char *param_wait = ini_get(ini, "graphics", "wait_for_device");
    const char *wait_packets = ini_get(ini, "graphics", "wait_packets");

    if (strcmpci(param_fs, "true") == 0) {
        conf->init_fullscreen = 1;
    } else
        conf->init_fullscreen = 0;

    if (param_gpu != NULL) {
        if (strcmpci(param_gpu, "true") == 0) {
            conf->init_use_gpu = 1;
        } else
            conf->init_use_gpu = 0;
    }

    if (idle_ms != NULL)
        conf->idle_ms = SDL_atoi(idle_ms);

    if (param_wait != NULL) {
        if (strcmpci(param_wait, "true") == 0) {
            conf->wait_for_device = 1;
        } else {
            conf->wait_for_device = 0;
        }
    }
    if (wait_packets != NULL)
        conf->wait_packets = SDL_atoi(wait_packets);
}

void read_key_config(ini_t *ini, config_params_s *conf) {
    // TODO: Some form of validation

    const char *key_up = ini_get(ini, "keyboard", "key_up");
    const char *key_left = ini_get(ini, "keyboard", "key_left");
    const char *key_down = ini_get(ini, "keyboard", "key_down");
    const char *key_right = ini_get(ini, "keyboard", "key_right");
    const char *key_select = ini_get(ini, "keyboard", "key_select");
    const char *key_select_alt = ini_get(ini, "keyboard", "key_select_alt");
    const char *key_start = ini_get(ini, "keyboard", "key_start");
    const char *key_start_alt = ini_get(ini, "keyboard", "key_start_alt");
    const char *key_opt = ini_get(ini, "keyboard", "key_opt");
    const char *key_opt_alt = ini_get(ini, "keyboard", "key_opt_alt");
    const char *key_edit = ini_get(ini, "keyboard", "key_edit");
    const char *key_edit_alt = ini_get(ini, "keyboard", "key_edit_alt");
    const char *key_delete = ini_get(ini, "keyboard", "key_delete");
    const char *key_reset = ini_get(ini, "keyboard", "key_reset");

    if (key_up)
        conf->key_up = SDL_atoi(key_up);
    if (key_left)
        conf->key_left = SDL_atoi(key_left);
    if (key_down)
        conf->key_down = SDL_atoi(key_down);
    if (key_right)
        conf->key_right = SDL_atoi(key_right);
    if (key_select)
        conf->key_select = SDL_atoi(key_select);
    if (key_select_alt)
        conf->key_select_alt = SDL_atoi(key_select_alt);
    if (key_start)
        conf->key_start = SDL_atoi(key_start);
    if (key_start_alt)
        conf->key_start_alt = SDL_atoi(key_start_alt);
    if (key_opt)
        conf->key_opt = SDL_atoi(key_opt);
    if (key_opt_alt)
        conf->key_opt_alt = SDL_atoi(key_opt_alt);
    if (key_edit)
        conf->key_edit = SDL_atoi(key_edit);
    if (key_edit_alt)
        conf->key_edit_alt = SDL_atoi(key_edit_alt);
    if (key_delete)
        conf->key_delete = SDL_atoi(key_delete);
    if (key_reset)
        conf->key_reset = SDL_atoi(key_reset);
}

