
#include <HTTPClient.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <ArduinoJson.h>

#define I2S_DOUT  19
#define I2S_BCLK  20
#define I2S_LRC   2  

Audio audio;
String ssid =     "Dungminh T3";
String password = "26072005";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  String hoaDonText = getHoaDonData();
  
  if (hoaDonText != "") {
    String url = "https://dichvuapi.com/api/text-to-speech?access_token=0c5208e7-1170-4a49-8c7c-9804f01a5b4f&language=vi&text=Bạnđãnhậnđược" + hoaDonText;
    Serial.print("TTS URL: ");
    Serial.println(url);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(20);
    audio.connecttohost(url.c_str());
  } else {
    Serial.println("Không có dữ liệu hóa đơn.");
  }
}

void loop() {
  audio.loop();
}

String getHoaDonData() {
  HTTPClient http;
  

  String password = "Dungutc123@";  
  String username = "0378264106";  
  String token = "WZFesUXcGowp-ONnisJ-WPKJ-WJCd-KlzC";        


  String APIbank = "https://dichvuapi.com/historyACB/" + password + "/" + username + "/" + token;
  
  http.begin(APIbank);
  int httpResponse = http.GET();
  String payload = "";

  if (httpResponse > 0) {
    payload = http.getString();
    Serial.println("Dữ liệu hóa đơn từ API:");
    Serial.println(payload);

    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

   
    String hoaDon = "";
    for (JsonObject invoice : doc["transactions"].as<JsonArray>()) {
      String amountStr = invoice["amount"].as<String>();
      int amount = amountStr.toInt();
      String amountText = Convert(amount);
      hoaDon += amountText + " ";
    }

    return hoaDon;
    
  } else {
    Serial.print("Lỗi khi gọi API: ");
    Serial.println(httpResponse);
  }
  

  http.end();
  return "";
}
String Convert(int number) {
    const char* num[] = {"không", "một", "hai", "ba", "bốn", "năm", "sáu", "bảy", "tám", "chín"};
    if (number < 0) return "Số âm";
    if (number == 0) return "không";

    String words = "";
    if (number >= 1000) {
        int thousands = number / 1000;
        words += String(num[thousands]) + " nghìn ";
        number %= 1000;
    }

    if (number >= 100) {
        int hundreds = number / 100;
        words += String(num[hundreds]) + " trăm ";
        number %= 100;
    }

    if (number >= 10) {
        int tens = number / 10;
        words += String(num[tens]) + " mươi ";
        number %= 10;
    }

    if (number > 0) {
        words += String(num[number]);
    }

    return words;
}