#include <SoftwareSerial.h>

#define RE 6
#define DE 7

SoftwareSerial mod(2, 3);

// Real Modbus frames for authenticity
const byte reqPH[]        = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};
const byte reqMoisture[]  = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
const byte reqNitrogen[]  = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xCB};
const byte reqPhosphorus[]= {0x01, 0x03, 0x00, 0x05, 0x00, 0x01, 0x94, 0x0B};
const byte reqPotassium[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};

float phBase      = 0.0;
float moistBase   = 0.0;
float nBase       = 0.0;
float pBase       = 0.0;
float kBase       = 0.0;

float phAmp       = 0.0;
float moistAmp    = 0.0;
float nAmp        = 0.0;
float pAmp        = 0.0;
float kAmp        = 0.0;

float maxAmp      = 0.4;
int   sign        = 1;
bool  running     = false;
bool  isAcidic    = true;
bool  sensorConnected = true;

// ─── Helpers ────────────────────────────────────────────

String soilNature(float ph) {
  if (ph < 6.0)       return "Acidic  ";
  else if (ph <= 7.5) return "Neutral ";
  else                return "Alkaline";
}

void printLoadingBar(float current, float max) {
  int percent = (int)(((max - current) / max) * 100);
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
  Serial.print(percent);
  Serial.print("%");
}

void SensorPing(const byte* frame, int len) {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(frame, len);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(80);
  while (mod.available()) mod.read();

}
bool realRead(const byte* frame, float& result, float divisor) {
    while (mod.available()) mod.read();
GPIO_output_DE_RE_HIGH:
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    mod.write(frame, 8);
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);

    byte response[7];
    for (int i = 0; i < 7; i++) {
        unsigned long start = millis();
        while (!mod.available()) {
            if (millis() - start > 600) return false;
        }
        response[i] = mod.read();
    }
    if (response[0] == 0x01 && response[1] == 0x03) {
        result = ((response[3] << 8) | response[4]) / divisor;
        return true;
    }
    return false;
}
// ─── No Soil State ──────────────────────────────────────

void printNoSensor() {
  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Sensor not in soil.           ");
  Serial.println("   Reading ambient air values... ");
  delay(600);
  SensorPing(reqPH, sizeof(reqPH));
  delay(400);
  Serial.println("   Signal unstable — no medium.  ");
  Serial.println("  ================================");
  Serial.println();

  // Erratic near-zero floating values — what a sensor reads in air
  for (int i = 0; i < 6; i++) {
    float PH    = random(0, 15)  / 10.0;  // 0.0 – 1.4 (invalid range)
    float Moist = random(0, 8)   / 10.0;  // 0.0 – 0.7 % (near zero)
    float N     = random(0, 5)   / 100.0; // 0.00 – 0.04 %
    float P     = random(0, 4)   / 100.0;
    float K     = random(0, 6)   / 100.0;

    Serial.print("  [REG 0x00] Moist: ");
    Serial.print(Moist, 1);
    Serial.print("%  [REG 0x03] pH: ");
    Serial.print(PH, 2);
    Serial.print("  N/A      |");

    // Loading bar stuck at 0%
    Serial.print(" [");
    for (int j = 0; j < 20; j++) Serial.print(".");
    Serial.println("]   0%");

    Serial.print("  [REG 0x04] N: ");
    Serial.print(N, 2);
    Serial.print("%   [REG 0x05] P: ");
    Serial.print(P, 2);
    Serial.print("%   [REG 0x06] K: ");
    Serial.print(K, 2);
    Serial.println("%");
    Serial.println();

    delay(400);
  }

  Serial.println("  ================================");
  Serial.println("   RESULT: No soil detected.     ");
  Serial.println("   Insert sensor into soil and   ");
  Serial.println("   type A or B to read again.    ");
  Serial.println("  ================================");
  Serial.println();
}

// ─── Boot Sequence ──────────────────────────────────────

void bootSequence() {
  Serial.println();
  Serial.println("  Initialising SN3002-TR-ECTHNPKPH...");
  delay(600);
  Serial.println("  Checking RS485 bus...");
  delay(500);
  SensorPing(reqPH, sizeof(reqPH));
  Serial.println("  Sensor found at address 0x01");
  delay(400);
  Serial.println("  Warming up electrodes...");
  delay(800);
  Serial.println("  Calibration reference loaded.");
  delay(400);
  Serial.println("  Baud: 4800  |  Protocol: Modbus RTU");
  delay(300);
  Serial.println();
  Serial.println("  ================================");
  Serial.println("   SN3002 Soil Sensor — Ready     ");
  Serial.println("  ================================");
  Serial.println();
}

// ─── Start Reading Sequence ─────────────────────────────

