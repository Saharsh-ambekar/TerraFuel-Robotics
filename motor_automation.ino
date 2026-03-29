// ── N20 MOTOR AUTOMATED TEST ─────────────────
#include <SoftwareSerial.h>
#define STARTUP_DELAY 5000
#define RUN_TIME      3000
#define PAUSE_TIME    1000
#define PUMP_TIME  2000
#define IN1 9
#define IN2 8
#define ENA 10
#define IN3 2
#define IN4 4
#define ENB 3
#define DREC 11  
#define SREC 5
#define MPUMP 12
#define APUMP 13
#define BPUMP 1

bool drillUp = false;
bool sensUp = false;
bool pumpStarted = false;
bool PHStarted = false;

int rcVal = 0;
int PHVal = 7;

SoftwareSerial fromArduino(6,7);

// ─── Motor Helpers ───────────────────────────
void drillStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  Serial.println("[DRILL] STOP");
  delay(PAUSE_TIME);
}

void drillForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 255);
  Serial.println("[DRILL] FORWARD");
  delay(RUN_TIME);
  drillUp = true;
  drillStop();
}

void drillReverse() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 255);
  Serial.println("[DRILL] REVERSE");
  delay(RUN_TIME);
  drillUp = false;
  drillStop();
}

void sensStop() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
  Serial.println("[SENS] STOP");
  delay(PAUSE_TIME);
}

void sensForward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 255);
  Serial.println("[SENS] FORWARD");
  delay(RUN_TIME);
  sensUp = true;
  sensStop();
}

void sensReverse() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 255);
  Serial.println("[SENS] REVERSE");
  delay(RUN_TIME);
  sensUp = false;
  sensStop();
}
void pumpStop(){
  digitalWrite(MPUMP, LOW);
  Serial.println("[Microbes pump] off");
  delay(PAUSE_TIME);
  pumpStarted = false;
}
void pumpStart(){
  digitalWrite(MPUMP, HIGH); 
  pumpStarted = true;
  Serial.println("[Microbes pump] on");
  delay(PUMP_TIME);
  pumpStop();

}
void acidStart(){
  digitalWrite(APUMP, HIGH);
  PHStarted = true;
  delay(PUMP_TIME);
  digitalWrite(APUMP, LOW);
  PHStarted = false; 
}
void baseStart(){
  Serial.end();
  digitalWrite(BPUMP, HIGH);
  PHStarted = true;
  delay(PUMP_TIME);
  digitalWrite(BPUMP, LOW);
  PHStarted = false; 
  Serial.begin(9600);
}


void allStop() {
  drillStop();
  sensStop();
}

// ─── Setup ───────────────────────────────────
void setup() {
  Serial.begin(9600);
  fromArduino.begin(9600);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(DREC, INPUT);  pinMode(SREC, INPUT); // FIXED
  pinMode(MPUMP, OUTPUT);



  allStop();

  Serial.println("==============================");
  Serial.println("N20 Automated Motor Test");
  Serial.println("Starting in 5 seconds...");
  Serial.println("==============================");

  for (int i = 5; i > 0; i--) {
    Serial.print(i);
    Serial.println("...");
    delay(1000);
  }

  Serial.println("==============================");
  Serial.println("GO!");
  Serial.println("==============================");
}

// ─── Main Loop ───────────────────────────────
void loop() {
  if (fromArduino.available()){
    String data = fromArduino.readStringUntil('\n');
    data.trim();
    Serial.print("Recieved: ");
    Serial.println(data);
    PHVal = data.toFloat();
  }
  int driveVal = pulseIn(DREC, HIGH, 25000);  // FIXED
  if (!drillUp && !pumpStarted){
    pumpStart();
    acidStart();
  }
  if (!sensUp && !pumpStarted){
    baseStart();
  }
  



  if (driveVal < 1200) {
    Serial.println("No signal. Stopping drill.");
    drillStop();
  } else {
    Serial.println("Signal found. Starting drill.");

    // Drill toggle
    if (drillUp) {
      drillReverse();
    } else {
      drillForward();
    }

    // Sensor toggle
   
  }
  int sensVal = pulseIn(SREC, HIGH, 25000);
  if (sensVal < 1200){
    Serial.println("No signal. Stopping sens");
    sensStop();
  }else{
    if(sensUp){
      sensReverse();
    }else{
      sensForward();
    }
  }

}