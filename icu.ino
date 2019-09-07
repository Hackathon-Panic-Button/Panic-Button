#include <WiFiManager.h>

#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "LedMatrix.h"

#include "SpringyValue.h"
#include "settings.h"
#include "config.h"
#include "WS2812_util.h"
#include "OTA_update.h"

Servo myServo;

long oldTime = 0;
int oscillationDuration = MAX_OSCILLATION_DURATION;
String chipID;
String serverURL = SERVER_URL;
long currentMillis = 0;

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDtzCCAp+gAwIBAgIQDOfg5RfYRv6P5WD8G/AwOTANBgkqhkiG9w0BAQUFADBl\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSQwIgYDVQQDExtEaWdpQ2VydCBBc3N1cmVkIElEIFJv\n" \
"b3QgQ0EwHhcNMDYxMTEwMDAwMDAwWhcNMzExMTEwMDAwMDAwWjBlMQswCQYDVQQG\n" \
"EwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3d3cuZGlnaWNl\n" \
"cnQuY29tMSQwIgYDVQQDExtEaWdpQ2VydCBBc3N1cmVkIElEIFJvb3QgQ0EwggEi\n" \
"MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCtDhXO5EOAXLGH87dg+XESpa7c\n" \
"JpSIqvTO9SA5KFhgDPiA2qkVlTJhPLWxKISKityfCgyDF3qPkKyK53lTXDGEKvYP\n" \
"mDI2dsze3Tyoou9q+yHyUmHfnyDXH+Kx2f4YZNISW1/5WBg1vEfNoTb5a3/UsDg+\n" \
"wRvDjDPZ2C8Y/igPs6eD1sNuRMBhNZYW/lmci3Zt1/GiSw0r/wty2p5g0I6QNcZ4\n" \
"VYcgoc/lbQrISXwxmDNsIumH0DJaoroTghHtORedmTpyoeb6pNnVFzF1roV9Iq4/\n" \
"AUaG9ih5yLHa5FcXxH4cDrC0kqZWs72yl+2qp/C3xag/lRbQ/6GW6whfGHdPAgMB\n" \
"AAGjYzBhMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n" \
"BBRF66Kv9JLLgjEtUYunpyGd823IDzAfBgNVHSMEGDAWgBRF66Kv9JLLgjEtUYun\n" \
"pyGd823IDzANBgkqhkiG9w0BAQUFAAOCAQEAog683+Lt8ONyc3pklL/3cmbYMuRC\n" \
"dWKuh+vy1dneVrOfzM4UKLkNl2BcEkxY5NM9g0lFWJc1aRqoR+pWxnmrEthngYTf\n" \
"fwk8lOa4JiwgvT2zKIn3X/8i4peEH+ll74fg38FnSbNd67IJKusm7Xi+fT8r87cm\n" \
"NW1fiQG2SVufAQWbqz0lwcy2f8Lxb4bG+mRo64EtlOtCt/qMHt1i8b5QZ7dsvfPx\n" \
"H2sMNgcWfzd8qVttevESRmCD1ycEvkvOl77DZypoEd+A5wwzZr8TDRRu838fYxAe\n" \
"+o0bJW1sj6W3YQGx0qMmoRBxna3iw/nDmVG3KwcIzi7mULKn+gpFL6Lw8g==\n" \
"-----END CERTIFICATE-----\n";


LedMatrix ledMatrix = LedMatrix(1, MATRIX_CS_PIN);

void printDebugMessage(String message) {
#ifdef DEBUG_MODE
  Serial.println(String(PROJECT_SHORT_NAME) + ": " + message);
#endif
}

void connectToDefault() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(BACKUP_SSID, BACKUP_PASSWORD);

  int timer = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timer += 500;
    if (timer > 10000)
      break;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
  }
}

void setup()
{
  pinMode(BUTTONLOW_PIN, OUTPUT);
  digitalWrite(BUTTONLOW_PIN, LOW);

  Serial.begin(115200); Serial.println("");
  strip.begin();
  strip.setBrightness(255);
  setAllPixels(0, 255, 255, 1.0);

  WiFiManager wifiManager;
  int counter = 0;
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ledMatrix.init();
  ledMatrix.setIntensity(LED_MATRIX_BRIGHTNESS);
  ledMatrix.clear();
  ledMatrix.commit();

  while (digitalRead(BUTTON_PIN) == LOW)
  {
    counter++;
    delay(10);

    if (counter > 500)
    {
      wifiManager.resetSettings();
      printDebugMessage("Remove all wifi settings!");
      setAllPixels(255, 0, 0, 1.0);
      fadeBrightness(255, 0, 0, 1.0);
      ESP.reset();
    }
  }

  chipID = generateChipID();
  printDebugMessage(SERVER_URL);
  printDebugMessage(String("Last 2 bytes of chip ID: ") + chipID);
  String configSSID = String(CONFIG_SSID) + "_" + chipID;

  connectToDefault();
  wifiManager.autoConnect(configSSID.c_str());
  fadeBrightness(0, 255, 255, 1.0);
  myServo.attach(SERVO_PIN);
  //  checkForUpdates();

}

