#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!
#define LOOP_WAIT 30000
#define LED_BUILTIN 2
#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 23 on the ESP32
#define ONE_WIRE_BUS 23

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

// ================================================ EXAMPLE VARS =========================================
// used in this example to print variables every 10 seconds
unsigned long printEntry;
String deviceName = "Vanguard";
String chipId;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* lbl         = "Vanguard";
char* updInt      = "7200";

char* botToken = "11:aa";
char* tempCLimit = "25";
char* vBattLimit = "11.8";
char* vHouseLimit = "11.8";
char* sleepTime = "300";

String chat_id2 = "725925511";
char* chat_id = "-1";

//  ****Telegram BOT Token (Get from Botfather) and define in credentials.txt ****

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done

float tempC = 0;
float vBatt = 0;
float vHouse = 0;
long lSleepTime = 0;

float v1 = 0;
float v2 = 0;
bool statusFlag = false;


WiFiClientSecure secured_client;
UniversalTelegramBot bot(String(botToken), secured_client);

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  String answer = "Answer message \n";

  Serial.print("chat_id: ");
  Serial.println(chat_id);

  if ((bot.messages[0].chat_id == chat_id2) || (bot.messages[0].chat_id == String(chat_id)))
  {

    Serial.print("chat_id: ");
    Serial.println(chat_id);

    for (int i = 0; i < numNewMessages; i++)
    {
      telegramMessage &msg = bot.messages[i];
      Serial.println("Answering message " +  String(i) + msg.text);
      answer += "Temperature: " ;
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
  Serial.println("Sending Parameters");

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
  Serial.println(botToken);
}

void bot_setup()
{
  const String commands = F("["

                            "{\"command\":\"status\",\"description\":\"Get current status\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);

}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  // create a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-" + chipId.substring(chipId.length() - 3);
  deviceName += chipId;

  /* TIP! delete lines below when not used */
  IAS.preSetDeviceName(deviceName);                         // preset deviceName this is also your MDNS responder: http://virginsoil-123.local
  IAS.preSetAutoUpdate(true);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  /* TIP! Delete Wifi cred. when you publish your App. */

  IAS.addField(updInt, "Update Interval", 8, 'I');
  //IAS.addField(ledPin, "ledPin", 2, 'P');
  //  IAS.addField(timeZone, "Timezone", 48, 'Z');

  IAS.addField(botToken, "botToken", 60, 'T');
  IAS.addField(tempCLimit, "tempCLimit", 25, 'N');
  IAS.addField(vBattLimit, "vBattLimit", 11.8, 'N');
  IAS.addField(vHouseLimit, "vHouseLimit", 11.8, 'N');
  IAS.addField(chat_id, "chat_id", 20, 'T');
  IAS.addField(sleepTime, "sleepTime", 5, 'N');

  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });

  IAS.onFirmwareUpdateProgress([](int written, int total) {
    Serial.print(".");
  });

  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production

  lSleepTime = atol(sleepTime);
  float ftempLimit = atof(tempCLimit);
  float fBattLimit = atof(vBattLimit);
  float fHouseLimit = atof(vHouseLimit);

  Serial.print("botToken: ");
  Serial.println(botToken);

  bot.updateToken(String(botToken));

  bot_setup();
  temp_setup();
  temp_loop();
  adc_setup();
  adc_loop();

  Serial.print("V1: ");
  Serial.print(v1);
  Serial.print(" Limit: ");
  Serial.println(fBattLimit);

  Serial.print("V2: ");
  Serial.print(v2);
  Serial.print(" Limit: ");
  Serial.println(fHouseLimit);

  Serial.print("Temp: ");
  Serial.print(tempC);
  Serial.print(" Limit: ");
  Serial.println(ftempLimit);

  if ((v1 < fBattLimit) || (v2 < fHouseLimit) || (tempC > ftempLimit))
  {
    Serial.println(fBattLimit);
    Serial.println(fHouseLimit);
    Serial.println(ftempLimit);
    sendMessage();
  }
  Serial.println("End setup");

  digitalWrite(LED_BUILTIN, 255);
}

void loop()
{
  IAS.loop();   // this routine handles the calling home functionality,

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
  if (millis() > LOOP_WAIT) //Wait for a while for the user to press a button
  {
    sleep_setup();
  } else {
    // We got a status message - loop for a minute
  }
}
