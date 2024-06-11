# RCLink - A versatile packet-based system to control a fixed-wing drone aircraft from any PC, with attitude, heading, and system status feedback.


![Project Preview](https://github.com/xzips/RCLink/assets/114827498/c78800bd-eb7e-492e-8ea1-3638ae7edb04)

## Objectives

TODO...



## Current Hardware/Software Setup
Raspberry Pi Pico + NRF2401L 2.4G tranciever module connects to any PC through serial over USB, controlled by interactive desktop GUI application shown in the above figure.

Remote system (aircraft) utilizes a Teensy 4.0 microcontroller with a matching NRF2401L tranciever to exchange information with the computer in real-time over large distances (up to 1km).

Desktop application controls the remote servos and brushless motor ESC using a PCA9685 16 channel pwm controller, and attitude information is transmitted back from the MPU6050 gyro/accelerometer.

Currently all "remote" hardware is on a 3D printed test rig to streamline the software development, however, soon design of the airframe (as is evidently missing from the CAD model above) will begin, and the electornics on the test rig will be compacted and bolted down there instead.
