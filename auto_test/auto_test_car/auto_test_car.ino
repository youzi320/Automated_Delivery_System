#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#define turn_1 110
#define turn_2 550
#define move_f1 1500

// 這裡放置網路名稱及密碼
const char* ssid = "";
const char* password = "";

String header;
int I1 = 17;
int I2 = 5;
int I3 = 22;
int I4 = 23;
int M1 = 13;
int M2 = 12;
int M3 = 14;
int M4 = 32;

uint8_t broadcastAddress[] = {0x40,0x22,0xD8,0x74,0xBF,0xC4};  // 改mac
typedef struct struct_message { // 發送資料的結構
  int data;
} struct_message;
struct_message myData;

// 資料發送回調函式
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println();
}

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

WiFiServer server(8888);


void WiFiReset() {
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

//兩個電池盒(的那一邊)在右為front

void move_1(int a,int b, int c, int d){
  digitalWrite(M1,a);
  digitalWrite(M2,b);
  digitalWrite(M3,c);
  digitalWrite(M4,d);
}
void move_2(int a,int b, int c, int d){
  digitalWrite(I1,a);
  digitalWrite(I2,b);
  digitalWrite(I3,c);
  digitalWrite(I4,d);
}

void m_front(){
  move_1(HIGH,LOW,HIGH,LOW);
  move_2(HIGH,LOW,HIGH,LOW);
}

void m_back(){
  move_1(LOW,HIGH,LOW,HIGH);
  move_2(LOW,HIGH,LOW,HIGH);
}

void m_spin(){
  move_1(HIGH,LOW,LOW,HIGH);
  move_2(HIGH,LOW,LOW,HIGH);
}

void m_stop(){
  move_1(LOW,LOW,LOW,LOW);
  move_2(LOW,LOW,LOW,LOW);
}


void setup()
{
    Serial.begin(115200);
    pinMode(I1,OUTPUT);
    pinMode(I2,OUTPUT);
    pinMode(I3,OUTPUT);
    pinMode(I4,OUTPUT);
    pinMode(M1,OUTPUT);
    pinMode(M2,OUTPUT);
    pinMode(M3,OUTPUT);
    pinMode(M4,OUTPUT);
    
    move_1(LOW,LOW,LOW,LOW);
    move_2(LOW,LOW,LOW,LOW);

    // 初始化 ESP-NOW
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    } 

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
   
    server.begin();
}

char c;
int mode = -9;
int ms = 0;
int forward_sec_1 = 2000; // 一開始自走車與電梯門前距離時間
int stop_sec_1 = 4000; // 等待電梯按鈕裝置時間
int ele_door_ocs = 6000; // 電梯門打開時間
int enter_ele_door = 3000; // 自走車進/出電梯時間


void loop(){
  m_stop();
  WiFiClient client = server.available();   // Listen for incoming clients
  myData.data = 0;
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character

          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println(); 
          
            if (header.indexOf("GET /f/go") >= 0) { // 往前 
              m_front();
              delay(ms);
              m_stop();
            }else if (header.indexOf("GET /f") >= 0) { // 往前秒數

              if(header.substring(7,8).toInt() == 0){
                if(header.substring(8,9).toInt() == 1){
                  if(ms + 500 < 100000){
                    ms += 500;
                  }
                }else if(header.substring(8,9).toInt() == 2){
                  if(ms - 500 >= 0){
                    ms -= 500;
                  }
                }
              }else{
                ms = header.substring(7,8).toInt();
                ms *= 1000;
              }

            }else if (header.indexOf("GET /auto/start") >= 0) { // 測試開始
              // 到電梯前停下
              Serial.println("1");    
              m_stop();
              m_front();
              delay(forward_sec_1);
              // 控制電梯按鈕裝置並等待電梯打開
              Serial.println("2");    
              m_stop();
              WiFi.disconnect();        

              // 發送資料給電梯按鈕裝置
              uint8_t primaryChan = 6;
              wifi_second_chan_t secondChan = WIFI_SECOND_CHAN_NONE;
              esp_wifi_set_channel(primaryChan, secondChan);
              Serial.println(WiFi.channel());
              esp_now_register_send_cb(OnDataSent);
              esp_now_peer_info_t peerInfo;
              memcpy(peerInfo.peer_addr, broadcastAddress, 6);
              peerInfo.channel = 6;
              peerInfo.encrypt = false;

              // 檢查裝置是否配對成功
              if (esp_now_add_peer(&peerInfo) != ESP_OK) {
                Serial.println("Failed to add peer");
                return;
              }

              delay(50);
              myData.data = 1;
              esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
              
              // 檢查數據是否發送成功
              if (result == ESP_OK) {
                Serial.println("Sent with success");
              }
              else {
                Serial.println("Error sending the data");
              }

              delay(stop_sec_1+ele_door_ocs);
              // 進電梯
              Serial.println("3");    
              m_front();
              delay(enter_ele_door);
              // 在電梯內停下
              Serial.println("4");    
              m_stop();

              WiFi.mode(WIFI_OFF);
              WiFi.mode(WIFI_STA);
              if (esp_now_init() != ESP_OK) {
                Serial.println("Error initializing ESP-NOW");
                return;
              }
              
              WiFi.reconnect();
            }

            Serial.println("auto test mode");
            Serial.println("");
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            client.println("<style>@import url('https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap');");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button_b {background-color: #77c7fc; border: none;border-radius: 10px;cursor: pointer; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println("</style></head>");
            client.println("");
            
            client.println("<body><h1>delivery system</h1>");
            
            client.println("<p> " + String(ms) + " (ms)</p>");
            client.println("<p> forward </p>");      
            client.println("<p><a href=/f/1s><button class=\"button_b\">1 sec</button></a></p>"); 
            client.println("<p><a href=/f/2s><button class=\"button_b\">2 sec</button></a></p>"); 
            client.println("<p><a href=/f/3s><button class=\"button_b\">3 sec</button></a></p>"); 
            client.println("<p><a href=/f/4s><button class=\"button_b\">4 sec</button></a></p>");
            client.println("<p><a href=/f/5s><button class=\"button_b\">5 sec</button></a></p>");
            client.println("<p><a href=/f/01><button class=\"button_b\">+0.5 sec</button></a></p>");
            client.println("<p><a href=/f/02><button class=\"button_b\">-0.5 sec</button></a></p>");
            client.println("<p><a href=/f/go><button class=\"button_b\">go forward</button></a></p>");

            client.println("<p> auto test start</p>");      
            client.println("<p><a href=/auto/start><button class=\"button_b\">auto start</button></a></p>"); 

            client.println("</body></html>");

            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
   
  } 
}