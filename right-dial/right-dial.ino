/**
 * The centre dial is responsible for the canbus communication with the car, and
 * shows the controls for the fan speed and the vent location, as well as the
 * display of the exterior temperature.
 */
#include "M5Dial.h"
#include "ESP32-TWAI-CAN.hpp"
#include <string>
#include <Wire.h>

#define RX_PIN 1
#define TX_PIN 2

CanFrame rxFrame;

struct {
  bool on = true;
  int fanSpeed = 1;
  int leftTemp = 7;
  int rightTemp = 7;
  bool dual = false;
  int ventPosition = 6;
  bool auto_recirc = false;
  bool timed_recirc = false;
  bool defrost = false;
  bool front_heater = false;
  bool rear_heater = false;
  bool dirty = true;
} climate;

#define LEFT_SLAVE_ADDRESS 0x08
struct LeftDialData {
    int leftTemp;
    bool dual;
    bool dirty;
};

LeftDialData leftClimate;

#define CENTRE_SLAVE_ADDRESS 0x06
struct CentreDialData {
  bool on;
  int fanSpeed;
  int ventPosition;
  bool timed_recirc;
  bool defrost;
  bool front_heater;
  bool rear_heater;
  bool dirty;
};;
CentreDialData centreClimate;

struct BroadcastClimateData {
  int rightTemp;
  int fanSpeed;
};

BroadcastClimateData broadcastClimateData;

int minFanSpeed = 0;
int maxFanSpeed = 8;
int minTemp = 0;
int maxTemp = 31;

const int PAGE_TIMEOUT = 10000;  // 10 seconds in milliseconds
const int BROADCAST_INTERVAL = 2000;  // 10 seconds in milliseconds
const int FUNCTION_CALL_INTERVAL = 800;
const int SLAVE_CALL_INTERVAL = 200;

unsigned long lastClimateBroadcast = 0;
unsigned long lastPageChange = 0;    // Stores timestamp of last page change
unsigned long lastCanBroadcast = 0;  // Stores timestamp of last function call
unsigned long lastSlaveRead = 0;
int PAGE = 0;

long oldPosition = -999;
long newPosition = 0;
long lastUpdatePosition = -999;

static m5::touch_state_t prev_state;

void setup() {
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::Font6);

  Serial.begin(9600);

  ESP32Can.setPins(TX_PIN, RX_PIN);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

  // You can also just use .begin()..
  if (ESP32Can.begin()) {
    Serial.println("CAN bus started!");
  } else {
    Serial.println("CAN bus failed!");
  }

  Wire.begin();
}

std::string int_to_binary_string(int num, int i) {
  std::string binary_str;
  i--;

  // Iterate through each bit (assuming int is 32 bits)
  for (i; i >= 0; --i) {
    // Check if the current bit is set using bitwise AND with 1 << i
    int bit_value = (num >> i) & 1;
    binary_str += std::to_string(bit_value);
  }

  return binary_str;
}

int binary_string_to_int(std::string binary_str) {
  int decimal_value = 0;
  int base = 1;

  // Iterate through the string in reverse order
  for (int i = binary_str.length() - 1; i >= 0; --i) {
    char c = binary_str[i];

    // Check for valid binary digits (0 or 1)
    if (c != '0' && c != '1') {
      throw std::invalid_argument("String must contain only 0s and 1s");
    }

    // Convert character to integer and accumulate value
    int digit_value = c - '0';
    decimal_value += digit_value * base;

    // Increase base for the next digit
    base *= 2;
  }

  return decimal_value;
}

void loop() {
  M5Dial.update();

  newPosition = M5Dial.Encoder.read();

  if (newPosition != oldPosition) {
    updateCounter();
  }

  if (millis() - lastCanBroadcast >= FUNCTION_CALL_INTERVAL) {
    sendCanMessage();

    lastCanBroadcast = millis();
  }

  if (millis() - lastSlaveRead >= SLAVE_CALL_INTERVAL) {
    readSlaveMessages();

    lastSlaveRead = millis();
  }
}

void broadcastClimate() {
  broadcastClimateData.rightTemp = climate.rightTemp;

  Wire.beginTransmission(LEFT_SLAVE_ADDRESS);  
  Wire.write((byte *)&broadcastClimateData, sizeof(BroadcastClimateData));
  Wire.endTransmission();

  lastClimateBroadcast = millis();
}

