#include <HID-Project.h>
#include <HID-Settings.h>

#include <Adafruit_NeoPixel.h>

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
Adafruit_NeoPixel RGBLED = Adafruit_NeoPixel(4, LED, NEO_RGB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(300);
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
  RGBLED.begin();// RGBLEDのライブラリを初期化する
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
  
  while (1) {
    RGBLED.setBrightness(20) ;
    for (i = 0; i < 4; i++) {
      if (Lighttime[i] == 0 && LN[i] == 0) {//もう光らせる必要が無いとき
        LEDvalue[i] = (int)((float)LEDvalue[i]/1.15);
        LEDvalue[i+4] = (int)((float)LEDvalue[i+4]/1.15);
        LEDvalue[i+8] = (int)((float)LEDvalue[i+8]/1.15);
        if(LEDvalue[i]<0)LEDvalue[i]=0;
        if(LEDvalue[i+4]<0)LEDvalue[i+4]=0;
        if(LEDvalue[i+8]<0)LEDvalue[i+8]=0;
        
      } else {
        if (Lighttime[i] >= 1)Lighttime[i] -= 1;
      }
    }

    while (Serial.available() > 0) {
      a = Serial.read();//シリアル通信で来たデータを読み込む
      if (a != -1) {
        if (a == '0')lane = 3;
        if (a == '1')lane = 2;
        if (a == '2')lane = 1;
        if (a == '3')lane = 0;


        data[0] = Serial.read();
        data[1] = Serial.read();
        Serial.println(data[0]);
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
        }
        if (data[0] == 'G') {
          LEDvalue[4+lane] = 255;
        }
        if (data[0] == 'B') {
          LEDvalue[8+lane] = 255;
        }
        if (data[0] == 'Y') {
          LEDvalue[lane] = 255;
          LEDvalue[4+lane] = 255;
        }
        if (data[0] == 'C') {
          LEDvalue[4+lane] = 255;
          LEDvalue[8+lane] = 255;
        }
        if (data[0] == 'M') {
          LEDvalue[lane] = 255;
          LEDvalue[8+lane] = 255;
        }
        if (data[0] == 'W') {
          LEDvalue[lane] = 255;
          LEDvalue[4+lane] = 255;
          LEDvalue[8+lane] = 255;
        }
        if (data[0] == 'F') {
          LEDvalue[lane] = random(100, 255) ;
          LEDvalue[4+lane] = random(100, 255) ;
          LEDvalue[8+lane] = random(100, 255) ;
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
