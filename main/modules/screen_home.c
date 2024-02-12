#include "screen_home.h"
#include "application.h"
#include "badge-communication-protocol.h"
#include "badge-communication.h"
#include "bsp.h"
#include "coprocessor.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"
#include "pax_codecs.h"
#include "pax_gfx.h"
#include "screen_settings.h"
#include "screens.h"
#include "textedit.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

extern const uint8_t diamondl_png_start[] asm("_binary_diamondl_png_start");
extern const uint8_t diamondl_png_end[] asm("_binary_diamondl_png_end");
extern const uint8_t diamondr_png_start[] asm("_binary_diamondr_png_start");
extern const uint8_t diamondr_png_end[] asm("_binary_diamondr_png_end");
extern const uint8_t diamondt_png_start[] asm("_binary_diamondt_png_start");
extern const uint8_t diamondt_png_end[] asm("_binary_diamondt_png_end");
extern const uint8_t diamondb_png_start[] asm("_binary_diamondb_png_start");
extern const uint8_t diamondb_png_end[] asm("_binary_diamondb_png_end");

extern const uint8_t map_png_start[] asm("_binary_map_png_start");
extern const uint8_t map_png_end[] asm("_binary_map_png_end");

static const char* TAG = "homescreen";

// THIS LIST NEED TO MATCH SCREEN.H
const char screen_name[13][30] = {
    "Carondelet",
    "Engine room",
    "Repertoire",
    "Deibler",
    "Library",

    "Name tag",
    "Point & Click",
    "Billboard",
    "Memorium",
    "Lighthouse",


    "placeholder",
    "placeholder",
    "placeholder",
};

const int screen_pos[13][2] = {
    {21, 56},
    {67, 62},
    {134, 76},
    {164, 67},
    {194, 44},

    {194, 101},
    {202, 68},
    {205, 94},
    {229, 14},
    {266, 57},

    {50, 50},
    {50, 50},
    {50, 50},
};


static screen_t edit_nickname(QueueHandle_t application_event_queue, QueueHandle_t keyboard_event_queue) {
    char nickname[nicknamelength] = {0};
    nvs_get_str_wrapped("owner", "nickname", nickname, sizeof(nickname));
    bool res =
        textedit("What is your name?", application_event_queue, keyboard_event_queue, nickname, sizeof(nickname));
    if (res) {
        nvs_set_str_wrapped("owner", "nickname", nickname);
    }
    return screen_nametag;
}

void DisplayHomeEntry(int cursor);

screen_t screen_home_entry(QueueHandle_t application_event_queue, QueueHandle_t keyboard_event_queue) {
    ESP_LOGE(TAG, "Enter screen_home_entry");
    // update the keyboard event handler settings
    InitKeyboard(keyboard_event_queue);
    configure_keyboard_presses(keyboard_event_queue, false, false, true, false, true);
    configure_keyboard_rotate_both(keyboard_event_queue, SWITCH_1, true);

    int cursor = 5;
    DisplayHomeEntry(cursor);

    while (1) {
        event_t event = {0};
        if (xQueueReceive(application_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case event_input_button: break;  // Ignore raw button input
                case event_input_keyboard:
                    switch (event.args_input_keyboard.action) {
                        case ROTATION_L1: cursor = Decrement(cursor, Nb_screen); break;
                        case ROTATION_R1: cursor = Increment(cursor, Nb_screen); break;
                        case SWITCH_1: break;
                        case SWITCH_2: break;
                        case SWITCH_3: return screen_nametag; break;
                        case SWITCH_4: break;
                        case SWITCH_5:
                            switch (cursor) {
                                case 0: return screen_battleship; break;
                                case 1: return screen_settings; break;
                                case 2: return screen_repertoire; break;
                                case 3: return screen_hangman; break;
                                case 4: return screen_library; break;
                                case 5: return screen_nametag; break;
                                case 6: return screen_pointclick; break;
                                case 7: return screen_billboard; break;
                                case 8: return screen_credits; break;
                                case 9: return screen_scrambled; break;
                            }
                            break;
                        default: break;
                    }
                    ESP_LOGE(TAG, "Cursor %d", cursor);
                    DisplayHomeEntry(cursor);
                    break;
                default: ESP_LOGE(TAG, "Unhandled event type %u", event.type);
            }
        }
    }
}

