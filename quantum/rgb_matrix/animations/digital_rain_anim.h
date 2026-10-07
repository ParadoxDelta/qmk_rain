#if defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_DIGITAL_RAIN)
RGB_MATRIX_EFFECT(DIGITAL_RAIN)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#        ifndef RGB_DIGITAL_RAIN_DROPS
// lower the number for denser effect/wider keyboard
#            define RGB_DIGITAL_RAIN_DROPS 24
#        endif

uint8_t rain_rgb_frame_buffer[MATRIX_ROWS][MATRIX_COLS] = {{0}};

bool DIGITAL_RAIN(effect_params_t* params) {
    // algorithm ported from https://github.com/tremby/Kaleidoscope-LEDEffect-DigitalRain
    const uint8_t drop_ticks            = 28;
    const uint8_t pure_green_intensity = (((uint16_t)rgb_matrix_config.hsv.v) * 3) >> 2;
    const uint8_t max_brightness_boost = (((uint16_t)rgb_matrix_config.hsv.v) * 3) >> 2;
    uint8_t max_intensity         = rgb_matrix_config.hsv.v;
    const uint8_t decay_ticks           = 0xff / (max_intensity ? max_intensity : 1);

    static uint8_t drop  = 0;
    static uint8_t decay = 0;
    static bool render = true;

    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    if (params->init) {
        rgb_matrix_region_set_color_all(params->region, 0, 0, 0);
        memset(rain_rgb_frame_buffer, 0, sizeof(rain_rgb_frame_buffer));
        drop = 0;
    }

    if (params->iter == 0) {

        if (max_intensity != rgb_matrix_config.hsv.v) {
            // Check if value is decreased
            if (max_intensity > rgb_matrix_config.hsv.v) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                        rain_rgb_frame_buffer[row][col] = rain_rgb_frame_buffer[row][col] * (uint16_t)rgb_matrix_config.hsv.v / max_intensity;
                    }
                }
            }

            max_intensity = rgb_matrix_config.hsv.v;
        }

        if (render)
            decay++;

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                if (render) {
                    if (row == 0 && drop == 0 && rand() < RAND_MAX / RGB_DIGITAL_RAIN_DROPS) {
                        // top row, pixels have just fallen and we're
                        // making a new rain drop in this column
                        rain_rgb_frame_buffer[row][col] = max_intensity;
                    } else if (rain_rgb_frame_buffer[row][col] > 0 && rain_rgb_frame_buffer[row][col] < max_intensity) {
                        // SANFTES ABLEUCHTEN: Schnellerer Abbau des Helligkeitswerts für weiches Ausfaden
                        uint8_t fade_step = (max_intensity > 32) ? (max_intensity / 16) : 1;
                        if (rain_rgb_frame_buffer[row][col] >= fade_step) {
                            rain_rgb_frame_buffer[row][col] -= fade_step;
                        } else {
                            rain_rgb_frame_buffer[row][col] = 0;
                        }
                    }
                }
                // set the pixel colour
                uint8_t led[LED_HITS_TO_REMEMBER];
                uint8_t led_count = rgb_matrix_map_row_column_to_led(row, col, led);

                // TODO: multiple leds are supported mapped to the same row/column
                if (led_count > 0) {
                    // SANFTES AUF- UND ABLEUCHTEN: Stufenlose HSV-Skalierung der gewählten Farbe
                    uint8_t val = rain_rgb_frame_buffer[row][col];
                    
                    hsv_t hsv = rgb_matrix_config.hsv;
                    hsv.v     = scale8(val, rgb_matrix_config.hsv.v);
                    
                    rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
                    rgb_matrix_region_set_color(params->region, led[0], rgb.r, rgb.g, rgb.b);
                }
            }
        }

        if (render) {
            if (decay == decay_ticks) {
                decay = 0;
            }

            if (++drop > drop_ticks) {
                // reset drop timer
                drop = 0;
                for (uint8_t row = MATRIX_ROWS - 1; row > 0; row--) {
                    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                        // if ths is on the bottom row and bright allow decay
                        if (row == MATRIX_ROWS - 1 && rain_rgb_frame_buffer[row][col] == max_intensity) {
                            rain_rgb_frame_buffer[row][col]--;
                        }
                        // Original-Logik zur Tropfenwanderung 1:1 unverändert
                        if (rain_rgb_frame_buffer[row - 1][col] >= max_intensity) { // Note: can be larger than max_intensity if val was recently decreased
                            // allow old bright pixel to decay
                            rain_rgb_frame_buffer[row - 1][col] = max_intensity - 1;
                            // make this pixel bright
                            rain_rgb_frame_buffer[row][col] = max_intensity;
                        }
                    }
                }
            }
        }
        render = false;
    } else {
        render = true;
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // defined(RGB_MATRIX_FRAMEBUFFER_EFFECTS) && defined(ENABLE_RGB_MATRIX_DIGITAL_RAIN)
