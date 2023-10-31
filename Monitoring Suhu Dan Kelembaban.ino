
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

const char* ssid = "BiboPororo";
const char* password = "biibo134";

#define DHTPIN 26    
#define DHTTYPE DHT11     
DHT dht(DHTPIN, DHTTYPE);

#define RELAYPIN 27
#define BOTtoken "5838832324:AAGuGozIjVJz_-_HKnHEvhI3jhYFamwcrtU"  
#define CHAT_ID "-905382241"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;



void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String control = "Selamat Datang di Bot Monitoring Suhu dan Kelembaban, " + from_name + ".\n\n";
      control += "Gunakan Perintah Di Bawah ini untuk Monitoring.\n\n";
      control += "/Temperature Untuk Monitoring Suhu. \n";
      control += "/Humidity Untuk Monitoring Kelembapan. \n";
      control += "/LampuON Untuk Menghidupkan Lampu. \n";
      control += "/LampuOFF Untuk Mematikan Lampu. \n";
      bot.sendMessage(chat_id, control, "");
    }

    if (text == "/Temperature") {
      int t= dht.readTemperature();
      if (isnan(t)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
      }
      String suhu = "Status Suhu ";
      suhu += t;
      suhu +=" ⁰C.\n";
      bot.sendMessage(chat_id, suhu, "");
    }
    
    if (text == "/Humidity") {
      int h= dht.readHumidity();
      if (isnan(h)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
      }
      String lembab = "Status Kelembapan ";
      lembab += h;
      lembab +=" %.\n";
      bot.sendMessage(chat_id, lembab, "");
    }

    if (text == "/LampuON") {
      turnOnRelay();
      turnOnOffLampu(true);
      bot.sendMessage(chat_id, "Lampu Telah Dihidupkan.");
    }
    if (text == "/LampuOFF") {
      turnOffRelay();
      delay(30000);
      bot.sendMessage(chat_id, "Lampu Telah Dimatikan.");
    }
  }
}

void turnOnRelay() {
    digitalWrite(RELAYPIN, HIGH);
}
void turnOffRelay() {
  digitalWrite(RELAYPIN, LOW);   // Mematikan relay
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAYPIN, OUTPUT);
  // Koneksi Ke Wifi
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}


    void checkTempHumidity() {
      int temperature = dht.readTemperature();
      int humidity = dht.readHumidity();

      String chat_id = "-905382241";
      String message = "";

      if (temperature < 29) {
        digitalWrite(RELAYPIN, HIGH);
        message += "Suhu dibawah 29 C, lampu dihidupkan.\n";
      } else if (humidity > 95) {
        digitalWrite(RELAYPIN, HIGH);
        message += "Kelembaban lebih dari 95 %, lampu dihidupkan.\n";
      } else {
        digitalWrite(RELAYPIN, LOW);
        message += "Suhu dan kelembaban telah normal.\n";
      }
      static unsigned long lastAutoMessageTime = 0;
      unsigned long autoMessageInterval = 60000;

      if (message != "" && millis() - lastAutoMessageTime >= autoMessageInterval) {
        bot.sendMessage(chat_id, message, "");
        lastAutoMessageTime = millis();
      }
    }

    void turnOnOffLampu(bool turnOn) {
      if (turnOn) {
        turnOnRelay();
        delay(30000);
        turnOffRelay();
      } else {
        digitalWrite(RELAYPIN, LOW);
      }
    }

    void loop() {
      checkTempHumidity();

    // Logika untuk menangani perintah lainnya
    if (millis() > lastTimeBotRan + botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {
        Serial.println("Menerima Tanggapan");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
    }
    }