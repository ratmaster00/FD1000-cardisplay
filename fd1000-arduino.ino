#include <Arduino.h>

// Pin Definitions
const int LCD_CLK = 2; 
const int LCD_DI  = 3; 
const int LCD_CE  = 4; 
const int LCD_INH = 5; 

const int BUS_DELAY = 50; 
byte displayBuffer[204];

// --- 14-SEGMENT BIT FLAG DEFINITIONS ---
// Based on your hardware mapping:
// Position: 0=E, 1=G1, 2=F, 3=EMPTY, 4=L, 5=JM, 6=H, 7=EMPTY, 
//           8=D, 9=N, 10=K, 11=A, 12=C, 13=G2, 14=B, 15=EMPTY
#define SEG_E   (1 << 0)
#define SEG_G1  (1 << 1)
#define SEG_F   (1 << 2)
#define SEG_N   (1 << 3)
#define SEG_JM  (1 << 4)  // J and M are physically tied together on your glass
#define SEG_H   (1 << 5)
#define SEG_D   (1 << 6)
#define SEG_L   (1 << 7)
#define SEG_K   (1 << 8)
#define SEG_A   (1 << 9)
#define SEG_C   (1 << 10)
#define SEG_G2  (1 << 11)
#define SEG_B   (1 << 12)

// --- 14-SEGMENT FONT MAP ---
// Full alphanumeric font with G1 and G2 independent
const uint16_t alphaFont14Seg[] PROGMEM = {
  // Numbers 0-9 (ASCII 48-57)
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_K | SEG_N),                 // 0
  (SEG_B | SEG_C),                                                                 // 1
  (SEG_A | SEG_B | SEG_D | SEG_E | SEG_G1 | SEG_G2),                               // 2
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G1 | SEG_G2),                               // 3
  (SEG_B | SEG_C | SEG_F | SEG_G1 | SEG_G2),                                       // 4
  (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),                               // 5
  (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G1 | SEG_G2),                       // 6
  (SEG_A | SEG_B | SEG_C),                                                         // 7
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G1 | SEG_G2),               // 8
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),                       // 9

  // Letters A-Z (ASCII 65-90) - 14-segment optimized
  (SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G1 | SEG_G2),                       // A
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_G2 | SEG_JM),                               // B
  (SEG_A | SEG_D | SEG_E | SEG_F),                                                 // C
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_JM),                                        // D
  (SEG_A | SEG_D | SEG_E | SEG_F | SEG_G1),                                        // E
  (SEG_A | SEG_E | SEG_F | SEG_G1),                                                // F
  (SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G2),                                // G
  (SEG_B | SEG_C | SEG_E | SEG_F | SEG_G1 | SEG_G2),                               // H
  (SEG_A | SEG_D | SEG_JM),                                                        // I
  (SEG_B | SEG_C | SEG_D | SEG_E),                                                 // J
  (SEG_E | SEG_F | SEG_G1 | SEG_K | SEG_N),                                        // K
  (SEG_D | SEG_E | SEG_F),                                                         // L
  (SEG_B | SEG_C | SEG_E | SEG_F | SEG_H | SEG_K),                                 // M
  (SEG_B | SEG_C | SEG_E | SEG_F | SEG_H | SEG_N),                                 // N
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),                                 // O
  (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G1 | SEG_G2),                               // P
  (SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_L),                         // Q
  (SEG_A | SEG_B | SEG_E | SEG_F | SEG_G1 | SEG_G2 | SEG_N),                       // R
  (SEG_A | SEG_C | SEG_D | SEG_F | SEG_G1 | SEG_G2),                               // S
  (SEG_A | SEG_JM),                                                                // T
  (SEG_B | SEG_C | SEG_D | SEG_E | SEG_F),                                         // U
  (SEG_E | SEG_F | SEG_L | SEG_K),                                                 // V
  (SEG_B | SEG_C | SEG_E | SEG_F | SEG_N | SEG_L),                                 // W
  (SEG_H | SEG_K | SEG_N | SEG_L),                                                 // X
  (SEG_H | SEG_K | SEG_JM),                                                        // Y
  (SEG_A | SEG_D | SEG_K | SEG_L)                                                  // Z
};

