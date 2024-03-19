// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#ifndef CONFIG_H_
#define CONFIG_H_

#include "ini.h"

typedef struct config_params_s {
    char *filename;
    int init_fullscreen;
    int init_use_gpu;
    int idle_ms;
    int wait_for_device;
    int wait_packets;
    int audio_enabled;
    int audio_buffer_size;
    const char *audio_device_name;

    int key_up;
    int key_left;
    int key_down;
    int key_right;
    int key_select;
    int key_select_alt;
    int key_start;
    int key_start_alt;
    int key_opt;
    int key_opt_alt;
    int key_edit;
    int key_edit_alt;
    int key_delete;
    int key_reset;

} config_params_s;


config_params_s init_config();

void read_config(config_params_s *conf);

void read_audio_config(ini_t *config, config_params_s *conf);

void read_graphics_config(ini_t *config, config_params_s *conf);

void read_key_config(ini_t *config, config_params_s *conf);

#endif
