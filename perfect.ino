/*

  ============================================================

       ESP32 SMART AIR QUALITY IoT MONITOR

       WIFI + TELEGRAM + BLUETOOTH BLE + WEB DASHBOARD

  ============================================================

  BOARD: ESP32 Dev Module

  SENSORS:

    PMS9103M

    MQ-135

    DHT11

  OUTPUTS:

    16x2 I2C LCD

    Red LED

    Green LED

    Active buzzer

  BUZZER WARNING LEVELS:

    AQI <= 200   : No new buzzer alert

    AQI 201-300  : 5 seconds ON / 5 seconds OFF

    AQI 301-400  : 1 second ON / 1 second OFF

    AQI > 400    : Continuous ON

  Each alert lasts a maximum of 30 seconds.

  The buzzer timer does not block other ESP32 tasks.

  SECURITY:

    Enter your Wi-Fi password and a NEW Telegram bot token.

*/

// ============================================================

// LIBRARIES&#x20;

// ============================================================

#include <WiFi.h>

#include <WebServer.h>

#include <HTTPClient.h>

#include <WiFiClientSecure.h>

#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <DHT.h>

#include <DIYables_BluetoothServer.h>

#include <DIYables_BluetoothChat.h>

#include <platforms/DIYables_Esp32BLE.h>

// ============================================================

// WIFI SETTINGS

// ============================================================

const char* WIFI_SSID = "wIFI SSID";

const char* WIFI_PASSWORD = "WIFI PASSWORD";

const char* AP_SSID = "ESP32_Air_Monitor";

const char* AP_PASSWORD = "12345678";

// ============================================================

// TELEGRAM SETTINGS

// ============================================================

const char* TELEGRAM_BOT_TOKEN = "YOUR_NEW_BOT_TOKEN";

const char* TELEGRAM_CHAT_ID = "YOUR_TELEGRAM_CHAT_ID";

// ============================================================

// PIN CONNECTIONS

// ============================================================

#define MQ135_PIN 34

#define DHT_PIN 4

#define DHT_TYPE DHT11

#define BUZZER_PIN 25

#define RED_LED 26

#define GREEN_LED 27

#define PMS_RX_PIN 16

#define PMS_TX_PIN 17

#define LCD_SDA 21

#define LCD_SCL 22

// ============================================================

// SENSOR OBJECTS

// ============================================================