//This method starts an oscillation movement in both the LED and servo
void oscillate(float springConstant, float dampConstant, int color)
{
  SpringyValue spring;
  ledMatrix.setIntensity(LED_MATRIX_BRIGHTNESS);

  byte red = (color >> 16) & 0xff;
  byte green = (color >> 8) & 0xff;
  byte blue = color & 0xff;

  spring.c = springConstant;
  spring.k = dampConstant / 100;
  spring.perturb(255);

  //Start oscillating
  for (int i = 0; i < oscillationDuration; i++)
  {
    spring.update(0.01);
    setAllPixels(red, green, blue, abs(spring.x) / 255.0);
    myServo.write(90 + spring.x / 4);

    //Check for button press
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      //Fade the current color out
      fadeMatrix(ledMatrix);
      fadeBrightness(red, green, blue, abs(spring.x) / 255.0);
      return;
    }

    if ((i % 6) == 0) {
      ledMatrix.clear();
      ledMatrix.scrollTextLeft();
      ledMatrix.drawText();
      ledMatrix.commit();
    }

    delay(10);
  }

  fadeBrightness(red, green, blue, abs(spring.x) / 255.0);
  fadeMatrix(ledMatrix);
}

void servo(boolean lock_opened)
{
  if (lock_opened)
  {
    myServo.write(90 - 255 / 4);
  }
  else
  {
    myServo.write(90 + 90 / 4);
  }
}

void lock()
{
  printDebugMessage("Lock Activated");
  servo(false);
  setAllPixels(255, 0, 0, 1.0);
}

void unlock()
{
  printDebugMessage("Lock Deactivated");
  servo(true);
  setAllPixels(0, 255, 0, 1.0);
}


void loop()
{
  //Check for button press
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    lock();
    sendButtonPress();
    delay(250);
  }

  currentMillis = millis();
  //Every requestDelay, send a request to the server
  if (currentMillis > oldTime + REQUEST_DELAY)
  {
    requestMessage();
    oldTime = currentMillis;
  }
}

void sendButtonPress()
{
  printDebugMessage("sending request");
  HTTPClient http;
  http.begin("https://oege.ie.hva.nl/~hamerj/panic_backend/public/push?hid=" + chipID, "36:AD:AB:89:42:52:9E:25:C5:8C:AF:F7:AD:8E:B3:9D:5C:A2:BA:D3"); //Specify the URL and certificate
  int httpCode = http.GET();
  printDebugMessage(String(httpCode));
  printDebugMessage(chipID);
  //Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  http.end();
}

void requestMessage()
{

  //hideColor();

  HTTPClient http;
  http.begin("https://oege.ie.hva.nl/~hamerj/panic_backend/public/status?hid=" + chipID, "36:AD:AB:89:42:52:9E:25:C5:8C:AF:F7:AD:8E:B3:9D:5C:A2:BA:D3"); //Specify the URL and certificate
  int httpCode = http.GET();

  String CMD_CLOSE = "0";
  String CMD_OPEN = "1";

  if (httpCode == 200)
  {
    String response;
    response = http.getString();
    printDebugMessage("Server: " + response);

    //Get the indexes of some commas, will be used to split strings
    int firstComma = response.indexOf(',');
    int secondComma = response.indexOf(',', firstComma + 1);
    int thirdComma = response.indexOf(',', secondComma + 1);
    int fourthComma = response.indexOf(',', thirdComma + 1);

    //Parse data as strings
    String hexColor = response.substring(0, 7);
    String springConstant = response.substring(firstComma + 1, secondComma);
    String dampConstant = response.substring(secondComma + 1, thirdComma);
    String message = response.substring(thirdComma + 1, fourthComma);
    String timeWait = response.substring(fourthComma + 1, response.length());

    response = message;
    
    if (response == "-1")
    {
      printDebugMessage("There are no messages waiting in the queue");
    }
    else if (response == CMD_CLOSE)
    {
      lock();
    }
    else if (response == CMD_OPEN)
    {
      unlock();
    }
  }
  else
  {
    printDebugMessage("Did not receive response code of 200");
  }

  http.end();
}

String generateChipID()
{
  String chipIDString = String(ESP.getChipId() & 0xffff, HEX);

  chipIDString.toUpperCase();
  while (chipIDString.length() < 4)
    chipIDString = String("0") + chipIDString;

  return chipIDString;
}
