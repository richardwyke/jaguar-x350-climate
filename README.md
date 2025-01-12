# Jaguar XJ X350 Climate
This project aims to replace the climate controls in the Jaguar XJ X350 with new dials, to allow the installation of an aftermarket headunit.

This works for me 2007 X356 XJR (With navigation Navigation, Alpine amplifier & speakers, no rear climate and no rear entertainment.) however i haven't tried on any others combinations so your mileage may vary.

## Hardware required:
* 3x [M5Stack Dial](https://shop.m5stack.com/products/m5stack-dial-esp32-s3-smart-rotary-knob-w-1-28-round-touch-screen)
* 1x [M5Stack CANBus Unit](https://shop.m5stack.com/products/canbus-unitca-is3050g)
* 3x [GROVE Cable](https://docs.m5stack.com/en/accessory/cable/grove_cable)

## Installation
+ Splice into the original navigation wiring to pick up Ignition 12V, GND, and the CAN High and CAN Low.
+ CAN High and CAN Low needs to be wired into to the CANBus Unit.
+ The three dials need to be wired together to enable the I2C network between them - i did this using Port A on each dial with grove cables spliced together.
+ The CANBus Unit needs to be connected to the right dial on Port B.
+ Remove all the modules (Nav, CD Changer, Amplifier etc) from the rear left of the trunk.
