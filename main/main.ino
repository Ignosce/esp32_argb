#include <FastLED.h>
#include <time.h>
#include <ezButton.h>

#define DATA_PIN 14
#define NUM_LEDS 10  // LED count

#define DELAY_COLOR_CHASE 45
#define DELAY_RAINBOW 15
#define DELAY_RAINBOW_CHASE 45
#define DELAY_SPARKS_FLICKER 30
#define LED_SPARKS_SPEED 0.98

#define BUTTON_1 12
#define DEBOUNCE_TIME 50
ezButton button_1(BUTTON_1);

CRGB leds[NUM_LEDS];
int index_colors = 0;

// color palettes
CHSV colors[3] = { CHSV(128, 200, 255), CHSV(117, 200, 255), CHSV(122, 200, 255) };
CHSV colors_2[3] = { CHSV(128, 200, 255), CHSV(171, 200, 255), CHSV(213, 200, 255) };

enum Modes { OFF,
             FLICKER,
             COLOR_CHASE,
             RAINBOW,
             RAINBOW_CHASE,
             NUM_MODES };
int currentMode = FLICKER;

unsigned long current_Mills = 0;
unsigned long previous_Mills;

void setup() {;
  button_1.setDebounceTime(DEBOUNCE_TIME);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  srand(time(NULL));
  Serial.begin(9600);
}

// spark
struct Spark {
  int led_index;
  CHSV color;
  bool active;
};
Spark sparks[2];

void setup_sparks() {
  for (auto &spark : sparks) {
    spark.active = false;
    spark.color = CHSV(0, 0, 0);
  }
}

void led_sparks(int FLICKER_DELAY, float speed) {
  static bool initialized = false;
  if (!initialized) {
    setup_sparks();
    initialized = true;
  }

  fill_solid(leds, NUM_LEDS, 0x000000);

  bool needs_new_sparks = true;
  for (auto &spark : sparks) {
    if (spark.active) {
      int flicker = random(-20, 25);
      spark.color.value = constrain(spark.color.value + flicker, 0, 255);

      spark.color.value *= speed;

      if (spark.color.value > 5) {
        leds[spark.led_index] = spark.color;
        needs_new_sparks = false;
      } else {
        spark.active = false;
      }
    }
  }

  if (needs_new_sparks) {
    for (auto &spark : sparks) {
      spark.led_index = random(NUM_LEDS);
      spark.color = colors[index_colors];
      spark.color.value = 200 + random(55);
      spark.active = true;

      index_colors++;
      if (index_colors > 2)
        index_colors = 0;

      for (const auto &other : sparks) {
        if (&spark != &other && spark.led_index == other.led_index) {
          spark.led_index = (spark.led_index + 1) % NUM_LEDS;
        }
      }
    }
  }

  FastLED.show();
  delay(FLICKER_DELAY);
}

// color chase
void led_color_chase(int DELAY) {
  static int color_chase_index = 0;
  current_Mills = millis();
  if (current_Mills - previous_Mills > (unsigned long)DELAY) {
    previous_Mills = current_Mills;
    leds[color_chase_index] = colors_2[index_colors];
    FastLED.show();
    color_chase_index = (color_chase_index + 1) % NUM_LEDS;
    if (color_chase_index == 0) {
      index_colors++;
      if (index_colors > 2)
        index_colors = 0;
    }
  }
}

// rainbow
void led_rainbow(int DELAY) {
  static uint8_t hue = 0;
  static uint8_t colors_invterval = UINT8_MAX / (uint8_t)NUM_LEDS;
  static uint8_t computed_hue[NUM_LEDS];
  static bool colors_initialized = false;

  if(!colors_initialized) {
    for (int i=0; i < NUM_LEDS; i++) {
      computed_hue[i] = (i+1) * colors_invterval;
    }
    colors_initialized = true;
  }

  current_Mills = millis();
  if (current_Mills - previous_Mills > (unsigned long)DELAY) {
    previous_Mills = current_Mills;

    for (int i=0; i < NUM_LEDS; i++) {
      auto color = CHSV(computed_hue[i]+hue, 255, 255);
      leds[i] = color;
    }
    FastLED.show();
    hue++;
  }
}

// rainbow chase
void led_rainbow_chase(int DELAY) {
  static uint8_t hue = 0;
  static uint8_t index = 0;

  current_Mills = millis();
  if (current_Mills - previous_Mills > (unsigned long)DELAY) {
    previous_Mills = current_Mills;
    auto color = CHSV(hue, 255, 255);
    leds[index] = color;
    FastLED.show();
    index = (index + 1) % NUM_LEDS;
    hue = hue + 5;
  }
}

void loop() {
  button_1.loop();
  if (button_1.isPressed())
    currentMode = (currentMode + 1) % NUM_MODES;
  switch (currentMode) {
    case OFF:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(100);
      break;
    case FLICKER:
      led_sparks(DELAY_SPARKS_FLICKER, LED_SPARKS_SPEED);
      break;
    case COLOR_CHASE:
      led_color_chase(DELAY_COLOR_CHASE);
      break;
    case RAINBOW:
      led_rainbow(DELAY_RAINBOW);
      break;
    case RAINBOW_CHASE:
      led_rainbow_chase(DELAY_RAINBOW_CHASE);
      break;
  }
}
