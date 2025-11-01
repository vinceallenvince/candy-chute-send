# Candy Chute Send

An Arduino-based Halloween candy chute. This is the **send** end. Boo!

## Demo

https://github.com/user-attachments/assets/591216a5-6a2a-46fb-b113-6bf3aede38e3

## Features

- **Arcade Button** - Press to trigger the candy drop animation
  - Idle state: Subtle pulsing/breathing LED effect
  - Active state: Fast flashing to confirm button press

- **NeoPixel LED Strip Animation** (240 LEDs)
  - Idle state: 3 purple LEDs traveling back and forth with acceleration
  - Active state: Colorful cascading waves simulating candy falling down the chute
  - Finale: Purple flash sequence

## Hardware Requirements

- Arduino (any model with PWM-capable pins)
- Arcade button with built-in LED
- NeoPixel LED strip (240 LEDs) - [Adafruit NeoPixel Strip](https://www.adafruit.com/product/1461)
- 220Ω resistor for button LED
- 5V power supply for NeoPixel strip

## Wiring

- **Button switch**: One pin to GND, other pin to BUTTON_PIN (default: pin 2)
- **Arcade button LED**: LED positive (+) → 220Ω resistor → LED_PIN (default: pin 9) → LED negative (-) → GND
- **NeoPixel strip**: Data pin → STRIP_PIN (default: pin 6), 5V and GND to power supply

## Configuration

All animation parameters can be easily customized at the top of `candy-chute-send.ino`:

```cpp
// Pin configuration
#define BUTTON_PIN 2
#define LED_PIN 9
#define STRIP_PIN 6
#define NUM_LEDS 240

// Animation settings
#define ANIMATION_STEP_SIZE 3        // Animation speed (higher = faster)
#define WAVE_LENGTH 6                // Length of colored segments
#define NUM_WAVES 3                  // Number of simultaneous waves
#define FINALE_FLASH_COUNT 6         // Number of finale flashes
// ... and more
```

## Installation

1. Install the [Adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel) via Arduino Library Manager
2. Open `candy-chute-send.ino` in Arduino IDE
3. Select your Arduino board and port
4. Upload the sketch

## How It Works

1. **Idle State**: The arcade button LED pulses gently, and 3 purple LEDs travel along the strip with acceleration/deceleration
2. **Button Press**: Triggers an explosive candy drop animation with multiple colorful waves cascading down the LED strip
3. **Finale**: A series of purple flashes signal the end of the candy drop
4. **Reset**: System returns to idle state when button is released

## License

MIT
