#include <Servo.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

typedef struct struct_message { // 接收資料的結構
  int num;
} struct_message;

struct_message myData;
Servo myservo;  

// 資料接收回調函式
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("num: ");
  Serial.println(myData.num);
  Serial.println();
  if(myData.num == 1){
    myservo.attach(18);
    myservo.write(90);
    delay(500);
    myservo.write(40);
    delay(2500);
    myservo.write(90);
    delay(2500);
    myservo.detach();
  }
}

void setup() {
  Serial.begin(115200);
  
  myservo.attach(18);  // 伺服馬達連接的PIN

  // 初始化 ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  WiFi.disconnect();
  uint8_t primaryChan = 6;
  wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
  esp_wifi_set_channel(primaryChan, secondChan);
  Serial.println(WiFi.channel());
  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
  esp_now_register_recv_cb(OnDataRecv); //設置資料接收回調函式
}