#define LED_PIN 13
#define BUTTON_PIN 2

unsigned long lastPressTime = 0;
unsigned long pressDuration = 0;
bool buttonState;
bool lastButtonState = HIGH;  // vì dùng INPUT_PULLUP
bool configMode = false;      // false = vận hành, true = cấu hình
int pressCount = 0;
unsigned long lastClickTime = 0;
long currentBaud = 9600;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(currentBaud);
  Serial.println("System running at 9600 bps");
}

void loop() {
  buttonState = digitalRead(BUTTON_PIN);

  // Nhấn giữ nút > 3s để đổi chế độ
  if (buttonState == LOW && lastButtonState == HIGH) {
    lastPressTime = millis();
  }

  if (buttonState == LOW && (millis() - lastPressTime > 3000)) {
    toggleMode();
    while (digitalRead(BUTTON_PIN) == LOW); // chờ thả nút
    delay(300);
  }

  // Nếu đang ở chế độ cấu hình
  if (configMode) {
    handleConfigMode();
  }

  lastButtonState = buttonState;
}

// -----------------------------------------------------
// HÀM CHUYỂN GIỮA HAI CHẾ ĐỘ
void toggleMode() {
  configMode = !configMode;
  if (configMode) {
    Serial.println("Enter CONFIG mode");
    blinkLED(5, 100);  // nháy nhanh 5 lần
  } else {
    Serial.println("Back to RUN mode");
    digitalWrite(LED_PIN, LOW);
  }
}

// -----------------------------------------------------
// HÀM XỬ LÝ TRONG CHẾ ĐỘ CẤU HÌNH
void handleConfigMode() {
  if (buttonState == LOW && lastButtonState == HIGH) {
    pressCount++;
    lastClickTime = millis();
  }

  // Khi chờ 1 giây mà không nhấn thêm → xác định số lần nhấn
  if (pressCount > 0 && (millis() - lastClickTime > 1000)) {
    if (pressCount == 1) {
      setBaud(9600);
      blinkLED(1, 300);
      Serial.println("Baud rate set to 9600 bps");
    } else if (pressCount == 2) {
      setBaud(115200);
      blinkLED(2, 300);
      Serial.println("Baud rate set to 115200 bps");
    }
    pressCount = 0;
  }
}

// -----------------------------------------------------
// HÀM ĐỔI BAUD RATE
void setBaud(long baud) {
  currentBaud = baud;
  Serial.end();
  Serial.begin(currentBaud);
}

// -----------------------------------------------------
// HÀM NHÁY LED
void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(LED_PIN, LOW);
    delay(delayTime);
  }
}