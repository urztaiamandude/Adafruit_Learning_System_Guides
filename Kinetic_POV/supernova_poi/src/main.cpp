// SPDX-FileCopyrightText: 2019 Phillip Burgess for Adafruit Industries
// Modified for single button control
// SPDX-License-Identifier: MIT

/*------------------------------------------------------------------------
  POV Supernova Poi sketch with simple button navigation
  Converted for Teensy 4.1 with single button control
  
  Hardware:
  - Teensy 4.1
  - 2200 mAh Lithium Ion Battery
  - LiPoly Backpack
  - 144 LED/m DotStar strip
  - Simple momentary pushbutton
  
  Button Functions:
  - Tap (< 800ms): Switch to next visual pattern
  - Hold (2-4s): Enable/disable automatic pattern cycling
  - Long hold (> 5s): Toggle display power on/off
  
  Requires: Adafruit_DotStar library
  ------------------------------------------------------------------------*/

#include <Arduino.h>
#include <Adafruit_DotStar.h>
#include <SPI.h>

typedef uint16_t line_t;

#include "graphics.h"

// Pin configuration
// Button wiring: Connect button between Pin 5 and GROUND (GND)
// INPUT_PULLUP mode means pin is HIGH when button is released, LOW when pressed
#define BUTTON_INPUT_PIN 5

// Timing thresholds in milliseconds
#define TAP_THRESHOLD_MS 800
#define HOLD_THRESHOLD_MS 2000
#define LONG_HOLD_THRESHOLD_MS 5000
#define DEBOUNCE_DELAY_MS 50

// Battery voltage monitoring thresholds
#define BATTERY_EMPTY_MILLIVOLTS 3350
#define BATTERY_FULL_MILLIVOLTS 4000

// Display configuration
bool enableAutomaticCycling = true;
uint32_t secondsBetweenPatterns = 12;

// Button state tracking
struct ButtonStateTracker {
  bool currentlyPressed;
  bool previouslyPressed;
  uint32_t pressStartTimestamp;
  uint32_t releaseTimestamp;
  uint32_t lastDebounceTime;
  bool actionExecuted;
} buttonState = {false, false, 0, 0, 0, false};

// LED strip configuration
// Uses hardware SPI on Teensy 4.1: Pin 11 (MOSI/Data), Pin 13 (SCK/Clock)
// MOSI = Master Out, Slave In: Teensy outputs data, LEDs receive as input
#if defined(LED_DATA_PIN) && defined(LED_CLOCK_PIN)
Adafruit_DotStar strip = Adafruit_DotStar(NUM_LEDS, LED_DATA_PIN, LED_CLOCK_PIN, DOTSTAR_BGR);
#else
Adafruit_DotStar strip = Adafruit_DotStar(NUM_LEDS, DOTSTAR_BGR);
#endif

// Function declarations
void initializeCurrentImage();
void advanceToNextImage();
void handleButtonInput();

void setup() {
  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);
  
  strip.begin();
  strip.clear();
  strip.show();
  
  initializeCurrentImage();
}

// Display state management
uint32_t timestampOfLastImageChange = 0L;
uint32_t timestampOfLastScanline = 0L;
uint8_t activeImageIndex = 0;
uint8_t activeImageFormat;
uint8_t *pointerToImagePalette;
uint8_t *pointerToImagePixelData;
uint8_t colorLookupTable[16][3];
line_t totalScanlines;
line_t currentScanlineIndex;

// Brightness levels array
const uint8_t PROGMEM intensityLevels[] = {15, 31, 63, 127, 255};
uint8_t currentIntensityIndex = sizeof(intensityLevels) - 1;

// Animation timing table
const uint16_t PROGMEM scanlineTimingMicros[] = {
  1000000L / 375,
  1000000L / 472,
  1000000L / 595,
  1000000L / 750,
  1000000L / 945,
  1000000L / 1191,
  1000000L / 1500
};
uint8_t timingTableIndex = 3;
uint16_t microsPerScanline = 1000000L / 750;

void initializeCurrentImage() {
  activeImageFormat = images[activeImageIndex].type;
  totalScanlines = images[activeImageIndex].lines;
  currentScanlineIndex = 0;
  pointerToImagePalette = (uint8_t *)images[activeImageIndex].palette;
  pointerToImagePixelData = (uint8_t *)images[activeImageIndex].pixels;
  
  if(activeImageFormat == PALETTE1) {
    memcpy_P(colorLookupTable, pointerToImagePalette, 2 * 3);
  } else if(activeImageFormat == PALETTE4) {
    memcpy_P(colorLookupTable, pointerToImagePalette, 16 * 3);
  }
  
  timestampOfLastImageChange = millis();
}

void advanceToNextImage() {
  activeImageIndex++;
  if(activeImageIndex >= NUM_IMAGES) {
    activeImageIndex = 0;
  }
  initializeCurrentImage();
}