// --- WRITE 14-SEGMENT CHARACTER ---
void writeChar14Seg(char c, int position) {
  if (position > 10) return;
  
  uint16_t segments = 0;
  
  if (c >= '0' && c <= '9') {
    segments = pgm_read_word(&alphaFont14Seg[c - '0']);
  } else if (c >= 'A' && c <= 'Z') {
    segments = pgm_read_word(&alphaFont14Seg[c - 'A' + 10]);
  } else if (c >= 'a' && c <= 'z') {
    segments = pgm_read_word(&alphaFont14Seg[c - 'a' + 10]); // Convert lowercase
  } else {
    segments = 0; // Space or unsupported
  }
  
  int digitStartBit = 20 + (position * 16);
  
  // Clear the digit slot
  for (int i = 0; i < 16; i++) {
    if (digitStartBit + i < 204) {
      displayBuffer[digitStartBit + i] = 0;
    }
  }
  
  // Map 14-segment flags to hardware positions
  if (segments & SEG_E)   displayBuffer[digitStartBit + 0]  = 1;
  if (segments & SEG_G1)  displayBuffer[digitStartBit + 1]  = 1;
  if (segments & SEG_F)   displayBuffer[digitStartBit + 2]  = 1;
  if (segments & SEG_L)   displayBuffer[digitStartBit + 4]  = 1;
  if (segments & SEG_JM)  displayBuffer[digitStartBit + 5]  = 1;
  if (segments & SEG_H)   displayBuffer[digitStartBit + 6]  = 1;
  if (segments & SEG_D)   displayBuffer[digitStartBit + 8]  = 1;
  if (segments & SEG_N)   displayBuffer[digitStartBit + 9]  = 1;
  if (segments & SEG_K)   displayBuffer[digitStartBit + 10] = 1;
  if (segments & SEG_A)   displayBuffer[digitStartBit + 11] = 1;
  if (segments & SEG_C)   displayBuffer[digitStartBit + 12] = 1;
  if (segments & SEG_G2)  displayBuffer[digitStartBit + 13] = 1;
  if (segments & SEG_B)   displayBuffer[digitStartBit + 14] = 1;
}

// --- CLEAR ALL DIGITS ---
void clearDigits() {
  for (int pos = 0; pos < 11; pos++) {
    int digitStartBit = 20 + (pos * 16);
    for (int i = 0; i < 16; i++) {
      if (digitStartBit + i < 204) {
        displayBuffer[digitStartBit + i] = 0;
      }
    }
  }
}

// --- DISPLAY REFRESH ENGINE ---
void updateDisplay() {
  digitalWrite(LCD_CLK, LOW);
  digitalWrite(LCD_CE, LOW);
  delayMicroseconds(BUS_DELAY); 

  byte address = 0x80;
  for (int i = 0; i < 8; i++) {
    digitalWrite(LCD_DI, (address >> i) & 0x01);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, HIGH);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, LOW);
  }
  
  delayMicroseconds(BUS_DELAY);
  digitalWrite(LCD_CE, HIGH);
  delayMicroseconds(BUS_DELAY);

  for (int i = 0; i < 204; i++) {
    digitalWrite(LCD_DI, displayBuffer[i]);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, HIGH);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, LOW);
  }

  byte controlBits[8] = {0, 0, 0, 0, 1, 0, 0, 0};
  for (int i = 0; i < 8; i++) {
    digitalWrite(LCD_DI, controlBits[i]);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, HIGH);
    delayMicroseconds(BUS_DELAY);
    digitalWrite(LCD_CLK, LOW);
  }

  delayMicroseconds(BUS_DELAY);
  digitalWrite(LCD_CE, LOW);
  digitalWrite(LCD_DI, LOW);
}

