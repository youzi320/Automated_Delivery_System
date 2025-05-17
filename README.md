# Automated_Delivery_System

## 簡介：
高二自主學習的主題是「自走車外送系統」，目前完成的部分如下:

### 自走車車體：
![自走車車體](https://github.com/pomelo-is-me/Automated_Delivery_System/assets/166627701/e4b78c03-da60-43e7-b7cc-e16598e24da9)

### 電梯按鈕裝置：
![電梯按鈕裝置](https://github.com/pomelo-is-me/Automated_Delivery_System/assets/166627701/82e34bab-416a-4a76-a79b-1ba017458c01)

## 程式碼說明
### auto_test：
這裡放置測試自走車進入電梯的程式碼，分別為：(以下程式碼皆分別上傳至ESP32)
#### 1. auto_test_car：(上傳至自走車上的ESP32)
- 負責控制自走車與傳送訊號給電梯按鈕裝置上的ESP32。
#### 2. auto_test_ele：(上傳至電梯按鈕裝置的ESP32)
- 負責接收自走車傳入訊號，並控制伺服馬達按電梯按鈕。
#### 3. see_mac：(上傳至需查看MAC的ESP32)
- 測試前先上傳至電梯按鈕裝置的ESP32，取得MAC後記錄於auto_test_car程式碼中。

### web_to_car_ESP32：
- 負責處理網站資料，並透過自走車運送。(程式碼上傳至ESP32)  

**(僅完成在ESP32上運行網頁，運送部分未完成。之後要改成將網頁運行在電腦上，待資料接收後再傳給自走車)**

### web_code_formatter_html：
#### 1. formatter：
可以先將原本在 VScode 中編輯的網站代碼透過網路上的 HTML Minifier 壓縮並整理後，丟進formatter進行排版，以符合語法。
#### 2. web_code_html：
存放原本在 VScode 中編輯的網站代碼，並無處理資料、運算等功能(無法選擇班級及記錄，顯示的班級僅為示例)，需進行整理上傳至 ESP32 (整理後的程式碼放在 web_to_car_ESP32 中)
##### 2.1 home.html
主頁
##### 2.2 sel_b.html
選擇樓
##### 2.3 deli_2.html
選擇班級
##### 2.4 rec.html
顯示已登記班級
##### 2.5 reload.html
重新導向(ESP32 發送資料並斷開網路連線前先重新導向)
##### 2.6 undone.html
未完成功能或需ESP32處理資料時跳轉的頁面

