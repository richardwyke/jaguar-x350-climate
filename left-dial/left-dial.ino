/**
 * The Right dial is responsible for controlling the right temperature,
 * as well as the settings for heated front & rear screens, defrost, 
 * and air recirculation and air conditioning.
 */

#include <Wire.h>
#include "M5Dial.h"
#include <string>

/**
 * The I2C address for this dial
 */
#define I2C_ADDRESS 0x08 

/**
 * This is the main state of the climate which the left dial will
 * receive from the center dial.
 */
struct {
  bool on = true;
  int rightTemp = 0;
} climate;

/**
 * The local data for the dial
 */
struct LocalData {
    int leftTemp;
    bool dual;
    bool dirty;
};
LocalData localLeftClimate = {7, false, true};

struct BroadcastClimateData {
  int rightTemp;
  int fanSpeed;
};
BroadcastClimateData broadcastClimate = {8, 1};

int minTemp = 1;
int maxTemp = 30;

long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);

  Serial.begin(9600);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveCommand);  
}

void loop() {
  M5Dial.update();

  newPosition = M5Dial.Encoder.read();

  if (newPosition != oldPosition) {
    updateCounter();
  }

  /**
  * Clicking the button resets the dual state
  */
  if (M5Dial.BtnA.wasPressed()) {
    localLeftClimate.leftTemp = broadcastClimate.rightTemp;
    localLeftClimate.dual = false;
    localLeftClimate.dirty = true;
    updatePage();
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
    
    if (oldPosition != -999) {
      localLeftClimate.leftTemp = constrain(localLeftClimate.leftTemp + change, minTemp, maxTemp);
      localLeftClimate.dual = true;
      localLeftClimate.dirty = true;
    }
  }

  updatePage();

  oldPosition = newPosition;
}

void page0() {
  Serial.print("Broadcast");
  Serial.println(broadcastClimate.fanSpeed);

  if (broadcastClimate.fanSpeed == 0) {
    M5Dial.Display.clear();
    return;
  }

  M5Dial.Display.setTextColor(ORANGE);
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Display.setTextSize(2);

  String displayTemp = String(16 + (localLeftClimate.leftTemp / 2));

  if (localLeftClimate.leftTemp == 0) {
    displayTemp = "Lo";
  }

  if (localLeftClimate.leftTemp > 30) {
    displayTemp = "Hi";
  }

  M5Dial.Display.drawString(
    String(displayTemp),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2);

  if (localLeftClimate.dual) {
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_24);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString(
      "Sync",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 + 80);
  }
}

void sendData() {
  Wire.write((byte *)&localLeftClimate, sizeof(LocalData));
}

/**
 * Receive updated climate settings
 */
void receiveCommand(int numBytes) {
  if (Wire.available() == sizeof(BroadcastClimateData)) {
      Wire.readBytes((byte *)&broadcastClimate, sizeof(BroadcastClimateData));

      if (localLeftClimate.dual == false) {
        localLeftClimate.leftTemp = broadcastClimate.rightTemp;
      }

      localLeftClimate.dirty = true;
      updatePage();
  }
}