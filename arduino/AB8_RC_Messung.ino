/* =====================================================================
   Arbeitsblatt 8  --  "Wir bauen uns ein CASSY"
   Messgeraet fuer die Entladekurve eines Kondensators
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
        DANACH als CSV gesendet. Senden waehrend der Messung
        wuerde die Zeitbasis zerstoeren.

   Eingestellt ist die Hauptmessung mit R = 47 kOhm:
        tau = 4,7 ms,  Messdauer 99 * 200 us = 19,8 ms = 4,2 tau
   Fuer einen anderen Widerstand wird nur DT geaendert.
   ===================================================================== */

const int PIN_RC   = 2;      // laedt und entlaedt
const int PIN_MESS = A0;     // misst die Kondensatorspannung

/* --- DT muss zum verwendeten Widerstand passen ---------------------- */
const int           N        = 100;    // Anzahl der Messpunkte
const unsigned long DT       = 200;    // Abstand der Messpunkte in us
const unsigned long LADEZEIT = 500;    // Ladedauer in ms  (>= 7*tau)
const unsigned long PAUSE    = 3000;   // Wartezeit bis zur naechsten Messung
/* -------------------------------------------------------------------- */

int werte[N];                // der Messwertspeicher

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RC, OUTPUT);
}

void loop() {
  /* 1. Laden ------------------------------------------------------- */
  digitalWrite(PIN_RC, HIGH);
  delay(LADEZEIT);

  /* 2. Entladen und messen ----------------------------------------- */
  digitalWrite(PIN_RC, LOW);
  unsigned long t0 = micros();          // t = 0 ist uns exakt bekannt

  for (int i = 0; i < N; i++) {
    while (micros() - t0 < (unsigned long)i * DT) {
      ;                                 // warten, bis der Takt faellig ist
    }
    werte[i] = analogRead(PIN_MESS);
  }
  unsigned long dauer = micros() - t0;  // zur Kontrolle der Zeitbasis

  /* 3. Erst jetzt senden ------------------------------------------- */
  Serial.println();
  Serial.print(F("# Punkte: "));            Serial.print(N);
  Serial.print(F("  Takt: "));              Serial.print(DT);
  Serial.print(F(" us  Messdauer soll: "));
  Serial.print((unsigned long)(N - 1) * DT);
  Serial.print(F(" us  ist: "));            Serial.print(dauer);
  Serial.println(F(" us"));
  Serial.println(F("t_us;adc"));

  for (int i = 0; i < N; i++) {
    Serial.print((unsigned long)i * DT);
    Serial.print(';');
    Serial.println(werte[i]);
  }

  delay(PAUSE);             // Pause, dann misst das Geraet erneut
}
