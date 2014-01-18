const int PIN_BTN_0 = 8;
const int PIN_BTN_1 = 9;
const int PIN_LED_0 = 4;
const int PIN_LED_1 = 5;
const int PIN_FOOT_SW_0 = A2;
const int PIN_FOOT_SW_1 = A3;
const int PIN_FOOT_SW_2 = A0;
const int PIN_FOOT_SW_3 = A1;
const int PIN_PEDAL_0 = A4;
const int PIN_PEDAL_1 = A5;

const int THRESHOLD1 = 300;
const int THRESHOLD2 = 350;

const byte PROTO_FOOT_SW_0 = 0x10;
const byte PROTO_FOOT_SW_1 = 0x20;
const byte PROTO_FOOT_SW_2 = 0x30;
const byte PROTO_FOOT_SW_3 = 0x40;
const byte PROTO_BTN_0 = 0x60;
const byte PROTO_BTN_1 = 0x70;
const byte PROTO_PEDAL_0 = 0x80;
const byte PROTO_PEDAL_1 = 0x90;

const byte PROTO_IN_LED_0 = 0x10;
const byte PROTO_IN_LED_1 = 0x20;
const byte PROTO_IN_SKIP_PEDAL_0 = 0x30;
const byte PROTO_IN_SKIP_PEDAL_1 = 0x40;
const byte PROTO_IN_INJECT_VALUE = 0x50;

boolean g_skip_pedal0 = false;
boolean g_skip_pedal1 = false;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(57600);
  // initialize the LED pin as an output:
  pinMode(A0, INPUT); 
  pinMode(A1, INPUT);  
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);  
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);  
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  pinMode(PIN_BTN_0, INPUT); 
  pinMode(PIN_BTN_0, INPUT);
}

void serialEvent(){
  int v = Serial.read();
  if(v == -1)
    return;

  if((v & B11110000) == PROTO_IN_LED_0) {
    digitalWrite(PIN_LED_0, (v & 1) ? HIGH : LOW);
    return;
  }

  if((v & B11110000) == PROTO_IN_LED_1) {
    digitalWrite(PIN_LED_1, (v & 1) ? HIGH : LOW);
    return;
  }
  
  if((v & B11110000) == PROTO_IN_SKIP_PEDAL_0) {
    g_skip_pedal0 = (v & 1) ? true : false;
    return; 
  }
  
  if((v & B11110000) == PROTO_IN_SKIP_PEDAL_1) {
    g_skip_pedal1 = (v & 1) ? true : false; 
    return;
  }
  
  if(v == PROTO_IN_INJECT_VALUE) {
    Serial.write(255); 
    return;
  }
}

void handleFootSwitch0() {
  static boolean old_state = false;
  int av = analogRead(PIN_FOOT_SW_0);
  boolean state = (av > THRESHOLD1) ? true : false;
  
  if(state == true && old_state == false) {
    old_state = true;
    Serial.write(PROTO_FOOT_SW_0); 
    Serial.write(1); 
  }
  
  if(state == false && old_state == true) {
    old_state = false;
    Serial.write(PROTO_FOOT_SW_0); 
    Serial.write(0); 
  }
}

void handleFootSwitch1() {
  static boolean old_state = false;
  int av = analogRead(PIN_FOOT_SW_1);
  boolean state = (av > THRESHOLD2) ? true : false;
  
  if(state == true && old_state == false) {
    old_state = true;
    Serial.write(PROTO_FOOT_SW_1); 
    Serial.write(1); 
  }
  
  if(state == false && old_state == true) {
    old_state = false;
    Serial.write(PROTO_FOOT_SW_1); 
    Serial.write(0); 
  }
}

void handleFootSwitch2() {
  static boolean old_state = false;
  int av = analogRead(PIN_FOOT_SW_2);
  boolean state = (av > THRESHOLD1) ? true : false;
  
  if(state == true && old_state == false) {
    old_state = true;
    Serial.write(PROTO_FOOT_SW_2); 
    Serial.write(1); 
  }
  
  if(state == false && old_state == true) {
    old_state = false;
    Serial.write(PROTO_FOOT_SW_2); 
    Serial.write(0); 
  }
}

void handleFootSwitch3() {
  static boolean old_state = false;
  int av = analogRead(PIN_FOOT_SW_3);
  boolean state = (av > THRESHOLD2) ? true : false;
  
  if(state == true && old_state == false) {
    old_state = true;
    Serial.write(PROTO_FOOT_SW_3); 
    Serial.write(1); 
  }
  
  if(state == false && old_state == true) {
    old_state = false;
    Serial.write(PROTO_FOOT_SW_3); 
    Serial.write(0);  
  }
}

void handleButton0() {
  static int btn0_state = LOW;
  int dv = digitalRead(PIN_BTN_0);
  if(dv == LOW && btn0_state == HIGH) {
    btn0_state = LOW;
    Serial.write(PROTO_BTN_0);
    Serial.write(0);
  }

  if(dv == HIGH && btn0_state == LOW) {
    btn0_state = HIGH;
    Serial.write(PROTO_BTN_0); 
    Serial.write(1);
  }
}

void handleButton1() {
  static int btn1_state = LOW;
  int dv = digitalRead(PIN_BTN_1);
  if(dv == LOW && btn1_state == HIGH) {
    btn1_state = LOW;
    Serial.write(PROTO_BTN_1); 
    Serial.write(0);
  }

  if(dv == HIGH && btn1_state == LOW) {
    btn1_state = HIGH;
    Serial.write(PROTO_BTN_1); 
    Serial.write(1);
  } 
}

void handlePedal0() {
  static int old_value = 0;
  
  if(g_skip_pedal0)
    return;
  
  int av = analogRead(PIN_PEDAL_0);
  byte val = map(av, 0, 1023, 0, 255); 
  
  if(val == old_value)
     return;
  
  old_value = val;   
     
  Serial.write(PROTO_PEDAL_0);
  Serial.write(val);
}

void handlePedal1() {
  static int old_value = 0;
  
  if(g_skip_pedal1)
    return;
    
  int av = analogRead(PIN_PEDAL_1);
  byte val = map(av, 0, 1023, 0, 255); 
  
  if(val == old_value)
     return;
  
  old_value = val;   
     
  Serial.write(PROTO_PEDAL_1);
  Serial.write(val);
}

void loop() {
  handleFootSwitch0();
  handleFootSwitch1();
  handleFootSwitch2();
  handleFootSwitch3();
  handleButton0();
  handleButton1();
  handlePedal0();
  handlePedal1();
  delay(7);
}