DHT dht(DHT_PIN, DHT_TYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial pmsSerial(2);

WebServer server(80);

// ============================================================

// DIYABLES BLUETOOTH BLE

// ============================================================

const char* DEVICE_NAME = "ESP32_Air_Monitor";

const char* SERVICE_UUID =

  "19B10000-E8F2-537E-4F6C-D104768A1214";

const char* TX_UUID =

  "19B10001-E8F2-537E-4F6C-D104768A1214";

const char* RX_UUID =

  "19B10002-E8F2-537E-4F6C-D104768A1214";

DIYables_Esp32BLE bluetooth(

  DEVICE_NAME,

  SERVICE_UUID,

  TX_UUID,

  RX_UUID

);

DIYables_BluetoothServer bluetoothServer(bluetooth);

DIYables_BluetoothChat bluetoothChat;

// ============================================================

// SENSOR VALUES

// ============================================================

float pm1 = 0;

float pm25 = 0;

float pm10 = 0;

float temperature = 0;

float humidity = 0;

int gasValue = 0;

int aqi = 0;

String airStatus = "STARTING";

// ============================================================

// PMS SENSOR WARMUP

// ============================================================

unsigned long pmsStartTime = 0;

const unsigned long PMS_WARMUP = 30000UL;

// ============================================================

// BUZZER SETTINGS

// ============================================================

bool buzzerActive = false;

bool buzzerSoundOn = false;

unsigned long buzzerStartTime = 0;

unsigned long lastBuzzerToggle = 0;

const unsigned long BUZZER_DURATION = 30000UL;

// AQI 201-300: toggle every 5 seconds

const unsigned long SLOW_BEEP_INTERVAL = 2000UL;

// AQI 301-400: toggle every 1 second

const unsigned long FAST_BEEP_INTERVAL = 1000UL;

// ============================================================

// AQI WARNING STATES

//

// 0 = AQI <= 200

// 1 = AQI 201-300

// 2 = AQI 301-400

// 3 = AQI > 400

// ============================================================

int previousAQIState = -1;

// ============================================================

// TELEGRAM COOLDOWN

// ============================================================

unsigned long lastTelegramTime = 0;

const unsigned long TELEGRAM_COOLDOWN = 60000UL;

// ============================================================

// WIFI STATUS

// ============================================================

bool normalWiFiConnected = false;

// ============================================================

// AQI CALCULATION

//

// Simplified project estimate based on PM2.5.

// NOT an official CPCB AQI calculation.

// ============================================================

int calculateAQI(float pm)

{

  if (pm <= 0)

    return 0;

  if (pm <= 30)

    return map((int)pm, 0, 30, 0, 50);

  if (pm <= 60)

    return map((int)pm, 31, 60, 51, 100);

  if (pm <= 90)

    return map((int)pm, 61, 90, 101, 200);

  if (pm <= 120)

    return map((int)pm, 91, 120, 201, 300);

  if (pm <= 250)

    return map((int)pm, 121, 250, 301, 400);

  return 500;

}

// ============================================================

// AQI STATUS

// ============================================================

String getAQIStatus(int value)

{

  if (value <= 50)

    return "GOOD";

  if (value <= 100)

    return "SATISFACTORY";

  if (value <= 200)

    return "MODERATE";

  if (value <= 300)

    return "POOR";

  if (value <= 400)

    return "VERY POOR";

  return "SEVERE";

}

// ============================================================

// TELEGRAM URL ENCODING

// ============================================================

String telegramURLencode(String text)

{

  String encoded = "";

  for (int i = 0; i < text.length(); i++)

  {

    char c = text.charAt(i);

    if (

      (c >= 'a' && c <= 'z') ||

      (c >= 'A' && c <= 'Z') ||

      (c >= '0' && c <= '9') ||

      c == '-' || c == '_' ||

      c == '.' || c == '\~'

    )

    {

      encoded += c;

    }

    else

    {

      char buf[4];

      sprintf(buf, "%%%02X", (unsigned char)c);

      encoded += buf;

    }

  }

  return encoded;

}

// ============================================================

// SEND TELEGRAM MESSAGE

// ============================================================

void sendTelegramMessage(String message)

{

  if (WiFi.status() != WL_CONNECTED)

  {

    Serial.println("Telegram: Wi-Fi not connected.");

    return;

  }

  if (

    String(TELEGRAM_BOT_TOKEN) == "YOUR_NEW_BOT_TOKEN" ||

    String(TELEGRAM_CHAT_ID) == "YOUR_TELEGRAM_CHAT_ID"

  )

  {

    Serial.println("Telegram credentials not configured.");

    return;

  }

  WiFiClientSecure client;

  // Diagnostic mode only.

  // For production, configure certificate validation.

  client.setInsecure();

  HTTPClient https;

  https.setConnectTimeout(10000);

  https.setTimeout(10000);

  https.setReuse(false);

  String url =

    String("https://api.telegram.org/bot") +

    TELEGRAM_BOT_TOKEN +

    "/sendMessage?chat_id=" +

    TELEGRAM_CHAT_ID +

    "&text=" +

    telegramURLencode(message);

  if (!https.begin(client, url))

  {

    Serial.println("Telegram HTTPS begin failed.");

    return;

  }

  int httpCode = https.GET();

  Serial.print("Telegram HTTP code: ");

  Serial.println(httpCode);

  if (httpCode > 0)

  {

    String response = https.getString();

    Serial.println(response);

  }

  else

  {

    Serial.println(https.errorToString(httpCode));

  }

  https.end();

}

// ============================================================

// START BUZZER ALERT

// ============================================================

void startBuzzer30Sec()

{

  buzzerActive = true;

  buzzerStartTime = millis();

  lastBuzzerToggle = millis();

  // Start sounding immediately

  buzzerSoundOn = true;

  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println("BUZZER ALERT STARTED");

}

// ============================================================

// STOP BUZZER

// ============================================================

void stopBuzzer()

{

  digitalWrite(BUZZER_PIN, LOW);

  buzzerActive = false;

  buzzerSoundOn = false;

  Serial.println("BUZZER OFF");

}

// ============================================================

// NON-BLOCKING BUZZER CONTROL

// ============================================================

void updateBuzzer()
{
  // Stop the buzzer immediately whenever AQI is MODERATE or lower.
  if (aqi <= 200)
  {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    buzzerSoundOn = false;
    return;
  }

  unsigned long now = millis();

  if (!buzzerActive)
    return;

  // Each alert lasts a maximum of 30 seconds.
  if (now - buzzerStartTime >= BUZZER_DURATION)
  {
    stopBuzzer();
    return;
  }

  // SEVERE: continuously ON for the remaining alert time.
  if (aqi > 400)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerSoundOn = true;
    return;
  }

  unsigned long interval;

  // VERY POOR: 1 second ON / 1 second OFF.
  if (aqi > 300)
    interval = FAST_BEEP_INTERVAL;
  // POOR: 5 seconds ON / 5 seconds OFF.
  else
    interval = SLOW_BEEP_INTERVAL;

  // Non-blocking toggle.
  if (now - lastBuzzerToggle >= interval)
  {
    lastBuzzerToggle = now;
    buzzerSoundOn = !buzzerSoundOn;
    digitalWrite(BUZZER_PIN, buzzerSoundOn ? HIGH : LOW);
  }
}

// ============================================================

// READ PMS9103M

// ============================================================

bool readPMS()

