// ── MASTER ARDUINO — PUMP ONLY ──────────────
// Controls both pumps via L298N
// Communicates with Display Arduino via SoftwareSerial
//
// PIN MAP:
//   L298N IN3 → D11 (Pump 1)
//   L298N IN4 → D12 (Pump 2)
//   TX to Display Arduino → D6
//   RX from Display Arduino → D7
//
// SERIAL COMMANDS (USB):
//   pump on  → Start both pumps
//   pump off → Stop  both pumps
// ─────────────────────────────────────────────

#include <SoftwareSerial.h>
SoftwareSerial toDisplay(6, 7); // TX=6, RX=7

#define IN3 11  // Pump 1
#define IN4 12  // Pump 2

void doPumps(bool on) {
  if (on) {
    digitalWrite(IN3, HIGH); digitalWrite(IN4, HIGH);
    Serial.println("  >> Pumps ON");
    toDisplay.println("Pumps: ON,Watering.....");
  } else {
    digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);
    Serial.println("  >> Pumps OFF");
    toDisplay.println("Pumps: OFF,Stopped.");
  }
}

void setup() {
  Serial.begin(9600);
  toDisplay.begin(9600);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);

  Serial.println("================================");
  Serial.println("  Pump System Ready             ");
  Serial.println("  pump on / pump off            ");
  Serial.println("================================");
  toDisplay.println("Pump System,Ready...");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if      (cmd == "pump on")  doPumps(true);
    else if (cmd == "pump off") doPumps(false);
    else {
      Serial.println("  Unknown. Use: pump on / pump off");
    }
  }
}
