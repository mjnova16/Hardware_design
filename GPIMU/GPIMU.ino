#include <SoftwareSerial.h>
#include <TinyGPS.h>
TinyGPS gps;
SoftwareSerial ss(8, 9);
char fec[32];
char hor[32];
float flat, flon;
unsigned long age;

//----------------------------------------------------------------------------------------------------------------------------------
#include "Simple_MPU6050.h"
#define MPU6050_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     MPU6050_ADDRESS_AD0_LOW
Simple_MPU6050 mpu;
//#define spamtimer(t) for (static uint32_t SpamTimer; (uint32_t)(millis() - SpamTimer) >= (t); SpamTimer = millis()) // (BLACK BOX) Ya, don't complain that I used "for(;;){}" instead of "if(){}" for my Blink Without Delay Timer macro. It works nicely!!!
//#define printfloatx(Name,Variable,Spaces,Precision,EndTxt) print(Name); {char S[(Spaces + Precision + 3)];Serial.print(F(" ")); Serial.print(dtostrf((float)Variable,Spaces,Precision ,S));}Serial.print(EndTxt);//Name,Variable,Spaces,Precision,EndTxt

//----------------------------------------------------------------------------------------------------------------------------------


int err1 = 13;

float yaw;
float pitch;
float roll;

char flatStr[15]; // Ajusta el tamaño según la longitud máxima que esperas
char flonStr[15];
char yawStr[15];
char pitchStr[15];
char rollStr[15];
char giro[30];

SoftwareSerial SIM7670Serial(2, 3); // RX, TX


void setup() {
  Serial.begin(9600);
  ss.begin(9800);
  pinMode(err1, OUTPUT);

  //------------------------------------------------------------------
  mpu.begin();
  mpu.Set_DMP_Output_Rate_Hz(100);           // Set the DMP output rate from 200Hz to 5 Minutes.

#ifdef OFFSETS
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.load_DMP_Image(OFFSETS); // Does it all for you
#else
  //while (Serial.available() && Serial.read()); // empty buffer
  //while (!Serial.available());                 // wait for data
  //while (Serial.available() && Serial.read()); // empty buffer again
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.CalibrateMPU();
  mpu.load_DMP_Image();// Does it all for you with Calibration
#endif
  mpu.on_FIFO(printpropio);


  //===================    4G conexión  =================================
  SIM7670Serial.begin(115200);

  // Iniciar el servicio TCP/IP
  sendATCommand("AT+NETOPEN");

  // Configurar el modo de aplicación TCP/IP
  sendATCommand("AT+CIPMODE=0");

  // Configurar el APN (Access Point Name)
  sendATCommand("AT+CGDCONT=1,\"LTE\",\"claro.pe\"");

  // Activar la conexión GPRS
  sendATCommand("AT+CGATT=1");

  // Configurar el perfil de conexión
  sendATCommand("AT+CSTT=\"claro.pe\",\"\",\"\"");

  // Establecer el puerto de destino
  sendATCommand("AT+UDPSERV=5000");

  // Configurar la dirección IP y puerto de destino
  sendATCommand("AT+CIPOPEN=1,\"TCP\",\"18.223.206.251\",5000");


}


void loop() {


  //===================================================================
  //============================  GPS GET   ===========================
  //===================================================================

  gps.f_get_position(&flat, &flon, &age);
  //  Serial.print("LAT: ");
  //  Serial.print(flat, 6);
  //  Serial.print("  Lon: ");
  //  Serial.print(flon, 6);
  //  Serial.print("  ");
  obtenerFechaHora(fec, hor);
  //  Serial.print("Fecha: ");
  //  Serial.print(fec);
  //  Serial.print("Hora: ");
  //  Serial.print(hor);

  char data[200];



  //===================================================================
  //============================  imu GET   ===========================
  //===================================================================
  mpu.dmp_read_fifo(false);





  dtostrf(flat, 10, 6, flatStr);
  dtostrf(flon, 10, 6, flonStr);
  //  dtostrf(yaw, 6, 2, yawStr);
  //  dtostrf(pitch, 6, 2, pitchStr);
  //  dtostrf(roll, 6, 2, rollStr);

  sprintf(data, "LAT: %s, Lon: %s, Fecha: %s, Hora: %s, Giro: %s",
          flatStr, flonStr, fec, hor, giro);

  sendTCPMessage(data);

  //Serial.println(data);

  //Serial.print(yaw);
  //Serial.print(" - ");
  //Serial.println(yaw);


  smartdelay(4000);


}


/*============================================================================*/



void obtenerFechaHora(char* fecha, char* hora) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  if (age == TinyGPS::GPS_INVALID_AGE)
    //Serial.print("********** ******** ");
    digitalWrite(err1, 1);
  else {
    digitalWrite(err1, 0);
    // Guardar fecha y hora en variables separadas
    sprintf(fecha, "%02d/%02d/%02d", month, day, year);
    sprintf(hora, "%02d:%02d:%02d", hour, minute, second);
  }
}


void printpropio(int16_t *gyro, int16_t *accel, int32_t *quat) { //, uint16_t SpamDelay = 100) {
  //uint8_t Spam_Delay = 100;
  Quaternion q;
  VectorFloat gravity;
  float ypr[3] = { 0, 0, 0 };
  float xyz[3] = { 0, 0, 0 };
  //spamtimer(Spam_Delay) {// non blocking delay before printing again. This skips the following code when delay time (ms) hasn't been met
  mpu.GetQuaternion(&q, quat);
  mpu.GetGravity(&gravity, &q);
  mpu.GetYawPitchRoll(ypr, &q, &gravity);
  mpu.ConvertToDegrees(ypr, xyz);
  //    Serial.print(xyz[0], 4); //printfloatx is a Helper Macro that works with Serial.print that I created (See #define above)
  //    Serial.print(",");
  //    Serial.print(xyz[1], 4);
  //    Serial.print(",");
  //    Serial.print(xyz[2], 4);
  //    Serial.print("  ;   ");
  //    Serial.print(accel[0]);
  //    Serial.print(",");
  //    Serial.print(accel[1]);
  //    Serial.print(",");
  //    Serial.print(accel[2]);
  //    Serial.println();

  //  yaw = xyz[0];
  //  pitch = xyz[1];
  //  roll = xyz[2];

  dtostrf(xyz[0], 6, 2, yawStr);
  dtostrf(xyz[1], 6, 2, pitchStr);
  dtostrf(xyz[2], 6, 2, rollStr);

  sprintf(giro, "%s,|%s,|%s", yawStr, pitchStr, rollStr);
}
//}

void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


void sendTCPMessage(String message) {

  int len = message.length();
  sendATCommand("AT+CIPSEND=1," + String(len));

  SIM7670Serial.println(message);

}

void sendATCommand(String cmd) {
  SIM7670Serial.println(cmd);
  //delay(500); // Espera 500ms para la respuesta
  while (SIM7670Serial.available()) {
    Serial.write(SIM7670Serial.read());
  }
}
