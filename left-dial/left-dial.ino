/**
 * The Right dial is responsible for controlling the right temperature,
 * as well as the settings for heated front & rear screens, defrost, 
 * and air recirculation and air conditioning.
 */

#include <Wire.h>
#include "M5Dial.h"
#include <string>
#include "images.h"

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
LocalData localLeftClimate = {8, false, true};

struct BroadcastClimateData {
  int rightTemp;
  int fanSpeed;
};
BroadcastClimateData broadcastClimate = {8, 1};

int minTemp = 2;
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

    change = (newPosition > oldPosition) ? 1 : -1;
    
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

  if (localLeftClimate.leftTemp == 0) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_lo);
  }
  if (localLeftClimate.leftTemp == 1) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_16);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 2) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_17);
  }
  if (localLeftClimate.leftTemp == 3) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_17);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 4) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_18);
  }
  if (localLeftClimate.leftTemp == 5) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_18);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 6) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_19);
  }
  if (localLeftClimate.leftTemp == 7) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_19);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 8) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_20);
  }
  if (localLeftClimate.leftTemp == 9) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_20);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 10) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_21);
  }
  if (localLeftClimate.leftTemp == 11) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_21);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 12) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_22);
  }
  if (localLeftClimate.leftTemp == 13) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_22);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 14) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_23);
  }
  if (localLeftClimate.leftTemp == 15) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_23);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 16) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_24);
  }
  if (localLeftClimate.leftTemp == 17) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_24);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 18) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_25);
  }
  if (localLeftClimate.leftTemp == 19) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_25);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 20) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_26);
  }
  if (localLeftClimate.leftTemp == 21) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_26);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 22) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_27);
  }
  if (localLeftClimate.leftTemp == 23) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_27);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 24) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_28);
  }
  if (localLeftClimate.leftTemp == 25) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_28);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 26) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_29);
  }
  if (localLeftClimate.leftTemp == 27) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_29);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 28) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_30);
  }
  if (localLeftClimate.leftTemp == 29) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_30);
      M5Dial.Display.drawBitmap(177,84,40,40,epd_bitmap_point_5);
  }
  if (localLeftClimate.leftTemp == 30) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_31);
  }
  if (localLeftClimate.leftTemp > 30) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_hi);
  }

  if (localLeftClimate.dual) {
    M5Dial.Display.drawBitmap(60,200,120,30,epd_bitmap_push_sync);
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