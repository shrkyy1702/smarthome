#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <time.h>

#define WIFI_SSID "Miko"
#define WIFI_PASSWORD "mikudayo"

#define API_KEY "AIzaSyDgbMv5B7KeDlpvABhwVTr38Rx-7t38_cY"
#define DATABASE_URL "https://smar-9c2b1-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "grace@gmail.com"
#define USER_PASSWORD "grace123"

#define LDR_PIN 34
#define SOIL_PIN 35
#define PIR_PIN 25

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== SMART PLANT GREENHOUSE ===");
  Serial.println("inisialisasi sistem...\n");

  pinMode(LDR_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(OBJECT_PIN, INPUT);

  connectWifi();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sinkronisasi waktu dengan NTP");
  delay(2000);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Serial.println("Menghubungkan ke firebase...");
  Firebase.begin(&config, &auth);
  Firebase.reconnectWifi(true);
  unsigned long fbstart = millis();
  while (!Firebase.ready() && millis() - fbstart < 10000) {
    Serial.print(".");
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println("\n Firebase terhubung!");
    Serial.println(" Sistem siap monitoring!\n");
  } else {
    Serial.println("\n Firebase gagal terhubung, sistem tetap berjalan...\n ");
  }
}
void loop() {
  if (Wifi.status() != WL_CONNECTED) {
    Serial.println("Wifi terputus! Mencoba reconnect...");
    connectWifi();
  }

  unsigned long now = millis();
  if ( now - lastSensorUpdate > sensorInterval) {
    lastSensorUpdate = now;
    bacaDanKirimData();
  }
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI PASSWORD);
  Serial.print("Menghubungkan ke WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() - start > 20000) {
      Serial.println("\n Gagal terhubung WiFi - restart...");
      ESP.restart();
    }
  }
  Serial.println();
  Serial.println(" WiFi terhubung!");
  Serial.print("IP Address");
  Serial.println(WiFi.localIP());
}

 unsigned long getTimestamp() {
  time_t now;
  struct tm timeinfo:
  if (!getLocalTime(&timeinfo)) {
    Serial.println(" Gagal mendapat waktu NIP, gunakan millis()");
    return millis();
  }
  time(&now);
  return (unsigned long)now * 1000
 }

 void bacaDanKirimData() {
  Serial.println("\n===========================");
  Serial.println("  PEMBACAAN SENSOR GREENHOUSE");
  Serial.println("=============================");

  int rawLdr = analogRead(LDR_PIN);
  lightlevel = map(rawLdr, 4095, 0, 0, 100);
  lightlevel = constrain(lightlevel, 0, 100);

  Serial.println(" Cahaya %d %% (ADC=%d)\n , lightlevel, rawLdr");
  if (soilPercent < 40) {
    Serial.prinln("  STATUS KERING - perlu penyiraman!");
  } else {
    Serial.println(" STATUS : kelembapan cukup");
  }
  motionDetected = digitalRead(PIR_PIN) == HIGH;
  flameDetected = digitalReaf(FLAME_PIN) == HIGH:
  objectDetected = digitalRead(OBJECT_PIN) == HIGH;

  Serial.printf(" Gerakan (PIR): %s\n" , motionDetected ? "TERDETEKSI" : "Tidak Ada");
  Serial.printf(" Api : %s\n", flameDetected ? "TERDETEKSI" :"Aman");
  Serial.printf("Objek: %s\n", objectDetected ? "TERDETEKSI" : "Tidak ada");

  if (Firebase.ready()) {
    Serial.println("\n Mengirim data ke Firebase...");

    String basePath = "/greenhouse/sensors";
    bool allSucces = true;

    if (Firebase.RTDB.setInt(&fbdo, basePath + "/lightLevel", lightLevel)) {
      Serial.println(" lightLevel Terikirim");
    } else {
      Serial.println(" lightlevel gagal: %s\n", fbdo.errorReason().c_str());
      allsucces = false;
    }
    if (Firebase.RTDB.setBool(&fbdo, basePath + "/motion" motionDetetcted)) {
      Serial.println(" motion terkirim ");
    } else {
      Serial.println(" motion gagal: %s\n", fbdo.errorReason().c_str());
      allsucces = false;

      if (Firebase.RTDB.setBool(&fbdo, basePath + "/flame", flameDetected)) {
        Serial.println(" Flame terikirim");
      } else {
        Serial.println("flame gagal: %s\n" , fbdo.errorReason().c_str());
        allSucces = false;
      }

      unsgined long timestamp = getTimestamp();
      if(Firebase.RTDB.setDouble(&fbdo, basePath + "/timestamp")) {
        Serial.println("Timestamp berhasil dikirim (%1u)\n" , timestamp);
      } elsse {
        Serial.println(" Timestamp gagal: %s\n", fbdo.errorReason().c_star());
        allSucces = false;
      }

      if (allSuccess) {
        Serial.println("\n Semua data berhasil dikirim!");
      } else {
        Serial.println("\n Beberapa data gagal dikirim");
      }

    } else {
      Serial.println("\n Firebase belum siap, skip pengiriman");
    }
    Serial.println("==================================\n");

    delay(100);
    }
  }
 }
}