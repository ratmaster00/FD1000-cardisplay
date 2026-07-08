#include "HyundaiGetzDisplay.h"

HyundaiGetzDisplay display(2, 3, 4, 5);

void setup() {
    Serial.begin(9600);
    display.begin();
    
    Serial.println("=== Hyundai Getz Display ===");
    Serial.println("Commands: text, scroll, scrollr, stop, clear");
    Serial.println("  symbols, radio, battery, signal, cd, ch");
    Serial.println("  icon <name> <0/1>, demo, test, help");
    Serial.println();
    Serial.println("Ready!");
    
    display.print("READY");
    display.update();
}

void loop() {
    display.update();
    
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() == 0) return;
        
        if (input.startsWith("text ") || input.startsWith("print ")) {
            String text = input.substring(input.indexOf(' ') + 1);
            if (text.length() > 11) text = text.substring(0, 11);
            display.stopScroll();
            display.clear();
            display.print(text);
            display.update();
            Serial.print("Displayed: ");
            Serial.println(text);
        }
        else if (input.startsWith("scroll ")) {
            String text = input.substring(7);
            if (text.length() > 0) {
                display.scroll(text, SCROLL_LEFT, 150);
                Serial.print("Scrolling: ");
                Serial.println(text);
            }
        }
        else if (input.startsWith("scrollr ")) {
            String text = input.substring(8);
            if (text.length() > 0) {
                display.scroll(text, SCROLL_RIGHT, 150);
                Serial.print("Scrolling right: ");
                Serial.println(text);
            }
        }
        else if (input == "stop") {
            display.stopScroll();
            Serial.println("Stopped");
        }
        else if (input == "clear") {
            display.stopScroll();
            display.clear();
            display.update();
            Serial.println("Cleared");
        }
        else if (input == "symbols") {
            display.stopScroll();
            display.clear();
            display.icon("CH", true);
            display.icon("CD-IN", true);
            display.icon("MP3", true);
            display.icon("WMA", true);
            display.icon("LOUD", true);
            display.icon("ST", true);
            display.icon("USB", true);
            display.icon("NO-SIGNAL", true);
            display.icon("SCN", true);
            display.icon("CALL", true);
            display.icon("COLON", true);
            display.icon("RPT", true);
            display.icon("TP", true);
            display.icon("RDM", true);
            display.icon("DP", true);
            display.icon("TA", true);
            display.icon("BT", true);
            display.icon("PTY", true);
            display.icon("EQ", true);
            display.icon("AF", true);
            display.battery(3);
            display.signal(5);
            display.cd(3);
            display.ch(8);
            display.update();
            Serial.println("All symbols ON");
        }
        else if (input.startsWith("radio ")) {
            int ch, freq;
            if (sscanf(input.c_str() + 6, "%d %d", &ch, &freq) == 2) {
                if (ch >= 1 && ch <= 9 && freq >= 0) {
                    display.stopScroll();
                    display.clear();
                    display.radio(ch, freq);
                    display.icon("ST", true);
                    display.icon("TP", true);
                    display.update();
                    Serial.print("Radio: CH");
                    Serial.print(ch);
                    Serial.print(" ");
                    Serial.println(freq);
                }
            }
        }
        else if (input.startsWith("battery ")) {
            int level = input.substring(8).toInt();
            if (level >= 0 && level <= 3) {
                display.battery(level);
                display.update();
                Serial.print("Battery: ");
                Serial.println(level);
            }
        }
        else if (input.startsWith("signal ")) {
            int level = input.substring(7).toInt();
            if (level >= 0 && level <= 5) {
                display.signal(level);
                display.update();
                Serial.print("Signal: ");
                Serial.println(level);
            }
        }
        else if (input.startsWith("cd ")) {
            int level = input.substring(3).toInt();
            if (level >= 0 && level <= 3) {
                display.cd(level);
                display.update();
                Serial.print("CD: ");
                Serial.println(level);
            }
        }
        else if (input.startsWith("ch ")) {
            int digit = input.substring(3).toInt();
            if (digit >= 0 && digit <= 9) {
                display.ch(digit);
                display.update();
                Serial.print("Small digit: ");
                Serial.println(digit);
            }
        }
        else if (input.startsWith("icon ")) {
            String rest = input.substring(5);
            int spacePos = rest.indexOf(' ');
            if (spacePos > 0) {
                String name = rest.substring(0, spacePos);
                int value = rest.substring(spacePos + 1).toInt();
                if (value >= 0 && value <= 1) {
                    display.icon(name.c_str(), value == 1);
                    display.update();
                    Serial.print(name);
                    Serial.println(value ? " ON" : " OFF");
                }
            }
        }
        else if (input == "demo") {
            display.stopScroll();
            demo();
            Serial.println("Demo done");
        }
        else if (input == "test") {
            display.stopScroll();
            testPattern();
            Serial.println("Test pattern");
        }
        else if (input == "help") {
            Serial.println("Commands: text, scroll, scrollr, stop, clear");
            Serial.println("  symbols, radio, battery, signal, cd, ch");
            Serial.println("  icon <name> <0/1>, demo, test, help");
        }
        else {
            if (input.length() > 11) input = input.substring(0, 11);
            display.stopScroll();
            display.clear();
            display.print(input);
            display.update();
            Serial.print("Displayed: ");
            Serial.println(input);
        }
    }
}

void demo() {
    display.clear();
    display.print("HELLO");
    display.icon("MP3", true);
    display.battery(3);
    display.signal(5);
    display.update();
    delay(2000);
    
    display.clear();
    display.print("CRAZY?");
    display.icon("LOUD", true);
    display.update();
    delay(2000);
    
    display.clear();
    display.print("I WAS");
    display.icon("CD-IN", true);
    display.update();
    delay(2000);
    
    display.clear();
    display.print("CRAZY ONCE");
    display.icon("LOUD", true);
    display.icon("MP3", true);
    display.update();
    delay(3000);
    
    display.clear();
    display.radio(5, 1013);
    display.icon("ST", true);
    display.icon("TP", true);
    display.update();
    delay(3000);
    
    display.clear();
    display.battery(2);
    display.signal(3);
    display.print("BATTERY");
    display.update();
    delay(2000);
    
    display.clear();
    display.cd(3);
    display.print("CD PLAY");
    display.update();
    delay(2000);
    
    display.scroll("THIS WILL SCROLL OFF", SCROLL_LEFT, 150);
    while (display.isScrolling()) {
        display.update();
        delay(10);
    }
    delay(1000);
    
    display.clear();
    display.update();
}

void testPattern() {
    display.clear();
    for (int i = 0; i < 11; i++) {
        display.print("8", i);
    }
    display.icon("CH", true);
    display.icon("CD-IN", true);
    display.icon("MP3", true);
    display.icon("WMA", true);
    display.icon("LOUD", true);
    display.icon("ST", true);
    display.icon("USB", true);
    display.icon("NO-SIGNAL", true);
    display.icon("SCN", true);
    display.icon("CALL", true);
    display.icon("COLON", true);
    display.icon("RPT", true);
    display.icon("TP", true);
    display.icon("RDM", true);
    display.icon("DP", true);
    display.icon("TA", true);
    display.icon("BT", true);
    display.icon("PTY", true);
    display.icon("EQ", true);
    display.icon("AF", true);
    display.battery(3);
    display.signal(5);
    display.cd(3);
    display.ch(8);
    display.update();
}