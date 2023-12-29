#define RAW_BUFFER_LENGTH 100

#include <Wire.h>
#include <IRremote.h>

#define DEBUG_LED 1
#define IR_SEND_PIN 3
#define IR_RECV_PIN 4
#define RETENTION_DURATION 5000
#define I2C_ADDRESS 8
#define DELAY_AFTER_SEND 250

uint32_t received_data_ir;
unsigned long time_stamp;
uint32_t received_data_i2c;
bool send_ir_signal = false;

void receive_ir_data() {
  if (IrReceiver.decodeNEC()) {
    received_data_ir = IrReceiver.decodedIRData.decodedRawData;
    digitalWrite(DEBUG_LED, HIGH);
    time_stamp = millis();
  }
}

void send_ir_data() {
  IrReceiver.disableIRIn();
  IrSender.enableIROut(38);
  IrSender.sendNECRaw(received_data_i2c,5);
  IrSender.IRLedOff();
  IrReceiver.enableIRIn();
  delay(DELAY_AFTER_SEND);
  send_ir_signal = false;
}

void requestEvent() {
  for (size_t i = 0; i < 4; ++i) {
    Wire.write((uint8_t)(received_data_ir >> 8*(4-i-1)));
  }
  received_data_ir = 0;
  digitalWrite(DEBUG_LED, LOW);
}


void receiveEvent(int numBytes) {
  if (numBytes == 4) {
    received_data_i2c = ((uint32_t) Wire.read()) << 24 | \
                        ((uint32_t) Wire.read()) << 16 | \
                        ((uint32_t) Wire.read()) << 8 | \
                        ((uint32_t) Wire.read());
    send_ir_signal = true;
  }
}

void setup() {
  pinMode(DEBUG_LED, OUTPUT);
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  IrReceiver.begin(IR_RECV_PIN);
  IrSender.begin(IR_SEND_PIN, false, 1);
}

void loop() {
  if (IrReceiver.available()) {
    receive_ir_data();
    IrReceiver.resume();
  }

  if (send_ir_signal) {
    send_ir_data();
  }

  if (millis() - time_stamp > RETENTION_DURATION) {
    received_data_ir = 0;
    digitalWrite(DEBUG_LED, LOW);
    time_stamp = millis();
  }
}