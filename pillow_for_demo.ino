

// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include "Adafruit_NeoPixel.h"
#include <Wire.h>
#include <Adafruit_MPR121.h>

boolean HAVE_CAP = false;

boolean is_button_pressed(int PIN, boolean OLD_STATE);

boolean isHugged();
boolean isSqueezed();
int isStroked();

void sendMessage(int);
void sendHello(); // if is stroked
void sendThought(); // if is squeezed
void sendHug(); // if is hugged

// functions for the simulated actions - for the simulated actions
boolean receivedHug();
boolean receivedThought();
boolean receivedHello();

void show_hug();
void show_thought();
void show_hello();

/////ACTIONS AND MESSAGES

#define NONE -1
#define MISS_YOU 1//hug
#define HELLO_0 2 //stroke 
#define THINK_OF_YOU 3 // squeeze
#define HELLO_1 4 // by sections of the spiral
#define HELLO_2 6
#define HELLO_3 8
#define HELLO_4 10

#define RECEIVED_HUG 5
#define RECEIVED_THOUGHT 7
#define RECEIVED_HELLO 9

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


#define YELLOW 0
#define TURQUOISE 1
#define GREEN 2
#define BLUE 3
#define RED 4
#define PURPLE 5

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

// 3, 255, 94
uint32_t green = strip.Color(3, 255, 94);
uint32_t my_color = green;
uint32_t yellow = strip.Color(255, 247, 9);
uint32_t other_color = yellow;

bool oldState = HIGH;
bool oldStateFakeHug = HIGH;
bool oldStateFakeThink = HIGH;
bool oldStateFakeHello = HIGH;
bool oldStateCornerSqueeze = HIGH;
bool oldStateSideHug = HIGH;

int showType = 0;
volatile int receivedMessages = 0; // number of received messages before clearing
volatile int receivedColors[5] = {0, 0, 0, 0, 0}; // all yellow
int message = 0;
int replied = 0;

volatile int what_lights_to_light_up[16] = {-1, -1, -1, -1, 
                                   -1, -1, -1, -1, 
                                   -1, -1, -1, -1,
                                   -1, -1, -1, -1 }; // for the hello lights 

volatile int section_0[2] = {15,14}; 
volatile int section_1[4] = {13,5,4,3}; 
volatile int section_2[3] = {12,6,2}; 
volatile int section_3[4] = {7,0,1,11}; 
volatile int section_4[3] = {10,9,8}; 

