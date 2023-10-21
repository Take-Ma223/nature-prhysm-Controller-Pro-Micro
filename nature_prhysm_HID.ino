#include <HID-Project.h>
#include <HID-Settings.h>

#include <Adafruit_NeoPixel.h>
#include "KickSort.h"

#define R1 15
#define R2 14
#define R3 16
#define R4 10
#define G1 6
#define G2 7
#define G3 8
#define G4 9
#define B1 2
#define B2 3
#define B3 4
#define B4 5
#define LED A1
#define VOLUME A0

#define VOLUME_INPUT 16
Adafruit_NeoPixel RGBLED = Adafruit_NeoPixel(4, LED, NEO_RGB + NEO_KHZ800);

#define LED_BRIGHTNESS 60

int volumeInput[VOLUME_INPUT];//ボリュームの値のバッファ、中央値を利用してノイズ緩和に使う

void setup() {
  // put your setup code here, to run once:
  Serial.begin(300);
  Serial.setTimeout(1000);
  pinMode(R1, INPUT_PULLUP);
  pinMode(R2, INPUT_PULLUP);
  pinMode(R3, INPUT_PULLUP);
  pinMode(R4, INPUT_PULLUP);
  pinMode(G1, INPUT_PULLUP);
  pinMode(G2, INPUT_PULLUP);
  pinMode(G3, INPUT_PULLUP);
  pinMode(G4, INPUT_PULLUP);
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(VOLUME, INPUT);
  RGBLED.begin();// RGBLEDのライブラリを初期化する

  for(int i=0;i<VOLUME_INPUT;i++){
    volumeInput[i]=0;
  }
}

