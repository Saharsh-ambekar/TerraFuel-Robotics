// ── MASTER ARDUINO — FULL MERGED ────────────
// Controls pH sensor, drill, and pumps
// Sends status to Display Arduino via SoftwareSerial
//
// PIN MAP:
//   pH Sensor      → A1
//   L298N IN1      → D2  (Drill direction A)
//   L298N IN2      → D3  (Drill direction B)
//   L298N ENA      → D10 (Drill speed PWM)
//   L298N IN3      → D11 (Pump 1)
//   L298N IN4      → D12 (Pump 2)
//   TX to Display  → D6
//   RX from Display→ D7
//
// SERIAL COMMANDS (USB):
//   a / A       → Simulate Acidic  soil
//   b / B       → Simulate Basic   soil
//   n / N       → Simulate Neutral soil
//   drill on    → Start drill
//   drill off   → Stop  drill
//   pump on     → Start both pumps
//   pump off    → Stop  both pumps
// ─────────────────────────────────────────────

#include <SoftwareSerial.h>
SoftwareSerial toDisplay(6, 7); // TX=6, RX=7

#define PH_PIN A1
#define IN1 2
#define IN2 3
#define ENA 10
#define IN3 11  // Pump 1
#define IN4 12  // Pump 2

// ─── Simulation State ────────────────────────
float phBase = 0.0;
float phAmp  = 0.0;
float maxAmp = 0.4;
int   sign   = 1;
bool  running = false;

// ─── Helpers ─────────────────────────────────
String soilNature(float ph) {
  if (ph < 6.0)       return "Acidic  ";
  else if (ph <= 7.5) return "Neutral ";
  else                return "Alkaline";
}

String soilNatureShort(float ph) {
  if (ph < 6.0)       return "Acid";
  else if (ph <= 7.5) return "Neut";
  else                return "Alka";
}

void printLoadingBar(float current, float maxVal) {
  int percent = (int)(((maxVal - current) / maxVal) * 100);
  int filled  = percent / 5;
  Serial.print(" [");
  for (int i = 0; i < 20; i++) {
    if (i < filled)       Serial.print("=");
    else if (i == filled) Serial.print(">");
    else                  Serial.print(".");
  }
  Serial.print("] ");
  if (percent < 10)       Serial.print("  ");
  else if (percent < 100) Serial.print(" ");
  Serial.print(percent); Serial.print("%");
}

// ─── Actions ─────────────────────────────────
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

void startSequence(float phMin, float phMax) {
  phBase  = phMin + random(0, (int)((phMax - phMin) * 10)) / 10.0;
  phAmp   = maxAmp;
  sign    = 1;
  running = true;

  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Sending read request...");
  toDisplay.println("Request,Sending.......");
  delay(300);
  Serial.println("   Response received. Parsing...");
  toDisplay.println("Response,Received OK");
  delay(400);
  Serial.println("   Stabilising sensor signal...  ");
  Serial.println("  ================================");
  toDisplay.println("Stabilising,Signal.........");
  delay(300);
}

// ─── Boot ────────────────────────────────────
void bootSequence() {
  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Master Arduino Booting        ");
  Serial.println("  ================================");
  toDisplay.println("Smart Farming,Initialising..");
  delay(600);
  toDisplay.println("pH Sensor,Warming up.....");
  delay(800);
  toDisplay.println("Calibration,Loaded OK");
  delay(400);
  toDisplay.println("Motors+Pumps,Standby OK");
  delay(400);

  Serial.println("  All systems ready.");
  Serial.println("  ================================");
  Serial.println("  pH: a=Acid  b=Basic  n=Neutral  ");
  Serial.println("  drill on/off  |  pump on/off     ");
  Serial.println("  ================================");

  toDisplay.println("Master Ready,Awaiting cmd..");
}

// ─── Setup ───────────────────────────────────
void setup() {
  Serial.begin(9600);
  toDisplay.begin(9600);
  randomSeed(analogRead(2));
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  delay(500);
  bootSequence();
}

// ─── Main Loop ───────────────────────────────
void loop() {
  if (!running) {
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();

      if      (cmd == "a" || cmd == "A") startSequence(3.0, 6.9);
      else if (cmd == "b" || cmd == "B") startSequence(7.1, 10.0);
      else if (cmd == "n" || cmd == "N") startSequence(6.0, 7.5);
      else if (cmd == "drill on")        doDrill(true);
      else if (cmd == "drill off")       doDrill(false);
      else if (cmd == "pump on")         doPumps(true);
      else if (cmd == "pump off")        doPumps(false);
      else {
        Serial.println("  Unknown command.");
        Serial.println("  pH: a/b/n | drill on/off | pump on/off");
      }
    }
  }

  // ─── pH Oscillation Tick ─────────────────
  if (running) {
    float currentPH = phBase + (sign * phAmp);
    currentPH = constrain(currentPH, 0.0, 14.0);

    Serial.print("  [pH] ");
    if (currentPH < 10.0) Serial.print(" ");
    Serial.print(currentPH, 2);
    Serial.print("  |  ");
    Serial.print(soilNature(currentPH));
    Serial.print(" |");
    printLoadingBar(phAmp, maxAmp);
    Serial.println();

    // Send live pH to display
    String dispLine = "pH:" + String(currentPH, 2) + " " + soilNatureShort(currentPH);
    toDisplay.println(dispLine + ",Stabilising...");

    phAmp -= 0.02;
    phAmp  = max(phAmp, 0.0);
    sign  *= -1;

    if (phAmp <= 0.02) {
      running = false;

      Serial.println();
      Serial.println("  ================================");
      Serial.println("   Signal stabilised.            ");
      Serial.print  ("   FINAL pH     : "); Serial.println(phBase, 2);
      Serial.print  ("   Soil Nature  : "); Serial.println(soilNature(phBase));
      Serial.println("  ================================");
      Serial.println("  pH: a/b/n | drill on/off | pump on/off");
      Serial.println();

      String finalLine = "pH:" + String(phBase, 2) + " " + soilNatureShort(phBase) + " DONE";
      toDisplay.println(finalLine + "," + soilNature(phBase));
    }

    delay(350);
  }
}
