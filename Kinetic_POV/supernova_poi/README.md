# Supernova POI for Teensy 4.1 - PlatformIO

This project contains the converted supernova POI Arduino sketch optimized for Teensy 4.1 using PlatformIO IDE.

## Hardware Requirements

- Teensy 4.1 microcontroller
- 144 LED/m DotStar LED strip
- 2200 mAh Lithium Ion Battery
- LiPoly charging backpack
- IR Sensor (Adafruit #157)
- Mini IR Remote Control (Adafruit #389)

## Setup Instructions

### 1. Install PlatformIO

If you haven't already, install PlatformIO:
- For VS Code: Install the PlatformIO IDE extension
- For command line: `pip install platformio`

### 2. Open Project

Open this directory in PlatformIO IDE or navigate to it in your terminal.

### 3. Build Project

```bash
pio run
```

### 4. Upload to Teensy 4.1

```bash
pio run --target upload
```

## Configuration

Edit `platformio.ini` to customize:
- Upload settings
- Serial monitor baud rate
- Build flags

## Pin Connections

- IR Receiver: Pin 5
- DotStar Data: Hardware SPI MOSI
- DotStar Clock: Hardware SPI SCK

## Dependencies

The following libraries are automatically installed:
- Adafruit DotStar
- IRremote (Teensy-compatible version)

## IR Remote Control Codes

The sketch responds to an Adafruit Mini Remote Control with these functions:
- VOL+/VOL-: Adjust brightness
- UP/DOWN: Change animation speed
- LEFT/RIGHT: Switch patterns
- PLAY/PAUSE: Restart current pattern
- ENTER: Toggle auto-cycle mode
- SETUP: Show battery level
- STOP: Turn off LEDs

## License

MIT License - See original file headers for copyright information.