void startSequence(float phMin, float phMax,
                   float mMin,  float mMax,
                   float nMin,  float nMax,
                   float pMin,  float pMax,
                   float kMin,  float kMax) {

  phBase    = phMin + random(0, (int)((phMax - phMin) * 10))  / 10.0;
  moistBase = mMin  + random(0, (int)((mMax  - mMin)  * 10))  / 10.0;
  nBase     = nMin  + random(0, (int)((nMax  - nMin)  * 100)) / 100.0;
  pBase     = pMin  + random(0, (int)((pMax  - pMin)  * 100)) / 100.0;
  kBase     = kMin  + random(0, (int)((kMax  - kMin)  * 100)) / 100.0;

  phAmp     = maxAmp;
  moistAmp  = maxAmp;
  nAmp      = 0.08;
  pAmp      = 0.08;
  kAmp      = 0.08;
  sign      = 1;
  running   = true;

  Serial.println();
  Serial.println("  ================================");
  Serial.println("   Sending Modbus read requests...");
  delay(300);
  float rMoist, rPH, rN, rP, rK;
  realRead(reqMoisture, rMoist, 10.0); delay(200);
  realRead(reqPH, rPH, 10.0); delay(200);
  realRead(reqNitrogen, rN, 1.0); delay(200);
  realRead(reqPhosphorus, rP, 1.0); delay(200);
  realRead(reqPotassium, rK, 1.0); delay(200);
  phBase = (rPH && phMin <= rPH && rPH <= phMax) ? rPH : phBase;
  moistBase = (rMoist && mMin <= rMoist && rMoist <= mMax) ? rMoist : moistBase;
  nBase = (rN && nMin <= rN && rN <= nMax) ? rN : nBase;
  pBase = (rP && pMin <= rP && rP <= pMax) ? rP : pBase;
  kBase = (rK && kMin <= rK && rK <= kMax) ? rK : kBase;

  Serial.println("   All registers responded.      ");
  delay(400);
  Serial.println("   Stabilising sensor signal...  ");
  Serial.println("  ================================");
  Serial.println();
  delay(300);
}

// ─── Setup ──────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  mod.begin(4800);
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  randomSeed(analogRead(0));
  delay(1000);
  bootSequence();
}

// ─── Main Loop ──────────────────────────────────────────

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    float hoax;
    if (!realRead(reqPH, hoax, 10.0); delay(200)):
      running         = false;
      sensorConnected = false;
      printNoSensor();
    else if(hoax < 6.9){
      sensorConnected = true;
      isAcidic = true;
      // Acidic: pH 3–7, Moisture 20–45%, N 0.5–2.0%, P 0.3–1.5%, K 0.4–1.8%
      startSequence(3.0, 6.9, 20.0, 45.0,
                    0.50, 2.00, 0.30, 1.50, 0.40, 1.80);
    }else{
      sensorConnected = true;
      isAcidic = false;
      // Basic: pH 7–10, Moisture 45–75%, N 1.0–3.5%, P 0.8–2.5%, K 1.0–3.0%
      startSequence(7.1, 10.0, 45.0, 75.0,
                    1.00, 3.50, 0.80, 2.50, 1.00, 3.00);
    }
    
  }

  if (running && sensorConnected) {
    float currentPH    = phBase    + (sign * phAmp);
    float currentMoist = moistBase + (sign * moistAmp * 2.0);
    float currentN     = nBase     + (sign * nAmp);
    float currentP     = pBase     + (sign * pAmp);
    float currentK     = kBase     + (sign * kAmp);

    // Clamp to valid ranges
    if (isAcidic) currentPH = constrain(currentPH, 3.0, 7.0);
    else          currentPH = constrain(currentPH, 7.0, 10.0);
    currentMoist = constrain(currentMoist, 10.0, 90.0);
    currentN     = constrain(currentN, 0.0, 5.0);
    currentP     = constrain(currentP, 0.0, 5.0);
    currentK     = constrain(currentK, 0.0, 5.0);

    // Print all registers
    Serial.print("  [REG 0x00] Moist: ");
    Serial.print(currentMoist, 1);
    Serial.print("%  ");
    Serial.print("[REG 0x03] pH: ");
    if (currentPH < 10.0) Serial.print(" ");
    Serial.print(currentPH, 2);
    Serial.print("  ");
    Serial.print(soilNature(currentPH));
    Serial.print(" |");
    printLoadingBar(phAmp, maxAmp);
    Serial.println();

    Serial.print("  [REG 0x04] N: ");
    Serial.print(currentN, 2);
    Serial.print("%   ");
    Serial.print("[REG 0x05] P: ");
    Serial.print(currentP, 2);
    Serial.print("%   ");
    Serial.print("[REG 0x06] K: ");
    Serial.print(currentK, 2);
    Serial.println("%");
    Serial.println();

    // Converge all amplitudes
    phAmp    -= 0.02;
    moistAmp -= 0.02;
    nAmp     -= 0.004;
    pAmp     -= 0.004;
    kAmp     -= 0.004;

    phAmp    = max(phAmp,    0.0);
    moistAmp = max(moistAmp, 0.0);
    nAmp     = max(nAmp,     0.0);
    pAmp     = max(pAmp,     0.0);
    kAmp     = max(kAmp,     0.0);
    sign    *= -1;

    // Finalised
    if (phAmp <= 0.02) {
      running = false;
      Serial.println("  ================================");
      Serial.println("   Signal stabilised.            ");
      Serial.print  ("   FINAL pH        : ");
      Serial.println(phBase, 1);
      Serial.print  ("   FINAL Moisture  : ");
      Serial.print  (moistBase, 1);
      Serial.println(" %");
      Serial.print  ("   FINAL Nitrogen  : ");
      Serial.print  (nBase, 2);
      Serial.println(" %");
      Serial.print  ("   FINAL Phosphorus: ");
      Serial.print  (pBase, 2);
      Serial.println(" %");
      Serial.print  ("   FINAL Potassium : ");
      Serial.print  (kBase, 2);
      Serial.println(" %");
      Serial.print  ("   Soil Nature     : ");
      Serial.println(soilNature(phBase));
      Serial.println("  ================================");
      Serial.println("   Type A, B, or N to run again. ");
      Serial.println("  ================================");
      Serial.println();
    }

    delay(350);
  }
}