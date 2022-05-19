#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64

#define SD_CS 10

#define OLED_DC 6
#define OLED_CS 7
#define OLED_RESET 8
Adafruit_SSD1306 display(WIDTH, HEIGHT, &SPI, OLED_RESET, OLED_CS, OLED_DC);

#define UP_BTN_PIN A3
#define DOWN_BTN_PIN A1
#define RIGHT_BTN_PIN A2
#define LEFT_BTN_PIN A0

#define UP_BTN 0
#define DOWN_BTN 1
#define RIGHT_BTN 2
#define LEFT_BTN 3

#define setBit(v, x) ((v) |= 1 << (x))
#define resetBit(v, x) ((v) &= ~(1 << (x)))
#define getBit(v, x) ((v) & (1 << (x)))

uint8_t btnsHeld = 0;
uint8_t prevBtnsHeld = 0;
uint8_t btnsPressed = 0;

void setup() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  pinMode(UP_BTN_PIN, INPUT_PULLUP);
  pinMode(DOWN_BTN_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);
  pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC);

  gameSetup();
}
void loop() {
  resetBit(btnsPressed, UP_BTN);
  resetBit(btnsPressed, DOWN_BTN);
  resetBit(btnsPressed, LEFT_BTN);
  resetBit(btnsPressed, RIGHT_BTN);
  prevBtnsHeld = btnsHeld;

  long t0 = micros();
  for (;;) {
    long t = micros();

    bool up = !digitalRead(UP_BTN_PIN);
    bool down = !digitalRead(DOWN_BTN_PIN);
    bool left = !digitalRead(LEFT_BTN_PIN);
    bool right = !digitalRead(RIGHT_BTN_PIN);

    if (up && !getBit(prevBtnsHeld, UP_BTN)) setBit(btnsPressed, UP_BTN);
    if (down && !getBit(prevBtnsHeld, DOWN_BTN)) setBit(btnsPressed, DOWN_BTN);
    if (left && !getBit(prevBtnsHeld, LEFT_BTN)) setBit(btnsPressed, LEFT_BTN);
    if (right && !getBit(prevBtnsHeld, RIGHT_BTN)) setBit(btnsPressed, RIGHT_BTN);

    if (up) setBit(btnsHeld, UP_BTN);
    else resetBit(btnsHeld, UP_BTN);

    if (down) setBit(btnsHeld, DOWN_BTN);
    else resetBit(btnsHeld, DOWN_BTN);

    if (left) setBit(btnsHeld, LEFT_BTN);
    else resetBit(btnsHeld, LEFT_BTN);

    if (right) setBit(btnsHeld, RIGHT_BTN);
    else resetBit(btnsHeld, RIGHT_BTN);

    if (t - t0 > 90000) break;
  }

  gameLoop();
}

// ###########################################################################

#define FLOOR_Y 59

int score = 0;

class Obstacle {
    uint8_t w;
    int16_t x = 128;
public:
    constexpr static uint8_t h = 10;
    constexpr static uint8_t y = FLOOR_Y - h - 1;
    constexpr static uint8_t baseW = 5;
    constexpr static uint8_t vx = 5;

    Obstacle() {
      w = random(1, 3) * baseW;
    }

    bool update() {
      x -= vx;
      return x < -w - 10;
    }

    void draw() {
      display.fillRect(x + w, y, vx, h, SSD1306_BLACK);
      display.fillRect(x, y, vx, h, SSD1306_WHITE);
    }

    uint8_t getW() { return w; }
    uint16_t getX() { return x; }
    /*
    Obstacle& Obstacle::operator=(Obstacle&& o) {
      w = o.w;
      x = o.x;
      return *this;
    }
    */
};

class Dino {
  const uint8_t w = 25;
  const uint8_t h = 26;

  const uint8_t x = 20;
  uint8_t oldy = FLOOR_Y;
  uint8_t y = FLOOR_Y;
  int16_t vy = 0;

  uint8_t jumpPower = 5;

public:
  bool update(Obstacle ob) {
    oldy = y;
    if (jumpPower > 0 && getBit(btnsHeld, UP_BTN)) {
      vy += 150;
      jumpPower--;
    }
    y -= vy / 100;
    vy -= 75;
    if (y >= FLOOR_Y) {
      jumpPower = 5;
      if (vy < 0) vy = 0;
      y = FLOOR_Y;
    }

    bool isInX = (ob.getX() < x + w && ob.getX() > x) || (ob.getX() + ob.getW() < x + w && ob.getX() + ob.getW() > x);
    //bool isInY = () || ();

    if (y > FLOOR_Y - ob.h && isInX) return true;
    return false;
  }

  void draw() {
    display.fillRect(x, oldy - h, w, h, SSD1306_BLACK);
    display.fillRect(x, y - h, w, h, SSD1306_WHITE);
  }
};

Dino d;
Obstacle o;

void gameSetup() {
  display.clearDisplay();
  display.drawLine(0, FLOOR_Y, WIDTH - 1, FLOOR_Y, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(4, 0);
  display.print(score);

  display.display();
}

void gameLoop() {
  if (o.update()) {
    display.fillRect(0, 0, WIDTH, 9, SSD1306_BLACK);
    display.setCursor(4, 0);
    display.print(++score);
    o = Obstacle();
  } else {
    o.draw();
  }

  bool dead = d.update(o);
  d.draw();
  display.display();

  if (dead) {
    display.clearDisplay();
    display.setCursor(4, 0);
    display.print(score);
    display.setTextSize(2);
    display.setCursor(11, 27);
    display.print(F("GAME OVER"));
    display.display();
    for (;;);
  }
}
