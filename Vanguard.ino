#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiManager.h>
#include "credentials.h"
// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

#define BUTTON_PIN 0

// Data wire is plugged into port 23 on the ESP32
#define ONE_WIRE_BUS 23

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// Wifi network station credentials
//#define WIFI_SSID "secret"
//#define WIFI_PASSWORD "secretpassword"

//  ****Telegram BOT Token (Get from Botfather) and define in credentials.txt ****

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done

float tempC = 0;
float vBatt = 0;
float vHouse = 0;

float v1 = 0;
float v2 = 0;

char botToken[40];

char tempCLimit[3] = "25";
char vBattLimit[5] = "11.8";
char vHouseLimit[5] = "11.8";

String chat_id2 = "-1";
String chat_id = "-1";
char chatId[20]  = "-1";

//flag for saving data
bool shouldSaveConfig = false;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  String answer = "";
  if ((bot.messages[0].chat_id == chat_id2)||(bot.messages[0].chat_id == chat_id))
  {
    for (int i = 0; i < numNewMessages; i++)
    {
      telegramMessage &msg = bot.messages[i];
      Serial.println("Received " + msg.text);
      answer = "Temperature: " ;
      answer.concat( tempC);
      answer.concat("\n");
      answer += "Vbatt: ";
      answer.concat(v1);
      answer += "V\n";
      answer += "Vdom: ";
      answer.concat(v2);
      answer += "V";

      bot.sendMessage(chat_id, answer, "Markdown");
      bot.sendMessage(chat_id2, answer, "Markdown");
      Serial.print("chat_id: ");
      Serial.println(chat_id);
      Serial.print("chat_id: ");
      Serial.println(chat_id2);

    }
  }
}

void sendMessage()
{
  telegramMessage &msg = bot.messages[0];
  Serial.println("M-Received " + msg.text);

  String answer = "Temperature: " ;
  answer.concat( tempC);
  answer.concat("\n");
  answer += "Vbatt: ";
  answer.concat(v1);
  answer += "V\n";
  answer += "Vdom: ";
  answer.concat(v2);
  answer += "V";
  Serial.print("M-chat_id: ");
  Serial.println(chat_id);
  bot.sendMessage(chat_id, answer, "Markdown");
  delay(1000);
  bot.sendMessage(chat_id2, answer, "Markdown");
  Serial.print("M-chat_id: ");
  Serial.println(chat_id2);
  Serial.println(BOT_TOKEN);
}


void bot_setup()
{
  const String commands = F("["

                            "{\"command\":\"status\",\"description\":\"Get current status\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);
  //bot.sendMessage("25235518", "Hola amigo!", "Markdown");
}

void setupSpiffs() {
  // clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(true)) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error) {
          Serial.println("\nparsed json");

          strcpy(botToken, json["botToken"]);
          strcpy(tempCLimit, json["tempCLimit"]);
          strcpy(vBattLimit, json["vBattLimit"]);
          strcpy(vHouseLimit, json["vHouseLimit"]);
          strcpy(chatId, json["chatId"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setupSpiffs();

  WiFiManager wm;

  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
  /*          strcpy(botToken, json["botToken"]);
            strcpy(tempCLimit, json["tempCLimit"]);
            strcpy(vBattLimit, json["vBattLimit"]);
            strcpy(vHouseLimit, json["vHouseLimit"]);
  */
  WiFiManagerParameter custom_bot_token("botToken", "Bot Token", botToken, 60);
  WiFiManagerParameter custom_tempCLimit("tempCLimit", "Temp Limit (C)", tempCLimit, 4);
  WiFiManagerParameter custom_batt_limit("vBattLimit", "Veh Batt Limit", vBattLimit, 4);
  WiFiManagerParameter custom_house_limit("vHouseLimit", "House Batt Limit", vHouseLimit, 4);
  WiFiManagerParameter custom_chat_id("chatId", "Chat ID", chatId, 20);

  //add all your parameters here
  wm.addParameter(&custom_bot_token);
  wm.addParameter(&custom_tempCLimit);
  wm.addParameter(&custom_batt_limit);
  wm.addParameter(&custom_house_limit);
  wm.addParameter(&custom_chat_id);

  //reset settings - wipe credentials for testing
  //wm.resetSettings();

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Entering Forced Config Mode");
    if (!wm.startConfigPortal("SmarterDisplay", "everyday")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  } else {
    if (!wm.autoConnect("SmarterDisplay", "everyday")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }

  strcpy(botToken, custom_bot_token.getValue());
  strcpy(tempCLimit, custom_tempCLimit.getValue());
  strcpy(vBattLimit, custom_batt_limit.getValue());
  strcpy(vHouseLimit, custom_house_limit.getValue());
  strcpy(vBattLimit, custom_batt_limit.getValue());
  strcpy(chatId, custom_chat_id.getValue());

  float ftempLimit = atof(tempCLimit);
  float fBattLimit = atof(vBattLimit);
  float fHouseLimit = atof(vHouseLimit);

  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument  json(200);
    json["botToken"] = botToken;
    json["tempCLimit"]   = tempCLimit;
    json["vHouseLimit"]   = vHouseLimit;
    json["vBattLimit"]   = vBattLimit;
    json["chatId"]   = chatId;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJsonPretty(json, Serial);
    if (serializeJson(json, configFile) == 0) {
      Serial.println(F("Failed to write to file"));
    }
    configFile.close();
    //end save
    shouldSaveConfig = false;
  }

  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  bot.updateToken(String(botToken));

  /*  // attempt to connect to Wifi network:
    Serial.print("Connecting to Wifi SSID ");
    Serial.print(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(500);
    }
  */
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  /*
    Serial.print("Retrieving time: ");
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    while (now < 24 * 3600)
    {
      Serial.print(".");
      delay(100);
      now = time(nullptr);
    }
    Serial.println(now);
  */
  bot_setup();
  temp_setup();
  temp_loop();
  adc_setup();
  adc_loop();
  /*
    float ftempLimit = atof(tempCLimit);
    float fBattLimit = atof(vBattLimit);
    float fHouseLimit = atof(vHouseLimit);
  */
  /*
    v1 = 10.8;
    v2 = 12.8;
    tempC = 24;
  */

  Serial.println(v1);
  Serial.println(v2);
  Serial.println(tempC);
  if ((v1 < fBattLimit) || (v2 < fHouseLimit) || (tempC > ftempLimit))
  {
    Serial.println(fBattLimit);
    Serial.println(fHouseLimit);
    Serial.println(ftempLimit);
    sendMessage();
  }
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
  sleep_setup();

}
