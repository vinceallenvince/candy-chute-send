/*
 * This Arduino app controls a button and Neopixel strip (https://www.adafruit.com/product/1461). 
 * The button idle state should indicate the chute is active with a subtle pulse/breathe effect.
 * The button active state should flash to confirm the button press.
 * The LED strip idle state should be power efficient and simply signal the chute is active.
 * The LED strip active state should signal the chute is sending candy down and should include
 * fast, bright and excited animation.
 * 
 * Candy Chute Controller with Neopixel Animation - SEND BUTTON
 * - Normal: Arcade button LED pulses (breathing effect)
 * - Button Press: Triggers exciting candy drop animation on LED strip
 * - LED strip wraps around chute in spiral pattern (240 LEDs)
 *
 * Wiring:
 * - Button switch: One pin to GND, other pin to BUTTON_PIN
 * - Arcade button LED: LED positive (+) → 220Ω resistor → LED_PIN → LED negative (-) → GND
 * - Neopixel strip: Data pin → STRIP_PIN, 5V and GND to power supply
 */

#include <Adafruit_NeoPixel.h>

#define  BUTTON_PIN  2       // Button switch input (any digital pin)
#define  LED_PIN  9          // Arcade button LED PWM output (must be PWM-capable: 3,5,6,9,10,11)
#define  STRIP_PIN  6        // Neopixel data pin
#define  NUM_LEDS  240       // Number of Neopixels in strip
#define  MAX_BRIGHTNESS  255
#define  MEDIUM_BRIGHTNESS  127

// Candy drop animation configuration
#define  ANIMATION_STEP_SIZE  3       // How many positions to advance per frame (higher = faster)
#define  WAVE_LENGTH  6               // Length of each colored segment
#define  NUM_WAVES  3                 // Number of simultaneous color waves
#define  WAVE_STAGGER_DIVISOR  3      // Wave stagger timing (NUM_LEDS / this value)
#define  BUTTON_FLASH_INTERVAL  50    // Button LED flash speed in milliseconds
#define  BUTTON_MAX_FLASHES  6        // Number of button flash cycles (on/off pairs)
#define  COLOR_SEGMENTS_PER_WAVE  5   // Number of color segments in each wave
#define  BRIGHTNESS_FADE_STEP  8      // Brightness reduction per LED in segment (255 - i * this)
#define  FINALE_FLASH_COUNT  6        // Number of purple finale flashes
#define  FINALE_FLASH_DELAY  100      // Delay between finale flashes in milliseconds

// Initialize Neopixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

uint8_t brightness = 0;
int8_t direction = 1;  // 1 = increasing, -1 = decreasing
bool buttonPressed = false;

// Strip idle animation variables - 3 LEDs traveling back and forth
float stripPosition = 0;        // Current position of traveling LEDs (float for smooth acceleration)
int8_t stripDirection = 1;      // 1 = forward, -1 = backward
float minSpeed = 0.3;           // Minimum speed at the ends (never stops)
float maxSpeed = 2.0;           // Maximum speed at the middle

void setup() {
  Serial.begin(115200);
  Serial.println(F("Candy Chute Controller"));

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button input with internal pullup
  pinMode(LED_PIN, OUTPUT);           // LED output

  // Initialize Neopixel strip
  strip.begin();
  strip.setBrightness(255);  // Max brightness
  strip.clear();
  strip.show();

  Serial.println(F("System ready!"));
}

// LED strip idle effect - 3 LEDs traveling back and forth with acceleration
void stripBreathingEffect() {
  // Calculate speed based on position (fastest at middle, slowest at ends)
  // Normalized position from 0.0 (start) to 1.0 (end)
  float normalizedPos = stripPosition / (NUM_LEDS - 1.0);

  // Calculate distance from center (0.0 at center, 0.5 at edges)
  float distanceFromCenter = abs(normalizedPos - 0.5);

  // Speed is inversely proportional to distance from center
  // At center (distanceFromCenter = 0): speed = maxSpeed
  // At edges (distanceFromCenter = 0.5): speed = minSpeed
  float speed = maxSpeed - ((distanceFromCenter / 0.5) * (maxSpeed - minSpeed));

  // Update position with current speed
  stripPosition += stripDirection * speed;

  // Reverse direction at ends
  if (stripPosition >= NUM_LEDS - 1) {
    stripPosition = NUM_LEDS - 1;
    stripDirection = -1;
  } else if (stripPosition <= 0) {
    stripPosition = 0;
    stripDirection = 1;
  }

  // Clear all LEDs
  strip.clear();

  // Light up 3 LEDs in purple traveling along the strip
  uint32_t purpleColor = strip.Color(60, 0, 60);  // Dim purple

  // Convert float position to integer for LED indexing
  int intPos = (int)stripPosition;

  // Center LED
  strip.setPixelColor(intPos, purpleColor);

  // LED before (if in bounds)
  if (intPos > 0) {
    strip.setPixelColor(intPos - 1, purpleColor);
  }

  // LED after (if in bounds)
  if (intPos < NUM_LEDS - 1) {
    strip.setPixelColor(intPos + 1, purpleColor);
  }

  strip.show();
}

