#include<MobaTools.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 400;
const long collectPos = 2200;
const long targetPos = 1850;

#define startPin 6
#define limitPin 9
const byte senPin = 8;
int buttonPressed = 0;
bool motorShouldRun = false;
bool clothDetected = false;
bool sPeed1 = false; bool sPeed2 = false; bool sPeed3 = false; bool sPeed4 = false;
bool stepperRunning;
int fwdSpeed;
int bwdSpeed;

int lim = 0;
int limit_count = 0;
bool limswitch = 0;
uint16_t accelarate;
int hOme = 0;
int pos = 0;

MoToStepper stepper1(stepsPerRev, STEPDIR);

char receivedChar;


void homing() {
  stepper1.setRampLen(400);
  stepper1.setSpeed(600);
  while ((digitalRead(limitPin) != HIGH) && (limit_count == 0)) {
    stepper1.rotate(1);
    Serial.print(limswitch);
    Serial.println("FINDING END");
  }

  if (digitalRead(limitPin) == HIGH && limit_count == 0) {
    stepper1.stop();
    Serial.println("END POSITION");
    limit_count += 1;
    limswitch = HIGH;
    hOme = 1;
  }
  stepper1.setZero();
  delay(2000);
  stepper1.setSpeed(500);
  stepper1.moveTo(-400);
  limswitch = LOW;
  delay(1000);
  while ((digitalRead(limitPin) != HIGH) && (limit_count == 1)) {
    stepper1.rotate(1);
    Serial.print(limswitch);
    Serial.println("FINDING END2");
  }
  if (digitalRead(limitPin) == HIGH && limit_count == 1) {
    stepper1.stop();
    Serial.println("END POSITION2");
    limit_count += 1;
    limswitch = HIGH;
    hOme = 2;
  }
  Serial.print("limit_count: "); Serial.println(limit_count);
  Serial.print("limswitch: "); Serial.println(limswitch);
  Serial.print("hOme: "); Serial.println(hOme);
  Serial.print("POS: "); Serial.println(pos);
  stepper1.setZero();
  delay(2000);
}

void set_pos() {
  while (hOme == 2 && limit_count > 1 && limswitch == HIGH && pos == 0) {
    delay(1000);
    stepper1.setRampLen(200);
    stepper1.setSpeed(600);
    stepper1.moveTo(-collectPos);
    while (stepper1.moving()) {
      Serial.println("MOving to POS1");
    }

    pos = 1;
    Serial.println("StartPosition Set, Ready to USE");
    buttonPressed = 0;
    stepperRunning = false;

  }
  Serial.print("POS:"); Serial.println(pos);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  stepper1.attach(stepPin, dirPin);

  pinMode(startPin, INPUT_PULLUP);
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(senPin, INPUT_PULLUP);

  homing();
  set_pos();
}

void loop() {

  receiveDataFromHMI();
  lim = digitalRead(limitPin);
  if (lim == 1) {
    stepper1.stop();
  }

  if (digitalRead(startPin) == LOW) {
    delay(50);//debouncedelay
    if (digitalRead(startPin) == LOW) {
      buttonPressed = 1;
    }
  }

  if ((hOme == 2) && (limit_count >= 1) && (pos == 1) && (sPeed1) && (buttonPressed == 1)) {
    int sen = digitalRead(senPin);
    if (sen == 0 && lim != 1) {
      clothDetected = false;
      motorA();
      if (lim == 1) {
        stepper1.stop();
      }
    }
    Serial.print("CLOTHDETCTED"); Serial.println(clothDetected);
  }
  if ((hOme == 2) && (limit_count >= 1) && (pos == 1) && (sPeed2 || sPeed3 || sPeed4) && (buttonPressed == 1)) {
    int sen = digitalRead(senPin);
    if (sen == 0 && lim != 1) {
      clothDetected = true;
      motorB();
      if (lim == 1) {
        stepper1.stop();
      }
    }
    Serial.print("CLOTHDETECTED:");Serial.println(clothDetected);
  }
}

void receiveDataFromHMI() {
  while (Serial2.available() > 0) {
    receivedChar = Serial2.read(); // Read a character from serial port
    processDataFromHMI(receivedChar); // Process the received character
  }
}

bool isValidButtonPress(char data) {
  // Check if 'data' corresponds to a valid button press
  return (data >= '1' && data <= '9'); // Adjust based on your valid button press criteria
}

void processDataFromHMI(char data) {
  if (isValidButtonPress(data)) {
    // Process the valid button press
    processButtonPress(data);
  }
}

void processButtonPress(char button) {
  switch (button) {
    case '1':
      Serial.println("Button 1 pressed");
      fwdSpeed = 455;
      bwdSpeed = 1800;
      accelarate = 100;
      sPeed1 = true;
      break;
    case '2':
      Serial.println("Button 2 pressed");
      fwdSpeed = 490;
      bwdSpeed = 1500;
      accelarate = 150;
      sPeed2 = true;
      break;
    case '3':
      Serial.println("Button 3 pressed");
      fwdSpeed =400;
      bwdSpeed = 1500;
      accelarate = 100;
      sPeed3 = true;
      break;
    case '4':
      Serial.println("Button 4 pressed");
      fwdSpeed = 330;
      bwdSpeed = 1400;
      accelarate = 50;
      sPeed4 = true;
      break;
    case '6':
      Serial.println("Speed RESET");
      fwdSpeed = 0;
      bwdSpeed = 0;
      sPeed1 = false; sPeed2 = false; sPeed3 = false; sPeed4 = false;
      buttonPressed = 0;
      break;
    default:
      Serial.println("Invalid button press");
      break;
  }
}

void motorA() {
  if (!stepperRunning && !clothDetected) {
    clothDetected = true;

    stepper1.setRampLen(100);
    stepper1.setSpeed(fwdSpeed);
    Serial.print("ForwardSpeed:"); Serial.println(fwdSpeed);
    stepper1.move(targetPos); Serial.println("MovingForward");
    stepperRunning = true;

    while (stepper1.moving()) {
      delay(10);
    }


    //MoveBAck
    stepper1.setRampLen(50);
    stepper1.setSpeed(bwdSpeed);
    Serial.print("BACKSpeed:"); Serial.println(bwdSpeed);
    stepper1.move(-targetPos);
    Serial.println("MOVINGBACK");
    while (stepper1.moving()) {
      delay(10);
    }
    stepperRunning = false;
  }
}
void motorB() {
  if (clothDetected) {
    stepper1.setRampLen(accelarate);
    stepper1.setSpeed(fwdSpeed);
    while ((digitalRead(senPin) == 0) && (digitalRead(limitPin) != HIGH)) {
      stepper1.rotate(1);
      if (digitalRead(limitPin) == HIGH) {
        stepper1.stop();
      }
      delay(1);  // Small delay to prevent motor stuttering
    }
    stepper1.rotate(1);
    stepper1.rotate(0);
    stepper1.setSpeed(bwdSpeed);
    stepper1.moveTo(-collectPos);
    Serial.println("Moving backward to collecting position");

    while (stepper1.moving()) {
      // No delay here to ensure continuous movement
    }

    motorShouldRun = false;  // Reset the flag when the complete oscillation is done
    clothDetected = false;  // Reset cloth detection flag
  }
}
