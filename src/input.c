// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

#include <SDL_events.h>

#include "config.h"
#include "input.h"
#include "render.h"

// Bits for M8 input messages
enum keycodes {
    key_left = 1 << 7,
    key_up = 1 << 6,
    key_down = 1 << 5,
    key_select = 1 << 4,
    key_start = 1 << 3,
    key_right = 1 << 2,
    key_opt = 1 << 1,
    key_edit = 1
};

uint8_t keyjazz_enabled = 0;
uint8_t keyjazz_base_octave = 2;
uint8_t keyjazz_velocity = 0x64;

static uint8_t keycode = 0; // value of the pressed key

input_msg_s key = {normal, 0};

uint8_t toggle_input_keyjazz() {
    keyjazz_enabled = !keyjazz_enabled;
    return keyjazz_enabled;
}

static input_msg_s handle_keyjazz(SDL_Event *event, uint8_t keyvalue) {
    input_msg_s key = {keyjazz, keyvalue, keyjazz_velocity, event->type};
    switch (event->key.keysym.sym) {
        case SDLK_z:
            key.value = keyjazz_base_octave * 12;
            break;
        case SDLK_s:
            key.value = 1 + keyjazz_base_octave * 12;
            break;
        case SDLK_x:
            key.value = 2 + keyjazz_base_octave * 12;
            break;
        case SDLK_d:
            key.value = 3 + keyjazz_base_octave * 12;
            break;
        case SDLK_c:
            key.value = 4 + keyjazz_base_octave * 12;
            break;
        case SDLK_v:
            key.value = 5 + keyjazz_base_octave * 12;
            break;
        case SDLK_g:
            key.value = 6 + keyjazz_base_octave * 12;
            break;
        case SDLK_b:
            key.value = 7 + keyjazz_base_octave * 12;
            break;
        case SDLK_h:
            key.value = 8 + keyjazz_base_octave * 12;
            break;
        case SDLK_n:
            key.value = 9 + keyjazz_base_octave * 12;
            break;
        case SDLK_j:
            key.value = 10 + keyjazz_base_octave * 12;
            break;
        case SDLK_m:
            key.value = 11 + keyjazz_base_octave * 12;
            break;
        case SDLK_q:
            key.value = 12 + keyjazz_base_octave * 12;
            break;
        case SDLK_2:
            key.value = 13 + keyjazz_base_octave * 12;
            break;
        case SDLK_w:
            key.value = 14 + keyjazz_base_octave * 12;
            break;
        case SDLK_3:
            key.value = 15 + keyjazz_base_octave * 12;
            break;
        case SDLK_e:
            key.value = 16 + keyjazz_base_octave * 12;
            break;
        case SDLK_r:
            key.value = 17 + keyjazz_base_octave * 12;
            break;
        case SDLK_5:
            key.value = 18 + keyjazz_base_octave * 12;
            break;
        case SDLK_t:
            key.value = 19 + keyjazz_base_octave * 12;
            break;
        case SDLK_6:
            key.value = 20 + keyjazz_base_octave * 12;
            break;
        case SDLK_y:
            key.value = 21 + keyjazz_base_octave * 12;
            break;
        case SDLK_7:
            key.value = 22 + keyjazz_base_octave * 12;
            break;
        case SDLK_u:
            key.value = 23 + keyjazz_base_octave * 12;
            break;
        case SDLK_i:
            key.value = 24 + keyjazz_base_octave * 12;
            break;
        case SDLK_9:
            key.value = 25 + keyjazz_base_octave * 12;
            break;
        case SDLK_o:
            key.value = 26 + keyjazz_base_octave * 12;
            break;
        case SDLK_0:
            key.value = 27 + keyjazz_base_octave * 12;
            break;
        case SDLK_p:
            key.value = 28 + keyjazz_base_octave * 12;
            break;
        case SDLK_KP_DIVIDE:
            key.type = normal;
            if (event->type == SDL_KEYDOWN && keyjazz_base_octave > 0) {
                keyjazz_base_octave--;
                display_keyjazz_overlay(1, keyjazz_base_octave, keyjazz_velocity);
            }
            break;
        case SDLK_KP_MULTIPLY:
            key.type = normal;
            if (event->type == SDL_KEYDOWN && keyjazz_base_octave < 8) {
                keyjazz_base_octave++;
                display_keyjazz_overlay(1, keyjazz_base_octave, keyjazz_velocity);
            }
            break;
        case SDLK_KP_MINUS:
            key.type = normal;
            if (event->type == SDL_KEYDOWN) {
                if ((event->key.keysym.mod & KMOD_ALT) > 0) {
                    if (keyjazz_velocity > 1)
                        keyjazz_velocity -= 1;
                } else {
                    if (keyjazz_velocity > 0x10)
                        keyjazz_velocity -= 0x10;
                }
                display_keyjazz_overlay(1, keyjazz_base_octave, keyjazz_velocity);
            }
            break;
        case SDLK_KP_PLUS:
            key.type = normal;
            if (event->type == SDL_KEYDOWN) {
                if ((event->key.keysym.mod & KMOD_ALT) > 0) {
                    if (keyjazz_velocity < 0x7F)
                        keyjazz_velocity += 1;
                } else {
                    if (keyjazz_velocity < 0x6F)
                        keyjazz_velocity += 0x10;
                }
                display_keyjazz_overlay(1, keyjazz_base_octave, keyjazz_velocity);
            }
            break;
        default:
            key.type = normal;
            break;
    }

    return key;
}