void handleButtonInput() {
  uint32_t currentTime = millis();
  bool pinReading = (digitalRead(BUTTON_INPUT_PIN) == LOW);
  
  // Debounce logic
  if(pinReading != buttonState.previouslyPressed) {
    buttonState.lastDebounceTime = currentTime;
  }
  
  if((currentTime - buttonState.lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if(pinReading != buttonState.currentlyPressed) {
      buttonState.currentlyPressed = pinReading;
      
      if(buttonState.currentlyPressed) {
        // Button pressed down
        buttonState.pressStartTimestamp = currentTime;
        buttonState.actionExecuted = false;
      } else {
        // Button released
        buttonState.releaseTimestamp = currentTime;
        uint32_t pressDuration = buttonState.releaseTimestamp - buttonState.pressStartTimestamp;
        
        if(!buttonState.actionExecuted) {
          if(pressDuration < TAP_THRESHOLD_MS) {
            // Quick tap: advance pattern
            if(strip.getBrightness() > 0) {
              advanceToNextImage();
            } else {
              // Wake from sleep
              strip.setBrightness(intensityLevels[currentIntensityIndex]);
            }
          }
        }
      }
    }
    
    // Check for held button actions while pressed
    if(buttonState.currentlyPressed && !buttonState.actionExecuted) {
      uint32_t heldDuration = currentTime - buttonState.pressStartTimestamp;
      
      if(heldDuration >= LONG_HOLD_THRESHOLD_MS) {
        // Very long hold: power toggle
        if(strip.getBrightness() > 0) {
          strip.setBrightness(0);
        } else {
          strip.setBrightness(intensityLevels[currentIntensityIndex]);
        }
        buttonState.actionExecuted = true;
      } else if(heldDuration >= HOLD_THRESHOLD_MS) {
        // Medium hold: toggle auto-cycling
        enableAutomaticCycling = !enableAutomaticCycling;
        buttonState.actionExecuted = true;
      }
    }
  }
  
  buttonState.previouslyPressed = pinReading;
}

void loop() {
  uint32_t currentMillis = millis();
  
  // Automatic pattern cycling
  if(enableAutomaticCycling) {
    if((currentMillis - timestampOfLastImageChange) >= (secondsBetweenPatterns * 1000L)) {
      advanceToNextImage();
    }
  }
  
  // Render current scanline based on image format
  switch(activeImageFormat) {
    case PALETTE1: {
      uint8_t ledIndex = 0;
      uint8_t byteIndex, bitIndex, packedPixels, paletteIndex;
      uint8_t *dataPointer = (uint8_t *)&pointerToImagePixelData[currentScanlineIndex * NUM_LEDS / 8];
      
      for(byteIndex = NUM_LEDS/8; byteIndex--; ) {
        packedPixels = *dataPointer++;
        for(bitIndex = 8; bitIndex--; packedPixels >>= 1) {
          paletteIndex = packedPixels & 1;
          strip.setPixelColor(ledIndex++,
            colorLookupTable[paletteIndex][0],
            colorLookupTable[paletteIndex][1],
            colorLookupTable[paletteIndex][2]);
        }
      }
      break;
    }
    
    case PALETTE4: {
      uint8_t ledIndex, firstNibble, secondNibble;
      uint8_t *dataPointer = (uint8_t *)&pointerToImagePixelData[currentScanlineIndex * NUM_LEDS / 2];
      
      for(ledIndex = 0; ledIndex < NUM_LEDS; ) {
        secondNibble = *dataPointer++;
        firstNibble = secondNibble >> 4;
        secondNibble &= 0x0F;
        
        strip.setPixelColor(ledIndex++,
          colorLookupTable[firstNibble][0],
          colorLookupTable[firstNibble][1],
          colorLookupTable[firstNibble][2]);
        strip.setPixelColor(ledIndex++,
          colorLookupTable[secondNibble][0],
          colorLookupTable[secondNibble][1],
          colorLookupTable[secondNibble][2]);
      }
      break;
    }
    
    case PALETTE8: {
      uint16_t paletteOffset;
      uint8_t ledIndex;
      uint8_t *dataPointer = (uint8_t *)&pointerToImagePixelData[currentScanlineIndex * NUM_LEDS];
      
      for(ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
        paletteOffset = *dataPointer++ * 3;
        strip.setPixelColor(ledIndex,
          pointerToImagePalette[paletteOffset],
          pointerToImagePalette[paletteOffset + 1],
          pointerToImagePalette[paletteOffset + 2]);
      }
      break;
    }
    
    case TRUECOLOR: {
      uint8_t ledIndex, redValue, greenValue, blueValue;
      uint8_t *dataPointer = (uint8_t *)&pointerToImagePixelData[currentScanlineIndex * NUM_LEDS * 3];
      
      for(ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
        redValue = *dataPointer++;
        greenValue = *dataPointer++;
        blueValue = *dataPointer++;
        strip.setPixelColor(ledIndex, redValue, greenValue, blueValue);
      }
      break;
    }
  }
  
  // Advance to next scanline
  currentScanlineIndex++;
  if(currentScanlineIndex >= totalScanlines) {
    currentScanlineIndex = 0;
  }
  
  // Process button input during wait period
  uint32_t currentMicros;
  while(((currentMicros = micros()) - timestampOfLastScanline) < microsPerScanline) {
    handleButtonInput();
  }
  
  strip.show();
  timestampOfLastScanline = currentMicros;
}
