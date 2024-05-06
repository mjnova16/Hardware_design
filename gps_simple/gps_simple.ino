#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(7, 6);

char fec[32];
char hor[32];


void setup() {
  Serial.begin(9600);//230400
  ss.begin(9800);
}


void loop() {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);


  Serial.print("LAT: ");
  Serial.print(flat, 6);
  Serial.print("  Lon: ");
  Serial.print(flon, 6);
  Serial.print("  ");


  obtenerFechaHora(fec, hor);

  // Usa las variables "fecha" y "hora" según sea necesario
  Serial.print("Fecha: ");
  Serial.print(fec);
  Serial.print("Hora: ");
  Serial.print(hor);

  Serial.println();

  smartdelay(1000);
}


/*============================================================================*/

void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


void print_date() {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  smartdelay(0);
}

void obtenerFechaHora(char* fecha, char* hora) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  // Guardar fecha y hora en variables separadas
  sprintf(fecha, "%02d/%02d/%02d", month, day, year);
  sprintf(hora, "%02d:%02d:%02d", hour, minute, second);
}
