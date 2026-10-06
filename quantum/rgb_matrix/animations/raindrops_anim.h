#ifdef ENABLE_RGB_MATRIX_RAINDROPS
RGB_MATRIX_EFFECT(RAINDROPS)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Speichert die aktuelle Phase/Helligkeit (0 = aus, 1-100 = Fade-In, 101-255 = Fade-Out)
static uint8_t led_state[RGB_MATRIX_LED_COUNT] = {0};

bool RAINDROPS(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // 1. Neuen Regentropfen triggern (Start bei Phase 1)
    if ((params->iter == 0) && (scale16by8(g_rgb_timer, qadd8(rgb_matrix_config.speed, 16)) % 4 == 0)) {
        uint8_t drop_index = random8_max(RGB_MATRIX_LED_COUNT);
        if (led_state[drop_index] == 0) { 
            led_state[drop_index] = 1; // Startet das schnelle Fade-In
        }
    }

    // 2. Alle LEDs im Frame verarbeiten
    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

        if (led_state[i] > 0) {
            hsv_t hsv = rgb_matrix_config.hsv;
            
            // Phase 1: Schnelles Fade-In (von 0 bis max. Helligkeit)
            if (led_state[i] < 100) {
                hsv.v = scale8(rgb_matrix_config.hsv.v, led_state[i] * 2.55); // Aufblenden
                led_state[i] += 15; // Hohes Inkrement = Sehr kurzes Ansteigen (ca. 6-7 Frames)
                if (led_state[i] >= 100) led_state[i] = 100; // Peak erreicht
            } 
            // Phase 2: Langsames Fade-Out (Verblassen)
            else {
                uint8_t fade_val = 255 - ((led_state[i] - 100) * 1.6);
                hsv.v = scale8(rgb_matrix_config.hsv.v, fade_val);
                
                led_state[i] = qadd8(led_state[i], 2); // Kleines Inkrement = Langsames Ausfaden
                
                if (led_state[i] >= 200) { // Ende der Phase
                    led_state[i] = 0; // Zurücksetzen auf aus
                }
            }

            rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
            rgb_matrix_region_set_color(params->region, i, rgb.r, rgb.g, rgb.b);
        } else {
            // Hintergrund-LED aus (oder hier ggf. eine Grundfarbe setzen)
            rgb_matrix_region_set_color(params->region, i, 0, 0, 0);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_RAINDROPS
