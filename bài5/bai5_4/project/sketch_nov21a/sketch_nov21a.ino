// Arduino DHT11 -> Serial (JSON)
// Thư viện: "DHT sensor library" của Adafruit
#include "DHT.h"

#define DHTPIN 2        // DATA của DHT11 nối vào digital pin 2
#define DHTTYPE DHT11   // DHT11

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMillis = 0;
const unsigned long interval = 2000; // đọc mỗi 2s

void setup() {
  Serial.begin(9600); // Baudrate khớp với Proteus / COM ảo / Node.js
  dht.begin();
}

void loop() {
  unsigned long now = millis();
  if (now - lastMillis >= interval) {
    lastMillis = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature(); // độ C

    // kiểm tra đọc hợp lệ
    if (isnan(h) || isnan(t)) {
      // gửi thông báo lỗi (tuỳ chọn)
      Serial.println("{\"error\":\"dht_read_failed\"}");
    } else {
      // gửi JSON: {"temp":25.00,"hum":60.00,"ts":163...}
      unsigned long ts = now;
      Serial.print("{\"temp\":");
      Serial.print(t, 2);
      Serial.print(",\"hum\":");
      Serial.print(h, 2);
      Serial.print(",\"ts\":");
      Serial.print(ts);
      Serial.println("}");
    }
  }

  // các tác vụ khác nếu cần
}
