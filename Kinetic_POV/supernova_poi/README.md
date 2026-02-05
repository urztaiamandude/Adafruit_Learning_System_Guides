# Supernova POI for Teensy 4.1 - PlatformIO

This project contains the converted supernova POI Arduino sketch optimized for Teensy 4.1 using PlatformIO IDE with simplified single-button control.

## Hardware Requirements

- Teensy 4.1 microcontroller
- 144 LED/m DotStar LED strip
- 2200 mAh Lithium Ion Battery
- LiPoly charging backpack
- Simple momentary pushbutton switch

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

- Button Input: Pin 5 (with internal pullup enabled)
- DotStar Data: Hardware SPI MOSI
- DotStar Clock: Hardware SPI SCK

## Button Control

The poi uses a single button for all navigation:

### Quick Tap (< 800ms)
- Switch to next animation pattern
- If display is off, wakes it up

### Medium Hold (2-4 seconds)  
- Toggle automatic pattern cycling on/off
- LED will briefly flash to confirm

### Long Hold (> 5 seconds)
- Toggle display power on/off
- Useful for conserving battery

## Dependencies

The following libraries are automatically installed:
- Adafruit DotStar

## License

MIT License - See original file headers for copyright information.
