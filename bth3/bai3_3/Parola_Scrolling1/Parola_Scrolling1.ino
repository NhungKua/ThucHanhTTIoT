#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// set to 1 if we are implementing the user interface pot, switch, etc
#define USE_UI_CONTROL 0

#if USE_UI_CONTROL
#include <MD_UISwitch.h>
#endif

// Turn on debug statements to the serial output
#define DEBUG 0

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif

// ====================== Cấu hình phần cứng ======================
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10

// ====================== Khai báo đối tượng ======================
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// ====================== Tham số cuộn chữ ======================
#if USE_UI_CONTROL
const uint8_t SPEED_IN = A5;
const uint8_t DIRECTION_SET = 8;  // change the effect
const uint8_t INVERT_SET = 9;     // change the invert
const uint8_t SPEED_DEADBAND = 5;
#endif // USE_UI_CONTROL

uint8_t scrollSpeed = 25;    
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000; 

// ====================== Biến toàn cục ======================
#define BUF_SIZE 75
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Hello! Enter new message?" };
bool newMessageAvailable = true;

// ====================== Đọc dữ liệu Serial ======================
void readSerial(void)
{
  static char *cp = newMessage;

  while (Serial.available())
  {
    char c = (char)Serial.read();

    // Bỏ ký tự \r (carriage return)
    if (c == '\r') continue;

    // Khi gặp newline hoặc đầy bộ đệm thì kết thúc chuỗi
    if ((c == '\n') || (cp - newMessage >= BUF_SIZE - 2))
    {
      *cp = '\0';
      cp = newMessage;
      newMessageAvailable = true;
    }
    else
    {
      *cp++ = c;
    }
  }
}

// ====================== Thiết lập ban đầu ======================
void setup()
{
  Serial.begin(57600); // phải trùng với Virtual Terminal
  Serial.println("\n[Parola Scrolling Display]");
  Serial.println("Type a message in Virtual Terminal, press ENTER");

#if USE_UI_CONTROL
  uiDirection.begin();
  uiInvert.begin();
  pinMode(SPEED_IN, INPUT);
  doUI();
#endif

  P.begin();
  P.setIntensity(5);
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

// ====================== Vòng lặp chính ======================
void loop()
{
#if USE_UI_CONTROL
  doUI();
#endif

  if (P.displayAnimate())
  {
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    P.displayReset();
  }

  readSerial();
}