// Candy drop animation - colorful cascade down the chute
void candyDropAnimation() {
  Serial.println("CANDY DROP!");

  // Define vibrant candy colors
  uint32_t colors[] = {
    strip.Color(255, 0, 0),     // Red
    strip.Color(255, 165, 0),   // Orange
    strip.Color(255, 255, 0),   // Yellow
    strip.Color(0, 255, 0),     // Green
    strip.Color(0, 0, 255),     // Blue
    strip.Color(255, 0, 255),   // Magenta
    strip.Color(255, 192, 203)  // Pink
  };
  int numColors = 7;

  // Multiple waves of candy colors cascading down - STAGGERED!
  int staggerOffset = NUM_LEDS / WAVE_STAGGER_DIVISOR;  // Each wave starts when previous is 1/3 across

  // Button LED flashing variables
  unsigned long lastFlashTime = millis();
  bool buttonLedState = true;
  int flashCount = 0;

  // Calculate total animation length (last wave needs to complete)
  int totalLength = (NUM_WAVES - 1) * staggerOffset + NUM_LEDS + WAVE_LENGTH;

  // Single loop - all waves animate simultaneously with stagger
  for (int pos = 0; pos < totalLength; pos += ANIMATION_STEP_SIZE) {
    // Handle button LED flashing (first 600ms)
    if (flashCount < BUTTON_MAX_FLASHES) {
      unsigned long currentTime = millis();
      if (currentTime - lastFlashTime >= BUTTON_FLASH_INTERVAL) {
        buttonLedState = !buttonLedState;
        analogWrite(LED_PIN, buttonLedState ? MAX_BRIGHTNESS : 0);
        lastFlashTime = currentTime;
        flashCount++;
      }
    } else {
      // After flashing, keep button LED on
      analogWrite(LED_PIN, MAX_BRIGHTNESS);
    }

    strip.clear();

    // Draw all active waves at their current positions
    for (int wave = 0; wave < NUM_WAVES; wave++) {
      int wavePos = pos - (wave * staggerOffset);  // Offset each wave

      // Only draw this wave if it's active (has started)
      if (wavePos >= 0 && wavePos < NUM_LEDS + WAVE_LENGTH) {
        // Draw multiple colorful segments in this wave
        for (int seg = 0; seg < COLOR_SEGMENTS_PER_WAVE; seg++) {
          int segStart = wavePos - (seg * WAVE_LENGTH);
          uint32_t color = colors[(wave + seg) % numColors];

          for (int i = 0; i < WAVE_LENGTH; i++) {
            int ledPos = segStart + i;
            if (ledPos >= 0 && ledPos < NUM_LEDS) {
              // Add sparkle/brightness variation
              uint8_t brightness = 255 - (i * BRIGHTNESS_FADE_STEP);
              uint32_t dimColor = strip.Color(
                ((color >> 16) & 0xFF) * brightness / 255,
                ((color >> 8) & 0xFF) * brightness / 255,
                (color & 0xFF) * brightness / 255
              );
              strip.setPixelColor(ledPos, dimColor);
            }
          }
        }
      }
    }

    strip.show();

    // BURST OF ENERGY - minimal delay for explosive effect!
    // No delay at all - full speed ahead!
  }

  // Flash finale - increasing purple brightness (6 flashes)
  uint32_t purpleFlashes[] = {
    strip.Color(85, 0, 85),    // Dim purple
    strip.Color(170, 0, 170),  // Medium purple
    strip.Color(255, 0, 255),  // Bright purple/magenta
    strip.Color(255, 0, 255),  // Bright purple/magenta
    strip.Color(255, 0, 255),  // Bright purple/magenta
    strip.Color(255, 0, 255)   // Bright purple/magenta
  };

  for (int flash = 0; flash < FINALE_FLASH_COUNT; flash++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, purpleFlashes[flash]);
    }
    strip.show();
    delay(FINALE_FLASH_DELAY);
    strip.clear();
    strip.show();
    delay(FINALE_FLASH_DELAY);
  }

  strip.clear();
  strip.show();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Button is pressed (LOW because of INPUT_PULLUP)
    if (!buttonPressed) {
      Serial.println("Button pressed - triggering candy drop!");
      buttonPressed = true;

      // Trigger the candy drop animation (button LED flashing happens inside)
      candyDropAnimation();
    }
    analogWrite(LED_PIN, MAX_BRIGHTNESS);  // Keep button LED bright while pressed
  } else {
    // Button is released - idle state
    if (buttonPressed) {
      Serial.println("Button released");
      buttonPressed = false;
      brightness = MEDIUM_BRIGHTNESS;  // Reset to medium brightness
      direction = 1;                   // Start increasing
    }

    // Arcade button LED breathing effect: fluctuate from 0 to MEDIUM_BRIGHTNESS
    brightness += direction;

    if (brightness >= MEDIUM_BRIGHTNESS) {
      brightness = MEDIUM_BRIGHTNESS;
      direction = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      direction = 1;
    }

    analogWrite(LED_PIN, brightness);

    // LED strip idle effect (3 traveling LEDs)
    stripBreathingEffect();
  }

  delay(0);
}
