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

#define IN1 9
#define IN2 8
#define ENA 10
#define IN3 2
#define IN4 4
#define ENB 3


void doDrill(int status) {
  if (1) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    analogWrite(ENA, 180);
    Serial.println("  >> Drill ON");
    toDisplay.println("Drill: ON,Running.....");
  } else if(2){
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    analogWrite(ENA, 180);
    Serial.println("  >> Drill ON BACK");
    toDisplay.println("Drill: ON BACK,Running.....");
  }else{
    digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    Serial.println("  >> Drill OFF");
    toDisplay.println("Drill: OFF,Stopped.");
  }
}

void doSens(int status){
  if (1){
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, 180);
    Serial.println("  >> Sens ON");
    toDisplay.println("SENS: ON, Running.....");
  }else if(2){
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, 180);
    Serial.println("  >> Sens ON BACK");
    toDisplay.println("SENS: ON BACK, Running.....");
  }else{
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0);
    Serial.println("  >> Sens OFF");
    toDisplay.println("SENS: OFF, Stopped. ");
  }
}

void setup() {
  Serial.begin(9600);
  toDisplay.begin(9600);
  
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

    if      (cmd == "drill on"){  doDrill(1);
    delay()
    }
    else if (cmd == "drill off") doDrill(0);
    else {
      Serial.println("  Unknown. Use: drill on / drill off");
    }
  }
}
