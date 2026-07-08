#ifndef HyundaiGetzDisplay_h
#define HyundaiGetzDisplay_h

#include <Arduino.h>

#define SCROLL_LEFT  0
#define SCROLL_RIGHT 1

class HyundaiGetzDisplay {
public:
    HyundaiGetzDisplay(uint8_t clk, uint8_t di, uint8_t ce, uint8_t inh, int busDelay = 50) {
        _clk = clk;
        _di = di;
        _ce = ce;
        _inh = inh;
        _busDelay = busDelay;
        _scrolling = false;
        _scrollSpeed = 150;
        _scrollDirection = SCROLL_LEFT;
        _scrollPosition = 0;
        _scrollComplete = false;
        // Buffer is 204 bytes - this is the main memory user
        for (int i = 0; i < 204; i++) _buffer[i] = 0;
    }
    
    void begin() {
        pinMode(_clk, OUTPUT);
        pinMode(_di, OUTPUT);
        pinMode(_ce, OUTPUT);
        pinMode(_inh, OUTPUT);
        
        digitalWrite(_clk, LOW);
        digitalWrite(_di, LOW);
        digitalWrite(_ce, LOW);
        digitalWrite(_inh, HIGH);
        
        clear();
        update();
    }
    
    void clear() {
        memset(_buffer, 0, 204);
    }
    
    void clearDisplay() {
        for (int pos = 0; pos < 11; pos++) {
            int digitStartBit = 20 + (pos * 16);
            memset(&_buffer[digitStartBit], 0, 16);
        }
    }
    
    void clearSymbols() {
        static const uint8_t symbols[] PROGMEM = {14, 35, 39, 51, 55, 67, 71, 75, 83, 87, 99, 103, 119, 131, 135, 147, 151, 163, 167, 179};
        for (int i = 0; i < 20; i++) {
            _buffer[pgm_read_byte(&symbols[i])] = 0;
        }
    }
    
    void print(const char* text, int startPos = 0) {
        if (startPos > 10) return;
        if (startPos < 0) startPos = 0;
        
        int len = strlen(text);
        int maxLen = 11 - startPos;
        if (len > maxLen) len = maxLen;
        
        clearDisplay();
        
        for (int i = 0; i < len; i++) {
            char c = text[i];
            if (c >= 'a' && c <= 'z') c = c - 32;
            _writeChar14Seg(c, startPos + i);
        }
    }
    
    void print(String text, int startPos = 0) {
        print(text.c_str(), startPos);
    }
    
    void print(char c, int position) {
        if (position > 10) return;
        if (c >= 'a' && c <= 'z') c = c - 32;
        _writeChar14Seg(c, position);
    }
    
    void scroll(const char* text, int direction = SCROLL_LEFT, int speed = 150) {
        _scrollText = String(text);
        _scrollDirection = direction;
        _scrollSpeed = speed;
        _scrollPosition = 0;
        _scrollComplete = false;
        _scrolling = true;
        _lastScrollUpdate = millis();
        
        if (_scrollText.length() <= 11) {
            clearDisplay();
            print(_scrollText);
            _scrolling = false;
            _scrollComplete = true;
        } else {
            clearDisplay();
            print(_scrollText.substring(0, 11));
        }
        update();
    }
    
    void scroll(String text, int direction = SCROLL_LEFT, int speed = 150) {
        scroll(text.c_str(), direction, speed);
    }
    
    void stopScroll() {
        _scrolling = false;
        _scrollComplete = true;
    }
    
    bool isScrolling() {
        return _scrolling;
    }
    
    bool isScrollComplete() {
        return _scrollComplete;
    }
    
