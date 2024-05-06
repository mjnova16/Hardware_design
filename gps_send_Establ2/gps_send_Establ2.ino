#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

SoftwareSerial SIM7670Serial(3, 4); // RX, TX

const int RX = 8;
const int TX = 9;
AltSoftSerial gpsSerial;

//----------------------------------------------------------------------------------------------------------------------------------
#include "Simple_MPU6050.h"
#define MPU6050_ADDRESS_AD0_LOW     0x68 // address pin low (GND), default for InvenSense evaluation board
#define MPU6050_ADDRESS_AD0_HIGH    0x69 // address pin high (VCC)
#define MPU6050_DEFAULT_ADDRESS     MPU6050_ADDRESS_AD0_LOW
Simple_MPU6050 mpu;
#define OFFSETS  -3632,    -160,     498,     101,     -73,      -1
//----------------------------------------------------------------------------------------------------------------------------------

char yawStr[15];
char pitchStr[15];
char rollStr[15];
char giro[30];
char dato[200];

char coordenadas[100];

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gpsSerial.setTimeout(100);

  //------------------------------------------------------------------
  mpu.begin();
  mpu.Set_DMP_Output_Rate_Hz(100);           // Set the DMP output rate from 200Hz to 5 Minutes.

#ifdef OFFSETS
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.load_DMP_Image(OFFSETS); // Does it all for you
#else
  mpu.SetAddress(MPU6050_DEFAULT_ADDRESS);
  mpu.CalibrateMPU();
  mpu.load_DMP_Image();// Does it all for you with Calibration
#endif
  mpu.on_FIFO(printpropio);

  //-------------------------------------------------------------
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
  //gpsSerial.begin(9600);
}

void loop() {
  mpu.dmp_read_fifo(false);

  static String currentSentence;

  while (gpsSerial.available() > 0) {
    char data = gpsSerial.read();

    if (data == '$') {
      if (currentSentence.startsWith("$GPRMC")) {

        currentSentence.toCharArray(coordenadas, 100);
        //Serial.println(currentSentence);
        int len = strlen(coordenadas);

        // Verificar si la cadena tiene al menos dos caracteres
        if (len >= 2) {
          // Eliminar los dos últimos caracteres
          coordenadas[len - 2] = '\0';
        }
        sprintf(dato, "%s ,%s", coordenadas, giro);

        Serial.println(dato);
        sendTCPMessage(dato);

      }
      currentSentence = "$";
    } else {
      currentSentence += data;
    }



  }




  delay(1000);
}

void sendTCPMessage(String message) {
  int len = message.length();
  sendATCommand("AT+CIPSEND=1," + String(len));
  SIM7670Serial.println(message);

}

void sendATCommand(String cmd) {
  SIM7670Serial.println(cmd);
  delay(500); // Espera 500ms para la respuesta
  while (SIM7670Serial.available()) {
    Serial.write(SIM7670Serial.read());
  }
}

void printpropio(int16_t *gyro, int16_t *accel, int32_t *quat) { //, uint16_t SpamDelay = 100) {
  Quaternion q;
  VectorFloat gravity;
  float ypr[3] = { 0, 0, 0 };
  float xyz[3] = { 0, 0, 0 };
  mpu.GetQuaternion(&q, quat);
  mpu.GetGravity(&gravity, &q);
  mpu.GetYawPitchRoll(ypr, &q, &gravity);
  mpu.ConvertToDegrees(ypr, xyz);

  dtostrf(xyz[0], 6, 2, yawStr);
  dtostrf(xyz[1], 6, 2, pitchStr);
  dtostrf(xyz[2], 6, 2, rollStr);

  sprintf(giro, "%s,%s,%s", yawStr, pitchStr, rollStr);
}
