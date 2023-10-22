#include <Servo.h>  
#include <SoftwareSerial.h>
const int lua = 2 ;
const int Pir = 3 ;
const int TouchTam = 5 ;
const int Coi = 7;
const int ServoMua = 8;
const int LedTam = 9;
const int QuatTam = 10;
const int Gas = A0;
const int Mua = A1;
// config Value 
float gasValue; 
int luaValue;
int PirState= 0 ;
int TouchTamValue = 0;
int coiState = 0;
int LedTamValue = 0;
int RainValue = 0;
// servo   
Servo myservo; 
int pos = 0;
// set Taste
bool isServoMoved = false;
bool isPersonSensor = false ;
bool previousStateBep = false ;
bool previousStateTam = false ;
bool previousStateNgu = false ;
unsigned long previousMillis = 0;
const long interval = 5000; // Thời gian 5 giây

SoftwareSerial arduinoSerial(12, 13); //RX TX
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  arduinoSerial.begin(115200);

  myservo.attach(ServoMua);
  pinMode(TouchTam, INPUT); 
  pinMode(lua, INPUT); 
  pinMode(Pir, INPUT); 
  pinMode(Mua, INPUT); 
  pinMode(Gas, INPUT); 
// Out Put 
   pinMode(Coi, OUTPUT);
   pinMode(ServoMua, OUTPUT);
   pinMode(LedTam, OUTPUT);
   pinMode(QuatTam, OUTPUT);
}

void loop() {
  
  // put your main code here, to run repeatedly:
  gasValue = analogRead(Gas);
  luaValue  = digitalRead(lua);
  PirState = digitalRead(Pir);
  TouchTamValue = digitalRead(TouchTam);
  RainValue = analogRead(Mua);
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // Làm điều gì đó mỗi 5 giây ở đây
    String sendData = String(gasValue) + "," + String(luaValue) + "," + String(PirState) + "," + String(TouchTamValue) + "," + String(RainValue);

  // Gửi dữ liệu qua Serial (TX)
   arduinoSerial.println(sendData);
    
    // Cập nhật thời gian trước đó
    previousMillis = currentMillis;
  }
  WarningFire(luaValue , gasValue);
  PirSendor(PirState,isPersonSensor);
  RainSensor(RainValue,isServoMoved );
  delay(1000);
}
void WarningFire(int luaValue , float gasValue){
// Gas and Fire
Serial.print(gasValue);
Serial.print("Lưa");
Serial.print(luaValue);
if(gasValue > 700 || luaValue ==  LOW){
      coiState = HIGH;
      digitalWrite(Coi, HIGH);
      Serial.println("COI ON");
      delay(150);
    } else {
      coiState = LOW;
      digitalWrite(Coi, LOW);
      Serial.println("COI OFF");
  }
}

void RainSensor(int RainValue, bool isServoMoved ){
  Serial.println("Rain");
  Serial.println(RainValue);
  if(RainValue < 700 && !isServoMoved ) {
    for(pos = 60; pos <= 110; pos++){ 
        myservo.write(pos);
        delay(50);
    }
    isServoMoved = true;
  } else   if (RainValue < 700 && !isServoMoved ){
    for(pos = 110; pos>=60; pos--) {                           
        myservo.write(pos);
        delay(50);
    } 
    isServoMoved = false;
  }
}

void PirSendor(int PirState, bool &isPersonSensor) {
    Serial.println("PirState");
  Serial.println(PirState);
  if (PirState == HIGH && !isPersonSensor) {
    Serial.println("LED TAM ON");
    digitalWrite(LedTam, HIGH);
    digitalWrite(QuatTam, HIGH);
    isPersonSensor = true;
  } else if (PirState == LOW && isPersonSensor) {
    Serial.println("LED TAM OFF");
    digitalWrite(LedTam, LOW);
    digitalWrite(QuatTam, LOW);
    isPersonSensor = false;
  }
}

