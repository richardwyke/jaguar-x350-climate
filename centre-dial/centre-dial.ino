/**
 * The centre dial is responsible for the fan speed, vent position, and other options
 */
#include <Wire.h>
#include "M5Dial.h"
#include <string>
#include "images.h"

/**
 * The I2C address for this dial
 */
#define I2C_ADDRESS 0x06 

/**
 * The local data for the dial
 */
struct LocalData {
  bool on;
  int fanSpeed;
  int ventPosition;
  bool timed_recirc;
  bool defrost;
  bool dirty;
};
LocalData localData = {true, 1, 6, false, false, true};

// Dial position
long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

bool shownStartupLogo = false;

// The active page shown on the dial
int PAGE = 0;
int lastPageChange = 0;
int PAGE_TIMEOUT = 10000;
int start = 0;

// Constants that define the maximum and minimum values for the dial
int minFanSpeed = 0;
int maxFanSpeed = 8;
int minVentPosition = 1;
int maxVentPosition = 6;

static m5::touch_state_t prev_state;

void setup() {
  auto cfg = M5.config();

  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);

  Serial.begin(9600);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(sendData);
}

void loop() {
  M5Dial.update();

  if (millis() < 5000) {
    pageLogo();
    return;
  }

  newPosition = M5Dial.Encoder.read();

  /*
   * Turning the encoder increments or decrements, depending on the page you're on
   */
  if (newPosition != oldPosition) {
    lastPageChange = millis();
    updateCounter();
    updatePage();
  }

  /**
    * Clicking the button changes the page
    */
  if (M5Dial.BtnA.wasPressed()) {
    /**
     * If the climate is turned off, pressing the button should do nothing
     */
    if (localData.fanSpeed == 0) {
      return;
    }

    localData.dirty = true;
    PAGE++;
    lastPageChange = millis();
    updatePage();
  }

  if (PAGE == 2) {
    handleTouch();
  }

  if (millis() - lastPageChange > PAGE_TIMEOUT && PAGE != 0) {
    localData.dirty = true;
    PAGE = 0;
    updatePage();
  }
}

void updatePage() {
  // We only update the display if the data has been modified
  if (localData.dirty) {
    M5Dial.Display.clear();

    if (PAGE % 3 == 0) {
      page0();
    }

    if (PAGE % 3 == 1) {
      page1();
    }

    if (PAGE % 3 == 2) {
      page2();
    }

    localData.dirty = false;
  }
}

void handleTouch() {
  bool click_left;
  bool click_top;

  auto t = M5Dial.Touch.getDetail();
  if (prev_state != t.state) {
    prev_state = t.state;
    static constexpr const char* state_name[16] = {
      "none", "touch", "touch_end", "touch_begin",
      "___", "hold", "hold_end", "hold_begin",
      "___", "flick", "flick_end", "flick_begin",
      "___", "drag", "drag_end", "drag_begin"
    };

    if (state_name[t.state] == "none") {
      lastPageChange = millis();

      localData.defrost = !localData.defrost;

      localData.dirty = true;
      updatePage();
    }
  }
}

void updateCounter() {
  int change = 0;

  if (newPosition % 4 == 0 && std::abs(newPosition - lastUpdatePosition) >= 2) {
    lastUpdatePosition = newPosition;

    if (oldPosition != -999) {
      localData.dirty = true;

      if (PAGE % 3 == 0) {
        change = (newPosition > oldPosition) ? 1 : -1;
        localData.fanSpeed = constrain(localData.fanSpeed + change, minFanSpeed, maxFanSpeed);
      }

      if (PAGE % 3 == 1) {
        change = (newPosition > oldPosition) ? 1 : -1;
        localData.ventPosition = constrain(localData.ventPosition + change, minVentPosition, maxVentPosition);
      }
    }
  }
  
  oldPosition = newPosition;
}
/**
 * Fan speed
 */
void pageLogo() {
  M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_jaguar_alt);
}

/**
 * Fan speed
 */
void page0() {
  if (localData.fanSpeed == 0) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_off);
  }
if (localData.fanSpeed == 1) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_auto);
  }
  if (localData.fanSpeed == 2) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_1);
  }
  if (localData.fanSpeed == 3) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_2);
  }
  if (localData.fanSpeed == 4) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_3);
  }
  if (localData.fanSpeed == 5) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_4);
  }
  if (localData.fanSpeed == 6) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_5);
  }
  if (localData.fanSpeed == 7) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_6);
  }
  if (localData.fanSpeed == 8) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_fan_7);
  }
}

/**
 * Vent position
 */
void page1() {
  if (localData.ventPosition == 6) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_vents_auto);
  }
  if (localData.ventPosition == 1) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_vents_face);
  }
  if (localData.ventPosition == 2) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_vents_face_feet);
  }
  if (localData.ventPosition == 3) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_vents_feet);
  }
  if (localData.ventPosition == 4) {
      M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_vents_screen_feet);
  }
}

/**
 * Buttons
 */
void page2() {

  if (localData.defrost) {
    M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_defrost_on);
  }

  if (!localData.defrost) {
    M5Dial.Display.drawBitmap(0,0,240,240,epd_bitmap_defrost_off);
  }
}

String getFanSpeedText() {
  String displayTemp = "Off";

  if (localData.fanSpeed == 1) {
    displayTemp = "Auto";
  }

  if (localData.fanSpeed > 1) {
    displayTemp = String(localData.fanSpeed - 1);
  }

  return displayTemp;
}

String getVentPosition() {
  return String(localData.ventPosition);
}

/**
 * Send the local data back to the main dial
 */
void sendData() {
  Wire.write((byte *)&localData, sizeof(LocalData));
}