// --- PRINT STRING TO DISPLAY ---
void printString(const char* text) {
  clearDigits();
  
  int len = strlen(text);
  if (len > 11) len = 11;
  
  for (int i = 0; i < len; i++) {
    char c = text[i];
    // Convert to uppercase
    if (c >= 'a' && c <= 'z') {
      c = c - 32;
    }
    writeChar14Seg(c, i);
  }
  
  updateDisplay();
}

// --- SPINNER DEMO ---
void drawSpinner(int frame) {
  clearDigits();
  
  // The 6 border segments in sequence
  uint8_t spinnerPatterns[6] = {
    SEG_A,    // 0: Top
    SEG_B,    // 1: Top-Right
    SEG_C,    // 2: Bottom-Right
    SEG_D,    // 3: Bottom
    SEG_E,    // 4: Bottom-Left
    SEG_F     // 5: Top-Left
  };
  
  uint16_t activeSegments = spinnerPatterns[frame % 6];
  
  // Apply to all 11 digits
  for (int pos = 0; pos < 11; pos++) {
    int digitStartBit = 20 + (pos * 16);
    
    // Clear the digit slot
    for (int i = 0; i < 16; i++) {
      if (digitStartBit + i < 204) {
        displayBuffer[digitStartBit + i] = 0;
      }
    }
    
    // Map the active segment
    if (activeSegments & SEG_E)  displayBuffer[digitStartBit + 0]  = 1;
    if (activeSegments & SEG_F)  displayBuffer[digitStartBit + 2]  = 1;
    if (activeSegments & SEG_D)  displayBuffer[digitStartBit + 8]  = 1;
    if (activeSegments & SEG_A)  displayBuffer[digitStartBit + 11] = 1;
    if (activeSegments & SEG_C)  displayBuffer[digitStartBit + 12] = 1;
    if (activeSegments & SEG_B)  displayBuffer[digitStartBit + 14] = 1;
  }
  
  updateDisplay();
}

void setup() {
  pinMode(LCD_CLK, OUTPUT);
  pinMode(LCD_DI,  OUTPUT);
  pinMode(LCD_CE,  OUTPUT);
  pinMode(LCD_INH, OUTPUT);

  digitalWrite(LCD_CLK, LOW);
  digitalWrite(LCD_DI,  LOW);
  digitalWrite(LCD_CE,  LOW);
  digitalWrite(LCD_INH, HIGH); 

  for (int i = 0; i < 204; i++) displayBuffer[i] = 0;
  updateDisplay();
  
  Serial.begin(9600);
  Serial.println("14-Segment Display Ready!");
  Serial.println("Commands:");
  Serial.println("  'spinner' - Show spinner animation");
  Serial.println("  Any text - Display it on the screen");
  Serial.println("  (max 11 chars, auto-uppercase)");
  Serial.println();
}

int spinnerFrame = 0;
unsigned long lastSpinnerUpdate = 0;
bool spinnerMode = false;
String lastDisplayedText = "";

void loop() {
  // Handle spinner mode
  if (spinnerMode) {
    if (millis() - lastSpinnerUpdate >= 150) {
      lastSpinnerUpdate = millis();
      drawSpinner(spinnerFrame);
      spinnerFrame++;
      if (spinnerFrame >= 6) spinnerFrame = 0;
    }
    return;
  }
  
  // Handle serial input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.length() > 0) {
      if (input == "spinner") {
        spinnerMode = true;
        spinnerFrame = 0;
        lastSpinnerUpdate = millis();
        Serial.println("Spinner mode activated! Type 'stop' to return to text mode.");
      } else if (input == "stop") {
        spinnerMode = false;
        // Redisplay last text
        if (lastDisplayedText.length() > 0) {
          printString(lastDisplayedText.c_str());
        } else {
          clearDigits();
          updateDisplay();
        }
        Serial.println("Text mode restored.");
      } else {
        // Limit to 11 characters
        if (input.length() > 11) {
          input = input.substring(0, 11);
        }
        lastDisplayedText = input;
        printString(input.c_str());
        Serial.print("Displayed: ");
        Serial.println(input);
      }
    }
  }
}