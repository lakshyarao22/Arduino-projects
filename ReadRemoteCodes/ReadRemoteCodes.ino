#include <IRremote.hpp>

const int IR_RECEIVE_PIN = 15;
#define maxLen 800
volatile  unsigned int irBuffer[maxLen];
volatile unsigned int x = 0;
String currentIRCode = "";
void setup(){
  Serial.begin(115200);
  Serial.println("setup");
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
}

void loop(){
 attachInterrupt(digitalPinToInterrupt(IR_RECEIVE_PIN), rxIR_Interrupt_Handler, CHANGE);
  if(IrReceiver.decode()){
    Serial.println(IrReceiver.decodedIRData.decodedRawData, DEC);
    IrReceiver.resume();
  }
if(x) {
      currentIRCode = currentIRCode+(x-1);
      detachInterrupt(digitalPinToInterrupt(IR_RECEIVE_PIN));
      for (int i = 1; i < x; i++) {
        currentIRCode = currentIRCode+","+(irBuffer[i] - irBuffer[i - 1]);
      }
      x = 0;
      Serial.print(currentIRCode);
      Serial.println();
    }
  }

void rxIR_Interrupt_Handler() {
  if (x > maxLen) return;
  irBuffer[x++] = micros();
}

//#include <Arduino.h>
//#include <IRremote.h>
//
//int RECV_PIN = 13;
//int SEND_PIN = 5;
//
//IRsend irsend(SEND_PIN);
//IRrecv irrecv(RECV_PIN);
//
//unsigned long infraRedCode[] = {
//  0xE0E040BF, //tv on-off
//  0xE0E0807f, //tv source
//  0x807F827D, //set-top volume up
//  0x807FE817, //set-top channel up
//  0x807F48B7, //set-top mute
//  0x807F42BD, //set-top volume down
//  0x807F58A7, //set-top channel down
//  0x807F807F  //set-top on-off
//};
//
//
//decode_results results;
//
//void setup() {
//  Serial.begin(115200);
//  irrecv.enableIRIn();
//  Serial.println("IR Receiver ready");
//}
//
//void loop() {
//  if (irrecv.decode(&results)) {
//    Serial.println(results.value, HEX);
//    Serial.print(" - ");
//    switch (results.decode_type){
//        case NEC: Serial.println("NEC"); break ;
//        case SONY: Serial.println("SONY"); break ;
//        case RC5: Serial.println("RC5"); break ;
//        case RC6: Serial.println("RC6"); break ;
//        case DISH: Serial.println("DISH"); break ;
//        case SHARP: Serial.println("SHARP"); break ;
//        case JVC: Serial.println("JVC"); break ;
//        case SANYO: Serial.println("SANYO"); break ;
////        case MITSUBISHI: Serial.println("MISUBISHI"); break ;
//        case SAMSUNG: Serial.println("SAMSUNG"); break ;
//        case LG: Serial.println("LG"); break ;
//        case WHYNTER: Serial.println("WHYNTER"); break ;
////        case AIWA_RC_T501: Serial.println("AIWARC_T501"); break ;
//        case PANASONIC: Serial.println("PNASONIC"); break ;
//        case DENON: Serial.println("DENON"); break ;
//      default:
//        case UNKNOWN: Serial.println("UNKNOWN"); break ;
//      }
//    irrecv.resume(); // Receive the next value
//  }
//  delay(300);
//}
