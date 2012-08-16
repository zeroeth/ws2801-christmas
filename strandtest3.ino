#include "SPI.h"
#include "WS2801.h"
#include "hue2rgb.h"

// led     jumpers
// white - black - VCC
// black - green - GND
// red   - red   - CLK
// blue  - blue  - DAT
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
  void cycle_hue();
  void loop_brightness();
  float brightness();
  float color();
  float saturation();

  private:

  float brightness_value;
  float hue;
  float hue_speed;
  float speed;
  float saturation_value;
  bool increasing;
};

float global_hue;

Light::Light() {
  hue = random(360.0)/360.0;
  saturation_value = 1.0;

  hue_speed = 0.001;
  brightness_value = random(100)/100.0;
  speed = random(100)/10000.0 + 0.0001;
  increasing = true;
}

void Light::tick() {
  cycle_hue();
  loop_brightness();
}


void Light::cycle_hue() {
  hue += hue_speed;
  if(hue >= 0.89) {
    hue = 0.0;
   }
}

void Light::loop_brightness() {
  if(increasing) {
    brightness_value += speed;
    if(brightness_value >= max_brightness) {
      brightness_value = max_brightness;
      increasing = false;
    }
  }
  else {
    brightness_value -= speed;
    if(brightness_value <= min_brightness) {
      hue = random(360.0)/360.0;
      speed = random(100)/10000.0 + 0.0001;

      brightness_value = min_brightness;
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
  return brightness_value;
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

  hue_sequence_fill();
  //tick_and_render_lights();

  Serial.begin(9600);
  delay(1000);
}


void tick_and_render_lights() {
  for (int i=0; i < strip.numPixels(); i++) {
    lights[i].tick();
    // TODO randomly wiggle the hue (vs cycling it?)
    set_color_at(i, hsl_color(lights[i].color(), lights[i].saturation(), lights[i].brightness()));
  }
}

void hue_sequence_fill() {
  int start = random(360);
  uint32_t color;
  int type = random(100);

  for (int i=0; i < strip.numPixels(); i++) {
    color = Wheel( (start+((360/strip.numPixels())*i)) % 360 );

    set_color_at(i, color);
  }
}


void loop() {
  // #1 just shift
  array_shift_index(1);

  // #2 push random? (broken)
  //int val = random(20,150);
  //color_push(Color(20,20,val));

  // #4 pulsate
  //tick_and_render_lights();

  // display
  display_colors();
  delay(80);
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
