

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include "Adafruit_NeoPixel.h"
#include <Wire.h>
#include <Adafruit_MPR121.h>

boolean is_button_pressed(int PIN, boolean OLD_STATE);



boolean isHugged();
boolean isSqueezed();
boolean isStroked();

void sendMessage(int);
void sendHello(); // if is stroked
void sendThought(); // if is squeezed
void sendHug(); // if is hugged


boolean is_reply_hug();
boolean is_reply_squeeze();
boolean is_reply_stroke();

boolean is_received_hug();
boolean is_received_thought();
boolean is_received_hello();

void show_hug();
void show_thought();
void show_hello();

/////ACTIONS

#define NONE 0
#define MISS_YOU 1//hug
#define THINK_OF_YOU 2 // squeeze
#define HELLO 3 //stroke 


#define FAKE_HUG_BUTTON A3
#define FAKE_THINK_BUTTON A2
#define FAKE_HELLO_BUTTON 11
#define CORNER_SQUEEZE 10
#define SIDE_HUG 9
//#define BUTTON_PIN   10    // Digital IO pin connected to the button.  This will be
// driven with a pull-up resistor so the switch should
// pull the pin to ground momentarily.  On a high -> low
// transition the button press logic will execute.

#define PIXEL_PIN    6    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 16

#define MY_COLOR "YELLOW"// 255, 247, 9
#define OTHER_COLOR "BLUE" // 76, 196, 255
#define LAST_COLOR "GREEN"// 3, 255, 94

boolean HAVE_CAP = false;

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = HIGH;
bool oldStateFakeHug = HIGH;
bool oldStateFakeThink = HIGH;
bool oldStateFakeHello = HIGH;
bool oldStateCornerSqueeze = HIGH;
bool oldStateSideHug = HIGH;

int showType = 0;

void setup() {
  while (!Serial);        // needed to keep leonardo/micro from starting too fast!

  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (HAVE_CAP && !cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }

  if (HAVE_CAP) {
    //lowering threshold
    Serial.println("MPR121 found!");
  }

  pinMode(FAKE_HUG_BUTTON, INPUT_PULLUP);
  pinMode(FAKE_THINK_BUTTON, INPUT_PULLUP);
  pinMode(FAKE_HELLO_BUTTON, INPUT_PULLUP);
  pinMode(CORNER_SQUEEZE, INPUT_PULLUP);
  pinMode(SIDE_HUG, INPUT_PULLUP);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'


}

void loop() {

  int message = 0;
  if (isHugged()) {
    Serial.println("is hugged");

    message = MISS_YOU;
  }

  else if (isSqueezed()) {
    Serial.println("is squeezed");
    message = THINK_OF_YOU;

  }

  else if (isStroked()) {
    Serial.println("is stroked");
    message = HELLO;
  }

  startShow(message);

  if (message == MISS_YOU ||
      message == THINK_OF_YOU || 
      message == HELLO || 
      message == NONE){
          startShow(NONE);// stop sendint message
    }

else if (is_reply_squeeze()) {
  Serial.println("is reply squeeze");
  startShow(4);
}

else if (is_reply_hug()) {
  Serial.println("is reply hug");
  startShow(5);
}

else if (is_reply_stroke()) {
  Serial.println("is reply stroke");
  startShow(6);
}

// comment out this line for detailed data from the sensor!
return;



}

void sendMessage(int message) {
  switch (message) {
    case NONE: startShow(NONE);    //off
      break;
  }
}


boolean is_button_pressed(int BUTTON, boolean OLD_STATE ) {
  /*
    //PUSH BUTTON
    // Get current button state.
    bool newState = digitalRead(BUTTON_PIN);
    //Serial.println(digitalRead(BUTTON_PIN));

    // Check if state changed from high to low (button press).
    if (newState == LOW && oldState == HIGH) {
      // Short delay to debounce button.
      delay(20);
      // Check if button is still low after debounce.
      newState = digitalRead(BUTTON_PIN);
      if (newState == LOW) {
        showType++;
        if (showType > 9)
          showType=0;
        startShow(showType);
      }
    }
    // Set the last button state to the old state.
    oldState = newState;
    */
  boolean is_on = false;
  bool newState = digitalRead(BUTTON);
  //Serial.println(digitalRead(BUTTON_PIN));
  // Check if state changed from high to low (button press).
  if (newState == LOW && OLD_STATE == HIGH) {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON);
    if (newState == LOW) {
      is_on = true;
    }
  }
  // Set the last button state to the old state.
  oldState = newState;
  return is_on;
}



boolean isStroked() {

  if (HAVE_CAP) {
    // CAPACITIVE TOUCH SENSOR
    // Get the currently touched pads
    currtouched = cap.touched();
    // to keep track of which pad is  touched
    for (uint8_t i = 0; i < 12; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(" touched");
        startShow(i);
      }
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(" released");
        startShow(0);
      }
    }
    // reset our state
    lasttouched = currtouched;
  }
  return false;
}



boolean isHugged() {
  return is_button_pressed(SIDE_HUG, oldStateSideHug);
}
boolean isSqueezed() {
  return is_button_pressed(CORNER_SQUEEZE, oldStateCornerSqueeze);
}
boolean is_reply_hug() {
  return is_button_pressed(FAKE_HUG_BUTTON, oldStateFakeHello);
}

boolean is_reply_squeeze() {
  return is_button_pressed(FAKE_THINK_BUTTON, oldStateFakeThink);
}

boolean is_reply_stroke() {
  return is_button_pressed(FAKE_HELLO_BUTTON, oldStateFakeHug);
}




void startShow(int i) {
  switch (i) {
    case 0: colorWipe(strip.Color(0, 0, 0), 10);    // Black/off
      break;
    case 1: colorWipe(strip.Color(255, 0, 0), 10);  // Red
      break;
    case 2: colorWipe(strip.Color(0, 255, 0), 10);  // Green
      break;
    case 3: colorWipe(strip.Color(0, 0, 255), 10);  // Blue
      break;
    case 4: theaterChase(strip.Color(127, 127, 127), 10); // White
      break;
    case 5: theaterChase(strip.Color(127,   0,   0), 10); // Red
      break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 10); // Blue
      break;
    case 7: rainbow(20);
      break;
    case 8: rainbowCycle(20);
      break;
    case 9: theaterChaseRainbow(50);
      break;
    case 10: colorWipe(strip.Color(0, 0, 0), 10);    // Black/off
      break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

