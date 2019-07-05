/************************************************************
  Desk Timer
  
  This sketch implements a simple Pomodoro Timer that 
    sits on my desk. The particular use case is to be able
    to indicate to coworkers that I am not interruptable for 
    20 minutes.  
   
  As the timer counts down the digits are red (not interruptable).
  When the timer finishes the display will display a 
    Game of Life simulation in green.

  To start a new 20 minute timer use the reset button.
 ************************************************************/

#include <SoftwareSerial.h>
#include <SparkFunESP8266WiFi.h>

#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 6
#define LINE_COUNT 5
#define LED_COUNT 40
#define GENERATION_HISTORY 10

#define BRIGHTNESS 16
#define ERR_BRIGHTNESS 32

#define GLYPH_BLANK 0x0
// 5x3 grid Glyphs for numerals
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
// 5x4 grid Glyphs for numerals
//  The pixels are represented by this number's bits
//    AAAA
//    BBBB
//    CCCC     EEEE DDDD CCCC BBBB AAAA
//    DDDD
//    EEEE
// e.g.  GLYPH_1
//    --1-
//    -11-
//    --1-     0010 0010 0010 0110 0010
//    --1-  0x    2    2    2    6    2
//    --1-
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

byte specialRepeater[LINE_COUNT];

unsigned long glyphs[10];
byte grid[LINE_COUNT];
byte generation[GENERATION_HISTORY][LINE_COUNT];
byte currentGeneration;
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
//  seconds = 1204; // jump right to Game of Life
  
  leds.begin();  // Call this to start up the LED strip.
  clearLEDs();   // This function, defined below, turns all LEDs off...
  leds.show();   // ...but the LEDs don't actually update until you call this.

  for (int i = 0; i < LINE_COUNT; i++) {
    grid[i] = 0;
  }
  currentGeneration = 1;
  for (int g = 0; g < GENERATION_HISTORY; g++) {
    for (int i = 0; i < LINE_COUNT; i++) {
      generation[g][i] = 0;
    }
  }

  specialRepeater[0] = 0x00;
  specialRepeater[1] = 0x70;
  specialRepeater[2] = 0x50;
  specialRepeater[3] = 0x70;
  specialRepeater[4] = 0x00;

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
    } else {
      int live = golStep();
      // if a gol cycle or stable population 
      //  was detected then restart with a 
      //  random population
      if (live == 0) {
        seconds = 1204;
      }
      // just wrap around to keep the
      //  seconds counter from overflowing
      if (seconds == 1300) {
        seconds = 1205;
      }
    }
    golLEDs();
  }
}

void golSetup()
{
  for (int i = 0; i < LINE_COUNT; i++) {
      grid[i] = random(256);
  }
  currentGeneration = 1;
  for (int g = 0; g < GENERATION_HISTORY; g++) {
    for (int i = 0; i < LINE_COUNT; i++) {
      generation[g][i] = 0;
    }
  }
}

int golStep()
{
  // set up memory for next generation
  currentGeneration++;
  if (currentGeneration >= GENERATION_HISTORY) {
    currentGeneration = 1; //  generation 0 is special
  }
  for (int i = 0; i < LINE_COUNT; i++) {
    generation[currentGeneration][i] = 0;
  }

  // calculation next generation
  for (int pt = 0; pt < LED_COUNT; pt++) {
    byte xmask = 1 << (pt % 8); 
    byte y = pt / 8;

    int count = 0;
    for (int dir = 0; dir < 8; dir++) {
        int pp = pointInDir(pt, dir);
        
        byte dxmask = 1 << (pp % 8); 
        byte dy = pp / 8;
        if (pp > -1 && (grid[dy] & dxmask) == dxmask) {
          count++;
        }
    }
    if (((grid[y] & xmask) == xmask && count == 2) || count == 3) {
      generation[currentGeneration][y] = generation[currentGeneration][y] | xmask;
    }
  }
  
  // copy next generation into display
  for (int i = 0; i < LINE_COUNT; i++) {
    grid[i] = generation[currentGeneration][i];
  }

  return golCheckForRepeaters();
}

int golCheckForRepeaters() {
  // check for an identical generation
  for (int g = 0; g < GENERATION_HISTORY; g++) {
    if (g != currentGeneration) {
      for (int i = 0; i < LINE_COUNT; i++) {
        if (generation[g][i] != generation[currentGeneration][i]) {
          break; // go to next generation
        } else {
          if (i == LINE_COUNT - 1) {
            return 0;   // found a complete match
          }
        }
      }
    }
  }

  // check for special repeaters
  for (int i = 0; i < LINE_COUNT; i++) {
    if (specialRepeater[i] != generation[currentGeneration][i]) {
      break; // no match
    } else {
      if (i == LINE_COUNT - 1) {
        // found a complete match so add this to the special 0 generation
        for (int j = 0; j < LINE_COUNT; j++) {
          generation[0][j] = specialRepeater[j];
        }
      }
    }
  }
  
  return 1; 
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
    for (int line=0; line<LINE_COUNT; line++) 
    {
      setLineOfLEDs(line, grid[line], leds.Color(0, 15, 0));
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

    const unsigned long F = 0xF;
    // set colors
    for (int line=0; line<LINE_COUNT; line++) 
    {
        unsigned long mask = F << (line * 4);
        unsigned long screen = (((glyphA & mask) >> (line * 4)) << 4) | ((glyphB & mask) >> (line * 4)); 
        setLineOfLEDs(line, screen, leds.Color(15, 0, 0));
    }  
    leds.show();
}

void setLineOfLEDs(int line, byte screen, long color) {
    for(int i=line*8; i<(line+1)*8; i++) {
        byte b = 1 << 7 - (i % 8);
        if ((screen & b) == b) {
            leds.setPixelColor(i, color);
        } else {
            leds.setPixelColor(i, 0);
        }
    }
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
