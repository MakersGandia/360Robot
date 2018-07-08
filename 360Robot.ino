/* p
 * Developed by Makers Gandía in colaboration with Immersive Interactive Lab.
 * Authors: Alejandro Marco Ibáñez, María Balagué, Leonardo Rodríguez, Alejandro Castilla García, Jair López Gutiérrez
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////VARIABLES/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////INCLUDE

#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include "WebServer.h"
#include <Preferences.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END INCLUDE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////DEFINE MOTOR

// use first channel of 16 channels (started from zero)
#define canal_0     0

// use 13 bit precission for LEDC timer
#define timer  13

// use 5000 Hz as a LEDC base frequency
#define frec     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN5            5

//////////////////////////////////////////////////////////////////////////


// use second channel of 16 channels (started from zero)
#define canal_1     1

// use 13 bit precission for LEDC timer
#define timer  13

// use 5000 Hz as a LEDC base frequency
#define frec     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN2            2

//////////////////////////////////////////////////////////////////////////


// use third channel of 16 channels (started from zero)
#define canal_2     2

// use 13 bit precission for LEDC timer
#define timer  13

// use 5000 Hz as a LEDC base frequency
#define frec     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN21            21


//////////////////////////////////////////////////////////////////////////


// use fourth channel of 16 channels (started from zero)
#define canal_3     3

// use 13 bit precission for LEDC timer
#define timer  13

// use 5000 Hz as a LEDC base frequency
#define frec     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN22            22

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END DEFINE MOTOR

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////MOTOR VARIABLES

int leftVel = 0;
int rightVel = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END MOTOR VARIABLES 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////WIFI VARIABLES 

const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
boolean settingMode;
String ssidList;
String wifi_ssid;
String wifi_password;

// DNSServer dnsServer;
WebServer webServer(80);

// wifi config store
Preferences preferences;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END WIFI VARIABLES 





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////METHODS///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////ACCELERATE
// Arduino like analogWrite
// value has to be between 0 and valueMax
void accelerate(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * value;
  //uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END ACCELERATE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////RESTORECONFIG

boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");
  Serial.print("WIFI-SSID: ");
  M5.Lcd.print("WIFI-SSID: ");
  Serial.println(wifi_ssid);
  M5.Lcd.println(wifi_ssid);
  Serial.print("WIFI-PASSWD: ");
  M5.Lcd.print("WIFI-PASSWD: ");
  Serial.println(wifi_password);
  M5.Lcd.println(wifi_password);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  if (wifi_ssid.length() > 0) {
    return true;
  } else {
    return false;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END RESTORECONFIG


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////CHECKCONNECTION

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  M5.Lcd.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      M5.Lcd.println();
      Serial.println("Connected!");
      M5.Lcd.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
    count++;
  }
  Serial.println("Timed out.");
  M5.Lcd.println("Timed out.");
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END CHECKCONNECTION

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////STARTWEBSERVER

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    M5.Lcd.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      M5.Lcd.print("SSID: ");
      Serial.println(ssid);
      M5.Lcd.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      M5.Lcd.print("Password: ");
      Serial.println(pass);
      M5.Lcd.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      M5.Lcd.println("Writing SSID to EEPROM...");

      // Store wifi config
      Serial.println("Writing Password to nvr...");
      M5.Lcd.println("Writing Password to nvr...");
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASSWD", pass);

      Serial.println("Write nvr done!");
      M5.Lcd.println("Write nvr done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    M5.Lcd.println(WiFi.localIP());
    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    webServer.on("/reset", []() {
      // reset the wifi config
      preferences.remove("WIFI_SSID");
      preferences.remove("WIFI_PASSWD");
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
  }
  webServer.begin();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END STARTWEBSERVER

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////SETUPMODE

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  M5.Lcd.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  // dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  M5.Lcd.print("Starting Access Point at \"");
  Serial.print(apSSID);
  M5.Lcd.print(apSSID);
  Serial.println("\"");
  M5.Lcd.println("\"");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END SETUPMODE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////MAKEPAGE

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END MAKEPAGE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////URLDECODE

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END URLDECODE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////SETUP

void setup() {
  m5.begin();

  ledcSetup(canal_0, frec, timer);
  ledcAttachPin(LED_PIN5, canal_0);

  ledcSetup(canal_1, frec, timer);
  ledcAttachPin(LED_PIN2, canal_1);

  ledcSetup(canal_2, frec, timer);
  ledcAttachPin(LED_PIN21, canal_2);

  ledcSetup(canal_3, frec, timer);
  ledcAttachPin(LED_PIN22, canal_3);

  
  preferences.begin("wifi-config");

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END SETUP

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////LOOP

void loop() {
  if (settingMode) {
  }
  webServer.handleClient();
  accelerate(canal_0, leftVel);
  accelerate(canal_1, leftVel);
  accelerate(canal_2, rightVel);
  accelerate(canal_3, rightVel);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////END LOOP


