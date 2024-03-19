// Copyright 2021 Jonne Kokkonen
// Released under the MIT licence, https://opensource.org/licenses/MIT

/* Uncomment this line to enable debug messages or call make with `make
   CFLAGS=-DDEBUG_MSG` */

#include <SDL.h>
#include <signal.h>
#include <libusb.h>

#include "SDL2_inprint.h"
#include "audio.h"
#include "command.h"
#include "config.h"
#include "input.h"
#include "render.h"
#include "serial.h"
#include "slip.h"
#include "SDL2_compat.h"

#define SDL_zero(x) SDL_memset(&(x), 0, sizeof((x)))

enum state {
    QUIT, WAIT_FOR_DEVICE, RUN
};

enum state run = WAIT_FOR_DEVICE;
uint8_t need_display_reset = 0;

static slip_handler_s slip;
static uint16_t zerobyte_packets = 0; // used to detect device disconnection
static uint8_t *serial_buf = 0;
static int port_inited = 0;
static config_params_s conf;

const int command_size = 4096;
int writeCursor = 0;
int readCursor = 0;
uint8_t **command;
uint32_t *command_sizes;

void close_serial_port() { disconnect(); }

int pullCommand(uint8_t *data, uint32_t size) {

    uint8_t *recv_buf = malloc(size * sizeof(uint8_t));
    memcpy(recv_buf, data, size);
    free(command[writeCursor % command_size]);
    command[writeCursor % command_size] = recv_buf;

    command_sizes[writeCursor % command_size] = size;
    writeCursor++;
    return 1;
}

int popCommand(uint8_t **data, uint32_t *size) {
    int compare = writeCursor - readCursor;
    if (compare == 0) return 0;
    *data = command[readCursor % command_size];
    *size = command_sizes[readCursor % command_size];
    readCursor++;
    return compare;
}

void callback(struct libusb_transfer *xfr) {

    if (xfr->status != LIBUSB_TRANSFER_COMPLETED) {
        if (libusb_submit_transfer(xfr) < 0) {
            SDL_Log("error re-submitting URB\n");
        }
        return;
    }

    int bytes_read = xfr->actual_length;
    if (bytes_read < 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Error %d reading serial. \n",
                        (int) bytes_read);
        run = QUIT;
    } else if (bytes_read > 0) {
        zerobyte_packets = 0;
        serial_buf = xfr->buffer;
        uint8_t *cur = serial_buf;
        const uint8_t *end = serial_buf + bytes_read;
        while (cur < end) {
            // process the incoming bytes into commands and draw them
            int n = slip_read_byte(&slip, *(cur++));
            if (n != SLIP_NO_ERROR) {
                if (n == SLIP_ERROR_INVALID_PACKET) {
                    need_display_reset = 1;
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SLIP error %d\n", n);
                }
            }
        }
    } else {
        // zero byte packet, increment counter
        zerobyte_packets++;
        if (zerobyte_packets > conf.wait_packets) {
            zerobyte_packets = 0;

            // try opening the serial port to check if it's alive
            if (!check_serial_port()) {
                port_inited = 0;
                run = WAIT_FOR_DEVICE;
                close_serial_port();
                if (conf.audio_enabled == 1) {
                    audio_destroy();
                }
                /* we'll make one more loop to see if the device is still there
                 * but just sending zero bytes. if it doesn't get detected when
                 * resetting the port, it will disconnect */
            }
        }
    }
    if (libusb_submit_transfer(xfr) < 0) {
        SDL_Log("error re-submitting URB\n");
//        SDL_free(xfr->buffer);
    }
}

// Handles CTRL+C / SIGINT
void intHandler(int dummy) { run = QUIT; }



int main(int argc, char *argv[]) {

    command = malloc(sizeof(uint8_t *) * command_size);
    command_sizes = malloc(sizeof(uint32_t *) * command_size);

    char *preferred_device = NULL;
    if (argc == 3 && strcmp(argv[1], "--dev") == 0) {
        preferred_device = argv[2];
        SDL_Log("Using preferred device %s.\n", preferred_device);
    }

    // Initialize the config to defaults read in the params from the
    // configfile if present
    conf = init_config();

    // TODO: take cli parameter to override default configfile location
    read_config(&conf);

    // allocate memory for serial buffer
    serial_buf = SDL_malloc(serial_read_size);

    static uint8_t slip_buffer[serial_read_size]; // SLIP command buffer

    SDL_zero(slip_buffer);

    // settings for the slip packet handler
    static const slip_descriptor_s slip_descriptor = {
            .buf = slip_buffer,
            .buf_size = sizeof(slip_buffer),
            .recv_message = pullCommand, // the function where complete slip
            // packets are processed further
    };

    uint8_t prev_input = 0;
    uint8_t prev_note = 0;

    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);
#ifdef SIGQUIT
    signal(SIGQUIT, intHandler);
