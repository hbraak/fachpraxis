/*
  Abstandswarner – Fachpraxis 12 (Musterloesung)
  Misst den Abstand mit HC-SR04 und warnt bei weniger als 20 cm.
  Verdrahtung: TRIG -> D10, ECHO -> D11, LED (+ Vorwiderstand) -> D8,
                Piezo-Summer -> D9, jeweils GND gemeinsam verbinden.
*/

const byte TRIG_PIN = 10;
const byte ECHO_PIN = 11;
const byte LED_PIN = 8;
const byte SUMMER_PIN = 9;
const int WARN_SCHWELLE_CM = 20;
const byte ANZAHL_MESSUNGEN = 3;
const unsigned long TIMEOUT_US = 30000;  // verhindert langes Warten ohne Echo
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SUMMER_PIN, OUTPUT);
  Serial.begin(9600);
}
float messeAbstandCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);      // Startimpuls fuer den HC-SR04
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dauer = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
  if (dauer == 0) return -1;          // kein Echo innerhalb des Timeouts
  return dauer * 0.0343 / 2.0;        // Schallweg hin und zurueck
}
void loop() {
  float summe = 0;
  byte gueltigeMessungen = 0;
  for (byte i = 0; i < ANZAHL_MESSUNGEN; i++) {
    float abstand = messeAbstandCm();
    if (abstand >= 0) {
      summe += abstand;
      gueltigeMessungen++;
    }
    delay(50);
  }
  if (gueltigeMessungen > 0) {
    float mittelwert = summe / gueltigeMessungen;
    Serial.println(mittelwert);       // eine Zahl pro Zeile fuer den Serial Plotter
    if (mittelwert < WARN_SCHWELLE_CM) {
      digitalWrite(LED_PIN, HIGH);
      tone(SUMMER_PIN, 1000);
    } else {
      digitalWrite(LED_PIN, LOW);
      noTone(SUMMER_PIN);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(SUMMER_PIN);
  }
}
