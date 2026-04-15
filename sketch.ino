#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define BOT_TOKEN "8556007347:AAHP-LZCEYIMsVqpFBS-PXCdKpsmSd-wBuE"
#define CHAT_ID " 5750987705"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define LED_PIN 2
#define LDR_PIN 34
#define DHTPIN 15
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ===================== CONTROL =====================
bool ledState = false;
bool alarmSent = false;

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  client.setInsecure();
}

String getLightStatus(int value) {
  if (value > 3000) return "GELAP 🌑";
  else if (value > 1500) return "REDUP 🌥️";
  else return "TERANG ☀️";
}

void handleMessage(String text, String chat_id) {

  // ===== START =====
  if (text == "/start") {
    String msg = "🤖 SMART MONITORING SYSTEM\n\n";
    msg += "💡 CONTROL:\n";
    msg += "/led_on /led_off /led_status\n\n";
    msg += "📊 MONITORING:\n";
    msg += "/cek_suhu\n/cek_kelembapan\n/cek_cahaya\n/status";
    bot.sendMessage(chat_id, msg, "");
  }

  else if (text == "/led_on") {
    digitalWrite(LED_PIN, HIGH);
    ledState = true;
    bot.sendMessage(chat_id, "💡 LED DINYALAKAN", "");
  }

  else if (text == "/led_off") {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
    bot.sendMessage(chat_id, "🌑 LED DIMATIKAN", "");
  }

  else if (text == "/led_status") {
    bot.sendMessage(chat_id, ledState ? "LED ON 💡" : "LED OFF 🌑", "");
  }

  else if (text == "/cek_suhu") {
    float t = dht.readTemperature();
    bot.sendMessage(chat_id, "🌡️ Suhu: " + String(t) + "°C", "");
  }

  else if (text == "/cek_kelembapan") {
    float h = dht.readHumidity();
    bot.sendMessage(chat_id, "💧 Kelembapan: " + String(h) + "%", "");
  }

  else if (text == "/cek_cahaya") {
    int ldr = analogRead(LDR_PIN);
    bot.sendMessage(chat_id, "☀️ Cahaya: " + getLightStatus(ldr), "");
  }

  else if (text == "/status") {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int ldr = analogRead(LDR_PIN);

    String msg = "🏠 SMART ROOM STATUS\n\n";
    msg += "🌡️ Suhu: " + String(t) + "°C\n";
    msg += "💧 Kelembapan: " + String(h) + "%\n";
    msg += "☀️ Cahaya: " + getLightStatus(ldr) + "\n";
    msg += "💡 LED: " + String(ledState ? "ON" : "OFF");

    bot.sendMessage(chat_id, msg, "");
  }
}

void loop() {

  int msgCount = bot.getUpdates(bot.last_message_received + 1);

  while (msgCount) {
    for (int i = 0; i < msgCount; i++) {
      String text = bot.messages[i].text;
      String chat_id = bot.messages[i].chat_id;

      handleMessage(text, chat_id);
    }
    msgCount = bot.getUpdates(bot.last_message_received + 1);
  }

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (t > 100 && h > 50 && !alarmSent) {

    String alert = "🚨 PERINGATAN!\n\n";
    alert += "🌡️ Suhu tinggi: " + String(t) + "°C\n";
    alert += "💧 Kelembapan tinggi: " + String(h) + "%\n";
    alert += "⚠️ Kondisi tidak aman!";

    bot.sendMessage(CHAT_ID, alert, "");
    alarmSent = true;
  }

  if (t <= 100 || h <= 50) {
    alarmSent = false;
  }
}