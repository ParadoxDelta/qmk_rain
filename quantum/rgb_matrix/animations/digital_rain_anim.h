C

#ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN
RGB_MATRIX_EFFECT(DIGITAL_RAIN)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static uint32_t g_digital_rain_spawn_time[RGB_MATRIX_LED_COUNT];

bool DIGITAL_RAIN(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Dynamische Berechnung der Ein-/Ausblenddauer basierend auf dem Speed-Regler
    uint16_t speed_scaled = scale8(rgb_matrix_config.speed, 225);
    uint16_t base_fade_ms = 1000 - (speed_scaled * 3);
    uint16_t attack_ms    = base_fade_ms;
    uint16_t decay_ms     = base_fade_ms;
    uint16_t total_ms     = attack_ms + decay_ms;

    // 1. Initialisierung beim Aktivieren des Effekts
    if (params->init) {
        for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
            g_digital_rain_spawn_time[i] = 0;
        }
    }

    // 2. Erzeugen der Regentropfen von OBEN nach UNTEN
    if (params->iter == 0) {
        uint8_t speed_factor = scale8(rgb_matrix_config.speed, 15) + 5;
        
        // Bestimme minimale und maximale Y-Koordinate der Tastatur-Matrix
        uint8_t min_y = 255;
        uint8_t max_y = 0;
        for (uint8_t i = led_min; i < led_max; i++) {
            if (g_led_config.point[i].y < min_y) min_y = g_led_config.point[i].y;
            if (g_led_config.point[i].y > max_y) max_y = g_led_config.point[i].y;
        }

        uint8_t y_range = (max_y - min_y) > 0 ? (max_y - min_y) : 1;

        for (uint8_t i = led_min; i < led_max; i++) {
            if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

            // Normiere die Y-Koordinate relativ zur Tastaturhöhe auf 0..5 (6 Reihen)
            uint8_t row = ((uint16_t)(g_led_config.point[i].y - min_y) * 6) / y_range;

            // Aktuelle Y-Position der Tropfenwelle
            uint8_t current_y = (g_rgb_timer / (64 - speed_factor)) % 6;

            if (row == current_y) {
                if ((random8() < 50) && (g_digital_rain_spawn_time[i] == 0)) {
                    g_digital_rain_spawn_time[i] = g_rgb_timer;
                }
            }
        }
    }

    // 3. Render-Schleife: Symmetrische Helligkeitskurve mit gewählter HSV-Farbe
    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

        uint32_t spawn = g_digital_rain_spawn_time[i];
        if (spawn == 0) {
            rgb_matrix_set_color(i, 0, 0, 0);
            continue;
        }

        uint32_t elapsed = g_rgb_timer - spawn;

        if (elapsed >= total_ms) {
            g_digital_rain_spawn_time[i] = 0;
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            uint8_t val = 0;
            if (elapsed < attack_ms) {
                val = (elapsed * 255) / attack_ms;
            } else {
                uint32_t decay_elapsed = elapsed - attack_ms;
                val = 255 - ((decay_elapsed * 255) / decay_ms);
            }

            // Nutze die im Launcher gewählte Farbe und skaliere die Helligkeit
            hsv_t hsv = rgb_matrix_config.hsv;
            hsv.v     = scale8(val, rgb_matrix_config.hsv.v);

            rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_DIGITAL_RAIN
