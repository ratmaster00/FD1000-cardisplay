# Hyundai Getz Display Library

[![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue.svg)](https://www.arduino.cc/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A complete Arduino library for controlling the 14-segment display from a 2006 Hyundai Getz radio. This library allows you to display text, scroll messages, control icons, and show custom graphics on the original FD1000A-driven LCD panel.

## 🎯 Features

- **14-Segment Display Control** - Full alphanumeric support with proper letter rendering
- **Text Display** - Show up to 11 characters with automatic uppercase conversion
- **Scrolling Text** - Scroll messages left or right (one-time, no wrap)
- **20+ Icons** - Control all original symbols (CH, MP3, LOUD, ST, USB, etc.)
- **Signal Bars** - Display 0-5 signal strength bars
- **Battery Icon** - Show 0-3 battery level
- **CD Graphic** - Control the CD icon with 3 segments
- **Small 7-Segment Digit** - Control the channel number display below CH
- **Radio Mode** - Display channel and frequency in original style
- **Memory Optimized** - Runs on Arduino Uno (≤2048 bytes RAM)
- **Serial Command Interface** - Control everything via Serial Monitor

## 📋 Supported Icons

| Icon | Description | Bit |
|------|-------------|-----|
| CH | Channel | 14 |
| CD-IN | CD Inserted | 35 |
| MP3 | MP3 Mode | 39 |
| WMA | WMA Mode | 51 |
| LOUD | Loudness | 55 |
| ST | Stereo | 67 |
| USB | USB Mode | 71 |
| NO-SIGNAL | No Signal | 75 |
| SCN | Scan | 83 |
| CALL | Call | 87 |
| COLON | Time Colon | 99 |
| RPT | Repeat | 103 |
| TP | Traffic Program | 119 |
| RDM | Random | 131 |
| DP | Display | 135 |
| TA | Traffic Announcement | 147 |
| BT | Bluetooth | 151 |
| PTY | Program Type | 163 |
| EQ | Equalizer | 167 |
| AF | Alternative Frequency | 179 |

## 🔧 Hardware Connection

### Pin Mapping

| Display Pin | Arduino Pin | Description |
|-------------|-------------|-------------|
| LCD-CLK | 2 | Clock (SCK) |
| LCD-DI | 3 | Data Input (MOSI) |
| LCD-CE | 4 | Chip Enable |
| LCD-INH | 5 | Display Inhibit (tie HIGH) |
| AVV-5V | 5V | Power |
| GND | GND | Ground |

### Wiring Diagram

```
Arduino Uno                    Hyundai Getz Display
─────────────                  ────────────────────
      D2  ─────────────────►   LCD-CLK
      D3  ─────────────────►   LCD-DI
      D4  ─────────────────►   LCD-CE
      D5  ─────────────────►   LCD-INH
      5V  ─────────────────►   AVV-5V
      GND ─────────────────►   GND
```

> **Note:** The display uses 1/4 duty mode (4 commons, 51 segments) with 1/3 bias for optimal contrast.

## USAGE

1. Download or clone the project
2. Go into the "Library" folder
3. Open Library.ino in Arduino IDE. The library will automatically open.
4. Upload the code, open Serial Monitor

## 🚀 Quick Start

```cpp
#include "HyundaiGetzDisplay.h"

HyundaiGetzDisplay display(2, 3, 4, 5); // CLK, DI, CE, INH

void setup() {
    display.begin();
    
    // Display text
    display.print("HELLO");
    
    // Add some icons
    display.icon("MP3", true);
    display.battery(3);
    display.signal(5);
    
    display.update();
}

void loop() {
    display.update(); // Keep scrolling alive
}
```

## 📖 API Reference

### Text Functions

```cpp
// Display text (max 11 chars, auto-uppercase)
display.print("HELLO");
display.print("WORLD", 2);  // Start at position 2

// Display a single character at position
display.print('A', 5);

// Scroll text once (left or right)
display.scroll("This will scroll across", SCROLL_LEFT, 150);
display.scroll("Scrolling right", SCROLL_RIGHT, 100);

// Control scrolling
display.stopScroll();
bool scrolling = display.isScrolling();
bool complete = display.isScrollComplete();
```

### Icon Control

```cpp
// Turn icons on/off
display.icon("MP3", true);
display.icon("LOUD", false);
display.icon("ST", true);

// All supported icons:
// CH, CD-IN, MP3, WMA, LOUD, ST, USB, NO-SIGNAL,
// SCN, CALL, COLON, RPT, TP, RDM, DP, TA, BT, PTY, EQ, AF
```

### Display Elements

```cpp
// Signal bars (0-5)
display.signal(5);

// Battery (0-3)
display.battery(3);

// CD graphic (0-3)
display.cd(3);

// Small digit below CH (0-9)
display.ch(8);

// Radio display (channel 1-9, frequency)
display.radio(5, 1013);
```

### Display Control

```cpp
// Clear everything
display.clear();

// Update the display
display.update();

// Test all segments
display.testPattern();

// Run demo sequence
display.demo();
```

## 💻 Serial Commands

Upload the included example sketch and control the display via Serial Monitor (9600 baud):

```
text HELLO          - Display text
scroll MESSAGE      - Scroll left once
scrollr MESSAGE     - Scroll right once
stop                - Stop scrolling
clear               - Clear display
symbols             - Show all symbols
radio 5 1013        - Radio CH5, 101.3 MHz
battery 3           - Set battery level
signal 5            - Set signal bars
cd 3                - Set CD graphic
ch 8                - Set small digit
icon MP3 1          - Turn icon on (1) or off (0)
demo                - Run demo sequence
test                - Test pattern
help                - Show commands
```

## 🔬 Technical Details

### FD1000A Driver

The display uses a **Fine Chips FD1000A** LCD driver in **1/4 duty mode**:
- 4 commons (COM0-COM3)
- 51 segments (SEG1-SEG51)
- 204-bit display buffer

### Display Buffer Layout

```
Digit 1: Bits 20-35
Digit 2: Bits 36-51
Digit 3: Bits 52-67
...
Digit 11: Bits 180-195
```

### 16-Bit Digit Pattern

```
Position:  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
Segment:   E  G1   F  SYM  L  JM   H  SYM  D   N   K   A   C  G2   B  SYM
```

### Memory Usage

- **RAM:** ~204 bytes (display buffer) + small overhead
- **Flash:** ~5KB (font table + code)
- **Runs on:** Arduino Uno, Nano, Mega, ESP8266, ESP32

## 📝 Examples

### Basic Text Display
```cpp
display.print("HELLO");
display.icon("MP3", true);
display.battery(3);
display.update();
```

### Radio Preset Display
```cpp
display.radio(5, 1013);
display.icon("ST", true);
display.icon("TP", true);
display.update();
```

### Scrolling Message
```cpp
display.scroll("THIS WILL SCROLL ONCE", SCROLL_LEFT, 150);
while (display.isScrolling()) {
    display.update();
    delay(10);
}
```

### Custom Layout
```cpp
display.clear();
display.print("CD");
display.icon("CD-IN", true);
display.cd(3);
display.battery(2);
display.signal(4);
display.update();
```

## 🤝 Contributing

Found a bug or want to add a feature? Feel free to:
1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Fine Chips** for the FD1000A datasheet
- The Arduino community for making hardware hacking accessible
- My grandpa, who unknowingly started this whole journey

## 🔗 Links

- [FD1000A Datasheet](https://www.alldatasheet.com/datasheet-pdf/pdf/437715/FINECHIPS/FD1000A.html)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

---

**Made with ❤️ and lots of patience**