#endif
    slip_init(&slip, &slip_descriptor);

    // First device detection to avoid SDL init if it isn't necessary. To be run
    // only if we shouldn't wait for M8 to be connected.
    if (conf.wait_for_device == 0) {
        if (init_serial(1, preferred_device) == 0) {
            SDL_free(serial_buf);
            return -1;
        }
    }

    // initialize all SDL systems
    if (initialize_sdl(conf.init_fullscreen, conf.init_use_gpu) == -1)
        run = QUIT;

    // main loop begin
    do {
        // try to init serial port
        port_inited = init_serial(1, preferred_device);
        // if port init was successful, try to enable and reset display
        if (port_inited == 1 && enable_and_reset_display(0) == 1) {
            // if audio routing is enabled, try to initialize audio devices
            if (conf.audio_enabled == 1) {
                audio_init(conf.audio_buffer_size, conf.audio_device_name);
                // if audio is enabled, reset the display for second time to avoid glitches
                reset_display();
            }
            run = RUN;
            async_read(serial_buf, serial_read_size, callback);
        } else {
            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
                            "Device not detected on begin loop.");
            if (conf.wait_for_device == 1) {
                run = WAIT_FOR_DEVICE;
            } else {
                run = QUIT;
            }
        }

        // wait until device is connected
        if (conf.wait_for_device == 1) {
            static uint32_t ticks_poll_device = 0;
            static uint32_t ticks_update_screen = 0;

            if (port_inited == 0) {
                screensaver_init();
            }

            while (run == WAIT_FOR_DEVICE) {
                // get current input
                input_msg_s input = get_input_msg(&conf);
                if (input.type == special && input.value == msg_quit) {
                    SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Input message QUIT.");
                    run = QUIT;
                }

                if (SDL_GetTicks() - ticks_update_screen > 16) {
                    ticks_update_screen = SDL_GetTicks();
                    screensaver_draw();
                    render_screen();
                }

                // Poll for M8 device every second
                if (port_inited == 0 && (SDL_GetTicks() - ticks_poll_device > 1000)) {
                    ticks_poll_device = SDL_GetTicks();
                    if (run == WAIT_FOR_DEVICE && init_serial(0, preferred_device) == 1) {

                        if (conf.audio_enabled == 1) {
                            if (audio_init(conf.audio_buffer_size, conf.audio_device_name) == 0) {
                                SDL_Log("Cannot initialize audio");
                                conf.audio_enabled = 0;
                            }
                        }

                        int result = enable_and_reset_display();
                        // Device was found; enable display and proceed to the main loop
                        if (result == 1) {
                            run = RUN;
                            port_inited = 1;
                            screensaver_destroy();
                            async_read(serial_buf, serial_read_size, callback);
                        } else {
                            SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Device not detected.");
                            run = QUIT;
                            screensaver_destroy();
                        }
                    }
                }
                SDL_Delay(conf.idle_ms);
            }
        } else {
            // classic startup behaviour, exit if device is not found
            if (port_inited == 0) {
                if (conf.audio_enabled == 1) {
                    audio_destroy();
                }
                close_renderer();
                kill_inline_font();
                SDL_free(serial_buf);
                SDL_Quit();
                return -1;
            }
        }

        // main loop
        while (run == RUN) {

            // get current inputs
            input_msg_s input = get_input_msg(&conf);
            switch (input.type) {
                case normal:
                    if (input.value != prev_input) {
                        prev_input = input.value;
                        send_msg_controller(input.value);
                    }
                    break;
                case keyjazz:
                    if (input.value != 0) {
                        if (input.eventType == SDL_KEYDOWN && input.value != prev_input) {
                            send_msg_keyjazz(input.value, input.value2);
                            prev_note = input.value;
                        } else if (input.eventType == SDL_KEYUP && input.value == prev_note) {
                            send_msg_keyjazz(0xFF, 0);
                        }
                    }
                    prev_input = input.value;
                    break;
                case special:
                    if (input.value != prev_input) {
                        prev_input = input.value;
                        switch (input.value) {
                            case msg_quit:
                                SDL_Log("Received msg_quit from input device.");
                                run = 0;
                                break;
                            case msg_reset_display:
                                reset_display();
                                break;
                            default:
                                break;
                        }
                        break;
                    }
            }


            uint8_t * com;
            uint32_t size;
            int draws = 0;
            while (popCommand(&com, &size) > 0) {
                process_command(com, size);
                draws++;
            }
            if (draws>0) {
                render_screen();
            }

            SDL_Delay(conf.idle_ms);
        }
    } while (run > QUIT);
    // main loop end

    // exit, clean up
    SDL_Log("Shutting down\n");
    if (conf.audio_enabled == 1) {
        audio_destroy();
    }
    close_renderer();
    close_serial_port();
    SDL_free(serial_buf);
    kill_inline_font();
    SDL_Quit();
    return 0;
}
