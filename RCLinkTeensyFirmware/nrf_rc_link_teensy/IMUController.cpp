
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include "RadioController.hpp"
#include "IMUController.hpp"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

#define OUTPUT_READABLE_YAWPITCHROLL
#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)

MPU6050 mpu;

namespace imu
{

    bool blinkState = false;

    // MPU control/status vars
    bool dmpReady = false;  // set true if DMP init was successful
    uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
    uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
    uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
    uint16_t fifoCount;     // count of all bytes currently in FIFO
    uint8_t fifoBuffer[64]; // FIFO storage buffer

    // orientation/motion vars
    Quaternion q;           // [w, x, y, z]         quaternion container
    VectorInt16 aa;         // [x, y, z]            accel sensor measurements
    VectorInt16 gy;         // [x, y, z]            gyro sensor measurements
    VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
    VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
    VectorFloat gravity;    // [x, y, z]            gravity vector
    float euler[3];         // [psi, theta, phi]    Euler angle container
    float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector



    volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
    
    void dmpDataReady()
    {
        mpuInterrupt = true;
    }


    void imu_setup() {

        last_imu_comm_update_millis = millis();

        Wire.begin();
        Wire.setClock(400000);

        //Fastwire::setup(400, true);

        mpu.initialize();
        pinMode(INTERRUPT_PIN, INPUT);

        devStatus = mpu.dmpInitialize();

        //-2755.00000, 33.00000, 979.00000, 100.00000, -32.00000, 27.00000

        mpu.setXGyroOffset(100);
        mpu.setYGyroOffset(-32);
        mpu.setZGyroOffset(27);
        mpu.setXAccelOffset(-2755);
        mpu.setYAccelOffset(33);
        mpu.setZAccelOffset(979);


        if (devStatus == 0) {
            // Calibration Time: generate offsets and calibrate our MPU6050
            mpu.CalibrateAccel(6);
            mpu.CalibrateGyro(6);
            Serial.println();
            mpu.PrintActiveOffsets();

            mpu.setDMPEnabled(true);

            attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
            mpuIntStatus = mpu.getIntStatus();

            dmpReady = true;

            packetSize = mpu.dmpGetFIFOPacketSize();
        } else {

            Serial.print(F("DMP Initialization failed (code "));
            Serial.print(devStatus);
            Serial.println(F(")"));
        }

        }


            
    //the ypr we're talking about here is imu::ypr which is a 3 array float
     void update_rotation_ypr()
     {
        // if programming failed, don't try to do anything
        if (!dmpReady) return;

        // read a packet from FIFO
        if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 

            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);


            /*

            Serial.print("ypr\t");
            Serial.print(ypr[0] * 180 / M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180 / M_PI);
            Serial.print("\t");
            Serial.print(ypr[2] * 180 / M_PI);
            Serial.println();

            */


            //*y = ypr[0];
            //*p = ypr[1];
            //*r = ypr[2];




        }
    }

    unsigned long last_imu_comm_update_millis;

    void time_conditional_send_ypr()
    {
        unsigned long current_millis = millis();
        unsigned long elapsed_millis = current_millis - last_imu_comm_update_millis;

        if (elapsed_millis < IMU_COMM_UPDATE_DELAY) return;

        last_imu_comm_update_millis = current_millis;

        //prepare string, keep in mind it must be max 32 bytes
        //format as: YPR_XXXXX_XXXXX_XXXXX where XXX is the value of the ypr times 100 rounded to the nearest integer, padded with zeros if necessary
        //e.g. YPR_00000_00000_00000

        int y = (int)(ypr[0] * 180 / M_PI * 100);
        int p = (int)(ypr[1] * 180 / M_PI * 100);
        int r = (int)(ypr[2] * 180 / M_PI * 100);

        char ypr_str[32];
        sprintf(ypr_str, "YPR_%05d_%05d_%05d", y, p, r);

        send_queue.push_back(ypr_str);

    }


}