    void icon(const char* name, bool on) {
        // Direct bit positions - no string comparisons needed
        char c = name[0];
        if (c == 'C') {
            if (name[1] == 'H') _setBit(14, on);
            else if (name[2] == 'D') _setBit(35, on); // CD-IN
            else if (name[3] == 'L') _setBit(87, on); // CALL
        } else if (c == 'M') {
            _setBit(39, on); // MP3
        } else if (c == 'W') {
            _setBit(51, on); // WMA
        } else if (c == 'L') {
            _setBit(55, on); // LOUD
        } else if (c == 'S') {
            if (name[1] == 'T') _setBit(67, on);
            else if (name[2] == 'N') _setBit(83, on); // SCN
        } else if (c == 'U') {
            _setBit(71, on); // USB
        } else if (c == 'N') {
            _setBit(75, on); // NO-SIGNAL
        } else if (c == 'R') {
            if (name[1] == 'P') _setBit(103, on); // RPT
            else if (name[1] == 'D') _setBit(131, on); // RDM
        } else if (c == 'T') {
            if (name[1] == 'P') _setBit(119, on);
            else if (name[1] == 'A') _setBit(147, on);
        } else if (c == 'D') {
            _setBit(135, on); // DP
        } else if (c == 'B') {
            _setBit(151, on); // BT
        } else if (c == 'P') {
            _setBit(163, on); // PTY
        } else if (c == 'E') {
            _setBit(167, on); // EQ
        } else if (c == 'A') {
            _setBit(179, on); // AF
        } else if (c == ':') {
            _setBit(99, on); // COLON
        }
    }
    
    void symbol(const char* name, bool on) {
        icon(name, on);
    }
    
    void signal(int level) {
        if (level < 0) level = 0;
        if (level > 5) level = 5;
        _setBit(91, level >= 1);
        _setBit(107, level >= 2);
        _setBit(115, level >= 3);
        _setBit(139, level >= 4);
        _setBit(123, level >= 5);
    }
    
    void battery(int level) {
        if (level < 0) level = 0;
        if (level > 3) level = 3;
        _setBit(171, true);
        _setBit(59, level >= 1);
        _setBit(43, level >= 2);
        _setBit(187, level >= 3);
    }
    
    void cd(int level) {
        if (level < 0) level = 0;
        if (level > 3) level = 3;
        _setBit(19, true);
        _setBit(11, level >= 1);
        _setBit(15, level >= 2);
        _setBit(23, level >= 3);
    }
    
    void ch(int digit) {
        if (digit < 0) digit = 0;
        if (digit > 9) digit = 9;
        _writeSmallDigit(digit);
    }
    
    void radio(int channel, int frequency) {
        clear();
        icon("CH", true);
        ch(channel);
        
        char freqStr[12];
        sprintf(freqStr, "%d", frequency);
        print(freqStr);
    }
    