static input_msg_s handle_normal_keys(SDL_Event *event, config_params_s *conf,
                                      uint8_t keyvalue) {
    input_msg_s key = {normal, keyvalue};

    if (event->key.keysym.sym == conf->key_up) {
        key.value = key_up;
    } else if (event->key.keysym.sym == conf->key_left) {
        key.value = key_left;
    } else if (event->key.keysym.sym == conf->key_down) {
        key.value = key_down;
    } else if (event->key.keysym.sym == conf->key_right) {
        key.value = key_right;
    } else if (event->key.keysym.sym == conf->key_select ||
               event->key.keysym.sym == conf->key_select_alt) {
        key.value = key_select;
    } else if (event->key.keysym.sym == conf->key_start ||
               event->key.keysym.sym == conf->key_start_alt) {
        key.value = key_start;
    } else if (event->key.keysym.sym == conf->key_opt ||
               event->key.keysym.sym == conf->key_opt_alt) {
        key.value = key_opt;
    } else if (event->key.keysym.sym == conf->key_edit ||
               event->key.keysym.sym == conf->key_edit_alt) {
        key.value = key_edit;
    } else if (event->key.keysym.sym == conf->key_delete) {
        key.value = key_opt | key_edit;
    } else if (event->key.keysym.sym == conf->key_reset) {
        key = (input_msg_s) {special, msg_reset_display};
    } else {
        key.value = 0;
    }
    return key;
}

// Handles SDL input events
void handle_sdl_events(config_params_s *conf) {

    static int prev_key_analog = 0;

    SDL_Event event;
    key = (input_msg_s) {normal, 0};
    // Read joysticks
//    int key_analog = handle_game_controller_buttons(conf);
//    if (prev_key_analog != key_analog) {
//        keycode = key_analog;
//        prev_key_analog = key_analog;
//    }

//    // Read special case game controller buttons quit and reset
//    for (int gc = 0; gc < num_joysticks; gc++) {
//        if (SDL_JoystickGetButton(game_controllers[gc], conf->gamepad_quit) &&
//            (SDL_JoystickGetButton(game_controllers[gc], conf->gamepad_select) ||
//                    SDL_JoystickGetAxis(game_controllers[gc], conf->gamepad_analog_axis_select)))
//            key = (input_msg_s) {special, msg_quit};
//        else if (SDL_JoystickGetButton(game_controllers[gc], conf->gamepad_reset) &&
//                 (SDL_JoystickGetButton(game_controllers[gc], conf->gamepad_select) ||
//                         SDL_JoystickGetAxis(game_controllers[gc], conf->gamepad_analog_axis_select)))
//            key = (input_msg_s) {special, msg_reset_display};
//    }

    SDL_PollEvent(&event);


    switch (event.type) {

            // Handle SDL quit events (for example, window close)
        case SDL_QUIT:
            key = (input_msg_s) {special, msg_quit};
            break;

//        case SDL_WINDOWEVENT:
//            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
//                static uint32_t ticks_window_resized = 0;
//                if (SDL_GetTicks() - ticks_window_resized > 500) {
//                    SDL_Log("Resizing window...");
//                    key = (input_msg_s) {special, msg_reset_display};
//                    ticks_window_resized = SDL_GetTicks();
//                }
//            }
//            break;

            // Keyboard events. Special events are handled within SDL_KEYDOWN.
        case SDL_KEYDOWN:

            // ALT+ENTER toggles fullscreen
            if (event.key.keysym.sym == SDLK_RETURN &&
                (event.key.keysym.mod & KMOD_ALT) > 0) {
                toggle_fullscreen();
                break;
            }

            // ALT+F4 quits program
            if (event.key.keysym.sym == SDLK_F4 &&
                (event.key.keysym.mod & KMOD_ALT) > 0) {
                key = (input_msg_s) {special, msg_quit};
                break;
            }

            // ESC = toggle keyjazz
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                display_keyjazz_overlay(toggle_input_keyjazz(), keyjazz_base_octave, keyjazz_velocity);
            }

            // Normal keyboard inputs
        case SDL_KEYUP:
            key = handle_normal_keys(&event, conf, 0);

            if (keyjazz_enabled)
                key = handle_keyjazz(&event, key.value);
            break;

        default:
            break;
    }

    switch (key.type) {
        case normal:
            if (event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYAXISMOTION) {
                keycode |= key.value;
            } else {
                keycode &= ~key.value;
            }
            break;
        case keyjazz:
            // Do not allow pressing multiple keys with keyjazz
        case special:
            if (event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYAXISMOTION) {
                keycode = key.value;
            } else {
                keycode = 0;
            }
            break;
        default:
            break;
    }

}

// Returns the currently pressed keys to main
input_msg_s get_input_msg(config_params_s *conf) {

    key = (input_msg_s) {normal, 0};

    // Query for SDL events
    handle_sdl_events(conf);

    if (!keyjazz_enabled && keycode == (key_start | key_select | key_opt | key_edit)) {
        key = (input_msg_s) {special, msg_reset_display};
    }

    if (key.type == normal) {
        /* Normal input keys go through some event-based manipulation in
           handle_sdl_events(), the value is stored in keycode variable */
        return (input_msg_s) {key.type, keycode};
    } else {
        // Special event keys already have the correct keycode baked in
        return key;
    }
}
