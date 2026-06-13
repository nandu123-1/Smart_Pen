#include <Wire.h>



//FSR 402 
const int fsrPin = 34;      
const int virtualVccFSR = 5;   
const int virtualGndFSR = 2;   

//MPU6050 
const int MPU_ADDR = 0x68;  

//Vibration Motor 
const int motorPin = 25;    

//LED 
const int ledPin = 13;      
const int virtualGndLED = 12;  

//3-Pin Active Buzzer 
const int buzzerPin = 26;      
const int virtualGndBuzzer = 14; 


const int TOO_TIGHT_GRIP = 3000;     
const int UNSTABLE_ROTATION = 10000; 

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

 
  pinMode(virtualVccFSR, OUTPUT);
  digitalWrite(virtualVccFSR, HIGH); 
  pinMode(virtualGndFSR, OUTPUT);
  digitalWrite(virtualGndFSR, LOW);  
  
  pinMode(virtualGndLED, OUTPUT);
  digitalWrite(virtualGndLED, LOW);     
  
  pinMode(virtualGndBuzzer, OUTPUT);
  digitalWrite(virtualGndBuzzer, LOW);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);      
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); 
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);


  Wire.begin(21, 22); 
  Wire.setClock(50000); 
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);     
  byte error = Wire.endTransmission(true);
  
  if(error == 0){
    Serial.println("Smart Pen Online: All 5 Systems Active.");
  } else {
    Serial.println("Error: MPU6050 missing. Check the D21/D22 wires!");
  }
  
 
  Serial.println("Running Hardware Test...");
  digitalWrite(ledPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(motorPin, HIGH);
  delay(500); 
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(motorPin, LOW);
  Serial.println("Test Complete. Reading sensors...");
}

void loop() {

  int gripPressure = analogRead(fsrPin);
  
  if (gripPressure > TOO_TIGHT_GRIP) {
    digitalWrite(buzzerPin, HIGH);  
  } else {
    digitalWrite(buzzerPin, LOW);   
  }

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 30, true); 
  
  int16_t AcX = Wire.read() << 8 | Wire.read();     
  int16_t AcY = Wire.read() << 8 | Wire.read(); 
  int16_t AcZ = Wire.read() << 8 | Wire.read(); 
  

  Wire.read(); Wire.read();
  

  int16_t GyX = Wire.read() << 8 | Wire.read();     
  int16_t GyY = Wire.read() << 8 | Wire.read(); 
  int16_t GyZ = Wire.read() << 8 | Wire.read(); 


  long totalRotationSpeed = abs(GyX) + abs(GyY) + abs(GyZ);

  Serial.print("Pressure: "); Serial.print(gripPressure);
  Serial.print(" | Rotation: "); Serial.print(totalRotationSpeed);

  if (totalRotationSpeed > UNSTABLE_ROTATION) {

    Serial.println("  <-- TREMOR DETECTED: MOTOR ON!"); 
    
    digitalWrite(ledPin, HIGH);    
    

    digitalWrite(motorPin, HIGH);  
    delay(500);                    
    digitalWrite(motorPin, LOW);   
    delay(400); 
  } else {
    Serial.println(""); 
    digitalWrite(ledPin, LOW);
    digitalWrite(motorPin, LOW);
  }
  
  delay(50); 
}