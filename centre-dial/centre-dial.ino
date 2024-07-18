/**
 * The centre dial is responsible for the fan speed, vent position, and other options
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
  bool on;
  int fanSpeed;
  int ventPosition;
  bool timed_recirc;
  bool defrost;
  bool front_heater;
  bool rear_heater;
  bool dirty;
};
LocalData localData = {true, 1, 1, false, false, false, false, true};

// Dial position
long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

// The active page shown on the dial
int PAGE = 0;
int lastPageChange = 0;
int PAGE_TIMEOUT = 10000;

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

  newPosition = M5Dial.Encoder.read();

  /*
   * Turning the encoder increments or decrements, depending on the page you're on
   */
  if (newPosition != oldPosition) {
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

  handleTouch();

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

      click_left = (t.x <= 120);
      click_top = (t.y <= 120);

      if (click_left && click_top) {
        localData.front_heater = !localData.front_heater;
      }

      if (!click_left && click_top) {
        localData.rear_heater = !localData.rear_heater;
      }

      if (click_left && !click_top) {
        localData.timed_recirc = !localData.timed_recirc;
      }

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
void page0() {
  M5Dial.Display.setTextColor(ORANGE);

  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Display.setTextSize(2);

  M5Dial.Display.drawString(
    getFanSpeedText(),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2
  );
  
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_24);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString(
      "Fan Speed",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 + 80);
}

/**
 * Vent position
 */
void page1() {
  M5Dial.Display.setTextColor(ORANGE);

  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Display.setTextSize(2);

  M5Dial.Display.drawString(
    getVentPosition(),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2
  );
  
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_24);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString(
      "Vents",
      M5Dial.Display.width() / 2,
      M5Dial.Display.height() / 2 + 80);
}

/**
 * Buttons
 */
void page2() {
  M5Dial.Display.setTextFont(&fonts::FreeMonoBold24pt7b);
  M5Dial.Display.setTextSize(1);

  M5Dial.Display.setTextColor(DARKGREY);
  if (localData.front_heater) {
    M5Dial.Display.setTextColor(ORANGE);
  }
  M5Dial.Display.drawString(
    "F",
    (M5Dial.Display.width() / 4) + 20,
    (M5Dial.Display.height() / 4) + 10);

  M5Dial.Display.setTextColor(DARKGREY);
  if (localData.rear_heater) {
    M5Dial.Display.setTextColor(ORANGE);
  }
  M5Dial.Display.drawString(
    "R",
    (M5Dial.Display.width() / 4 * 3) - 20,
    (M5Dial.Display.height() / 4) + 10);

  M5Dial.Display.setTextColor(DARKGREY);
  if (localData.timed_recirc) {
    M5Dial.Display.setTextColor(ORANGE);
  }
  M5Dial.Display.drawString(
    "RC",
    (M5Dial.Display.width() / 4) + 20,
    (M5Dial.Display.height() / 4 * 3) - 10);
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