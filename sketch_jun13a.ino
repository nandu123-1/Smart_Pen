#include <Wire.h>

// ==========================================
// PIN DEFINITIONS & VIRTUAL POWER ROUTING
// ==========================================

// 1. FSR 402 (Pressure)
const int fsrPin = 34;      
const int virtualVccFSR = 5;   
const int virtualGndFSR = 2;   

// 2. MPU6050 (Motion)
const int MPU_ADDR = 0x68;  
// SDA = 21, SCL = 22

// 3. Vibration Motor (Tremor Feedback)
const int motorPin = 25;    
// Uses Physical GND

// 4. LED (Visual Feedback)
const int ledPin = 13;      
const int virtualGndLED = 12;  

// 5. 3-Pin Active Buzzer (Grip Feedback)
const int buzzerPin = 26;      
const int virtualGndBuzzer = 14; 
// Middle pin uses Physical 3V3

// ==========================================
// SENSITIVITY THRESHOLDS
// ==========================================
const int TOO_TIGHT_GRIP = 3000;     // Trigger point for the buzzer
const int UNSTABLE_ROTATION = 10000; // DROPPED to 10000 for easier triggering!

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // --- SPAWN VIRTUAL POWER & GROUNDS ---
  pinMode(virtualVccFSR, OUTPUT);
  digitalWrite(virtualVccFSR, HIGH); 
  pinMode(virtualGndFSR, OUTPUT);
  digitalWrite(virtualGndFSR, LOW);  
  
  pinMode(virtualGndLED, OUTPUT);
  digitalWrite(virtualGndLED, LOW);     
  
  pinMode(virtualGndBuzzer, OUTPUT);
  digitalWrite(virtualGndBuzzer, LOW);

  // --- INITIALIZE OUTPUT PINS ---
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);      
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); 
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  // --- INITIALIZE MPU6050 ---
  Wire.begin(21, 22); 
  Wire.setClock(50000); 
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Wake up command
  Wire.write(0);     
  byte error = Wire.endTransmission(true);
  
  if(error == 0){
    Serial.println("Smart Pen Online: All 5 Systems Active.");
  } else {
    Serial.println("Error: MPU6050 missing. Check the D21/D22 wires!");
  }
  
  // --- SYSTEM BOOT SEQUENCE ---
  Serial.println("Running Hardware Test...");
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(motorPin, HIGH);
  delay(500); // 0.5 seconds to easily feel the motor test
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(motorPin, LOW);
  Serial.println("Test Complete. Reading sensors...");
}

void loop() {
  // ==========================================
  // LOOP 1: GRIP PRESSURE (Audio)
  // ==========================================
  int gripPressure = analogRead(fsrPin);
  
  if (gripPressure > TOO_TIGHT_GRIP) {
    digitalWrite(buzzerPin, HIGH);  
  } else {
    digitalWrite(buzzerPin, LOW);   
  }

  // ==========================================
  // LOOP 2: TREMOR TRACKING (Haptic & Visual)
  // ==========================================
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 30, true); 
  
  // Read Accelerometer
  int16_t AcX = Wire.read() << 8 | Wire.read();     
  int16_t AcY = Wire.read() << 8 | Wire.read(); 
  int16_t AcZ = Wire.read() << 8 | Wire.read(); 
  
  // Skip Temperature Bytes
  Wire.read(); Wire.read();
  
  // Read Gyroscope
  int16_t GyX = Wire.read() << 8 | Wire.read();     
  int16_t GyY = Wire.read() << 8 | Wire.read(); 
  int16_t GyZ = Wire.read() << 8 | Wire.read(); 

  // Calculate Instability
  long totalRotationSpeed = abs(GyX) + abs(GyY) + abs(GyZ);

  // ==========================================
  // TELEMETRY OUTPUT & TRIGGERS
  // ==========================================
  Serial.print("Pressure: "); Serial.print(gripPressure);
  Serial.print(" | Rotation: "); Serial.print(totalRotationSpeed);

  if (totalRotationSpeed > UNSTABLE_ROTATION) {
    // Print this directly to the monitor so you know the logic fired!
    Serial.println("  <-- TREMOR DETECTED: MOTOR ON!"); 
    
    digitalWrite(ledPin, HIGH);    
    
    // Haptic Heartbeat
    digitalWrite(motorPin, HIGH);  
    delay(500);                    
    digitalWrite(motorPin, LOW);   
    delay(400); 
  } else {
    Serial.println(""); // Drop to next line cleanly
    digitalWrite(ledPin, LOW);
    digitalWrite(motorPin, LOW);
  }
  
  delay(50); // Loop speed
}