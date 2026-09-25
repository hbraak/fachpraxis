/*
  Start_Schwimmergeber - Fachpraxis 12, AB 7 (Jahrgang NP2025)

  Startpunkt fuer Teil 3 und Teil 4: Der Sketch liest den Geber an A0 ein,
  mittelt gegen das ADC-Rauschen und gibt EINE Zahl pro Zeile aus. Damit
  laeuft sowohl der Serielle Monitor als auch der Serielle Plotter.

  Anschluss: Poti aussen an 5V und GND, Schleifer an A0.

  Bewusst NICHT enthalten: die Umrechnung des Rohwerts in Prozent.
  Die baut ihr in Teil 5 selbst ein - das ist die eigentliche Aufgabe.

  Zusatzfunktion fuer die Messreihe:
  Im Seriellen MONITOR ein beliebiges Zeichen abschicken -> der Sketch
  meldet den kleinsten und groessten Wert seit dem letzten Bericht und
  faengt danach neu an zu zaehlen. Praktisch, um die Anschlaege aus
  Aufgabe 9 und die Stuetzstellen aus Teil 4 sauber abzulesen.
  (Im PLOTTER stoert dieser Textblock die Kurve - dort einfach nichts senden.)
*/

const byte POTI_PIN         = A0;
const byte ANZAHL_MESSUNGEN = 10;   // Mittelwert gegen ADC-Rauschen
const int  TAKT_MS          = 50;   // zusammen mit der Mittelung rund 14 Werte/s

int minWert = 1023;
int maxWert = 0;

void setup() {
  Serial.begin(9600);
  delay(300);
}

int lesePotiGemittelt() {
  long summe = 0;
  for (byte i = 0; i < ANZAHL_MESSUNGEN; i++) {
    summe += analogRead(POTI_PIN);
    delay(2);
  }
  return summe / ANZAHL_MESSUNGEN;
}

void zeigeBericht(int aktuell) {
  int spanne = maxWert - minWert;

  Serial.println();
  Serial.println(F("--- Bericht ---"));
  Serial.print(F("aktuell : ")); Serial.println(aktuell);
  Serial.print(F("kleinster: ")); Serial.println(minWert);
  Serial.print(F("groesster: ")); Serial.println(maxWert);
  Serial.print(F("Spanne  : ")); Serial.println(spanne);

  // Hinweise, die auf eine Auffaelligkeit zeigen - die Ursache findet ihr selbst.
  if (spanne < 10) {
    Serial.println(F("Der Wert hat sich kaum bewegt."));
  }
  if (minWert > 1000) {
    Serial.println(F("Der Wert klebt oben am Anschlag (1023)."));
  }
  if (maxWert < 20) {
    Serial.println(F("Der Wert klebt unten am Anschlag (0)."));
  }

  Serial.println(F("--- Zaehlung neu gestartet ---"));
  Serial.println();

  minWert = 1023;
  maxWert = 0;
}

void loop() {
  int rohwert = lesePotiGemittelt();

  if (rohwert < minWert) minWert = rohwert;
  if (rohwert > maxWert) maxWert = rohwert;

  if (Serial.available() > 0) {
    while (Serial.available() > 0) Serial.read();   // Eingabepuffer leeren
    zeigeBericht(rohwert);
  }

  Serial.println(rohwert);

  delay(TAKT_MS);
}
