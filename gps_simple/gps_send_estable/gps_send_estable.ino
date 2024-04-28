#include <AltSoftSerial.h>

const int RX = 8;
const int TX = 9;

AltSoftSerial gpsSerial;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gpsSerial.setTimeout(100); // Establece un tiempo de espera para la lectura de GPS
}

void loop() {
  static String currentSentence;

  while (gpsSerial.available() > 0) {
    char data = gpsSerial.read();

    if (data == '$') {
      if (currentSentence.startsWith("$GPRMC")) {
        // Procesa la sentencia $GPRMC
        processGPRMC(currentSentence);
      }
      currentSentence = "$";
    } else {
      currentSentence += data;
    }
  }
}

void processGPRMC(String sentence) {
  // Aquí puedes hacer lo que necesites con la sentencia $GPRMC
  Serial.println("Sentencia $GPRMC recibida:");
  Serial.println(sentence);
}
