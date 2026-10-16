/* =====================================================================
   Arbeitsblatt 9  --  "Wie schnell bist du wirklich?"
   Ein Messgeraet fuer Reaktionszeiten
   Fachpraxis 12NP, Lore-Lorentz-Schule Duesseldorf

   SCHALTUNG
       D8 ---[ 220 Ohm ]---|>|--- GND        (LED, das Startsignal)

       D2 ---+--- Taster ---- GND            (der Eingang)
             |
         INPUT_PULLUP im Arduino
         -> Taster offen  = HIGH
         -> Taster gedrueckt = LOW

   ABLAUF einer Messung
     1. LED aus, der Arduino wartet eine ZUFAELLIGE Zeit (2 bis 5 s).
        Wer in dieser Zeit drueckt, bekommt einen FEHLSTART.
     2. LED an  -> in genau diesem Moment wird t0 = micros() gemerkt.
     3. Der Arduino fragt den Taster in einer engen Schleife ab und
        merkt sich t1, sobald er LOW wird.
     4. Reaktionszeit = t1 - t0. Nebenbei wird gezaehlt, WIE OFT der
        Taster in dieser Zeit abgefragt wurde -- daraus laesst sich
        die Abtastrate des Geraets ausrechnen.
     5. Gesendet wird ERST NACH der Messung. Waehrend der Zeitmessung
        darf nichts gesendet werden, sonst wandert die Sendedauer in
        das Messergebnis.

   Nach N_MESSUNGEN Versuchen gibt das Geraet die ganze Messreihe als
   Tabelle (CSV) aus. Mittelwert und Streuung rechnet ihr selbst --
   das Geraet liefert nur die Rohdaten.

   Serieller Monitor:  115200 Baud
   ===================================================================== */

const int PIN_LED    = 8;     // Startsignal
const int PIN_TASTER = 2;     // Eingang, gegen GND geschaltet

/* --- Messparameter --------------------------------------------------- */
const int           N_MESSUNGEN = 10;      // Versuche pro Messreihe
const unsigned long WARTE_MIN   = 2000;    // kuerzeste Wartezeit (ms)
const unsigned long WARTE_MAX   = 5000;    // laengste  Wartezeit (ms)
const unsigned long ABKLINGEN   = 400;     // Pause nach dem Druecken (ms)
const unsigned long GRENZE      = 2000000; // Abbruch nach 2 s (us)
/* -------------------------------------------------------------------- */

unsigned long zeit[N_MESSUNGEN];       // Reaktionszeiten in us
unsigned long takte[N_MESSUNGEN];      // Abfragen je Messung
int           n = 0;                   // gueltige Messungen bisher

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TASTER, INPUT_PULLUP);
  digitalWrite(PIN_LED, LOW);

  /* Der Zufallsgenerator wird aus dem Rauschen eines offenen
     Analogeingangs gestartet -- sonst kaeme nach jedem Reset
     dieselbe Folge von Wartezeiten heraus. */
  randomSeed(analogRead(A5));

  Serial.println(F("\n===== Reaktionszeit-Messgeraet ====="));
  Serial.print(F("Messreihe mit "));
  Serial.print(N_MESSUNGEN);
  Serial.println(F(" Versuchen. Taster druecken, sobald die LED leuchtet."));
  delay(1500);
}

/* ---- wartet zufaellig lange und meldet einen Fehlstart ------------- */
bool wartephase() {
  unsigned long warte = random(WARTE_MIN, WARTE_MAX + 1);
  unsigned long t = millis();
  while (millis() - t < warte) {
    if (digitalRead(PIN_TASTER) == LOW) return false;   // zu frueh
  }
  return true;
}

/* ---- die eigentliche Zeitmessung ----------------------------------- */
bool messen(unsigned long &dauer, unsigned long &abfragen) {
  abfragen = 0;
  digitalWrite(PIN_LED, HIGH);
  unsigned long t0 = micros();          // t = 0: der Arduino setzt es selbst
  unsigned long t1;
  while (true) {
    abfragen++;
    if (digitalRead(PIN_TASTER) == LOW) {
      t1 = micros();                    // Stoppzeit SOFORT merken
      break;
    }
    if (micros() - t0 > GRENZE) {       // niemand hat gedrueckt
      digitalWrite(PIN_LED, LOW);
      return false;
    }
  }
  digitalWrite(PIN_LED, LOW);
  dauer = t1 - t0;
  return true;
}

/* ---- Ausgabe der fertigen Messreihe -------------------------------- */
void reihe_ausgeben() {
  unsigned long kleinste = zeit[0], groesste = zeit[0];
  for (int i = 1; i < n; i++) {
    if (zeit[i] < kleinste) kleinste = zeit[i];
    if (zeit[i] > groesste) groesste = zeit[i];
  }

  Serial.println(F("\n--- MESSREIHE ------------------------------"));
  Serial.print(F("# Versuche: "));   Serial.print(n);
  Serial.print(F("   kleinste: ")); Serial.print(kleinste / 1000.0, 1);
  Serial.print(F(" ms   groesste: ")); Serial.print(groesste / 1000.0, 1);
  Serial.print(F(" ms   Spannweite: "));
  Serial.print((groesste - kleinste) / 1000.0, 1);
  Serial.println(F(" ms"));
  Serial.println(F("nr;t_us;t_ms;abfragen"));
  for (int i = 0; i < n; i++) {
    Serial.print(i + 1);              Serial.print(';');
    Serial.print(zeit[i]);            Serial.print(';');
    Serial.print(zeit[i] / 1000.0, 1); Serial.print(';');
    Serial.println(takte[i]);
  }
  Serial.println(F("--- Ende der Messreihe. Reset fuer eine neue. ---"));
}

void loop() {
  if (n >= N_MESSUNGEN) return;         // Messreihe fertig, nichts mehr tun

  Serial.print(F("\nVersuch "));
  Serial.print(n + 1);
  Serial.print('/');
  Serial.print(N_MESSUNGEN);
  Serial.println(F("  -- Achtung ..."));

  if (!wartephase()) {
    Serial.println(F("FEHLSTART: zu frueh gedrueckt. Versuch wird verworfen."));
    while (digitalRead(PIN_TASTER) == LOW) { ; }   // Taster loslassen
    delay(ABKLINGEN);
    return;
  }

  unsigned long dauer, abfragen;
  if (!messen(dauer, abfragen)) {
    Serial.println(F("ABBRUCH: in 2 s kein Tastendruck."));
    delay(ABKLINGEN);
    return;
  }

  zeit[n]  = dauer;
  takte[n] = abfragen;
  n++;

  /* Jetzt -- nach der Messung -- darf gesendet werden. */
  Serial.print(F("  Reaktionszeit: "));
  Serial.print(dauer / 1000.0, 1);
  Serial.print(F(" ms   ("));
  Serial.print(dauer);
  Serial.print(F(" us, "));
  Serial.print(abfragen);
  Serial.println(F(" Abfragen)"));

  while (digitalRead(PIN_TASTER) == LOW) { ; }     // Taster loslassen
  delay(ABKLINGEN);                                 // Prellen abwarten

  if (n >= N_MESSUNGEN) reihe_ausgeben();
}