void loop() {
  // put your main code here, to run repeatedly
  int i = 0;
  int R[4] = {0, 0, 0, 0};//LED輝度
  int G[4] = {0, 0, 0, 0};
  int B[4] = {0, 0, 0, 0};
  unsigned long PressedTimeR[4] = {0, 0, 0, 0};//押した瞬間の時間(ミリ秒)
  unsigned long PressedTimeG[4] = {0, 0, 0, 0};
  unsigned long PressedTimeB[4] = {0, 0, 0, 0};


  
  int data[2] = {0, 0};
  char a = 0;
  int lane = 0;
  int LN[4] = {0, 0, 0, 0}; //オートプレイのときLN中かどうか
  int Lighttime[4] = {0, 0, 0, 0}; //オートプレイでLEDを光らせる時間

  char key[12]={'v','b','n','m','f','g','h','j','r','t','y','u',};
  int keyState;
  int keyStatePrevious[12];//前フレームのボタンの押下情報
  int LEDvalue[12];//LED輝度
  unsigned long KeyPressedTime[12]={0,0,0,0,0,0,0,0,0,0,0,0};//押した瞬間の時間(ms)
  int pinNumber[12]={R1,R2,R3,R4,G1,G2,G3,G4,B1,B2,B3,B4};
  unsigned long chatteringDecayTime = 15;//チャタリング考慮時間(ms) 押した瞬間からこの時間が経つまでは押しているとみなす

  float attenuationRate = 1.015;
  int volume = 0;
  unsigned char writeVal = 0;

  //ボリュームが無いコントローラはシリアル送信機能をオフにしてください
  int isSerialSendAvailable = true;
  
  int volumeInputIndex = 0;
  
  while (1) {
  //小さいLEDでは30が良い
  //デカいLEDでは60が良い
    RGBLED.setBrightness(LED_BRIGHTNESS);
    for (i = 0; i < 4; i++) {
      if (Lighttime[i] == 0 && LN[i] == 0) {//もう光らせる必要が無いとき
        LEDvalue[i] = (int)((float)LEDvalue[i]/attenuationRate);
        LEDvalue[i+4] = (int)((float)LEDvalue[i+4]/attenuationRate);
        LEDvalue[i+8] = (int)((float)LEDvalue[i+8]/attenuationRate);
        if(LEDvalue[i]<0)LEDvalue[i]=0;
        if(LEDvalue[i+4]<0)LEDvalue[i+4]=0;
        if(LEDvalue[i+8]<0)LEDvalue[i+8]=0;
        
      } else {
        if (Lighttime[i] >= 1)Lighttime[i] -= 1;
      }
    }

    //シリアルデータ送信
    if(Serial.availableForWrite()>0 && isSerialSendAvailable){
      volumeInput[volumeInputIndex] = analogRead(VOLUME);//0~1023
      volumeInputIndex++;
      if(volumeInputIndex==VOLUME_INPUT)volumeInputIndex=0;
      
      KickSort<uint16_t>::quickSort(volumeInput, VOLUME_INPUT, KickSort_Dir::DESCENDING);

      int newVolumeVal = volumeInput[VOLUME_INPUT/2];//0~1023
      if(abs(newVolumeVal - volume) >= 4){
        volume = newVolumeVal;

        writeVal = (unsigned char)(volume/4);//0~255
        Serial.write(writeVal);
      }
      Serial.flush();
    }

    
    
    
    while (Serial.available() > 0) {
      a = Serial.read();//シリアル通信で来たデータを読み込む

      if(a == 'L'){
        //ボリューム値送信リクエスト受信
        if(Serial.availableForWrite()>0){
          Serial.write(writeVal);
          Serial.flush();
        }
        continue;
      }

      if(a == 'B'){
        //1拍毎発光リクエスト受信
        for(int i = 0;i<4;i++){
          int bright=100;
          if(LN[i] == 0){
            LEDvalue[i] = bright;
            LEDvalue[4+i] = bright;
            LEDvalue[8+i] = bright;
          }
        }
        continue;
      }
      
      if (a != -1) {
        if (a == '0')lane = 0;
        if (a == '1')lane = 1;
        if (a == '2')lane = 2;
        if (a == '3')lane = 3;


        data[0] = Serial.read();
        data[1] = Serial.read();
        //Serial.println(data[0]);
        if (data[1] == '0') {
          Lighttime[lane] = 1;
        }
        if (data[1] == '1') {
          LN[lane] = 1;
        }
        if (data[1] == '2') {
          LN[lane] = 0;
        }

        if (data[0] == 'R') {
          LEDvalue[lane] = 255;
          LEDvalue[4 + lane] = 0;
          LEDvalue[8 + lane] = 0;
        }
        if (data[0] == 'G') {
          LEDvalue[lane] = 0;
          LEDvalue[4 + lane] = 255;
          LEDvalue[8 + lane] = 0;
        }
        if (data[0] == 'B') {
          LEDvalue[lane] = 0;
          LEDvalue[4 + lane] = 0;
          LEDvalue[8 + lane] = 255;
        }
        if (data[0] == 'Y') {
          LEDvalue[lane] = 255;
          LEDvalue[4 + lane] = 255;
          LEDvalue[8 + lane] = 0;
        }
        if (data[0] == 'C') {
          LEDvalue[lane] = 0;
          LEDvalue[4 + lane] = 255;
          LEDvalue[8 + lane] = 255;
        }
        if (data[0] == 'M') {
          LEDvalue[lane] = 255;
          LEDvalue[4 + lane] = 0;
          LEDvalue[8 + lane] = 255;
        }
        if (data[0] == 'W') {
          LEDvalue[lane] = 255;
          LEDvalue[4 + lane] = 255;
          LEDvalue[8 + lane] = 255;
        }
        if (data[0] == 'F') {
          LEDvalue[lane] = random(100, 255) ;
          LEDvalue[4 + lane] = random(100, 255) ;
          LEDvalue[8 + lane] = random(100, 255) ;
        }

      }
    }

    for(i=0;i<12;i++){
      keyState = digitalRead(pinNumber[i]);

      if (keyState == 0) {//ON
        //Serial.println(1);
        LEDvalue[i] = 255;
        KeyPressedTime[i]=millis();
        NKROKeyboard.add(key[i]);
      } else {//OFF
        if((millis()-KeyPressedTime[i])>=chatteringDecayTime){//チャタリング防止処理
          NKROKeyboard.remove(key[i]);
        }
      }
      keyStatePrevious[i] = keyState;//次のフレームでチャタリング防止処理に使うためキー押下状態保存
    }

    NKROKeyboard.send();

    RGBLED.setPixelColor(3, LEDvalue[0], LEDvalue[4], LEDvalue[8]) ;
    RGBLED.setPixelColor(2, LEDvalue[1], LEDvalue[5], LEDvalue[9]) ;
    RGBLED.setPixelColor(1, LEDvalue[2], LEDvalue[6], LEDvalue[10]) ;
    RGBLED.setPixelColor(0, LEDvalue[3], LEDvalue[7], LEDvalue[11]) ;

    RGBLED.show() ;
    //delay(8);
  }
}