const int up_to_0[2] = {0,1};
const int up_to_1[6] = {0,1,2,10,11,12}; 
const int up_to_2[9] = {0,1,2,3,9,10,11,12,13}; 
const int up_to_3[13] = {0,1,2,3,4,8,9,10,11,12,13,14,15}; 
const int all[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};         // need it? would be all

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
  // interaction with pillow by user
  int strokedMessage = 0;
  
  if (isHugged()) {
    Serial.println("is hugged");
    message = MISS_YOU;
  }

  else if (isSqueezed()) {
    Serial.println("is squeezed");
    message = THINK_OF_YOU;
  }

  else if (strokedMessage = isStroked()) {
    Serial.println("is stroked");
    // depending on which touch pad is stroked, should light up different parts of the spiral
    Serial.print("Stroked message ");
    Serial.println(strokedMessage);
    message = strokedMessage;
  }
 
 //if the message is one of the hello's
  if (message == MISS_YOU ||
      message == THINK_OF_YOU ||
      message == HELLO_0 ||
      message == HELLO_1 ||
      message == HELLO_2 ||
      message == HELLO_3 ||
      message == HELLO_4
      ) {
    startShow(message, my_color, 10);
    message=NONE;//clear message
    startShow(NONE);// stop sendint message
    receivedMessages = 0; // reset the number of received messages
  }

  /// received a message. ideally should be in interrupts but since we are using simple lilypad, we don't have those pins available
  //determine color received (i.e. from who)
  // depending on the number of messages queued, should display one or more colors
  
  if (receivedHug()) {
    receivedMessages++;
    Serial.println("received hug");
    message = RECEIVED_HUG;
    //startShow(14);
  }

  else if (receivedThought()) {
    receivedMessages++;
    Serial.println("received thought");
    message = RECEIVED_THOUGHT;
    //startShow(15);
  }
  else if (receivedHello()) {
    receivedMessages++;
    Serial.println("received hello");
    message = RECEIVED_HELLO;
    //startShow(16);
  }
  
  
  if(message == RECEIVED_HUG || 
  message ==  RECEIVED_THOUGHT || 
  message == RECEIVED_HELLO){
    
    startShow(message, other_color, 10);
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

// returns 0 if false or returns the number of the conductive strip touched
//so options are 2, 4, 6 and 8
int isStroked() {
  if (HAVE_CAP) {
    // CAPACITIVE TOUCH SENSOR
    // Get the currently touched pads
    currtouched = cap.touched();
    // to keep track of which pad is  touched
    for (uint8_t i = 0; i < 12; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(" touched");
        //startShow(i);
        return i+2;// so that it never returns 0, shift by two
      }
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(" released");
        //startShow(NONE);
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
boolean receivedHug() {
  return is_button_pressed(FAKE_HUG_BUTTON, oldStateFakeHug);
}

boolean receivedThought() {
  return is_button_pressed(FAKE_THINK_BUTTON, oldStateFakeThink);
}

boolean receivedHello() {
  return is_button_pressed(FAKE_HELLO_BUTTON, oldStateFakeHello);
}

/*
#define NONE -1
#define MISS_YOU 1//hug
#define HELLO_0 2 //stroke 
#define THINK_OF_YOU 3 // squeeze
#define HELLO_1 4 // by sections of the spiral
#define HELLO_2 6
#define HELLO_3 8
#define HELLO_4 10

#define RECEIVED_HUG 5
#define RECEIVED_THOUGHT 7
#define RECEIVED_HELLO 9
*/

void startShow(int i) {
  switch (i) {
    //-1
    case NONE: startShow(NONE, strip.Color(0, 0, 0), 10);    // Black/off
      break;
    default:
      startShow(i, strip.Color(0, 0, 0), 10);// basically do nothing.. 
 }
}


void startShow(int i, uint32_t c, uint8_t wait) {
  switch (i) {
    //-1
    case NONE: colorWipe(strip.Color(0, 0, 0), 10);    // Black/off
      break;
      //1
     case MISS_YOU: spiralInAndOut(c, wait); //spiralInAndOut(strip.Color(3, 255, 94), 20);  // Green 3, 255, 94 colorGlow();//colorWipe(strip.Color(255, 0, 0), 10);  // Red
      break;
      //2
    case HELLO_0:lightSection(c, wait, up_to_0,  sizeof(up_to_0)/sizeof(up_to_0[0])); //colorWipe(strip.Color(0, 0, 0), 10);    // Black/off
      break;
      //3
    case THINK_OF_YOU: colorGlow(c, wait);//colorGlow(strip.Color(127,   0,   0), 10); // Red
      break;
      //4
    case HELLO_1: lightSection(c, wait, up_to_1, sizeof(up_to_1)/sizeof(up_to_1[0]));//lightSection(strip.Color(3,255,94), 20, up_to_1, sizeof(up_to_1)/sizeof(up_to_1[0]));
      break;
      //6
    case HELLO_2: lightSection(c, wait, up_to_2,sizeof(up_to_2)/sizeof(up_to_2[0]));//colorWipe(strip.Color(0, 0, 255), 10);  // Blue
      break;
      //8
    case HELLO_3: lightSection(c, wait, up_to_3, sizeof(up_to_3)/sizeof(up_to_3[0]));//lightSection(strip.Color(3,255,94), 20, up_to_3, sizeof(up_to_3)/sizeof(up_to_3[0]));//theaterChase(strip.Color(127, 127, 127), 10); // White
      break;
      //10
    case HELLO_4: 
      lightSection(c, wait, all,  sizeof(all)/sizeof(all[0]));//theaterChase(strip.Color(  0,   0, 127), 10); // Blue
      break;
    case RECEIVED_HUG: spiralInAndOut(c, wait); //rainbow(20);
      break;
    case RECEIVED_THOUGHT: colorGlow(c, wait);//theaterChase(strip.Color(  0,   0, 127), 10);//rainbowCycle(20);
      break;
    case RECEIVED_HELLO: 
          lightSection(c, wait, up_to_0,  sizeof(up_to_0)/sizeof(up_to_0[0]));
          lightSection(c, wait, up_to_1, sizeof(up_to_1)/sizeof(up_to_1[0]));
          lightSection(c, wait, up_to_2,sizeof(up_to_2)/sizeof(up_to_2[0]));
          lightSection(c, wait, up_to_3, sizeof(up_to_3)/sizeof(up_to_3[0]));
          lightSection(c, wait, all,  sizeof(all)/sizeof(all[0]));          
      break;
    case 11: theaterChase(strip.Color(  0,   0, 127), 10);//colorWipe(strip.Color(0, 0, 0), 10);    // Black/off
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


// Fill the dots one after the other with a color
void spiralInAndOut(uint32_t c, uint8_t wait) {
// turn them on one by one, starting with center pixel
  for (int i = strip.numPixels()-1; i>-1 ; i--) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  delay(wait * 100);
  
// turn them off one by one, starting with outer pixel
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0)); 
    strip.show();
    delay(wait);
  }
}

// Fill the dots one after the other with a color
void colorGlow() {
  for(int j = 0; j < 256; j++) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(j, j, j));
      strip.show();
      delay(2);
    }
  }
  for(int j = 255; j > 0; j--) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(j, j, j));
      strip.show();
      delay(2);
    }
  }
}


void colorGlow(uint32_t c, uint8_t wait){
  for(int j = 0; j < 256; j++) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.setBrightness(j);
      strip.show();
    }
  }
  
  delay(wait*3);
  for(int j = 255; j > 0; j--) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.setBrightness(j);
      strip.show();
    }
  }
}


void lightSection(uint32_t c, uint8_t wait, const int pixels[], int numberOfPixels){
  Serial.print("Lighting up pixels ");
  Serial.println(numberOfPixels);

  for (uint16_t i =0; i<numberOfPixels; i++){
      strip.setPixelColor(pixels[i], c);

  }
  strip.show();
  delay(wait*40);


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

