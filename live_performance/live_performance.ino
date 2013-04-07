/*
Sends data via serial line in special format:
Digital message (1 Byte):
1 0 0 p p p p v
Analog message (2 Bytes):
0 0 p p p v v v     1 v v v v v v v

p = Bits for port
v = Bits for value

Copyright (c) 2013 Thomas Mayer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define FOOT_1 2
#define FOOT_2 3
#define FOOT_3 4
#define HAND_R_1 5
#define HAND_R_2 6
#define HAND_R_3 7

#define ANALOG_ROUGHNESS 10

#define RESISTOR_KEYPAD 1
#define RESISTOR_R 2

int footState_1 = LOW;
int footState_2 = LOW;
int footState_3 = LOW;

int handRightState_1 = LOW;
int handRightState_2 = LOW;
int handRightState_3 = LOW;

int resistorValue_R = 0;
int resistorValue_Keypad = 0;

int analogChange;

void setup(){
  Serial.begin(9600);
 
  pinMode(FOOT_1, INPUT);
  pinMode(FOOT_2, INPUT);
  pinMode(FOOT_3, INPUT);
  pinMode(HAND_R_1, INPUT);
  pinMode(HAND_R_2, INPUT);
  pinMode(HAND_R_3, INPUT);

  analogChange = 0;
}

void loop() {
  readDigitalValue(FOOT_1, footState_1);
  readDigitalValue(FOOT_2, footState_2);
  readDigitalValue(FOOT_3, footState_3);
  readDigitalValue(HAND_R_1, handRightState_1);
  readDigitalValue(HAND_R_2, handRightState_2);
  readDigitalValue(HAND_R_3, handRightState_3);
  readRoughValue(RESISTOR_KEYPAD, resistorValue_Keypad);
  if (analogChange == 20) {
    readAnalogValue(RESISTOR_R, resistorValue_R);
    analogChange = 0;
  }
  delay(10); 
  analogChange += 1;

}


/**
 * Creates a message for digital value and port number.
 */ 
void makeDigitalMessage (int port, int value, byte *data) {
  data[0] = 0x80 | (port << 1) | value;
}

/*
 * Creates a message for analog value and port number.
 */
void makeAnalogMessage (int port, int value, byte *data) {
  data[0] = 0 | (port<<3) | ((value >> 7) &0xff);
  data[1] = 0x80 | (value &0x7F);
}

/**
 * Sends message via Serial port.
 */
void sendMessage(byte *data, int arraySize) {
  int i;
  for (i = 0; i < arraySize; i++) {
    /* for Arduino < 1.0 */
    /*Serial.print(data[i], BYTE);*/
    /* for Arduino > 1.0 */
    Serial.write(data[i]);
  } 
}

/**
 * Reads a digital value and outputs the value, when it has changed.
 */
void readDigitalValue(int trigger, int &value) {
  int arraySize = 1;
  int tempValue = digitalRead(trigger);
  if (value != tempValue) {
    value = tempValue;
    byte data[arraySize];
    makeDigitalMessage(trigger, value, data);
    sendMessage(data, arraySize);
  }
}

/**
 * Reads a analog value and outputs the value, when it has changed.
 */
void readAnalogValue(int trigger, int &value) {
  int arraySize = 2;
  int tempValue = analogRead(trigger);
  if (value != tempValue) {
    value = tempValue;
    byte data[arraySize];
    makeAnalogMessage(trigger, value, data);
    sendMessage(data, arraySize);
  }
}

/**
 * Reads a analog value and outputs the value only,
 * when change in value is greater than roughness.
 *
 * Useful for Keypads and resistor array.
 */
void readRoughValue(int trigger, int &value) {
  int arraySize = 2;
  int tempValue = analogRead(trigger);
  if (abs(value - tempValue) > ANALOG_ROUGHNESS) {
    value = tempValue;
    byte data[arraySize];
    makeAnalogMessage(trigger, value, data);
    sendMessage(data, arraySize);
  }
}
