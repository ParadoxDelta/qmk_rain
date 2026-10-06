#ifdef ENABLE_RGB_MATRIX_DIGITAL_RAIN
RGB_MATRIX_EFFECT(DIGITAL_RAIN)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Zeitstempel für den Start des Leuchtens jeder LED
static uint32_t g_digital_rain_spawn_time[DRIVER_LED_TOTAL];

bool DIGITAL_RAIN(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Dynamische Berechnung der Ein-/Ausblenddauer basierend auf dem Speed-Regler
    uint16_t base_fade_ms = 1000 - scale16by8(rgb_matrix_config.speed, 750);
    uint16_t attack_ms    = base_fade_ms; // Dynamisches Aufleuchten
    uint16_t decay_ms     = base_fade_ms; // Dynamisches Abklingen (exakt symmetrisch)
    uint16_t total_ms     = attack_ms + decay_ms;

    // 1. Initialisierung beim Aktivieren des Effekts
    if (params->init) {
        for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
            g_digital_rain_spawn_time[i] = 0;
        }
    }

    // 2. Erzeugen und Bewegen der Regentropfen von OBEN nach UNTEN
    if (params->iter == 0) {
        // Fallgeschwindigkeit der Tropfen-Welle (gesteuert über den Speed-Regler)
        uint8_t speed_factor = scale16by8(rgb_matrix_config.speed, 15) + 5;
        
        // Aktuelle virtuelle Y-Position der Tropfen-Front (Reihen 0 bis 6)
        uint8_t current_y = (g_rgb_timer / (64 - speed_factor)) % 7;

        // Gehe durch alle LEDs und prüfe, ob sie auf der aktuellen Y-Ebene liegen
        for (uint8_t i = led_min; i < led_max; i++) {
            if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

            // QMK Y-Koordinate (0-255) auf Tastaturreihen (0-6) umrechnen
            uint8_t row = g_led_config.pt[i].y / 38;

            if (row == current_y) {
                // Nur mit einer gewissen Wahrscheinlichkeit aktivieren (Spalten-Effekt)
                if ((random8() < 40) && (g_digital_rain_spawn_time[i] == 0)) {
                    g_digital_rain_spawn_time[i] = g_rgb_timer;
                }
            }
        }
    }

    // 3. Render-Schleife: Symmetrische, dynamische Helligkeitskurve & Wunschfarbe
    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

        uint32_t spawn = g_digital_rain_spawn_time[i];
        if (spawn == 0) {
            rgb_matrix_set_color(i, 0, 0, 0);
            continue;
        }

        uint32_t elapsed = g_rgb_timer - spawn;

        if (elapsed >= total_ms) {
            // Tropfen ist fertig abgeklungen
            g_digital_rain_spawn_time[i] = 0;
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            uint8_t val = 0;
            if (elapsed < attack_ms) {
                // Symmetrisches Ansteigen der Helligkeit (0 -> 255)
                val = (elapsed * 255) / attack_ms;
            } else {
                // Symmetrisches Abfallen der Helligkeit (255 -> 0)
                uint32_t decay_elapsed = elapsed - attack_ms;
                val = 255 - ((decay_elapsed * 255) / decay_ms);
            }

            // Helligkeit an die globale RGB-Matrix-Helligkeit anpassen
            val = scale8(val, rgb_matrix_config.hsv.v);

            // Nutzt die im Keychron Launcher / per FN gewählte Wunschfarbe & Sättigung
            hsv_t hsv = {
                .h = rgb_matrix_config.hsv.h,
                .s = rgb_matrix_config.hsv.s,
                .v = val
            };
            rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_DIGITAL_RAIN
