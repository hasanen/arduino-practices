#include <FastLED.h>#include <SPI.h>
#include "U8glib.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


#define ROTARY_LEFT_PIN 12
#define ROTARY_RIGHT_PIN 11
#define ROTARY_BUTTON_PIN 10

#define BRIGHTNESS_STEP 5
#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 100
int lastLeftState;
int lastRightState;
int brightness = BRIGHTNESS_MAX / 2;
int buttonLastState;

#define DATA_PIN    5
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    5
CRGB leds[NUM_LEDS];

const char *patternNames[] = {"rainbow", "rainbowWithGlitter", "confetti", "sinelon", "juggle", "bpm"};


#define FRAMES_PER_SECOND  120

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);

void setup() {
  Serial.begin (9600);
  Serial.println("=== SETUP START ===");
  delay(3000); // 3 second delay for recovery


  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(brightness);

  pinMode(ROTARY_LEFT_PIN, INPUT);
  pinMode(ROTARY_RIGHT_PIN, INPUT);
  pinMode(ROTARY_BUTTON_PIN, INPUT);

  lastLeftState = digitalRead(ROTARY_LEFT_PIN);
  lastRightState = digitalRead(ROTARY_RIGHT_PIN);
  buttonLastState = digitalRead(ROTARY_BUTTON_PIN);

  Serial.println("=== SETUP DONE ===");
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{

  brightnessSelection();
  buttonListener();
  u8g.firstPage();
  do {
    updateDisplay();
  } while ( u8g.nextPage() );

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  FastLED.setBrightness(brightness);
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  //  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS - 1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16( i + 7, 0, NUM_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
void brightnessSelection() {
  int rightState = digitalRead(ROTARY_RIGHT_PIN);
  int leftState = digitalRead(ROTARY_LEFT_PIN);
  if (leftState != lastLeftState) {
    if (rotateLeft() ) {
      if (BRIGHTNESS_MIN <= (brightness - BRIGHTNESS_STEP)) {
        brightness -= BRIGHTNESS_STEP;
      } else {
        brightness = BRIGHTNESS_MIN;
      }
    }
    if ( rotateRight()) {
      if (BRIGHTNESS_MAX >= (brightness + BRIGHTNESS_STEP)) {
        brightness += BRIGHTNESS_STEP;
      } else {
        brightness = BRIGHTNESS_MAX;
      }
    }
  }
  lastRightState = rightState;
  lastLeftState = leftState;
}

bool rotateRight() {
  return digitalRead(ROTARY_LEFT_PIN) == digitalRead(ROTARY_RIGHT_PIN) &&
         digitalRead(ROTARY_RIGHT_PIN) == 1;
}
bool rotateLeft() {
  return digitalRead(ROTARY_LEFT_PIN) != digitalRead(ROTARY_RIGHT_PIN) &&
         digitalRead(ROTARY_LEFT_PIN) == 1;
}
void buttonListener() {
  int buttonState = digitalRead(ROTARY_BUTTON_PIN);
  if (buttonState != buttonLastState && buttonState == 1) {
    nextPattern();
  }
  buttonLastState = buttonState;
}

String currentPatternName() {
  return patternNames[gCurrentPatternNumber];
}
void updateDisplay() {
  u8g.setFont(u8g_font_6x10r);
  u8g.setPrintPos(0, 8);
  u8g.print("Brightness:");
  u8g.setPrintPos(70, 8);
  u8g.print(brightness);
  u8g.setPrintPos(0, 16);
  u8g.print("Pattern:");
  u8g.setPrintPos(0, 24);
  u8g.print(currentPatternName());
}
