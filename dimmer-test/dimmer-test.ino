#include <MsTimer2.h>
#include "dimmers.h"

#define DELAY 50

#define DIM_MIN_LEVEL 0
#define DIM_MAX_LEVEL 100

static int light = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello from dimmer!");
  dimmers_init();
  dimmers_light_set(50);
}

void loop()
{
  static const bool blinking = false;
  static bool rising = true;
  
  if (blinking) 
  {
    if (rising)
    {
      if (light < DIM_MAX_LEVEL)
      {
        light += 10;        
        dimmers_light_set(light);
      }
      else
      {
        rising = false;
      }
    }
    else
    {
      if (light > DIM_MIN_LEVEL)
      {
        light -= 10;
        dimmers_light_set(light);
      }
      else
      {
        rising = true;
      }
    }

    delay(DELAY);
  }
}

