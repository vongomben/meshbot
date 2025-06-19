
/* 
 * Example of using the ChainableRGB library for controlling a Grove RGB.
 * This code fades in an out colors in a strip of leds.
 */


#include <ChainableLED.h>

#define NUM_LEDS 1

//ChainableLED leds(1, 2, NUM_LEDS); // works
ChainableLED leds(D0, D1, NUM_LEDS); // works

void setup() {
}

byte power = 0;

void loop() {

  blinkTimes(1);

}

void blinkTimes(int times) {

  for (int i = 0; i < times; i++) {

    leds.setColorRGB(0, 255, 0, 0);
    delay(100);
    leds.setColorRGB(0, 0, 0, 0);
    delay(100);
  }
}
