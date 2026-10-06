#ifdef ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
RGB_MATRIX_EFFECT(JELLYBEAN_RAINDROPS)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Speicher für den Zeitstempel (Spawn-Zeit) und den Farbton (Hue) jeder einzelnen LED
static uint32_t g_jellybean_spawn_time[RGB_MATRIX_LED_COUNT];
static uint8_t  g_jellybean_hue[RGB_MATRIX_LED_COUNT];

bool JELLYBEAN_RAINDROPS(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Timing-Parameter für den Asymmetrischen ASR-Ramp (NuPhy Air Style)
    uint16_t attack_ms = 80;   // Blitzschnelles Aufleuchten (Fade-In)
    uint16_t decay_ms  = 1200; // Sanftes, langes Ausfaden (Fade-Out)
    uint16_t total_ms  = attack_ms + decay_ms;

    // 1. Initialisierung beim Aktivieren des Effekts
    if (params->init) {
        for (uint8_t i = led_min; i < led_max; i++) {
            g_jellybean_spawn_time[i] = 0;
            g_jellybean_hue[i]        = 0;
        }
    }

    // 2. Zufälliges Spawnen neuer Regentropfen (abhängig vom Speed-Regler)
    if (params->iter == 0) {
        // Höhere Zahl bei speed = schnelleres Spawnen
        uint8_t chance = scale16by8(rgb_matrix_config.speed, 15) + 2;
        if (random8() < chance) {
            uint8_t drop_led = random8_max(RGB_MATRIX_LED_COUNT);
            if (drop_led >= led_min && drop_led < led_max) {
                g_jellybean_spawn_time[drop_led] = g_rgb_timer;
                g_jellybean_hue[drop_led]        = random8(); // Bunte Jellybean-Farben
            }
        }
    }

    // 3. Berechnung und Zeichnen des Helligkeitsverlaufs für jede LED
    for (uint8_t i = led_min; i < led_max; i++) {
        if (!HAS_ANY_FLAGS(g_led_config.flags[i], params->flags)) continue;

        uint32_t spawn = g_jellybean_spawn_time[i];
        if (spawn == 0) {
            rgb_matrix_set_color(i, 0, 0, 0);
            continue;
        }

        uint32_t elapsed = g_rgb_timer - spawn;

        if (elapsed >= total_ms) {
            // Tropfen ist komplett ausgeblendet
            g_jellybean_spawn_time[i] = 0;
            rgb_matrix_set_color(i, 0, 0, 0);
        } else {
            uint8_t val = 0;
            if (elapsed < attack_ms) {
                // Schneller Attack (Fade-In)
                val = (elapsed * 255) / attack_ms;
            } else {
                // Langsamer Decay (Fade-Out)
                uint32_t decay_elapsed = elapsed - attack_ms;
                val = 255 - ((decay_elapsed * 255) / decay_ms);
            }

            // Helligkeit an die globale RGB-Matrix-Helligkeit anpassen
            val = scale8(val, rgb_matrix_config.hsv.v);

            hsv_t hsv = {
                .h = g_jellybean_hue[i],
                .s = random8_min_max(160, 255), // Bunte Farbsättigung
                .v = val
            };
            rgb_t rgb = rgb_matrix_hsv_to_rgb(hsv);
            rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // ENABLE_RGB_MATRIX_JELLYBEAN_RAINDROPS
