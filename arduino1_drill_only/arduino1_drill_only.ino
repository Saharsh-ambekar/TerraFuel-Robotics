// ── MASTER ARDUINO — DRILL ONLY ─────────────
// Controls seed drill motor via L298N
// Communicates with Display Arduino via SoftwareSerial
//
// PIN MAP:
//   L298N IN1 → D2  (Drill direction A)
//   L298N IN2 → D3  (Drill direction B)
//   L298N ENA → D10 (Drill speed PWM)
//   TX to Display Arduino → D6
//   RX from Display Arduino → D7
//
// SERIAL COMMANDS (USB):
//   drill on  → Start drill
//   drill off → Stop  drill
// ─────────────────────────────────────────────

#include <SoftwareSerial.h>
SoftwareSerial toDisplay(6, 7); // TX=6, RX=7

#define IN1 2
#define IN2 3
#define ENA 10

void doDrill(bool on) {
  if (on) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    analogWrite(ENA, 180);
    Serial.println("  >> Drill ON");
    toDisplay.println("Drill: ON,Running.....");
  } else {
    digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    Serial.println("  >> Drill OFF");
    toDisplay.println("Drill: OFF,Stopped.");
  }
}

void setup() {
  Serial.begin(9600);
  toDisplay.begin(9600);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);

  Serial.println("================================");
  Serial.println("  Drill System Ready            ");
  Serial.println("  drill on / drill off          ");
  Serial.println("================================");
  toDisplay.println("Drill System,Ready...");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if      (cmd == "drill on")  doDrill(true);
    else if (cmd == "drill off") doDrill(false);
    else {
      Serial.println("  Unknown. Use: drill on / drill off");
    }
  }
}
