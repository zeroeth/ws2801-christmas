#include "SPI.h"
#include "WS2801.h"
#include "hue2rgb.h"

int dataPin = 2;
int clockPin = 3;

const int strip_size = 25;
WS2801 strip = WS2801(strip_size, dataPin, clockPin);

// ------------------------------------------------------
// TODO move into class
uint32_t colors[strip_size];
uint8_t color_index = 0;

uint32_t color_at(uint8_t index) {
  return colors[array_offset(index)];
}

void set_color_at(uint8_t index, uint32_t color) {
  colors[array_offset(index)] = color;
}


uint8_t array_offset(uint8_t index) {
  return (color_index + index) % strip_size;
}

void array_shift_index(uint8_t offset = 1) {
  color_index = array_offset(offset);
}

void color_push(uint32_t color) {
  set_color_at(color_index, color);
  array_shift_index(1);
}

void color_unshift(uint32_t color) {
  array_shift_index(-1);
  set_color_at(color_index, color);
}

void display_colors() {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color_at(i));
  }
  strip.show();   // write all the pixels out
}
// ------------------------------------------------------

// add color helper methods.. like fade/invert/lighten/darken/rotate, and 'off/pure_bright' maybe?
float max_brightness = 0.2;
float min_brightness = 0.0;

class Light {
  public:

  Light();
  void tick();
  float brightness();
  float color();
  float saturation();

  private:

  float value;
  float hue;
  float hue_speed;
  float speed;
  float saturation_value;
  bool increasing;
};

float global_hue;

Light::Light() {
  hue = global_hue;
  saturation_value = 0.85;

  //hue = random(360)/360.0;

  //hue = 0.427; // MAJESTIC

  //hue = 0.58888; // CERULEAN
  //saturation_value = 0.41;

  //hue = 0.95833; // Burgundy... PINK
  //saturation_value = 1.0;

  //hue = 0.25; // chartues
  //saturation_value = 1.0;

  hue_speed = 0.001;
  value = random(100)/100.0;
  speed = random(100)/10000.0;
  increasing = true;
}

void Light::tick() {
  //hue += hue_speed;
  //if(hue >= 0.89) {
  //  hue = 0.0;
  // }
  if(increasing) {
    value += speed;
    if(value >= max_brightness) {
      value = max_brightness;
      increasing = false;
    }
  }
  else {
    value -= speed;
    if(value <= min_brightness) {
      value = min_brightness;
      increasing = true;
    }
  }
}

float Light::saturation() {
  return saturation_value;
}

float Light::color() {
  return hue;
}

float Light::brightness() {
  //return sin(value + millis()/speed) * 0.5 + 0.5;
  return value;
}

Light lights[25];
int light_count = 25;


void setup() {
  strip.begin();
  strip.show();

  randomSeed(analogRead(0));
  global_hue = random(360.0)/360.0;

  for(int i = 0; i < light_count; i++) {
    lights[i] = Light();
  }
  //random_hue_fill();
  random_brightness();

  Serial.begin(9600);
  delay(1000);
}


void random_brightness() {
  for (int i=0; i < strip.numPixels(); i++) {
    lights[i].tick();
    // TODO randomly wiggle the hue (vs cycling it?)
    //0.427 MAJESTIC
    set_color_at(i, hsl_color(lights[i].color(), lights[i].saturation(), lights[i].brightness()));
  }
}

void random_hue_fill() {
  int start = random(360);
  uint32_t color;
  int type = random(100);

  for (int i=0; i < strip.numPixels(); i++) {
    // #1 spread hue
    //color = Wheel( (start+((255/strip.numPixels())*i)) % 255 );

    if(type < 50) {
      // #2 random throughout
      color = Wheel( random(360) );
    }
    else {
      // #3 solid random
      color = Wheel( start );
    }

    set_color_at(i, color);
  }
}


void loop() {
  // #1 just shift
  //array_shift_index(random(25));
  //array_shift_index(1);

  // #2 push random? (broken)
  //int val = random(20,150);
  //color_push(Color(20,20,val));

  // #3 refill each time
  //random_hue_fill();

  // #4 pulsate
  random_brightness();

  // display
  display_colors();
  //delay(4000);
  delay(10);
}

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

uint32_t hsl_color(float h, float s, float v)
{
  float Rval,Gval,Bval;
  HSL(h,s,v,Rval, Gval, Bval);
  return Color(Rval, Gval, Bval);
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(int WheelPos)
{
  float Rval,Gval,Bval;
  HSL(WheelPos/360.0,1.0,0.2,Rval, Gval, Bval);

  return Color(Rval, Gval, Bval);
}
