#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>
#define turn_1 110
#define turn_2 550
#define move_f1 1500

// 這裡放置網路名稱及密碼
const char* ssid = "";
const char* password = "";

int I1 = 17;
int I2 = 5;
int I3 = 22;
int I4 = 23;
int M1 = 13;
int M2 = 12;
int M3 = 14;
int M4 = 32;

String header,path;
int mode=-9,pos=0,now_go_pos=-1;
int gogo[11];
char m2_tmp[11];


uint8_t broadcastAddress[] = {0xCC,0xDB,0xA7,0x47,0x09,0x9C}; // 接收端的MAC
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

void setup()
{
    Serial.begin(115200);
    WiFiReset();
    esp_wifi_set_channel(6,WIFI_SECOND_CHAN_NONE);

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

int value = 0;
int class_tmp[100]; // (暫時)紀錄選擇班級
int class_rec[100]; // (已登記)紀錄選擇班級
int class_rec_f[5][100]; // 紀錄登記班級(待運送)
int class_order_f[5][8] = {
                          {0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0},
                          {1565,1564,1563,1562,1584,1558,0,0},
                          {1566,1567,1568,1569,1570,1571,1572,1573},
                          {1581,1580,1579,1578,1577,1576,1575,1574}
                        }; // 紀錄班級順序
int class_num[5] = {0,0,6,8,8}; // 記錄每層猴班級數量
int class_tmp_pos=-1;
int class_rec_pos=-1;
String class_tmp_show;
char c;

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
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

            int class_rec_pos_f[5] = {-1,-1,-1,-1,-1};
            String class_rec_show_f[5];

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("GET /sel_b") >= 0){ // /sel_b 請選擇(所在樓)
              mode = 1;
            }else if(header.indexOf("GET /undone") >= 0){ // /undone 功能、頁面未完成
              mode = -1;
            }else if(header.indexOf("GET /deli_2/rec") >= 0){ // /deli_2/rec 中正樓登記
              mode = 3;
            }else if(header.indexOf("GET /deli_2/start") >= 0){ // /deli_2/start 開始運送
              mode = 4;
            }else if(header.indexOf("GET /deli_2") >= 0){ // /deli_2 中正樓_選擇
              mode = 2; 
            }else if(header.indexOf("GET /") >= 0){ // / 主頁
              mode = 0;
            }
            /*Debug
            Serial.println("*****************************************");
            Serial.println("Debug:");
            Serial.print("class_tmp_pos: ");
            Serial.println( class_tmp_pos);
            Serial.print("class_rec_pos: ");
            Serial.println( class_rec_pos);
            Serial.print("mode: ");
            Serial.println(mode);
            Serial.println("*****************************************");
            */

            if(mode == 0){ // / 主頁     

              // home
              client.println("<!DOCTYPE html><html><head><meta charset=utf-8>");
              client.println("<meta content=\"width=device-width,initial-scale=1\"name=viewport>");
              client.println("<title>主頁</title><style>@import url(https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap);");
              client.println("#sitebody{width:650px;margin:0 auto;font-size:13px;text-align:center}#header{line-height:80px;text-align:center;margin-top:180px;width:650px;height:80px;font-size:90px;font-family:'Noto Sans TC',sans-serif}");
              client.println("#content{height:400px;width:600px;margin:0 auto;text-align:center}#enter{position:absolute;top:405px;left:0;right:0;margin:auto;text-align:center}.button_b{width:230px;height:110px;background-color:#77c7fc;color:#fff;font-size:45px;border:none;border-radius:20px;cursor:pointer;font-family:'Noto Sans TC',sans-serif}</style></head>");
              client.println("<body><div id=sitebody><div id=header>自走車外送系統</div><div id=content><div id=enter><a href=/sel_b><button class=button_b>點此進入</button></a></div></div></div></body></html>");
              //
              client.println();

            }else if(mode == 1){ // /sel_b 請選擇(所在樓)

              // sel_b
              client.println("<!DOCTYPE html><html><head><meta charset=utf-8>");
              client.println("<meta content=\"width=device-width,initial-scale=1\"name=viewport>");
              client.println("<title>請選擇</title><style>@import url(https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap);");
              client.println("#sitebody{width:1000px;height:950px;margin:0 auto;font-size:13px;text-align:center}#header{position:absolute;top:150px;left:0;right:0;margin:auto;line-height:80px;text-align:center;width:250px;height:85px;font-size:80px;font-family:'Noto Sans TC',sans-serif}");
              client.println("#content{position:absolute;top:200px;left:0;right:0;margin:auto;height:680px;width:1000px;text-align:center}.button_b{position:absolute;width:180px;height:90px;background-color:#77c7fc;color:#fff;font-size:40px;border:none;border-radius:20px;cursor:pointer;font-family:'Noto Sans TC',sans-serif}");
              client.println(".button_h{position:absolute;width:220px;height:110px;background-color:#77c7fc;color:#fff;font-size:45px;border:none;border-radius:20px;cursor:pointer;font-family:'Noto Sans TC',sans-serif}#B1{position:absolute;margin:100px 410px;text-align:center}#B2{position:absolute;margin:240px 410px;text-align:center}");
              client.println("#B3{position:absolute;margin:380px 410px;text-align:center}#go_home{position:absolute;margin:545px 390px;text-align:center}</style></head><body><div id=sitebody><div id=header>請選擇</div><div id=content><div id=B1><a href=/undone><button class=button_b>至善樓</button></a></div><div id=B2><a href=/deli_2><button class=button_b>中正樓</button></a></div>");
              client.println("<div id=B3><a href=/undone><button class=button_b>新民樓</button></a></div><div id=go_home><a href=/ ><button class=button_h>返回主頁</button></a></div></div></div></body></html>");
              //
              client.println();

            }else if(mode == -1){ // /undone 功能、頁面未完成

              // undone
              client.println("<!DOCTYPE html><html><head><meta charset=utf-8>");
              client.println("<meta content=\"width=device-width,initial-scale=1\"name=viewport>");
              client.println("<title>功能未完成</title><style>@import url(https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap);");
              client.println("#sitebody{width:650px;margin:0 auto;font-size:13px;text-align:center}#header{line-height:80px;text-align:center;margin-top:180px;width:650px;height:80px;font-size:90px;font-family:'Noto Sans TC',sans-serif}");
              client.println("#content{height:400px;width:600px;margin:0 auto;text-align:center}#enter{position:absolute;top:405px;left:0;right:0;margin:auto;text-align:center}");
              client.println(".button_b{width:230px;height:110px;background-color:#77c7fc;color:#fff;font-size:45px;border:none;border-radius:20px;cursor:pointer;font-family:'Noto Sans TC',sans-serif}</style></head>");
              client.println("<body><div id=sitebody><div id=header>功能尚未完成</div><div id=content><div id=enter><a href=/ ><button class=button_b>返回主頁</button></a></div></div></div></body></html>");
              //
              client.println();

            }else if(mode == 2){ // /deli_2 中正樓_選擇

              String F;
              String F_class="<option value=\"\">班級</option>";
              int f_num = header.substring(13,14).toInt();

              if(header.indexOf("GET /deli_2/clear") >= 0){ // 清空選擇
                class_tmp_show="";
                class_tmp_pos=-1;   
              }else if(header.indexOf("GET /deli_2/f") >= 0){ // 將選擇記錄

                F+="<div id=\"floor_show\">" + String(f_num) + "F" + "</div>";

                for(int i=0;i<class_num[f_num];i++){
                  F_class+="<option value=\"/deli_2/f" + String(f_num) + "/" + String(class_order_f[f_num][i]) + "\">" + String(class_order_f[f_num][i]) + "</option>";
                  
                  if(header.indexOf( String(class_order_f[f_num][i]) ) >= 0){
                    class_tmp_pos++;  
                    class_tmp[class_tmp_pos] = class_order_f[f_num][i]; 
                  }
                }

              }
              class_tmp_show="";
              for(int i=0;i<=class_tmp_pos;i++){ // 紀錄字串，顯示在畫面上
                class_tmp_show += "<div id=\"class_show\">";
                class_tmp_show += String(class_tmp[i]);
                class_tmp_show += "</div>";
              }

              // deli_2
              client.println("<!DOCTYPE html><html><head><meta charset=utf-8>");
              client.println("<meta content=\"width=device-width,initial-scale=1\"name=viewport>");
              client.println("<title>班級樓層選擇</title><style>@import url(https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap);");
              client.println("#sitebody{width:1500px;height:950px;margin:0 auto;font-size:13px}#header{position:absolute;top:100px;left:0;right:0;margin:auto;line-height:80px;text-align:center;width:700px;height:80px;font-size:65px;font-family:'Noto Sans TC',sans-serif}");
              client.println("#content{position:absolute;top:255px;left:0;right:0;margin:auto;width:1000px;height:650px;text-align:center;font-family:'Noto Sans TC',sans-serif}#sel_a{position:absolute;left:0;right:0;margin:auto;width:750px;height:450px;border:2px solid #000}");
              client.println("#btn_all{position:absolute;top:530px;left:0;right:0;margin:auto;width:750px;height:120px}#clear{position:absolute;top:0;left:105.5px}#rec{position:absolute;top:0;left:305.5px}#go_home{position:absolute;top:0;left:505.5px}");
              client.println("#class_tmp_title{text-align:center;font-size:37px;border:none}#floor{position:absolute;top:67px;left:90px;width:90px;height:35px;font-size:20px;font-family:'Noto Sans TC',sans-serif;border:1px solid #000}");
              client.println("#floor_show{position:absolute;top:122px;left:90px;width:45px;height:45px;font-size:15px;font-family:'Noto Sans TC',sans-serif;text-align:center;background-color:#77c7fc;color:#fff;font-size:30px;border-radius:10px}");
              client.println("#class{position:absolute;top:202px;left:90px;width:90px;height:35px;font-size:20px;font-family:'Noto Sans TC',sans-serif;border:1px solid #000}#class_show_box{position:absolute;top:257px;left:80px;width:600px;height:70px;font-family:'Noto Sans TC',sans-serif}");
              client.println("#class_show{background-color:#77c7fc;float:left;width:80px;height:50px;font-size:30px;margin-top:2px;margin-left:10px;color:#fff;border:none;border-radius:10px;text-align:center}");
              client.println(".button_b{position:absolute;width:150px;height:80px;background-color:#77c7fc;color:#fff;font-size:30px;border-radius:10px;border:none;cursor:pointer;font-family:'Noto Sans TC',sans-serif}</style></head>");
              client.println("<body><div id=sitebody><div id=header>請選擇班級</div><div id=content><div id=sel_a><div id=class_tmp_title>目前選擇</div><select id=floor onchange=\"location=this.options[this.selectedIndex].value\">");
              client.println("<option value=\"\">樓層</option><option value=/deli_2/f2>2</option><option value=/deli_2/f3>3</option><option value=/deli_2/f4>4</option></select>");
              client.println(F);
              client.println("<select id=class onchange=\"location=this.options[this.selectedIndex].value\">");
              client.println(F_class);
              client.println("</select><div id=class_show_box>");
              client.println(class_tmp_show);
              client.println("</div></div><div id=btn_all><div id=rec><a href=/deli_2/rec><button class=button_b>登記教室</button></a></div><div id=clear><a href=/deli_2/clear><button class=button_b>清空選擇</button></a></div><div id=go_home><a href=/ ><button class=button_b>返回主頁</button></a></div></div></div></div></body></html>");
              //
              client.println(); 

              }else if(mode == 3){ // /deli_2/rec 中正樓登記

                class_rec_pos++; // 先加1
                for(int i=class_rec_pos;i<=class_rec_pos+class_tmp_pos;i++){
                  class_rec[i] = class_tmp[i-class_rec_pos];
                }
                class_rec_pos += class_tmp_pos; // 當class_tmp_pos沒東西(-1)，class_rec_pos不改變。若class_tmp_pos>=0則class_rec_pos增加
                class_tmp_pos=-1;
                Serial.println("class_rec: ");

                for(int i=2;i<=4;i++){ // 進行不同樓層的班級排序(對class_rec[]內的班級照class_order_f[][]的順序進行排序，儲存到class_rec_f[][])
                  for(int j=0;j<class_num[i];j++){
                    for(int k=0;k<=class_rec_pos;k++){
                      if(class_order_f[i][j] == class_rec[k]){
                        int chec=0;
                        for(int m=0;m<=class_rec_pos_f[i];m++){
                          if(class_rec_f[i][m] == class_rec[k]){
                            chec=1;
                          }
                        }
                        if(!chec){
                          class_rec_pos_f[i]++;
                          class_rec_f[i][class_rec_pos_f[i]] = class_rec[k];
                        }
                      }
                    }
                  }
                }

                for(int i=2;i<=4;i++){ // 紀錄字串，顯示在畫面上
                    for(int j=0;j<=class_rec_pos_f[i];j++){
                      class_rec_show_f[i] += "<div class=\"floor_rec_show\">" + String(class_rec_f[i][j]) + "</div>";
                    }
                }
                /* Debug 儲存樓層
                Serial.println("----------------------------------");
                Serial.println("Degug F: ");
                Serial.print("class_rec[]: ");
                for(int i=0;i<=class_rec_pos;i++){
                  Serial.print(class_rec[i]);
                  Serial.print(" ");
                }
                Serial.println("");
                for(int i=2;i<=4;i++){
                  Serial.print(String(i) + "F: ");
                  for(int j=0;j<=class_rec_pos_f[i];j++){
                    Serial.print(class_rec_f[i][j]);
                    Serial.print(" ");
                  }
                  Serial.println("");
                }
                Serial.println("----------------------------------");
                */
                
                // rec
                client.println("<!DOCTYPE html><html><head><meta charset=utf-8>");
                client.println("<meta content=\"width=device-width,initial-scale=1\"name=viewport>");
                client.println("<title>已登記教室</title><style>@import url(https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@500&display=swap);");
                client.println("#sitebody{width:1500px;height:950px;margin:0 auto;font-size:13px}#header{position:absolute;top:100px;left:0;right:0;margin:auto;line-height:80px;text-align:center;width:700px;height:80px;font-size:65px;font-family:'Noto Sans TC',sans-serif}");
                client.println("#content{position:absolute;top:255px;left:0;right:0;margin:auto;width:1000px;height:650px;text-align:center;font-family:'Noto Sans TC',sans-serif}#sel_a{position:absolute;left:0;right:0;margin:auto;width:750px;height:450px;border:2px solid #000}");
                client.println("#btn_all{position:absolute;top:530px;left:0;right:0;margin:auto;width:750px;height:120px}#start{position:absolute;top:0;left:150px}#go_back{position:absolute;top:0;left:450px}#class_rec_title{text-align:center;font-size:37px;border:none}");
                client.println("#class_rec_box{text-align:left;width:720px;height:380px;margin:0 auto}.floor_rec_box{text-align:left;font-size:35px;margin:15px}.floor_rec_show{background-color:#77c7fc;text-align:center;width:80px;height:50px;font-size:30px;margin:1px;color:#fff;border:none;border-radius:10px;display:inline-block}");
                client.println(".button_b{display:flex;align-items:center;justify-content:center;position:absolute;width:150px;height:80px;background-color:#77c7fc;color:#fff;font-size:30px;border:none;border-radius:10px;cursor:pointer;font-family:'Noto Sans TC',sans-serif}</style></head>");
                client.println("<body><div id=sitebody><div id=header>已登記班級</div><div id=content><div id=sel_a><div id=class_rec_title>目前登記</div><div id=class_rec_box><div class=floor_rec_box>2F：<br>");
                client.println(class_rec_show_f[2]);
                client.println("</div><div class=floor_rec_box>3F：<br>");
                client.println(class_rec_show_f[3]);
                client.println("</div><div class=floor_rec_box>4F：<br>");
                client.println(class_rec_show_f[4]);
                client.println("</div></div></div><div id=btn_all><div id=start><a href=/deli_2/start><button class=button_b>運送</button></a></div><div id=go_back><a href=/deli_2><button class=button_b>返回</button></a></div></div></div></div></body></html>");
                //
                client.println();
                
              }else if(mode == 4){ // /deli_2/start 開始運送

                // reload
                client.println("<!DOCTYPE html><html><head><meta charset=utf-8><meta content=\"width=device-width,initial-scale=1\"name=viewport><script>location.href=\"/undone\"</script></head></html>"); // 導至 undone 頁面
                client.println();
                delay(3000);
                // Clear the header variable
                header = "";
                // Close the connection
                client.stop();
                Serial.println("Client disconnected.");
                Serial.println("");

                /*To Do 以下為發送資料給自走車代碼

                  WiFi.disconnect();

                  esp_now_register_send_cb(OnDataSent);
                  esp_now_peer_info_t peerInfo;
                  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
                  peerInfo.channel = 6;
                  peerInfo.encrypt = false;

                  // 檢查裝置是否配對成功
                  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
                    Serial.println("Failed to add peer");
                    Serial.println("Error!");
                  }    
                  
                  myData.data = 1;
                  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

                  // 檢查數據是否發送成功
                  if (result == ESP_OK) {
                    Serial.println("Sent with success");
                  }
                  else {
                    Serial.println("Error sending the data");
                  }

                  delay(500);
                  WiFi.reconnect();
                  delay(3000);
                */
                
                break;
              }
            }
            client.println();
            break;
          }else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
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










