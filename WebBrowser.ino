#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);
struct WiFiInFo {
    String ssid;
    String password;
};
  WiFiInFo wifiList[5] = {
        {"FFT_5G", "11235813"},
        {"FFT-VT", "11235813"},
        {"PhamDung","24022002"},
        {"Thanh", "66668888"},
        {"FFT", "11235813"}
    };
Preferences preferences;
const char* ap_ssid = "FFTVN"; 
const char* ap_password = "12345678"; 
void setupAP() {
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    Serial.println("Access Point created. Connect to 'ESP32_Config'. IP: 192.168.4.1");
    server.begin();
    Serial.println("HTTP server started");
}

void setupWebServer() {
    server.on("/", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><body>"
                      "<h1> WiFi Set up</h1>"
                      "<form action='/save' method='POST'>"
                      "SSID: <select name='ssid'>";

        // Quét các mạng WiFi gần đó
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i) {
            html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
        }

        html += "</select><br>"
                "Password : <input type='password' name='password'><br>"
                "<input type='submit' value='Connect'>"
                "</form></body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/save", HTTP_POST, []() {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        if (!ssid.isEmpty()) {
            WiFi.begin(ssid.c_str(), password.c_str());
            if (WiFi.waitForConnectResult() == WL_CONNECTED) {
                server.send(200, "text/html", "Đã kết nối thành công đến WiFi: " + ssid + ". Khởi động lại thiết bị.");
                ESP.restart();
            } else {
                server.send(200, "text/html", "Không thể kết nối đến WiFi. Vui lòng thử lại.");
            }
        } else {
            server.send(400, "text/html", "SSID không hợp lệ.");
        }
    });

    server.begin();
}
bool scanWifi() {
    int n = WiFi.scanNetworks();
    Serial.println("Scanning WiFi networks...");
    
    for (int i = 0; i < n; ++i) {
        String foundSSID = WiFi.SSID(i);
        Serial.println("Found SSID: " + foundSSID);
        
        for (int j = 0; j < 5; ++j) {
            if (foundSSID == wifiList[j].ssid) {
                Serial.println("Attempting to connect to: " + wifiList[j].ssid);
                WiFi.begin(wifiList[j].ssid.c_str(), wifiList[j].password.c_str());
                if (WiFi.waitForConnectResult() == WL_CONNECTED) {
                    return true; 
                } else {
                    Serial.println("Failed to connect to: " + wifiList[j].ssid);
                }
            }
        }
    }
    return false; 
}
void AccessPoint() {
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("Access Point started. Connect to: " + String(ap_ssid));
}
void saveWiFiInFo(const WiFiInFo* wifiList) {
    preferences.begin("WiFiData", false);
    for (int i = 0; i < 5; i++) {
        String key_ssid = "wifi_ssid_" + String(i);
        String key_password = "wifi_pass_" + String(i);
        preferences.putString(key_ssid.c_str(), wifiList[i].ssid);
        preferences.putString(key_password.c_str(), wifiList[i].password);
    }
    preferences.end();
}


void loadWiFiInFo(WiFiInFo* wifiList) {
    preferences.begin("WiFiData", true);
    for (int i = 0; i < 5; i++) {
        String key_ssid = "wifi_ssid_" + String(i);
        String key_password = "wifi_pass_" + String(i);
        wifiList[i].ssid = preferences.getString(key_ssid.c_str(), "");
        wifiList[i].password = preferences.getString(key_password.c_str(), "");
    }
    preferences.end();
}
void addWiFiToList(const String& newSSID, const String& newPassword) {
  
    for (int i = 0; i < 4; i++) {
        wifiList[i] = wifiList[i + 1];
    }

   
    wifiList[4].ssid = newSSID;
    wifiList[4].password = newPassword;

    saveWiFiInFo(wifiList);

    Serial.println("Added new WiFi and removed the oldest one.");
}


void setup() {
    Serial.begin(115200);
    setupAP();
    setupWebServer();
    
    saveWiFiInFo(wifiList);
    loadWiFiInFo(wifiList);

  
    if (scanWifi()) {
        Serial.println("Connected to saved WiFi!");
    } else {
        Serial.println("No matching saved WiFi found. Starting AP...");
        AccessPoint();
    }
    addWiFiToList("New_SSID", "New_Password");
}

void loop() {
    server.handleClient();
}
