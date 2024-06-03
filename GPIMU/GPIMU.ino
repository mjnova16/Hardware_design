
#include <TinyGPS.h>


TinyGPS gps;


char fec[32];
char hor[32];
float flat, flon;
unsigned long age;

//----------------------------------------------------------------------------------------------------------------------------------
#include "Simple_MPU6050.h"
#define MPU6050_ADDRESerial3_AD0_LOW     0x68 // addreSerial3 pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESerial3_AD0_HIGH    0x69 // addreSerial3 pin high (VCC)
#define MPU6050_DEFAULT_ADDRESerial3     MPU6050_ADDRESerial3_AD0_LOW
Simple_MPU6050 mpu;

//----------------------------------------------------------------------------------------------------------------------------------


int err1 = 13;


char flatStr[15]; // Ajusta el tamaño según la longitud máxima que esperas
char flonStr[15];
char yawStr[15];
char pitchStr[15];
char rollStr[15];
char giro[30];

//SoftwareSerial Serial2(3, 4); // RX, TX


void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
  //Serial3.setTimeout(100);
  pinMode(err1, OUTPUT);

  //------------------------------------------------------------------
  mpu.begin();
  mpu.Set_DMP_Output_Rate_Hz(100);           // Set the DMP output rate from 200Hz to 5 Minutes.

#ifdef OFFSETS
  mpu.SetAddreSerial3(MPU6050_DEFAULT_ADDRESerial3);
  mpu.load_DMP_Image(OFFSETS); // Does it all for you
#else
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESerial3);
  mpu.CalibrateMPU();
  mpu.load_DMP_Image();// Does it all for you with Calibration
#endif
  mpu.on_FIFO(printpropio);


  //===================    4G conexión  =================================
  Serial2.begin(115200);

  // Iniciar el servicio TCP/IP
  sendATCommand("AT+NETOPEN");

  // Configurar el modo de aplicación TCP/IP
  sendATCommand("AT+CIPMODE=0");

  // Configurar el APN (AcceSerial3 Point Name)
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
  gps.f_get_position(&flat, &flon, &age);
  obtenerFechaHora(fec, hor);
  char data[200];
  mpu.dmp_read_fifo(false);

  dtostrf(flat, 10, 6, flatStr);
  dtostrf(flon, 10, 6, flonStr);

  sprintf(data, "%s,%s,%s,%s|%s|",
          flatStr, flonStr, fec, hor, giro);


   Serial.println(data);
  sendTCPMessage(data);
  smartdelay(2000);

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
//  Serial.print(accel[0]);
//  Serial.print(",");
//  Serial.print(accel[1]);
//  Serial.print(",");
//  Serial.print(accel[2]);
//  Serial.println();

  //  yaw = xyz[0];
  //  pitch = xyz[1];
  //  roll = xyz[2];

  dtostrf(xyz[0], 6, 2, yawStr);
  dtostrf(xyz[1], 6, 2, pitchStr);
  dtostrf(xyz[2], 6, 2, rollStr);

  sprintf(giro, "%s,%s,%s", yawStr, pitchStr, rollStr);
}
//}

void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}


void sendTCPMessage(String message) {
  int len = message.length();
  sendATCommand("AT+CIPSEND=1," + String(len));
  Serial2.println(message);

}

void sendATCommand(String cmd) {
  Serial2.println(cmd);
  //delay(500); // Espera 500ms para la respuesta
  while (Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
