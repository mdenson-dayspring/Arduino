/************************************************************
  ping-lights
  Modified from SparkFun ESP8266 AT library - Ping Demo
  This pings a destination and will display on a NeoPixel LED
  array.
 ************************************************************/

#include <SoftwareSerial.h>
#include <SparkFunESP8266WiFi.h>

#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 6
#define LED_COUNT 40

#define BRIGHTNESS 16
#define ERR_BRIGHTNESS 32

#define GLYPH_BLANK 0x0
//#define GLYPH_0 0xEAAAE
//#define GLYPH_1 0x444C4
//#define GLYPH_2 0xE842C
//#define GLYPH_3 0xC242C
//#define GLYPH_4 0x22EAA
//#define GLYPH_5 0xC2C8E
//#define GLYPH_6 0x4AC86
//#define GLYPH_7 0x4442E
//#define GLYPH_8 0x4A4A4
//#define GLYPH_9 0xC26A4
#define GLYPH_0 0x69DB6
#define GLYPH_1 0x22262
#define GLYPH_2 0xF4296
#define GLYPH_3 0xE161E
#define GLYPH_4 0x11F99  
#define GLYPH_5 0xE1E8F
#define GLYPH_6 0x69E86
#define GLYPH_7 0x2221F
#define GLYPH_8 0x69696
#define GLYPH_9 0x61796

// Create an instance of the Adafruit_NeoPixel class called "leds".
// That'll be what we refer to from here on...
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

unsigned long glyphs[10];
int grid[LED_COUNT];
int seconds;

void setup() 
{
  Serial.begin(9600);

  glyphs[0] = GLYPH_0;
  glyphs[1] = GLYPH_1;
  glyphs[2] = GLYPH_2;
  glyphs[3] = GLYPH_3;
  glyphs[4] = GLYPH_4;
  glyphs[5] = GLYPH_5;
  glyphs[6] = GLYPH_6;
  glyphs[7] = GLYPH_7;
  glyphs[8] = GLYPH_8;
  glyphs[9] = GLYPH_9;
  
  seconds = 0;
  
  leds.begin();  // Call this to start up the LED strip.
  clearLEDs();   // This function, defined below, turns all LEDs off...
  leds.show();   // ...but the LEDs don't actually update until you call this.

  for (int i = 0; i < LED_COUNT; i++) {
    grid[i] = (i+1) * 30;
  }

  int zero = analogRead(0);
  randomSeed(zero);

}

void loop() 
{
  delay(1000);
  
  seconds++;

  if (seconds < 1205) {
    countdownLEDs();
  } else {
    if (seconds == 1205) {
      golSetup();
    } else if (seconds > 1205) {
      golLEDs();
      int live = golStep();
      if (live == 0) {
        seconds = 1204;
      }
      if (seconds == 1300) {
        seconds = 1210;
      }
    }
  }
}

void golSetup()
{
  for (int i = 0; i < LED_COUNT; i++) {
    if (random(300) < 100) {
//    if (i == 8 || i == 16 || i == 24 || i == 7 || i == 15 || i == 23 || i == 31) {
      grid[i] = 1;
    } else {
      grid[i] = 0;
    }
  }
}

int golStep()
{
  int tmp[LED_COUNT];

  for (int pt = 0; pt < LED_COUNT; pt++) {
      int count = 0;
      for (int dir = 0; dir < 8; dir++) {
          int pp = pointInDir(pt, dir);
          if (pp > -1 && grid[pp] == 1) {
            count++;
          }
      }
      if ((grid[pt] == 1 && count == 2) || count == 3) {
        tmp[pt] = 1;
      } else {
        tmp[pt] = 0;
      }
  }
  
  int diffs = 0;
  for (int i = 0; i < LED_COUNT; i++) {
    if (tmp[i] != grid[i]) {
      diffs++;
      grid[i] = tmp[i];
    }
  }
  if (diffs > 0) {
    return 1; 
  } else {
    return 0;
  }
}

int pointInDir(int pt, int dir) {
  int x = pt % 8; 
  int y = pt / 8;

  int dx = 0;
  int dy = 0;
  if (dir == 0) {
    dx = -1; dy = -1;
  } else if (dir == 1) {
    dx = 0; dy = -1;
  } else if (dir == 2) {
    dx = 1; dy = -1;
  } else if (dir == 3) {
    dx = 1; dy = 0;
  } else if (dir == 4) {
    dx = 1; dy = 1;
  } else if (dir == 5) {
    dx = 0; dy = 1;
  } else if (dir == 6) {
    dx = -1; dy = 1;
  } else {
    dx = -1; dy = 0;
  }

  x = x + dx;
  y = y + dy;

  if (x < 0 || 7 < x || 
      y < 0 || 4 < y) {
    return -1;
  } else {
    return (y * 8) + x;;
  }
}

void golLEDs()
{
    // set colors
    for (int i = 0; i < LED_COUNT; i++) 
    {
      if (grid[i] == 1) 
      {
        leds.setPixelColor(i, leds.Color(0, 15, 0));
      }
      else
      {
        leds.setPixelColor(i, 0);
      }
    }  
    leds.show();
}

void countdownLEDs()
{
    int minutes = 20 - (seconds / 60);
    unsigned long glyphA = glyphs[minutes / 10];
    unsigned long glyphB = glyphs[minutes % 10];
    if (minutes / 10 == 0) {
      glyphA = GLYPH_BLANK;  
    }

    // set colors
    unsigned long mask = 0xF;
    int line = 0;
    byte screen = (((glyphA & mask) >> (line * 4)) << 4) + ((glyphB & mask) >> (line * 4)); 
    for (int i = 0; i < LED_COUNT; i++) 
    {
        if (i / 8 != line) {
          line = i / 8;
          mask = mask << 4;
          screen =  (((glyphA & mask) >> (line * 4)) << 4) + ((glyphB & mask) >> (line * 4)); 
        }
        byte b = 1 << 7 - (i % 8);
        if ((screen & b) == b) {
            leds.setPixelColor(i, leds.Color(15, 0, 0));
        } else {
            leds.setPixelColor(i, 0);
        }

//      if (grid[i] > seconds) 
//      {
//        leds.setPixelColor(i, leds.Color(15, 0, 0));
//      }
//      else
//      {
//        leds.setPixelColor(i, 0);
//      }
    }  
    leds.show();
}

// Sets all LEDs to off, but DOES NOT update the display;
// call leds.show() to actually turn them off after this.
void clearLEDs()
{
  for (int i=0; i<LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
  }
}

void errorLoop()
{
  for (;;) {    
  }
}
