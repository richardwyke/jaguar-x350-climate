/**
 * The Right dial is responsible for controlling the right temperature,
 * as well as the settings for heated front & rear screens, defrost, 
 * and air recirculation and air conditioning.
 */

#include <Wire.h>
#include "M5Dial.h"
#include <string>

/**
 * This is the main state of the climate which the right dial will
 * receive from the center dial.
 */
struct {
  bool on = true;
} climate;

/**
 * The local state for this dial which is sent to the right dial.
 */
struct {
  int rightTemp = 7;
  bool dirty = true;
} localClimate;

int minTemp = 0;
int maxTemp = 31;
unsigned long lastPageChange = 0;
int PAGE = 0;

long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::Font8);

  Serial.begin(9600);
  while(!Serial);

  Wire.begin(3);
  Wire.onRequest(requestEvent); 
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
  if (localClimate.dirty) {
    M5Dial.Display.clear();
    page0();
    localClimate.dirty = false;
  }

  lastPageChange = millis();
}

void updateCounter() {
  int change = 0;

  if (newPosition % 4 == 0 && std::abs(newPosition - lastUpdatePosition) >= 2) {
    lastUpdatePosition = newPosition;

    if (climate.on == false) {
      return;
    }

    change = (newPosition > oldPosition) ? 2 : -2;
    localClimate.rightTemp = constrain(localClimate.rightTemp + change, minTemp, maxTemp);
    localClimate.dirty = true;
  }

  updatePage();

  oldPosition = newPosition;
}

void page0() {
  int offset = 0;

  M5Dial.Display.setTextColor(ORANGE);

  if (localClimate.rightTemp == 0 || localClimate.rightTemp == 31) {
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
  } else {
    M5Dial.Display.setTextFont(&fonts::Font6);
    M5Dial.Display.setTextSize(2);
    offset = -10;
  }

  String displayTemp = String(16 + (localClimate.rightTemp / 2));

  if (localClimate.rightTemp == 0) {
    displayTemp = "Low";
  }

  if (localClimate.rightTemp > 30) {
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
  Wire.write(localClimate.rightTemp);
}