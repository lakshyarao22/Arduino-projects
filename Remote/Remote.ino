#include <IRremote.hpp>
int IR_RECEIVE_PIN = 13 ;
void setup()
{
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start the receiver
}

void loop() {
      Serial.println(IrReceiver.IRData.decodedRawData, HEX);
      IrReceiver.printIRResultShort(&Serial); // optional use new print version
      IrReceiver.resume(); // Enable receiving of the next value
}
