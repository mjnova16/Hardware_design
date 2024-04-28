#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

SoftwareSerial SIM7670Serial(3, 4); // RX, TX


int i = 0;




const int RX = 8;
const int TX = 9;

AltSoftSerial gpsSerial;



void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gpsSerial.setTimeout(100);
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
  // Envía el mensaje TCP

  //  sprintf(mensajito, "Hola Mensaje #%i", i);
  //
  //  sendTCPMessage(mensajito);
  //  Serial.print("Mensaje #");
  //  Serial.print(i);
  //  Serial.println(" enviado");
  //
  //  delay(2000); // Espera 5 segundos antes de enviar el siguiente mensaje
  //  i = i + 1;


  //=================================================================0

  static String currentSentence;

  while (gpsSerial.available() > 0) {
    char data = gpsSerial.read();

    if (data == '$') {
      if (currentSentence.startsWith("$GPRMC")) {
        // Procesa la sentencia $GPRMC
        //String mensaje = processGPRMC(currentSentence);
        sendTCPMessage(currentSentence);
        Serial.println(currentSentence);

      }
      currentSentence = "$";
    } else {
      currentSentence += data;
    }
  }

  delay(4000);
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
