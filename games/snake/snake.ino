#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH 128
#define HEIGHT 64

#define SD_CS 10

#define OLED_DC     6
#define OLED_CS     7
#define OLED_RESET  8
Adafruit_SSD1306 display(WIDTH, HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

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
  
  GameSetup();
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

enum Directions { LEFT, RIGHT, DOWN, UP };

struct Pos { uint8_t x, y; };

class Snake {
  Directions direction = LEFT;
  uint8_t length = 4;
  Pos body[128] = { { 90, 20 }, { 91, 20 }, { 92, 20 }, { 93, 20 } };
  Pos tail;
  bool grown = false;

public:
  void firstDraw(Adafruit_SSD1306* display) {
    for (uint16_t i = 0; i < length; i++)
      display->drawPixel(body[i].x, body[i].y, SSD1306_WHITE);
  }

  void setDirection(Directions newDir) {
    direction = newDir;
  }

  Directions getDirection() {
    return direction;
  }

  bool update(Pos food) {
    tail = body[length - 1];
    grown = false;

    Pos head = body[0];

    if (direction == RIGHT) ++head.x;
    else if (direction == LEFT) --head.x;
    else if (direction == UP) --head.y;
    else if (direction == DOWN) ++head.y;

    if (head.x < 5) head.x = 122;
    if (head.x > 122) head.x = 5;
    if (head.y < 10) head.y = 60;
    if (head.y > 60) head.y = 10;

    for (uint16_t i = length - 1; i > 0; i--) {
      body[i] = body[i - 1];
      if (head.x == body[i].x && head.y == body[i].y)
        return true;
    }

    body[0] = head;

    bool isOnFoodX = body[0].x == food.x || body[0].x == food.x + 1;
    bool isOnFoodY = body[0].y == food.y || body[0].y == food.y + 1;
    if (isOnFoodX && isOnFoodY) {
      body[length++] = tail;
      grown = true;
    }

    return false;
  }

  bool hasEatenFood() { return grown; }

  void draw(Adafruit_SSD1306* display) {
    display->drawPixel(body[0].x, body[0].y, SSD1306_WHITE);
    if (!grown) display->drawPixel(tail.x, tail.y, SSD1306_BLACK);
  }
};

Snake snake;
Pos food;
uint16_t score = 0;

void GameSetup() {
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(4,0);             // Start at top-left corner

  display.clearDisplay();
  display.drawRect(4, 9, WIDTH - 8, HEIGHT - 11, SSD1306_WHITE);

  display.print(score);

  snake.firstDraw(&display);

  food = { random(5, 122), random(11, 59) };
  display.drawPixel(food.x + 0, food.y + 0, SSD1306_WHITE);
  display.drawPixel(food.x + 1, food.y + 0, SSD1306_WHITE);
  display.drawPixel(food.x + 0, food.y + 1, SSD1306_WHITE);
  display.drawPixel(food.x + 1, food.y + 1, SSD1306_WHITE);

  display.display();
}

void gameLoop() {
  Directions curDir = snake.getDirection();
  if (curDir != DOWN  && getBit(btnsPressed, UP_BTN)) snake.setDirection(UP);
  if (curDir != UP && getBit(btnsPressed, DOWN_BTN)) snake.setDirection(DOWN);
  if (curDir != RIGHT && getBit(btnsPressed, LEFT_BTN)) snake.setDirection(LEFT);
  if (curDir != LEFT && getBit(btnsPressed, RIGHT_BTN)) snake.setDirection(RIGHT);

  if (snake.update(food)) {
    display.clearDisplay();

    display.setCursor(4,0);
    display.print(score);

    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setCursor(11,27);             // Start at top-left corner

    display.drawRect(4, 9, WIDTH - 8, HEIGHT - 11, SSD1306_WHITE);

    display.print(F("GAME OVER"));

    display.display();
    for (;;);
  }
  if (snake.hasEatenFood()) {
    display.drawPixel(food.x + 0, food.y + 0, SSD1306_BLACK);
    display.drawPixel(food.x + 1, food.y + 0, SSD1306_BLACK);
    display.drawPixel(food.x + 0, food.y + 1, SSD1306_BLACK);
    display.drawPixel(food.x + 1, food.y + 1, SSD1306_BLACK);
    display.fillRect(0, 0,WIDTH, 9, SSD1306_BLACK);
    display.setCursor(4,0);
    display.print(++score);
    food = { random(5, 122), random(11, 59) };
    display.drawPixel(food.x + 0, food.y + 0, SSD1306_WHITE);
    display.drawPixel(food.x + 1, food.y + 0, SSD1306_WHITE);
    display.drawPixel(food.x + 0, food.y + 1, SSD1306_WHITE);
    display.drawPixel(food.x + 1, food.y + 1, SSD1306_WHITE);
    
  }
  snake.draw(&display);

  display.display();
}
