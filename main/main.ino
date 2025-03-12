#include <FastLED.h>
#include <time.h>

#define DATA_PIN 14
#define NUM_LEDS 10  // LED count

// different delays to choose
#define DELAY_0 10
#define DELAY_02 30
#define DELAY_1 50
#define DELAY_2 100

CRGB leds[NUM_LEDS];
int index_colors = 0;
CHSV colors[3] = { CHSV(128, 200, 255), CHSV(117, 200, 255), CHSV(122, 200, 255) };
CHSV colors_2[3] = { CHSV(128, 200, 255), CHSV(171, 200, 255), CHSV(213, 200, 255) };

#define SWITCH_PIN1 16
#define SWITCH_PIN2 17
#define DEBOUNCE_DELAY 100

enum Modes { OFF,
             FLICKER,
             COLOR_CHASE,
             NUM_MODES };
int currentMode = FLICKER;
unsigned long lastDebounceTime = 0;
bool buttonPressed = false;

void setup() {
  pinMode(SWITCH_PIN1, OUTPUT);
  digitalWrite(SWITCH_PIN1, LOW);  // Act as ground
  pinMode(SWITCH_PIN2, INPUT_PULLUP);

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

// circle
unsigned long current_Mills = 0;
unsigned long previous_Mills;
int circle_index = 0;

void led_color_chase(int DELAY) {
  current_Mills = millis();
  if (current_Mills - previous_Mills > (unsigned long)DELAY) {
    previous_Mills = current_Mills;
    leds[circle_index] = colors_2[index_colors];
    FastLED.show();
    circle_index = (circle_index + 1) % NUM_LEDS;
    if (circle_index == 0) {
      index_colors++;
      if (index_colors > 2)
        index_colors = 0;
    }
  }
}

// button check
void checkButton() {
  bool currentState = digitalRead(SWITCH_PIN2);

  if (currentState == LOW) {
    if (!buttonPressed && (millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      currentMode = (currentMode + 1) % NUM_MODES;
      buttonPressed = true;
      lastDebounceTime = millis();
    }
  } else {
    buttonPressed = false;
  }
}

void loop() {
  checkButton();
  switch (currentMode) {
    case OFF:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(100);
      break;
    case FLICKER:
      led_sparks(DELAY_02, 0.98);
      break;
    case COLOR_CHASE:
      led_color_chase(DELAY_1);
      break;
  }
}
