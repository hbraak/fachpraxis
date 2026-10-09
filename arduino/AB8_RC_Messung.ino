/* =====================================================================
   Arbeitsblatt 8  --  "Wir bauen uns ein CASSY"
   Messgeraet fuer die Lade- und Entladekurve eines Kondensators
   Fachpraxis 12NP, Lore-Lorentz-Schule Duesseldorf

   SCHALTUNG
       D2 ---[ R ]---+--- A0
                     |
                   C 100 nF
                     |
                    GND

   ABLAUF des Geraets
     1. D2 auf HIGH  -> Kondensator laedt ueber R auf
     2. D2 auf LOW   -> Kondensator entlaedt ueber denselben R,
                        ab diesem Moment laeuft die Zeitmessung
     3. Die Messwerte werden ERST im Speicher gesammelt und
        DANACH gesendet. Senden waehrend der Messung wuerde die
        Zeitbasis zerstoeren: eine Zeile mit 50 Balken braucht bei
        115200 Baud rund 5 ms -- mehr als ein ganzes tau.

   ===================================================================
   ZWEI BETRIEBSARTEN -- hier wird umgeschaltet:
   ===================================================================
   BALKEN = true    SEHEN.  Zeichnet Lade- und Entladekurve als
                            Balken in den Seriellen Monitor.
                            Damit fangt ihr an.
   BALKEN = false   MESSEN. Gibt die Entladekurve als Zahlentabelle
                            (CSV) aus, zum Auswerten in der
                            Tabellenkalkulation.
   ===================================================================

   Serieller Monitor:  115200 Baud
   ===================================================================== */

const bool BALKEN = true;    // <<< true = sehen,  false = messen

const int PIN_RC   = 2;      // laedt und entlaedt
const int PIN_MESS = A0;     // misst die Kondensatorspannung

/* --- Messparameter. DT muss zum verwendeten Widerstand passen ------- */
const int           N_SEHEN  = 20;     // Zeilen je Phase im Balkenmodus
const unsigned long DT_SEHEN = 2000;   // Zeilenabstand im Balkenmodus (us)

const int           N_MESSEN  = 100;   // Messpunkte im CSV-Modus
const unsigned long DT_MESSEN = 200;   // Punktabstand im CSV-Modus (us)

const unsigned long LADEZEIT = 500;    // Ladedauer in ms  (>= 7*tau)
const unsigned long PAUSE    = 3000;   // Wartezeit bis zur naechsten Messung
/* -------------------------------------------------------------------- */

int werte[N_MESSEN];                   // der Messwertspeicher
int           anzahl;                  // tatsaechlich benutzte Punktzahl
unsigned long dt;                      // tatsaechlicher Takt
unsigned long dauer;                   // gemessene Gesamtdauer

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RC, OUTPUT);
  digitalWrite(PIN_RC, LOW);
  anzahl = BALKEN ? N_SEHEN  : N_MESSEN;
  dt     = BALKEN ? DT_SEHEN : DT_MESSEN;
  delay(500);                          // Kondensator sicher leer
}

/* ---- aufnehmen: speichert anzahl Werte im Takt dt, sendet NICHTS --- */
void aufnehmen() {
  unsigned long t0 = micros();         // t = 0 ist uns exakt bekannt
  for (int i = 0; i < anzahl; i++) {
    while (micros() - t0 < (unsigned long)i * dt) {
      ;                                // warten, bis der Takt faellig ist
    }
    werte[i] = analogRead(PIN_MESS);
  }
  dauer = micros() - t0;               // zur Kontrolle der Zeitbasis
}

/* ---- Ausgabe als Balken -------------------------------------------- */
void balken_ausgeben() {
  for (int i = 0; i < anzahl; i++) {
    int adc = werte[i];
    if      (adc <   10) Serial.print(F("   "));
    else if (adc <  100) Serial.print(F("  "));
    else if (adc < 1000) Serial.print(F(" "));
    Serial.print(adc);
    Serial.print(F(" |"));
    for (int k = 0; k < adc / 20; k++) Serial.print('#');  // 1023 -> 51
    Serial.println();
  }
}

/* ---- Ausgabe als CSV ----------------------------------------------- */
void csv_ausgeben() {
  Serial.println();
  Serial.print(F("# Punkte: "));            Serial.print(anzahl);
  Serial.print(F("  Takt: "));              Serial.print(dt);
  Serial.print(F(" us  Messdauer soll: "));
  Serial.print((unsigned long)(anzahl - 1) * dt);
  Serial.print(F(" us  ist: "));            Serial.print(dauer);
  Serial.println(F(" us"));
  Serial.println(F("t_us;adc"));
  for (int i = 0; i < anzahl; i++) {
    Serial.print((unsigned long)i * dt);
    Serial.print(';');
    Serial.println(werte[i]);
  }
}

void loop() {
  if (BALKEN) {
    /* --- Betriebsart SEHEN: beide Kurven als Balken --------------- */
    Serial.println(F("\n--- LADEN ----------------------------------"));
    digitalWrite(PIN_RC, HIGH);
    aufnehmen();
    balken_ausgeben();

    Serial.println(F("--- ENTLADEN -------------------------------"));
    digitalWrite(PIN_RC, LOW);
    aufnehmen();
    balken_ausgeben();

  } else {
    /* --- Betriebsart MESSEN: Entladekurve als CSV ----------------- */
    digitalWrite(PIN_RC, HIGH);          // erst vollstaendig laden
    delay(LADEZEIT);
    digitalWrite(PIN_RC, LOW);           // Entladung startet JETZT
    aufnehmen();
    csv_ausgeben();
  }

  delay(PAUSE);                          // Pause, dann noch einmal
}