void DisplayHomeEntry(int cursor) {
    pax_buf_t* gfx = bsp_get_gfx_buffer();
    pax_insert_png_buf(gfx, map_png_start, map_png_end - map_png_start, 0, 0, 0);
    AddOneTextSWtoBuffer(SWITCH_5, "Select");
    DrawArrowHorizontal(SWITCH_1);
    pax_draw_line(gfx, WHITE, screen_pos[cursor][0] + 1, 0, screen_pos[cursor][0] + 1, 100);
    pax_draw_line(gfx, WHITE, screen_pos[cursor][0] - 1, 0, screen_pos[cursor][0] - 1, 100);
    pax_draw_line(gfx, WHITE, 0, screen_pos[cursor][1] - 1, gfx->height, screen_pos[cursor][1] - 1);
    pax_draw_line(gfx, WHITE, 0, screen_pos[cursor][1] + 1, gfx->height, screen_pos[cursor][1] + 1);
    pax_draw_line(gfx, RED, screen_pos[cursor][0], 0, screen_pos[cursor][0], 100);
    pax_draw_line(gfx, RED, 0, screen_pos[cursor][1], gfx->height, screen_pos[cursor][1]);
    pax_center_text(gfx, BLACK, font1, fontsizeS * 2, gfx->height / 2, gfx->width - fontsizeS * 2, screen_name[cursor]);
    bsp_display_flush_with_lut(lut_4s);
}

void sendstuff() {
    badge_message_chat_t message = {
        .nickname = "H4CK3R",
        .payload  = "CRASH AND BURN :D\r\n",
    };
    badge_communication_send_chat_fuck(&message, 42);
    ESP_LOGW(TAG, "stuff.");
}

screen_t screen_Nametag(QueueHandle_t application_event_queue, QueueHandle_t keyboard_event_queue) {
    ESP_LOGE(TAG, "Enter screen_Nametag");
    // set screen font and buffer

    pax_buf_t* gfx = bsp_get_gfx_buffer();


    // memory
    nvs_handle_t handle;
    esp_err_t    res = nvs_open("owner", NVS_READWRITE, &handle);

    // read nickname from memory
    char   nickname[64] = {0};
    size_t size         = 0;
    res                 = nvs_get_str(handle, "nickname", NULL, &size);
    if ((res == ESP_OK) && (size <= sizeof(nickname) - 1)) {
        res = nvs_get_str(handle, "nickname", nickname, &size);
        if (res != ESP_OK || strlen(nickname) < 1) {
            sprintf(nickname, "No nickname configured");
        }
    }

    nvs_close(handle);

    // scale the nickame to fit the screen and display(I think)
    pax_vec1_t dims = {
        .x = 999,
        .y = 999,
    };
    float scale = 100;
    while (scale > 1) {
        dims = pax_text_size(font1, scale, nickname);
        if (dims.x <= 286 && dims.y <= 100)
            break;
        scale -= 0.2;
    }
    ESP_LOGW(TAG, "Scale: %f", scale);
    pax_background(gfx, WHITE);
    AddSWtoBufferLR("Map", "Rename");
    pax_center_text(gfx, RED, font1, scale, (dims.x) / 2, (100 - dims.y) / 2, nickname);
    bsp_display_flush_with_lut(lut_8s);

    InitKeyboard(keyboard_event_queue);
    configure_keyboard_presses(keyboard_event_queue, true, true, false, false, true);

    while (1) {
        bsp_set_addressable_led(0xFF0000);
        sendstuff();
        vTaskDelay(pdMS_TO_TICKS(500));
        bsp_set_addressable_led(0x000000);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    while (1) {
        event_t event = {0};
        if (xQueueReceive(application_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            switch (event.type) {
                case event_input_button: break;  // Ignore raw button input
                case event_input_keyboard:
                    switch (event.args_input_keyboard.action) {
                        case SWITCH_1: return screen_home; break;
                        case SWITCH_2: sendstuff(); break;
                        case SWITCH_5: return edit_nickname(application_event_queue, keyboard_event_queue); break;
                        default: break;
                    }
                    break;
                case event_communication:
                    switch (event.args_communication.type) {
                        case MESSAGE_TYPE_CHAT:
                            char str[256] = {0};
                            sprintf(
                                str,
                                "<%s> %s\nRSSI: %d",
                                event.args_communication.data_chat.nickname,
                                event.args_communication.data_chat.payload,
                                event.args_communication.info.rssi
                            );
                            pax_background(gfx, WHITE);
                            pax_center_text(gfx, RED, font1, scale, (dims.x) / 2, (100 - dims.y) / 2, nickname);
                            pax_draw_text(gfx, BLACK, pax_font_sky_mono, 12, 15, 90, str);
                            bsp_display_flush_with_lut(lut_1s);
                            break;
                        default: break;
                    }
                    break;
                default: ESP_LOGE(TAG, "Unhandled event type %u", event.type);
            }
        }
    }
}
