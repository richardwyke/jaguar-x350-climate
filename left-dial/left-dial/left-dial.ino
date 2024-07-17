/**
 * The Right dial is responsible for controlling the right temperature,
 * as well as the settings for heated front & rear screens, defrost, 
 * and air recirculation and air conditioning.
 */

#include <Wire.h>
#include "M5Dial.h"
#include <string>

/**
 * This is the main state of the climate which the left dial will
 * receive from the center dial.
 */
struct {
  bool on = true;
  int rightTemp = 0;
} climate;

/**
 * The local state for this dial which is sent to the right dial.
 */
struct {
  int leftTemp = 0;
  bool dual = false;
  bool dirty = true;
} localClimate;

typedef struct {
  uint8_t leftTemp;
  bool dual;
  bool dirty;
  }
LeftClimate;
LeftClimate localLeftClimate;

int minTemp = 0;
int maxTemp = 31;

long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);

  Serial.begin(9600);
  while(!Serial);

  Wire.begin(1);
  Wire.onRequest(requestEvent); 

  localLeftClimate.leftTemp = 6;
  localLeftClimate.dual = false;
  localLeftClimate.dirty = true;
}

void loop() {
  M5Dial.update();

  newPosition = M5Dial.Encoder.read();

  if (newPosition != oldPosition) {
    updateCounter();
  }
}

void updatePage() {
  // We only update the display if the data has been modified
  if (localLeftClimate.dirty) {
    M5Dial.Display.clear();
    page0();
    localLeftClimate.dirty = false;
  }
}

void updateCounter() {
  int change = 0;

  if (newPosition % 4 == 0 && std::abs(newPosition - lastUpdatePosition) >= 2) {
    lastUpdatePosition = newPosition;

    if (climate.on == false) {
      return;
    }

    change = (newPosition > oldPosition) ? 2 : -2;
    
    localLeftClimate.leftTemp = constrain(localLeftClimate.leftTemp + change, minTemp, maxTemp);
    localLeftClimate.dual = true;
    localLeftClimate.dirty = true;
  }

  updatePage();

  oldPosition = newPosition;
}

void page0() {
  int offset = 0;

  M5Dial.Display.setTextColor(ORANGE);

  if (localLeftClimate.leftTemp == 0 || localLeftClimate.leftTemp == 31) {
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
  } else {
    M5Dial.Display.setTextFont(&fonts::Font6);
    M5Dial.Display.setTextSize(2);
    offset = -10;
  }

  String displayTemp = String(16 + (localLeftClimate.leftTemp / 2));

  if (localLeftClimate.leftTemp == 0) {
    displayTemp = "Low";
  }

  if (localLeftClimate.leftTemp > 30) {
    displayTemp = "High";
  }

  M5Dial.Display.drawString(
    String(displayTemp),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + offset);

  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_24);
  M5Dial.Display.setTextSize(1);
  M5Dial.Display.drawString(
    "Temp",
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + 70);
}

/**
 * Respond to the request event by sending the status of this dial
 */
void requestEvent() {
  Serial.println("Request event");

  Wire.write((byte *)&localLeftClimate, sizeof localLeftClimate);
}

/**
 * Receive updated climate settings
 */
void receiveEvent() {

}