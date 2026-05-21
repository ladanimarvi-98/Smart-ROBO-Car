#include <AFMotor.h>
#include <Servo.h>

/* MODES  */
#define MODE_BT     1
#define MODE_OBS    2
#define MODE_VOICE  3
int mode = MODE_BT;

/*  SPEED  */
#define AUTO_SPEED   120
#define TURN_SPEED   160
#define RIGHT_SPEED 150
#define LEFT_SPEED  140

/*  ULTRASONIC */
#define Trig 4
#define Echo 3

/*  SERVO */
#define SERVO_PIN 10
#define SERVO_CENTER 90
#define SERVO_LEFT   150
#define SERVO_RIGHT  30
Servo servo;

/*  MOTORS  */
AF_DCMotor M1(1);
AF_DCMotor M2(2);
AF_DCMotor M3(3);
AF_DCMotor M4(4);

/*  DISTANCE  */
#define STOP_DIST      30
#define CRITICAL_DIST  12

char bt = 'S';
String voice = "";

/*  SETUP  */
void setup() {
  Serial.begin(9600);

  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);
  delay(600);

  stopCar();
}

/*  LOOP  */
void loop() {

  while (Serial.available()) {
    delay(10);
    char c = Serial.read();

    if (c == '1') { mode = MODE_BT; stopCar(); }
    else if (c == '2') { mode = MODE_OBS; stopCar(); }
    else if (c == '3') { mode = MODE_VOICE; stopCar(); }

    bt = c;
    voice += c;
  }


  if (mode == MODE_OBS) obstacleMode();
  else if (mode == MODE_BT) bluetoothControl();
  else if (mode == MODE_VOICE) voiceControl();
}

/*  BLUETOOTH MODE  */
void bluetoothControl() {
  if (bt == 'F') forwardBT();
  else if (bt == 'B') backwardBT();
  else if (bt == 'L') rightBT();
  else if (bt == 'R') leftBT();
  else if (bt == 'S') stopCar();
}

/*  VOICE MODE */
void voiceControl() {
  if (voice.length() > 0) {
    voice.toLowerCase();

    if (voice.indexOf("forward") >= 0) forwardBT();
    else if (voice.indexOf("back") >= 0) backwardBT();
    else if (voice.indexOf("left") >= 0) voiceRight();
    else if (voice.indexOf("right") >= 0) voiceLeft();
    else if (voice.indexOf("stop") >= 0) stopCar();

    voice = "";
  }
}

/*  OBSTACLE MODE  */
void obstacleMode() {

  int front = frontScan();

  // Move forward ONLY if safe distance
  if (front > STOP_DIST) {
    forwardAuto();
    return;
  }

  // Obstacle within 30 cm → STOP
  stopCar();
  delay(150);

  // ALWAYS reverse to create 30 cm turning space
  backwardAuto();
  delay(450);        
  stopCar();
  delay(150);

  // Scan sides AFTER reverse
  int rightDist = rightsee();
  int leftDist  = leftsee();

  // If any side open → turn from safe distance
  if (rightDist > STOP_DIST || leftDist > STOP_DIST) {
    if (rightDist >= leftDist) turnRight();
    else turnLeft();
    return;
  }

  // Still blocked → reverse again (NO forward)
  backwardAuto();
  delay(450);
  stopCar();
  delay(150);

  // Final scan & forced turn
  rightDist = rightsee();
  leftDist  = leftsee();

  if (rightDist >= leftDist) turnRight();
  else turnLeft();
}

/* ================= MOTION  */
void forwardBT() {
  M1.setSpeed(LEFT_SPEED); 
  M2.setSpeed(LEFT_SPEED);
  M3.setSpeed(RIGHT_SPEED); 
  M4.setSpeed(RIGHT_SPEED);
  M1.run(FORWARD); 
  M2.run(FORWARD);
  M3.run(FORWARD); 
  M4.run(FORWARD);
}

void backwardBT() {
  setAllSpeed(AUTO_SPEED);
  M1.run(BACKWARD); 
  M2.run(BACKWARD);
  M3.run(BACKWARD); 
  M4.run(BACKWARD);
}

void leftBT() {
  setAllSpeed(TURN_SPEED);
  M1.run(BACKWARD); 
  M2.run(BACKWARD);
  M3.run(FORWARD);  
  M4.run(FORWARD);
}

void rightBT() {
  setAllSpeed(TURN_SPEED);
  M1.run(FORWARD); M2.run(FORWARD);
  M3.run(BACKWARD); M4.run(BACKWARD);
}

void forwardAuto() {
  setAllSpeed(AUTO_SPEED);
  M1.run(FORWARD); 
  M2.run(FORWARD);
  M3.run(FORWARD); 
  M4.run(FORWARD);
}

void backwardAuto() {
  setAllSpeed(AUTO_SPEED);
  M1.run(BACKWARD); 
  M2.run(BACKWARD);
  M3.run(BACKWARD); 
  M4.run(BACKWARD);
}

void turnRight() {
  setAllSpeed(TURN_SPEED);
  M1.run(FORWARD); M2.run(FORWARD);
  M3.run(BACKWARD); M4.run(BACKWARD);
  delay(420);
  stopCar();
}

void turnLeft() {
  setAllSpeed(TURN_SPEED);
  M1.run(BACKWARD); 
  M2.run(BACKWARD);
  M3.run(FORWARD);  
  M4.run(FORWARD);
  delay(420);
  stopCar();
}

void uTurn() {
  backwardAuto();
  delay(350);
  turnRight();
}

/*  VOICE TURN  */
void voiceRight() {
  setAllSpeed(TURN_SPEED);
  M1.run(FORWARD); 
  M2.run(FORWARD);
  M3.run(BACKWARD); 
  M4.run(BACKWARD);
  delay(650);
  forwardBT();
}

void voiceLeft() {
  setAllSpeed(TURN_SPEED);
  M1.run(BACKWARD); 
  M2.run(BACKWARD);
  M3.run(FORWARD);  
  M4.run(FORWARD);
  delay(650);
  forwardBT();
}

/*  COMMON  */
void stopCar() {
  M1.run(RELEASE); 
  M2.run(RELEASE);
  M3.run(RELEASE); 
  M4.run(RELEASE);
}

void setAllSpeed(int spd) {
  M1.setSpeed(spd); 
  M2.setSpeed(spd);
  M3.setSpeed(spd); 
  M4.setSpeed(spd);
}

/*  SENSOR  */
int frontScan() {
  servo.write(SERVO_CENTER);
  delay(120);
  return ultrasonic();
}

int leftsee() {
  servo.write(SERVO_LEFT);
  delay(250);
  return ultrasonic();
}

int rightsee() {
  servo.write(SERVO_RIGHT);
  delay(250);
  return ultrasonic();
}

int ultrasonic() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(5);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  long d = pulseIn(Echo, HIGH, 20000);
  if (d == 0) return 300;
  return d / 29 / 2;
}
