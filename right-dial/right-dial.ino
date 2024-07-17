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
#define I2C_ADDRESS 0x06 

/**
 * The local data for the dial
 */
struct LocalData {
    int fanSpeed;
    bool dirty;
};
LocalData localCentreClimate = {1, true};

// struct BroadcastClimateData {
//   int rightTemp;
// };
// BroadcastClimateData broadcastClimate;

long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

int minFanSpeed = 0;
int maxFanSpeed = 8;

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
    Serial.println("Update");
  }
}

void updatePage() {
  // We only update the display if the data has been modified
  if (localCentreClimate.dirty) {
    M5Dial.Display.clear();
    page0();
    localCentreClimate.dirty = false;
  }
}

void updateCounter() {
  int change = 0;

  if (newPosition % 4 == 0 && std::abs(newPosition - lastUpdatePosition) >= 2) {
    lastUpdatePosition = newPosition;

    change = (newPosition > oldPosition) ? 1 : -1;
    localCentreClimate.fanSpeed = constrain(localCentreClimate.fanSpeed + change, minFanSpeed, maxFanSpeed);
    Serial.println(localCentreClimate.fanSpeed);
    localCentreClimate.dirty = true;
  }

  updatePage();

  oldPosition = newPosition;
}

void page0() {
  int offset = 0;

  M5Dial.Display.setTextColor(ORANGE);

  // if (localCentreClimate.fanSpeed > 1) {
  //   M5Dial.Display.setTextFont(&fonts::Font6);
  //   M5Dial.Display.setTextSize(2);
  //   offset = -10;
  // } else {
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(2);
  // }

  M5Dial.Display.drawString(
    getFanSpeedText(),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2 + offset);

  // M5Dial.Display.setTextFont(&fonts::Orbitron_Light_24);
  // M5Dial.Display.setTextSize(1);
  // M5Dial.Display.drawString(
  //   "Fan Speed",
  //   M5Dial.Display.width() / 2,
  //   M5Dial.Display.height() / 2 + 70);
}

String getFanSpeedText() {
  String displayTemp = "Off";

  if (localCentreClimate.fanSpeed == 1) {
    displayTemp = "Auto";
  }

  if (localCentreClimate.fanSpeed > 1) {
    displayTemp = String(localCentreClimate.fanSpeed - 1);
  }
  return displayTemp;
}

void sendData() {
  Wire.write((byte *)&localCentreClimate, sizeof(LocalData));
}

/**
 * Receive updated climate settings
 */
void receiveCommand(int numBytes) {
  // if (Wire.available() == sizeof(BroadcastClimateData)) {
  //     Wire.readBytes((byte *)&broadcastClimate, sizeof(BroadcastClimateData));

  //     if (localLeftClimate.dual == false) {
  //       localLeftClimate.leftTemp = broadcastClimate.rightTemp;
  //       localLeftClimate.dirty = true;

  //       updatePage();
  //     }
  // }
}