{

  static uint8_t buffer[32];

  while (pmsSerial.available())

  {

    if (pmsSerial.peek() != 0x42)

    {

      pmsSerial.read();

      continue;

    }

    if (pmsSerial.available() < 32)

      return false;

    size_t bytesRead = pmsSerial.readBytes(buffer, 32);

    if (bytesRead != 32)

      continue;

    if (buffer[0] != 0x42 || buffer[1] != 0x4D)

      continue;

    uint16_t checksum = 0;

    for (int i = 0; i < 30; i++)

      checksum += buffer[i];

    uint16_t receivedChecksum =

      ((uint16_t)buffer[30] << 8) |

      buffer[31];

    if (checksum != receivedChecksum)

    {

      Serial.println("PMS checksum error");

      continue;

    }

    // Atmospheric concentration values

    pm1 =

      ((uint16_t)buffer[10] << 8) |

      buffer[11];

    pm25 =

      ((uint16_t)buffer[12] << 8) |

      buffer[13];

    pm10 =

      ((uint16_t)buffer[14] << 8) |

      buffer[15];

    return true;

  }

  return false;

}

// ============================================================

// READ DHT11 AND MQ-135

// ============================================================

void readOtherSensors()

{

  float newTemperature = dht.readTemperature();

  float newHumidity = dht.readHumidity();

  if (!isnan(newTemperature))

    temperature = newTemperature;

  if (!isnan(newHumidity))

    humidity = newHumidity;

  gasValue = analogRead(MQ135_PIN);

}

// ============================================================

// SEND BLUETOOTH STATUS

// ============================================================

void sendBluetoothStatus()

{

  if (!bluetooth.isConnected())

    return;

  bluetoothChat.send("ESP32 AIR MONITOR");

  bluetoothChat.send("AQI: " + String(aqi));

  bluetoothChat.send("Status: " + airStatus);

  bluetoothChat.send(

    "PM1.0: " + String(pm1, 1) + " ug/m3"

  );

  bluetoothChat.send(

    "PM2.5: " + String(pm25, 1) + " ug/m3"

  );

  bluetoothChat.send(

    "PM10: " + String(pm10, 1) + " ug/m3"

  );

  bluetoothChat.send(

    "Temperature: " + String(temperature, 1) + " C"

  );

  bluetoothChat.send(

    "Humidity: " + String(humidity, 1) + " %"

  );

  bluetoothChat.send(

    "MQ-135: " + String(gasValue)

  );

}

// ============================================================

// AQI UPDATE, LED CONTROL AND ALERTS

// ============================================================

void updateAQI()

{

  aqi = calculateAQI(pm25);

  airStatus = getAQIStatus(aqi);

  int currentAQIState;

  if (aqi <= 200)

    currentAQIState = 0;

  else if (aqi <= 300)

    currentAQIState = 1;

  else if (aqi <= 400)

    currentAQIState = 2;

  else

    currentAQIState = 3;

  // ----------------------------------------------------------

  // LED CONTROL

  // ----------------------------------------------------------

  if (aqi <= 100)

  {

    digitalWrite(GREEN_LED, HIGH);

    digitalWrite(RED_LED, LOW);

  }

  else if (aqi <= 200)

  {

    digitalWrite(GREEN_LED, LOW);

    digitalWrite(RED_LED, LOW);

  }

  else

  {

    digitalWrite(GREEN_LED, LOW);

    digitalWrite(RED_LED, HIGH);

  }

  // ----------------------------------------------------------

  // FIRST VALID STATE

  // ----------------------------------------------------------

  if (previousAQIState == -1)

  {

    previousAQIState = currentAQIState;

    // Alert immediately if first reading is above 200

    if (currentAQIState > 0)

    {

      startBuzzer30Sec();

      if (bluetooth.isConnected())

      {

        bluetoothChat.send("WARNING: AIR QUALITY HIGH");

        bluetoothChat.send("AQI: " + String(aqi));

        bluetoothChat.send("Status: " + airStatus);

      }

      String message = "AIR QUALITY ALERT\n\n";

      message += "AQI: " + String(aqi) + "\n";

      message += "Status: " + airStatus + "\n";

      message += "PM2.5: " + String(pm25, 1) + " ug/m3\n";

      message += "Temperature: " + String(temperature, 1) + " C\n";

      message += "Humidity: " + String(humidity, 1) + " %\n";

      message += "MQ-135: " + String(gasValue);

      sendTelegramMessage(message);

      lastTelegramTime = millis();

    }

    return;

  }

  // ----------------------------------------------------------

  // AQI CROSSED ABOVE 200

  // Start one 30-second alert.

  // The buzzer pattern changes automatically if AQI rises.

  // ----------------------------------------------------------

  if (previousAQIState == 0 && currentAQIState > 0)

  {

    startBuzzer30Sec();

    if (bluetooth.isConnected())

    {

      bluetoothChat.send("WARNING: AIR QUALITY HIGH");

      bluetoothChat.send("AQI: " + String(aqi));

      bluetoothChat.send("Status: " + airStatus);

      bluetoothChat.send("Buzzer alert: 30 seconds");

    }

    if (

      lastTelegramTime == 0 ||

      millis() - lastTelegramTime >= TELEGRAM_COOLDOWN

    )

    {

      String message = "AIR QUALITY ALERT\n\n";

      message += "AQI: " + String(aqi) + "\n";

      message += "Status: " + airStatus + "\n";

      message += "PM1.0: " + String(pm1, 1) + " ug/m3\n";

      message += "PM2.5: " + String(pm25, 1) + " ug/m3\n";

      message += "PM10: " + String(pm10, 1) + " ug/m3\n";

      message += "Temperature: " + String(temperature, 1) + " C\n";

      message += "Humidity: " + String(humidity, 1) + " %\n";

      message += "MQ-135: " + String(gasValue) + "\n";

      message += "Buzzer duration: 30 seconds";

      sendTelegramMessage(message);

      lastTelegramTime = millis();

    }

  }

  // ----------------------------------------------------------

  // AQI IMPROVED TO 200 OR BELOW

  // ----------------------------------------------------------

  if (previousAQIState > 0 && currentAQIState == 0)

  {

    if (bluetooth.isConnected())

    {

      bluetoothChat.send("AIR QUALITY IMPROVED");

      bluetoothChat.send("AQI: " + String(aqi));

      bluetoothChat.send("Status: " + airStatus);

    }

    if (

      lastTelegramTime == 0 ||

      millis() - lastTelegramTime >= TELEGRAM_COOLDOWN

    )

    {

      String message = "AIR QUALITY IMPROVED\n\n";

      message += "AQI: " + String(aqi) + "\n";

      message += "Status: " + airStatus + "\n";

      message += "PM2.5: " + String(pm25, 1) + " ug/m3";

      sendTelegramMessage(message);

      lastTelegramTime = millis();

    }

  }

  // Store the new state.

  // Do not restart the buzzer timer when AQI moves from

  // 250 to 350 or from 350 to 450 during an active alert.

  previousAQIState = currentAQIState;

}