    void update() {
        if (_scrolling && !_scrollComplete) {
            if (millis() - _lastScrollUpdate >= _scrollSpeed) {
                _lastScrollUpdate = millis();
                
                int len = _scrollText.length();
                int maxLen = 11;
                
                clearDisplay();
                String displayText = "";
                
                if (_scrollDirection == SCROLL_LEFT) {
                    int startIdx = _scrollPosition;
                    int endIdx = min(startIdx + maxLen, len);
                    
                    for (int i = startIdx; i < endIdx; i++) {
                        displayText += _scrollText[i];
                    }
                    
                    while (displayText.length() < maxLen) {
                        displayText += " ";
                    }
                    
                    _scrollPosition++;
                    
                    if (_scrollPosition >= len) {
                        _scrolling = false;
                        _scrollComplete = true;
                        clearDisplay();
                        update();
                        return;
                    }
                } else {
                    int startIdx = max(0, len - maxLen - _scrollPosition);
                    int endIdx = min(startIdx + maxLen, len);
                    
                    for (int i = startIdx; i < endIdx; i++) {
                        displayText += _scrollText[i];
                    }
                    
                    while (displayText.length() < maxLen) {
                        displayText = " " + displayText;
                    }
                    
                    _scrollPosition++;
                    
                    if (_scrollPosition >= len + maxLen) {
                        _scrolling = false;
                        _scrollComplete = true;
                        clearDisplay();
                        update();
                        return;
                    }
                }
                
                print(displayText);
            }
        }
        
        // Send data to hardware
        digitalWrite(_clk, LOW);
        digitalWrite(_ce, LOW);
        delayMicroseconds(_busDelay);
        
        byte address = 0x80;
        for (int i = 0; i < 8; i++) {
            digitalWrite(_di, (address >> i) & 0x01);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, HIGH);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, LOW);
        }
        
        delayMicroseconds(_busDelay);
        digitalWrite(_ce, HIGH);
        delayMicroseconds(_busDelay);
        
        for (int i = 0; i < 204; i++) {
            digitalWrite(_di, _buffer[i]);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, HIGH);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, LOW);
        }
        
        byte controlBits[8] = {0, 0, 0, 0, 1, 0, 0, 0};
        for (int i = 0; i < 8; i++) {
            digitalWrite(_di, controlBits[i]);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, HIGH);
            delayMicroseconds(_busDelay);
            digitalWrite(_clk, LOW);
        }
        
        delayMicroseconds(_busDelay);
        digitalWrite(_ce, LOW);
        digitalWrite(_di, LOW);
    }
    
    void demo() {
        clear();
        print("HELLO");
        icon("MP3", true);
        battery(3);
        signal(5);
        update();
        delay(2000);
        
        clear();
        print("CRAZY?");
        icon("LOUD", true);
        update();
        delay(2000);
        
        clear();
        print("I WAS");
        icon("CD-IN", true);
        update();
        delay(2000);
        
        clear();
        print("CRAZY ONCE");
        icon("LOUD", true);
        icon("MP3", true);
        update();
        delay(3000);
        
        clear();
        radio(5, 1013);
        icon("ST", true);
        icon("TP", true);
        update();
        delay(3000);
        
        clear();
        battery(2);
        signal(3);
        print("BATTERY");
        update();
        delay(2000);
        
        clear();
        cd(3);
        print("CD PLAY");
        update();
        delay(2000);
        
        scroll("THIS WILL SCROLL OFF", SCROLL_LEFT, 150);
        while (isScrolling()) {
            update();
            delay(10);
        }
        delay(1000);
        
        clear();
        update();
    }
    
    void testPattern() {
        clear();
        for (int i = 0; i < 11; i++) {
            print("8", i);
        }
        icon("CH", true);
        icon("CD-IN", true);
        icon("MP3", true);
        icon("WMA", true);
        icon("LOUD", true);
        icon("ST", true);
        icon("USB", true);
        icon("NO-SIGNAL", true);
        icon("SCN", true);
        icon("CALL", true);
        icon("COLON", true);
        icon("RPT", true);
        icon("TP", true);
        icon("RDM", true);
        icon("DP", true);
        icon("TA", true);
        icon("BT", true);
        icon("PTY", true);
        icon("EQ", true);
        icon("AF", true);
        battery(3);
        signal(5);
        cd(3);
        ch(8);
        update();
    }
    
