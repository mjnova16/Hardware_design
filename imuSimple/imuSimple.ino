
#include "Simple_MPU6050.h"
#define MPU6050_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     MPU6050_ADDRESS_AD0_LOW
Simple_MPU6050 mpu;
#define spamtimer(t) for (static uint32_t SpamTimer; (uint32_t)(millis() - SpamTimer) >= (t); SpamTimer = millis()) // (BLACK BOX) Ya, don't complain that I used "for(;;){}" instead of "if(){}" for my Blink Without Delay Timer macro. It works nicely!!!
#define printfloatx(Name,Variable,Spaces,Precision,EndTxt) print(Name); {char S[(Spaces + Precision + 3)];Serial.print(F(" ")); Serial.print(dtostrf((float)Variable,Spaces,Precision ,S));}Serial.print(EndTxt);//Name,Variable,Spaces,Precision,EndTxt


void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for Leonardo enumeration, others continue immediately
  mpu.begin();
  mpu.Set_DMP_Output_Rate_Hz(100);           // Set the DMP output rate from 200Hz to 5 Minutes.

#ifdef OFFSETS
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.load_DMP_Image(OFFSETS); // Does it all for you
#else
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available());                 // wait for data
  while (Serial.available() && Serial.read()); // empty buffer again
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.CalibrateMPU();
  mpu.load_DMP_Image();// Does it all for you with Calibration
#endif
  mpu.on_FIFO(printpropio);
}



void loop() {
  mpu.dmp_read_fifo(false);// Must be in loop  No Interrupt pin required at the expense of polling the i2c buss
}



void printpropio(int16_t *gyro, int16_t *accel, int32_t *quat){//, uint16_t SpamDelay = 100) {
  uint8_t Spam_Delay = 100;
  Quaternion q;
  VectorFloat gravity;
  float ypr[3] = { 0, 0, 0 };
  float xyz[3] = { 0, 0, 0 };
  spamtimer(Spam_Delay) {// non blocking delay before printing again. This skips the following code when delay time (ms) hasn't been met
    mpu.GetQuaternion(&q, quat);
    mpu.GetGravity(&gravity, &q);
    mpu.GetYawPitchRoll(ypr, &q, &gravity);
    mpu.ConvertToDegrees(ypr, xyz);
        Serial.print(xyz[0],4); //printfloatx is a Helper Macro that works with Serial.print that I created (See #define above)
        Serial.print(",");
        Serial.print(xyz[1],4);
        Serial.print(",");
        Serial.print(xyz[2],4);
        Serial.print("  ;   ");
        Serial.print(accel[0]);
        Serial.print(",");
        Serial.print(accel[1]);
        Serial.print(",");
        Serial.print(accel[2]);
        Serial.println();
  }
}