// ============================================================

// LCD DISPLAY

// ============================================================

void updateLCD()

{

  static unsigned long lastLCD = 0;

  static int screen = 0;

  if (millis() - lastLCD < 3000)

    return;

  lastLCD = millis();

  lcd.clear();

  if (screen == 0)

  {

    lcd.setCursor(0, 0);

    lcd.print("PM2.5:");

    lcd.print(pm25, 1);

    lcd.setCursor(0, 1);

    lcd.print("AQI:");

    lcd.print(aqi);

    lcd.print(" ");

    lcd.print(airStatus.substring(0, 7));

  }

  else if (screen == 1)

  {

    lcd.setCursor(0, 0);

    lcd.print("Temp:");

    lcd.print(temperature, 1);

    lcd.print(" C");

    lcd.setCursor(0, 1);

    lcd.print("Hum:");

    lcd.print(humidity, 1);

    lcd.print(" %");

  }

  else

  {

    lcd.setCursor(0, 0);

    lcd.print("PM1:");

    lcd.print(pm1, 1);

    lcd.setCursor(0, 1);

    lcd.print("PM10:");

    lcd.print(pm10, 1);

  }

  screen++;

  if (screen > 2)

    screen = 0;

}

// ============================================================

// BLUETOOTH COMMANDS

// ============================================================

void handleBluetoothCommand(const String& message)

{

  String cmd = message;

  cmd.trim();

  cmd.toUpperCase();

  Serial.print("Bluetooth command: ");

  Serial.println(cmd);

  if (cmd == "STATUS")

  {

    sendBluetoothStatus();

  }

  else if (cmd == "PM")

  {

    bluetoothChat.send(

      "PM1.0: " + String(pm1, 1) + " ug/m3"

    );

    bluetoothChat.send(

      "PM2.5: " + String(pm25, 1) + " ug/m3"

    );

    bluetoothChat.send(

      "PM10: " + String(pm10, 1) + " ug/m3"

    );

  }

  else if (cmd == "TEMPERATURE" || cmd == "TEMP")

  {

    bluetoothChat.send(

      "Temperature: " + String(temperature, 1) + " C"

    );

  }

  else if (cmd == "HUMIDITY")

  {

    bluetoothChat.send(

      "Humidity: " + String(humidity, 1) + " %"

    );

  }

  else if (cmd == "GAS")

  {

    bluetoothChat.send(

      "MQ-135: " + String(gasValue)

    );

  }

  else if (cmd == "AQI")

  {

    bluetoothChat.send("AQI: " + String(aqi));

    bluetoothChat.send("Status: " + airStatus);

  }

  else if (cmd == "TELEGRAM")

  {

    String message = "ESP32 Air Monitor\n";

    message += "AQI: " + String(aqi) + "\n";

    message += "Status: " + airStatus + "\n";

    message += "PM2.5: " + String(pm25, 1) + " ug/m3\n";

    message += "Temperature: " + String(temperature, 1) + " C\n";

    message += "Humidity: " + String(humidity, 1) + " %";

    sendTelegramMessage(message);

    bluetoothChat.send(

      "Telegram request attempted. Check Serial Monitor."

    );

  }

  else if (cmd == "HELP")

  {

    bluetoothChat.send("Commands: STATUS, PM, TEMP");

    bluetoothChat.send("HUMIDITY, GAS, AQI, TELEGRAM, HELP");

  }

  else

  {

    bluetoothChat.send("Unknown command. Type HELP.");

  }

}