private:
    uint8_t _clk, _di, _ce, _inh;
    int _busDelay;
    byte _buffer[204];
    
    bool _scrolling;
    bool _scrollComplete;
    unsigned long _lastScrollUpdate;
    int _scrollSpeed;
    int _scrollDirection;
    int _scrollPosition;
    String _scrollText;
    
    void _setBit(int bitIndex, bool value) {
        if (bitIndex >= 0 && bitIndex < 204) {
            _buffer[bitIndex] = value ? 1 : 0;
        }
    }
    
    bool _getBit(int bitIndex) {
        if (bitIndex >= 0 && bitIndex < 204) {
            return _buffer[bitIndex] == 1;
        }
        return false;
    }
    
    void _clearDigit(int position) {
        if (position > 10) return;
        int digitStartBit = 20 + (position * 16);
        memset(&_buffer[digitStartBit], 0, 16);
    }
    
    void _writeChar14Seg(char c, int position) {
        if (position > 10) return;
        
        #define SEG_E   (1 << 0)
        #define SEG_G1  (1 << 1)
        #define SEG_F   (1 << 2)
        #define SEG_L   (1 << 4)
        #define SEG_JM  (1 << 5)
        #define SEG_H   (1 << 6)
        #define SEG_D   (1 << 8)
        #define SEG_N   (1 << 9)
        #define SEG_K   (1 << 10)
        #define SEG_A   (1 << 11)
        #define SEG_C   (1 << 12)
        #define SEG_G2  (1 << 13)
        #define SEG_B   (1 << 14)
        
        static const uint16_t FONT_TABLE[36] PROGMEM = {
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),
            (SEG_B | SEG_C),
            (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G1 | SEG_G2),
            (SEG_B | SEG_C | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G2 | SEG_JM),
            (SEG_A | SEG_D | SEG_E | SEG_F),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_JM),
            (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G1),
            (SEG_A | SEG_E | SEG_F | SEG_G1),
            (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G2),
            (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_D | SEG_JM),
            (SEG_B | SEG_C | SEG_D | SEG_E),
            (SEG_E | SEG_F | SEG_G1 | SEG_K | SEG_N),
            (SEG_D | SEG_E | SEG_F),
            (SEG_B | SEG_C | SEG_E | SEG_F | SEG_H | SEG_K),
            (SEG_B | SEG_C | SEG_E | SEG_F | SEG_H | SEG_N),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),
            (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_L),
            (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G1 | SEG_G2 | SEG_N),
            (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),
            (SEG_A | SEG_JM),
            (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),
            (SEG_E | SEG_F | SEG_L | SEG_K),
            (SEG_B | SEG_C | SEG_E | SEG_F | SEG_N | SEG_L),
            (SEG_H | SEG_K | SEG_N | SEG_L),
            (SEG_H | SEG_K | SEG_JM),
            (SEG_A | SEG_D | SEG_K | SEG_L)
        };
        
        uint16_t segments = 0;
        
        if (c >= '0' && c <= '9') {
            segments = pgm_read_word(&FONT_TABLE[c - '0']);
        } else if (c >= 'A' && c <= 'Z') {
            segments = pgm_read_word(&FONT_TABLE[c - 'A' + 10]);
        } else if (c >= 'a' && c <= 'z') {
            segments = pgm_read_word(&FONT_TABLE[c - 'a' + 10]);
        } else {
            segments = 0;
        }
        
        int digitStartBit = 20 + (position * 16);
        _clearDigit(position);
        
        if (segments & SEG_E)   _buffer[digitStartBit + 0]  = 1;
        if (segments & SEG_G1)  _buffer[digitStartBit + 1]  = 1;
        if (segments & SEG_F)   _buffer[digitStartBit + 2]  = 1;
        if (segments & SEG_L)   _buffer[digitStartBit + 4]  = 1;
        if (segments & SEG_JM)  _buffer[digitStartBit + 5]  = 1;
        if (segments & SEG_H)   _buffer[digitStartBit + 6]  = 1;
        if (segments & SEG_D)   _buffer[digitStartBit + 8]  = 1;
        if (segments & SEG_N)   _buffer[digitStartBit + 9]  = 1;
        if (segments & SEG_K)   _buffer[digitStartBit + 10] = 1;
        if (segments & SEG_A)   _buffer[digitStartBit + 11] = 1;
        if (segments & SEG_C)   _buffer[digitStartBit + 12] = 1;
        if (segments & SEG_G2)  _buffer[digitStartBit + 13] = 1;
        if (segments & SEG_B)   _buffer[digitStartBit + 14] = 1;
    }
    
    void _writeSmallDigit(uint8_t digit) {
        static const uint8_t smallFont[10] PROGMEM = {
            0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
            0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111
        };
        
        uint8_t seg = pgm_read_byte(&smallFont[digit % 10]);
        _setBit(13, seg & 0x01);
        _setBit(18, seg & 0x02);
        _setBit(16, seg & 0x04);
        _setBit(12, seg & 0x08);
        _setBit(8,  seg & 0x10);
        _setBit(10, seg & 0x20);
        _setBit(9,  seg & 0x40);
        _setBit(17, seg & 0x40);
    }
};

#endif