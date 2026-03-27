// ── MASTER ARDUINO — DRILL + PUMP ───────────
// Controls drill and pumps via L298N
// Communicates with Display Arduino via SoftwareSerial
//
// PIN MAP:
//   L298N IN1 → D2  (Drill direction A)
//   L298N IN2 → D3  (Drill direction B)
//   L298N ENA → D10 (Drill speed PWM)
//   L298N IN3 → D11 (Pump 1)
//   L298N IN4 → D12 (Pump 2)
//   TX to Display Arduino → D6
//   RX from Display Arduino → D7
//
// SERIAL COMMANDS (USB):
//   drill on  / drill off
//   pump on   / pump off
// ─────────────────────────────────────────────

#include <SoftwareSerial.h>
SoftwareSerial toDisplay(6, 7); // TX=6, RX=7

#define IN1 2
#define IN2 3
#define ENA 10
#define IN3 11  // Pump 1
#define IN4 12  // Pump 2

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
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);

  Serial.println("================================");
  Serial.println("  Drill + Pump System Ready     ");
  Serial.println("  drill on/off | pump on/off    ");
  Serial.println("================================");
  toDisplay.println("Drill+Pump,Ready...");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if      (cmd == "drill on")  doDrill(true);
    else if (cmd == "drill off") doDrill(false);
    else if (cmd == "pump on")   doPumps(true);
    else if (cmd == "pump off")  doPumps(false);
    else {
      Serial.println("  Unknown command.");
      Serial.println("  drill on/off | pump on/off");
    }
  }
}