// ============================================================
// WEB DASHBOARD
// ============================================================

void handleRoot()
{
  String html = R"rawliteral(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta name="theme-color" content="#0b1220"><title>AirSense | ESP32 Air Quality Monitor</title><style>
:root{color-scheme:dark;--bg:#08111f;--panel:#111e30;--panel2:#14243a;--line:#24364c;--text:#edf5ff;--muted:#8da2bb;--green:#42e0a1;--cyan:#54d8ed;--yellow:#f7cf63;--orange:#ff9a55;--red:#ff657a}*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at 10% 0%,#12324a 0,transparent 35%),radial-gradient(ellipse at 100% 30%,#102b2a 0,transparent 28%),var(--bg);font-family:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif;color:var(--text);min-height:100vh}.wrap{width:min(1120px,calc(100% - 32px));margin:auto;padding:28px 0 24px}.top{display:flex;align-items:center;justify-content:space-between;gap:16px;margin-bottom:28px}.brand{display:flex;align-items:center;gap:12px}.logo{width:44px;height:44px;display:grid;place-items:center;border-radius:14px;background:linear-gradient(135deg,#2bd7a0,#39b9d9);color:#062019;font-size:24px;font-weight:900;box-shadow:0 8px 30px #2bd7a025}.eyebrow{font-size:11px;letter-spacing:.18em;color:var(--green);font-weight:800;text-transform:uppercase}.brand h1{font-size:20px;margin:3px 0 0;letter-spacing:.02em}.connection{display:flex;align-items:center;gap:8px;border:1px solid var(--line);background:#101c2c;border-radius:999px;padding:10px 13px;color:var(--muted);font-size:12px;white-space:nowrap}.dot{width:8px;height:8px;background:var(--yellow);border-radius:50%;box-shadow:0 0 12px currentColor}.connection.online .dot{background:var(--green)}.hero{display:grid;grid-template-columns:1.2fr .8fr;gap:16px;margin-bottom:16px}.panel,.metric{background:linear-gradient(145deg,#14243aee,#0e1a2aee);border:1px solid var(--line);border-radius:22px;box-shadow:0 14px 40px #00000018}.aqi-panel{padding:25px;position:relative;overflow:hidden;min-height:238px}.aqi-panel:after{content:"";position:absolute;width:210px;height:210px;border-radius:50%;border:1px solid #42e0a118;right:-65px;top:-70px;box-shadow:0 0 0 22px #42e0a108,0 0 0 45px #42e0a105;pointer-events:none}.section-tag{color:var(--muted);font-size:12px;font-weight:700;letter-spacing:.11em;text-transform:uppercase}.aqi-row{display:flex;align-items:center;gap:22px;margin-top:18px}.aqi-number{font-size:clamp(58px,8vw,88px);line-height:1;font-weight:800;letter-spacing:-.07em}.aqi-sub{color:var(--muted);font-size:13px;margin-top:8px}.status-pill{display:inline-flex;align-items:center;gap:7px;margin-top:12px;padding:8px 11px;border-radius:999px;background:#42e0a118;color:var(--green);font-weight:750;font-size:12px;border:1px solid #42e0a13b}.status-pill .dot{background:currentColor}.scale{display:flex;gap:4px;margin-top:24px}.scale span{height:5px;flex:1;border-radius:99px;background:var(--line)}.scale span:nth-child(1){background:#42e0a1}.scale span:nth-child(2){background:#b4dc62}.scale span:nth-child(3){background:#f7cf63}.scale span:nth-child(4){background:#ff9a55}.scale span:nth-child(5){background:#ff657a}.scale span:nth-child(6){background:#b85c68}.scale-note{display:flex;justify-content:space-between;color:var(--muted);font-size:10px;margin-top:7px}.side-panel{padding:22px;display:flex;flex-direction:column;justify-content:space-between}.side-panel h2{font-size:16px;margin:8px 0;color:var(--text)}.side-panel p{font-size:12px;line-height:1.7;color:var(--muted);margin:0}.mini-stat{display:flex;align-items:center;justify-content:space-between;margin-top:18px;padding-top:15px;border-top:1px solid var(--line);font-size:12px;color:var(--muted)}.mini-stat strong{color:var(--text);font-weight:700}.grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:12px}.metric{padding:18px;min-height:128px;transition:transform .2s,border-color .2s}.metric:hover{transform:translateY(-2px);border-color:#3d6278}.metric-head{display:flex;align-items:center;justify-content:space-between;gap:8px}.metric-label{font-size:12px;color:var(--muted);font-weight:650}.metric-icon{width:30px;height:30px;display:grid;place-items:center;border-radius:10px;background:#54d8ed12;color:var(--cyan);font-size:15px}.metric-value{font-size:clamp(22px,3vw,29px);font-weight:780;letter-spacing:-.04em;margin-top:15px;white-space:nowrap}.unit{font-size:12px;font-weight:600;letter-spacing:0;color:var(--muted);margin-left:4px}.metric-foot{font-size:10px;color:var(--muted);margin-top:7px}.lower{display:grid;grid-template-columns:1fr;gap:12px;margin-top:16px}.chart-panel{padding:20px 22px}.chart-head{display:flex;align-items:flex-start;justify-content:space-between;gap:12px}.chart-head h2{font-size:15px;margin:5px 0}.chart-head p{font-size:11px;color:var(--muted);margin:0}.chart-badge{font-size:11px;color:var(--cyan);border:1px solid #54d8ed33;background:#54d8ed0d;border-radius:99px;padding:7px 10px;white-space:nowrap}canvas{display:block;width:100%;height:135px;margin-top:14px}.footer{display:flex;justify-content:space-between;gap:12px;flex-wrap:wrap;color:#71869f;font-size:10px;line-height:1.7;margin-top:18px;padding:0 3px}.footer strong{color:#9eb1c7}.error{color:var(--orange)!important}@media(max-width:760px){.wrap{width:calc(100% - 22px);padding-top:18px}.top{align-items:flex-start}.brand h1{font-size:16px}.connection{font-size:10px;padding:8px 10px}.hero{grid-template-columns:1fr}.aqi-panel{min-height:215px}.side-panel{min-height:150px}.grid{grid-template-columns:repeat(2,minmax(0,1fr));gap:9px}.metric{padding:14px;min-height:116px}.metric-value{font-size:23px}.aqi-row{gap:16px}.footer{font-size:10px}}@media(max-width:390px){.brand{gap:8px}.logo{width:38px;height:38px}.brand h1{font-size:14px}.connection{gap:5px}.metric-value{font-size:21px}}
</style></head><body><main class="wrap"><header class="top"><div class="brand"><div class="logo" aria-hidden="true">⌁</div><div><div class="eyebrow">Live Air Monitoring</div><h1>AIRSENSE <span style="color:var(--muted);font-weight:500">/ AIR MONITOR</span></h1></div></div><div id="connection" class="connection"><span class="dot"></span><span id="connectionText">Connecting to ESP32…</span></div></header><section class="hero"><article class="panel aqi-panel"><div class="section-tag">Air Quality Index</div><div class="aqi-row"><div><div id="aqi" class="aqi-number">--</div><div class="aqi-sub">Current air quality</div></div><div><div id="status" class="status-pill"><span class="dot"></span><span>Waiting for data</span></div><div class="aqi-sub" id="statusHelp">Live sensor status</div></div></div><div class="scale" aria-label="Air quality colour scale"><span title="Good: 0–50"></span><span title="Satisfactory: 51–100"></span><span title="Moderate: 101–200"></span><span title="Poor: 201–300"></span><span title="Very Poor: 301–400"></span><span title="Severe: 401–500"></span></div><div class="scale-note"><span>Good<br>0–50</span><span>Satisfactory<br>51–100</span><span>Moderate<br>101–200</span><span>Poor<br>201–300</span><span>Very Poor<br>301–400</span><span>Severe<br>401–500</span></div></article><aside class="panel side-panel"><div><div class="section-tag">Live monitoring</div><h2>Know the air around you.</h2><p>Sensor readings update automatically from the ESP32. View live environmental readings from your device.</p></div><div class="mini-stat"><span>Last updated</span><strong id="updated">Waiting…</strong></div></aside></section><section class="grid" aria-label="Live sensor readings"><article class="metric"><div class="metric-head"><span class="metric-label">PM1.0</span><span class="metric-icon">◌</span></div><div class="metric-value" id="pm1">-- <span class="unit">µg/m³</span></div><div class="metric-foot">Fine particles</div></article><article class="metric"><div class="metric-head"><span class="metric-label">PM2.5</span><span class="metric-icon">◉</span></div><div class="metric-value" id="pm25">-- <span class="unit">µg/m³</span></div><div class="metric-foot">Fine particulate matter</div></article><article class="metric"><div class="metric-head"><span class="metric-label">PM10</span><span class="metric-icon">◍</span></div><div class="metric-value" id="pm10">-- <span class="unit">µg/m³</span></div><div class="metric-foot">Inhalable particles</div></article><article class="metric"><div class="metric-head"><span class="metric-label">MQ-135</span><span class="metric-icon">⌁</span></div><div class="metric-value" id="gas">--</div><div class="metric-foot">Raw ADC sensor value</div></article><article class="metric"><div class="metric-head"><span class="metric-label">Temperature</span><span class="metric-icon">♨</span></div><div class="metric-value" id="temp">-- <span class="unit">°C</span></div><div class="metric-foot">Ambient temperature</div></article><article class="metric"><div class="metric-head"><span class="metric-label">Humidity</span><span class="metric-icon">⌇</span></div><div class="metric-value" id="hum">-- <span class="unit">%</span></div><div class="metric-foot">Relative humidity</div></article><article class="metric"><div class="metric-head"><span class="metric-label">Data refresh</span><span class="metric-icon">↻</span></div><div class="metric-value">2<span class="unit">sec</span></div><div class="metric-foot">Automatic refresh interval</div></article><article class="metric"><div class="metric-head"><span class="metric-label">Device</span><span class="metric-icon">⌘</span></div><div class="metric-value" style="font-size:20px">ESP32</div><div class="metric-foot" id="deviceState">Awaiting connection</div></article></section><section class="lower"><article class="panel chart-panel"><div class="chart-head"><div><div class="section-tag">Recent trend</div><h2>PM2.5 readings</h2><p>Recent samples collected while this page is open</p></div><div class="chart-badge">LIVE · PM2.5</div></div><canvas id="chart" aria-label="Recent PM2.5 readings chart"></canvas></article></section><footer class="footer"><span><strong>AirSense</strong> · ESP32 Air Quality Monitor</span><span>Live sensor readings from your device.</span></footer></main><script>
const $=id=>document.getElementById(id);const history=[];let lastSuccess=0;function setValue(id,value,unit=''){ $(id).innerHTML=`${value}<span class="unit">${unit}</span>`; }function setStatus(status,aqi){const s=String(status??'Unknown');$('status').innerHTML=`<span class="dot"></span><span>${s}</span>`;let color='var(--green)';const t=s.toLowerCase();if(t.includes('severe')||t.includes('hazard'))color='var(--red)';else if(t.includes('poor')||t.includes('unhealthy'))color='var(--orange)';else if(t.includes('moderate'))color='var(--yellow)';$('status').style.color=color;$('status').style.background=`color-mix(in srgb, ${color} 10%, transparent)`;$('status').style.borderColor=`color-mix(in srgb, ${color} 30%, transparent)`;$('aqi').style.color=color;$('statusHelp').textContent=Number.isFinite(Number(aqi))?`AQI: ${aqi}`:'Live air quality status';}function drawChart(){const c=$('chart'),r=c.getBoundingClientRect(),dpr=window.devicePixelRatio||1;c.width=Math.max(1,r.width*dpr);c.height=Math.max(1,r.height*dpr);const x=c.getContext('2d');x.scale(dpr,dpr);const w=r.width,h=r.height,pad=22; x.clearRect(0,0,w,h);x.strokeStyle='#26384c';x.lineWidth=1;for(let i=0;i<4;i++){let y=pad+i*(h-pad*2)/3;x.beginPath();x.moveTo(0,y);x.lineTo(w,y);x.stroke()}if(history.length<2){x.fillStyle='#8da2bb';x.font='12px system-ui';x.fillText('Waiting for more readings to build the trend…',10,h/2);return}const vals=history.map(v=>v.value),min=Math.min(0,...vals),max=Math.max(10,...vals),range=max-min||1;const pts=vals.map((v,i)=>({x:8+i*(w-16)/(vals.length-1),y:h-pad-(v-min)/range*(h-pad*2)}));const g=x.createLinearGradient(0,pad,0,h);g.addColorStop(0,'#42e0a133');g.addColorStop(1,'#42e0a100');x.beginPath();x.moveTo(pts[0].x,h-pad);pts.forEach(pt=>x.lineTo(pt.x,pt.y));x.lineTo(pts.at(-1).x,h-pad);x.closePath();x.fillStyle=g;x.fill();x.beginPath();pts.forEach((pt,i)=>i?x.lineTo(pt.x,pt.y):x.moveTo(pt.x,pt.y));x.strokeStyle='#42e0a1';x.lineWidth=2.5;x.lineJoin='round';x.stroke();pts.forEach(pt=>{x.beginPath();x.arc(pt.x,pt.y,2.5,0,Math.PI*2);x.fillStyle='#54d8ed';x.fill()})}async function updateData(){try{const response=await fetch('/data',{cache:'no-store'});if(!response.ok)throw Error('HTTP '+response.status);const data=await response.json();setValue('pm1',data.pm1??'--','µg/m³');setValue('pm25',data.pm25??'--','µg/m³');setValue('pm10',data.pm10??'--','µg/m³');setValue('gas',data.gas??'--');setValue('temp',data.temperature??'--','°C');setValue('hum',data.humidity??'--','%');$('aqi').textContent=data.aqi??'--';setStatus(data.status,data.aqi);$('connection').classList.add('online');$('connectionText').textContent='ESP32 connected';$('deviceState').textContent='Receiving live data';$('updated').textContent=new Date().toLocaleTimeString([], {hour:'2-digit',minute:'2-digit',second:'2-digit'});lastSuccess=Date.now();if(Number.isFinite(Number(data.pm25))){history.push({value:Number(data.pm25)});if(history.length>24)history.shift();drawChart()}}catch(e){$('connection').classList.remove('online');$('connectionText').textContent='Connection lost';$('deviceState').textContent='Check ESP32 connection';$('updated').textContent=lastSuccess?'Last good update '+new Date(lastSuccess).toLocaleTimeString():'No data received';}}window.addEventListener('resize',drawChart);drawChart();updateData();setInterval(updateData,2000);
</script></body></html>
)rawliteral";

  server.send(200, "text/html", html);
}

