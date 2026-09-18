/*
  Servo als Zeigerinstrument - Fachpraxis 12 (Muster-Sketch)

  Ein Potentiometer (Winkelgeber, z. B. am Schwimmerhebel) wird eingelesen
  und auf den Winkelbereich eines Servos SG90 abgebildet: analoge Anzeigenadel.

  Verdrahtung:
    Poti:   Aussenanschluesse an 5V und GND, Schleifer -> A0
    Servo:  braun/schwarz -> GND, rot -> 5V, orange/gelb (Signal) -> D9

  Hinweis: Die Bibliothek Servo.h belegt Timer1. Deshalb funktioniert
  analogWrite() auf den Pins 9 und 10 NICHT mehr. LEDs zum Dimmen also
  auf 3, 5, 6 oder 11 legen.
*/

#include <Servo.h>

Servo zeiger;

const byte POTI_PIN   = A0;
const byte SERVO_PIN  = 9;

// Kalibrierwerte aus der eigenen Messreihe eintragen!
// (Messwerte Jahrgang NP2025 am Demo-Aufbau: leer 620, voll 993)
const int ADC_LEER = 620;
const int ADC_VOLL = 993;

// Mechanische Reserve: nicht bis an die Endanschlaege fahren, sonst zittert der Servo
const int WINKEL_MIN = 5;
const int WINKEL_MAX = 175;

const byte ANZAHL_MESSUNGEN = 10;   // Mittelwert gegen ADC-Rauschen
const int  TOTBAND = 2;             // Grad; darunter wird die Nadel nicht bewegt

int letzterWinkel = -100;

void setup() {
  zeiger.attach(SERVO_PIN);
  Serial.begin(9600);
  zeiger.write(WINKEL_MIN);          // Nadel definiert auf Nullstellung fahren
  delay(500);
}

int lesePotiGemittelt() {
  long summe = 0;
  for (byte i = 0; i < ANZAHL_MESSUNGEN; i++) {
    summe += analogRead(POTI_PIN);
    delay(2);
  }
  return summe / ANZAHL_MESSUNGEN;
}

void loop() {
  int rohwert = lesePotiGemittelt();

  // Kalibrierbereich begrenzen, damit map() keine Winkel ausserhalb 0..180 liefert
  int begrenzt = constrain(rohwert, ADC_LEER, ADC_VOLL);

  int prozent = map(begrenzt, ADC_LEER, ADC_VOLL, 0, 100);
  int winkel  = map(begrenzt, ADC_LEER, ADC_VOLL, WINKEL_MIN, WINKEL_MAX);

  // Nur bewegen, wenn sich wirklich etwas geaendert hat -> Nadel steht ruhig
  if (abs(winkel - letzterWinkel) >= TOTBAND) {
    zeiger.write(winkel);
    letzterWinkel = winkel;
  }

  Serial.print(rohwert);
  Serial.print('\t');
  Serial.print(prozent);
  Serial.print('\t');
  Serial.println(winkel);

  delay(50);                         // 20 Hz reichen fuer eine Anzeige voellig
}