void readSlaveMessages() {
  Wire.requestFrom(LEFT_SLAVE_ADDRESS, sizeof(LeftDialData)); 
  if (Wire.available() == sizeof(LeftDialData)) {
      Wire.readBytes((byte *)&leftClimate, sizeof(LeftDialData));
  }

  Wire.requestFrom(CENTRE_SLAVE_ADDRESS, sizeof(CentreDialData)); 
  if (Wire.available() == sizeof(CentreDialData)) {
      Wire.readBytes((byte *)&centreClimate, sizeof(CentreDialData));
  }

  syncDialState();
}

void syncDialState() {
  if (climate.fanSpeed != centreClimate.fanSpeed) {
    climate.fanSpeed = centreClimate.fanSpeed;
    climate.dirty = true;
    broadcastClimate();
  }

  climate.ventPosition = centreClimate.ventPosition;
  climate.front_heater = centreClimate.front_heater;
  climate.rear_heater = centreClimate.rear_heater;
  climate.dual = centreClimate.dual;
  if (climate.dual) {
    climate.leftTemp = leftClimate.leftTemp;
  } else {
    climate.leftTemp = climate.rightTemp;
  }
}

void updateCounter() {
  int change = 0;

  if (newPosition % 4 == 0 && std::abs(newPosition - lastUpdatePosition) >= 2) {
    lastUpdatePosition = newPosition;

    if (PAGE % 3 == 0) {
      if (climate.fanSpeed == 0) {
        return;
      }

      change = (newPosition > oldPosition) ? 2 : -2;
      climate.rightTemp = constrain(climate.rightTemp + change, minTemp, maxTemp);

      broadcastClimate();
    }

    updatePage();
  }

  oldPosition = newPosition;
}

void updatePage() {
  M5Dial.Display.clear();

  page0();
}

void page0() {
  /**
   * We don't show anything if the system is off
   */
  if (climate.fanSpeed == 0) {
    return;
  }

  M5Dial.Display.setTextColor(ORANGE);
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Display.setTextSize(2);

  String displayTemp = String(16 + (climate.rightTemp / 2));

  if (climate.rightTemp == 0) {
    displayTemp = "Lo";
  }

  if (climate.rightTemp > 30) {
    displayTemp = "Hi";
  }

  M5Dial.Display.drawString(
    String(displayTemp),
    M5Dial.Display.width() / 2,
    M5Dial.Display.height() / 2
  );
}

void sendCanMessage() {
  CanFrame obdFrame = { 0 };
  obdFrame.identifier = 0x696;
  obdFrame.extd = 0;
  obdFrame.data_length_code = 8;
  obdFrame.data[0] = 3;
  obdFrame.data[1] = getPart1();
  obdFrame.data[2] = getPart2();
  obdFrame.data[3] = getPart3();
  obdFrame.data[4] = getPart4();
  obdFrame.data[5] = 0;
  obdFrame.data[6] = 0;
  obdFrame.data[7] = 0;

  ESP32Can.writeFrame(obdFrame, 0);

  // Serial.println("ESP32Can.writeFrame: " + String(obdFrame.data[0]) + ":" + String(obdFrame.data[1]) + ":" + String(obdFrame.data[2]) + ":" + String(obdFrame.data[3]) + ":" + String(obdFrame.data[4]) + ":" + String(obdFrame.data[5]) + ":" + String(obdFrame.data[6]) + ":" + String(obdFrame.data[7]));
}

int getPart1() {
  std::string Part1 = "1";

  if (climate.fanSpeed == 1) {
    Part1 += "1111";
  } else {
    Part1 += "0" + int_to_binary_string(climate.fanSpeed - 1, 3);
  }

  // Serial.println("Part1:");
  // Serial.println(Part1.c_str());

  return binary_string_to_int(Part1);
}

int getPart2() {
  std::string Part2 = "110";
  int temp = (climate.dual == false) ? climate.rightTemp : climate.leftTemp;

  Part2 = Part2 + int_to_binary_string(temp, 5);

  // Serial.println("Part2:");
  // Serial.println(Part2.c_str());

  return binary_string_to_int(Part2);
}

int getPart3() {
  std::string Part3 = "0";
  Part3 += (climate.dual) ? "1" : "0";
  Part3 += "1";

  int temp = (climate.dual == false) ? climate.rightTemp : climate.leftTemp;

  Part3 += int_to_binary_string(temp, 5);

  // Serial.println("Part3:");
  // Serial.println(Part3.c_str());

  return binary_string_to_int(Part3.c_str());
}

int getPart4() {
  std::string Part4 = "1";

  Part4 += (climate.on) ? "1" : "0";
  Part4 += "0000";
  Part4 += (climate.front_heater) ? "1" : "0";
  Part4 += (climate.rear_heater) ? "1" : "0";

  // Serial.println("Part4:");
  // Serial.println(Part4.c_str());

  return binary_string_to_int(Part4);
}