// ============================================================

// WEB JSON DATA

// ============================================================

void handleData()

{

  String json = "{";

  json += "\"pm1\":" + String(pm1, 1);

  json += ",\"pm25\":" + String(pm25, 1);

  json += ",\"pm10\":" + String(pm10, 1);

  json += ",\"aqi\":" + String(aqi);

  json += ",\"status\":\"" + airStatus + "\"";

  json += ",\"temperature\":" + String(temperature, 1);

  json += ",\"humidity\":" + String(humidity, 1);

  json += ",\"gas\":" + String(gasValue);

  json += "}";

  server.send(200, "application/json", json);

}

// ============================================================

// WIFI CONNECTION WITH BACKUP HOTSPOT

// ============================================================

void connectToWiFi()

{

  WiFi.mode(WIFI_STA);

  WiFi.setSleep(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

  unsigned long wifiStart = millis();

  while (

    WiFi.status() != WL_CONNECTED &&

    millis() - wifiStart < 20000

  )

  {

    delay(500);

    Serial.print(".");

  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)

  {

    normalWiFiConnected = true;

    Serial.println("Wi-Fi connected!");

    Serial.print("ESP32 IP: ");

    Serial.println(WiFi.localIP());

  }

  else

  {

    normalWiFiConnected = false;

    Serial.println("Wi-Fi connection failed.");

    Serial.println("Starting backup hotspot...");

    WiFi.mode(WIFI_AP);

    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

    if (apStarted)

    {

      Serial.println("Backup hotspot started.");

      Serial.print("Hotspot IP: ");

      Serial.println(WiFi.softAPIP());

      Serial.print("Network: ");

      Serial.println(AP_SSID);

      Serial.print("Password: ");

      Serial.println(AP_PASSWORD);

      Serial.println(

        "Dashboard available from a connected device."

      );

    }

    else

    {

      Serial.println("Backup hotspot failed.");

    }

  }

}

// ============================================================

// SETUP

// ============================================================

void setup()

{

  Serial.begin(115200);

  delay(1000);

  Serial.println();

  Serial.println("================================");

  Serial.println("ESP32 SMART AIR QUALITY MONITOR");

  Serial.println("================================");

  // OUTPUTS

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(RED_LED, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RED_LED, LOW);

  digitalWrite(GREEN_LED, LOW);

  // LCD

  Wire.begin(LCD_SDA, LCD_SCL);

  lcd.init();

  lcd.backlight();

  lcd.setCursor(0, 0);

  lcd.print("AIR MONITOR");

  lcd.setCursor(0, 1);

  lcd.print("Starting...");

  // DHT11

  dht.begin();

  // PMS9103M UART

  pmsSerial.begin(

    9600,

    SERIAL_8N1,

    PMS_RX_PIN,

    PMS_TX_PIN

  );

  pmsStartTime = millis();

  Serial.println("PMS UART started.");

  // WIFI

  connectToWiFi();

  // WEB SERVER

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.begin();

  Serial.println("Web server started.");

  // DIYABLES BLE

  bluetoothServer.begin();

  bluetoothServer.addApp(&bluetoothChat);

  bluetoothServer.setOnConnected(

    []()

    {

      Serial.println("Bluetooth connected!");

      bluetoothChat.send("ESP32 Air Monitor ready!");

      bluetoothChat.send("Type HELP for commands.");

    }

  );

  bluetoothServer.setOnDisconnected(

    []()

    {

      Serial.println("Bluetooth disconnected!");

    }

  );

  bluetoothChat.onChatMessage(

    [](const String& message)

    {

      handleBluetoothCommand(message);

    }

  );

  Serial.println("BLE initialized.");

  Serial.print("BLE device name: ");

  Serial.println(DEVICE_NAME);

  // TELEGRAM STARTUP MESSAGE

  if (normalWiFiConnected)

  {

    sendTelegramMessage(

      "ESP32 Air Monitor is ONLINE!"

    );

  }

  else

  {

    Serial.println(

      "Telegram startup message skipped: no internet Wi-Fi."

    );

  }

  Serial.println("System ready.");

}

// ============================================================

// MAIN LOOP

// ============================================================

void loop()

{

  // Keep the web server responsive

  server.handleClient();

  // Process BLE

  bluetoothServer.loop();

  // PMS SENSOR

  if (millis() - pmsStartTime >= PMS_WARMUP)

  {

    if (readPMS())

    {

      updateAQI();

      // Re-check the buzzer immediately after updating AQI.
      updateBuzzer();

    }

  }

  // Keep checking the non-blocking buzzer timer.
  updateBuzzer();

  // DHT11 AND MQ-135

  static unsigned long lastSensorRead = 0;

  if (millis() - lastSensorRead >= 2000)

  {

    lastSensorRead = millis();

    readOtherSensors();

  }

  // LCD

  updateLCD();

  delay(10);

}
