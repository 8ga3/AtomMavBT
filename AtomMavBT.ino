// PixhawkのTeremetoryをM5 Atom LiteのBluetooth Serialに転送するプログラム
// 2023/5/25
//
// ・特徴
// PixhawkとはGrove コネクタで接続
// 通信状態によりLED(WS2812B)の色が変わる
//
// ・仕組み
// M5 AtomにはUARTがないため、SoftwareSerialを利用している
// Baud rateは19200で5%のパケットエラー率
// 38400で20%、57600で66%
//
// ・既知の不具合
// macOS 13.4でペアリングして使用した場合、最初は通信できるが２回目以降疎通できない
// ペアリング解除し、改めてペアリングすると疎通する
//
// Pixhawk Cube Blackに電源が入った直後、M5 Atom Liteは起動してすぐにフリーズしていると思われる。
// Atomのリセットボタンを押して再起動すると期待通り動作する。
// 電圧/電流の問題か?

#include <M5Atom.h>
#include <SoftwareSerial.h>
#include <BluetoothSerial.h>

#define FCPORT_TX 26
#define FCPORT_RX 32

SoftwareSerial SerialFC;

uint64_t chipid;
char chipname[64];
BluetoothSerial SerialBT;

void setup() {
  M5.begin(true, false, true);
  Serial.begin(115200);
  M5.dis.drawpix(0, 0x7f0000);

  SerialFC.begin(19200, SWSERIAL_8N1, FCPORT_RX, FCPORT_TX, false);
  if (!SerialFC) {
    Serial.printf("Invalid EspSoftwareSerial pin configuration, check config");
    while (1) {
      delay (1000);
    }
  }

  chipid = ESP.getEfuseMac();
  sprintf( chipname, "MAVBT_%04X", (uint16_t)(chipid >> 32));
  Serial.printf("Bluetooth: %s\n", chipname);
  SerialBT.begin(chipname);

  M5.dis.drawpix(0, 0x7f007f);
}

void loop() {
  bool isRead = false;

  while (SerialFC.available() > 0) {
      int c = SerialFC.read();
      SerialBT.write(c);
      if (c == 0xFD) {
        M5.dis.drawpix(0, 0x7f7f00);
      } else if (isRead == false) {
        M5.dis.drawpix(0, 0x007f00);
      }
      isRead = true;
      yield();
  }

  isRead = false;
  while (SerialBT.available() > 0) {
      SerialFC.write(SerialBT.read());
      if (isRead == false) {
        isRead = true;
        M5.dis.drawpix(0, 0x00007f);
      }
      yield();
  }

  if (isRead == false) {
    M5.dis.drawpix(0, 0x7f007f);
  }
}
