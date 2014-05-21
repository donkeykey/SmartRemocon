#include <SPISRAM.h>
#include <SPI.h>
#include <SoftModem.h>

#define IR_DATA_SIZE 768
#define CH_SIZE 40
byte ir_data[CH_SIZE][IR_DATA_SIZE];
byte tmp[IR_DATA_SIZE];// = {24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,25,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,255,24,5,12,5,6,5,12,5,6,5,12,5,6,5,6,5,12,5,6,5,6,5,6,5,6,0};
#define PIN_IR_OUT 5
#define PIN_IR_IN 4
#define READING_FLAG 8

// SotfModemインスタンス生成
SoftModem myModem;
SPISRAM myRAM(10); // CS pin


void setup(){
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  //pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_IR_OUT, OUTPUT);
  pinMode(READING_FLAG, OUTPUT);
  pinMode(PIN_IR_IN, INPUT);
   // SoftModem受信開始
  myModem.begin();
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  init_data();
}

void loop()
{
  // データ受信したか？
  if(myModem.available()){
    // 1byte読み出す
    int c = myModem.read();
    check(c);
  }else{
    delay(30);// ポーリングの間隔を適度にあける
  }
}

void init_data(){
  Serial.print("initializeing...");
  for(int i=0;i<IR_DATA_SIZE*CH_SIZE;i++){
    myRAM[i] = 0;
  }
  Serial.println("complete!");
}

void check(int c){
  Serial.println("get data:");
  Serial.println(c);
  if(!(c)){
    Serial.println("no data");
  }else if(c > 100){//学習のとき
    digitalWrite(READING_FLAG, true);
    ir_read(PIN_IR_IN,c);
    copyIRdata(c);
    digitalWrite(READING_FLAG, false);
    //ir_print();
  }else{//赤外線発信命令
    ir_write(PIN_IR_OUT,c);
  }
}

void ir_print(){
  unsigned int i;
  for(i = 0; i < IR_DATA_SIZE; i++){
    Serial.print(tmp[i]);
    if(tmp[i] < 1) break;
    Serial.print(",");
  }
  Serial.println();
}

void ir_write(byte ir_pin, int c){
  for(int k=0;k<IR_DATA_SIZE;k++){
    tmp[k] = myRAM[(c-1)*IR_DATA_SIZE+k];
  }  
  //ir_print();
  Serial.println("ir_write");
  unsigned int i;
  unsigned long interval_sum, start_at;
  interval_sum = 0;
  start_at = micros();
  for(i = 0; i < IR_DATA_SIZE; i++){
    if(tmp[i] < 1) break;
    interval_sum += tmp[i] * 100;
    if(i % 2 == 0){
      while(micros() - start_at < interval_sum){
        digitalWrite(ir_pin, true);
        delayMicroseconds(6);
        digitalWrite(ir_pin, false);
        delayMicroseconds(8);
      }
    }
    else{
      while(micros() - start_at < interval_sum);
    }
  }
}

void ir_read(byte ir_pin,int c){
  boolean no_data_flag = false;
  unsigned int i, j;

  unsigned long now, last, start_at;
  boolean stat;
  start_at = micros();
  while(stat = digitalRead(ir_pin)){
    if(micros() - start_at > 2500000) {
      Serial.println("no data received");
      no_data_flag = true;
     // no_data_notification();
      return;
    }    
  }
  
  if(no_data_flag == false){
    for(i = 0; i < IR_DATA_SIZE; i++){
      tmp[i] = 0;
    }
    start_at = last = micros();
    for(i = 0; i < IR_DATA_SIZE; i++){
      for(j = 0; ; j++){
        if(stat != digitalRead(ir_pin)) break;
          if(j > 65534) return;
        }
      now = micros();
      tmp[i] = (now - last)/100;
      last = now;
      stat = !stat;
    }
  }
}

void copyIRdata(int c){
  Serial.println("copyIRdata");
  for(int i=0;i<IR_DATA_SIZE;i++){
    myRAM[(c-101)*IR_DATA_SIZE+i] = tmp[i];
  }
}


