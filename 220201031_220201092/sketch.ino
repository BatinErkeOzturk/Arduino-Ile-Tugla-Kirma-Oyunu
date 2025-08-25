#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // ekran genisligi //
#define SCREEN_HEIGHT 64 // ekran yuksekligi //

#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PADDLE_WIDTH 40 // pedal genisligi //
#define PADDLE_HEIGHT 2 // pedal yuksekligi //
#define PADDLE_SPEED 6 // pedal hizi //

#define BALL_SIZE 4 // top boyutu //

#define BRICK_ROWS 4 // tugla satir sayisi //
#define BRICK_COLS 7 // tugla sutun sayisi //
#define BRICK_WIDTH 16 // tugla genisligi //
#define BRICK_HEIGHT 5 // tugla yuksekligi //
#define BRICK_GAP 2 // tuglalar arası bosluk //
#define BRICK_START_X 5 // tuglaların baslamak icin x koordinati //
#define BRICK_START_Y 5 // tuglalarin baslamak icin y koordinati //

#define PADDLE_BOUND_1 (paddleX + (PADDLE_WIDTH / 5)) // pedal x' in 0-8 arasi //
#define PADDLE_BOUND_2 (paddleX + (PADDLE_WIDTH * 2 / 5)) // pedal x' in 8-16 arasi //
#define PADDLE_BOUND_3 (paddleX + (PADDLE_WIDTH * 3 / 5)) // pedal x' in 16-24 arasi //
#define PADDLE_BOUND_4 (paddleX + (PADDLE_WIDTH * 4 / 5)) // pedal x' in 24-32 arasi //

#define BUTTON_UP 22 // yukarı butonunun baglandigi pin //
#define BUTTON_DOWN 24 // asagi butonunun baglandigi pin //
#define BUTTON_SELECT 26 // secim butonunun baglandigi pin //

#define OBJE_SIZE 3 // can icin dusen objenin boyutu //

// menu icin olusturulan enum //
enum MenuState { START, END };
MenuState menuState = START;

float BALL_SPEED = 1.6; // topun baslangictaki hizi //

const int segmentPins1[7] = {30, 31, 32, 33, 34, 35, 36}; // a, b, c, d, e, f, g //
const int commonPin1 = 37; // ortak pin //
const int segmentPins2[7] = {38, 39, 40, 41, 42, 43, 44}; // a, b, c, d, e, f, g //
const int commonPin2 = 45; // ortak pin //

// pedalin baslama koordinatlari //
int paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
int paddleY = SCREEN_HEIGHT - PADDLE_HEIGHT - 2;
int paddleDirection;

// topun baslangictaki koordinatlari ve hareket degiskenleri //
float ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
float ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
float ballDX = 0.0f;
float ballDY = 1.0f;

// kirilan tuglanin konumunu almak icin degiskenler //
int brickBreakingCoordinateX = 0;
int brickBreakingCoordinateY = 0;

// potansiyometrenin pininin nereye bagli oldugu //
int potPin = A0;
// isik sensorunun pininin nereye bagli oldugu //
int lightSensorPin = A1;

// bolumler icin tugla sayaci ve toplam kirilan tugla sayisi //
int brickCount = 0;
int brokenBrickCount = 0;

// ledlerin bagli oldugu pinler //
int ledPin1 = 2;
int ledPin2 = 3;
int ledPin3 = 4;

// baslangictaki can sayisi ve hangi bolumden baslanacagi //
int lives = 3;
int level = 1;

// bolumlerde bulunan tugla sayilari //
int level1Count = 26;
int level2Count = 16;
int level3Count = 14;
int level4Count = 13;
int level5Count = 18;

// ledlerin yanıp-sonmesi icin kontrol degiskenleri //
boolean ledPin1Control = true;
boolean ledPin2Control = true;
boolean ledPin3Control = true;

// oyun basladı mı diye kontrol degiskeni //
boolean gameStarted = false;

// tugla yapisi //
struct Brick {
  float x;
  float y;
  bool exists;
};

// dusen objenin yapısı //
struct Object {
  int x;
  int y;
  boolean active;
};

// duserken yakalayip can kazanmamiz icin olusturdugum obje //
Object obj;

// birinci bolumun tugla matrisi //
Brick level1[BRICK_ROWS][BRICK_COLS] = {
  {{2, 5, true}, {20, 5, true}, {38, 5, true}, {56, 5, true}, {74, 5, true}, {92, 5, true}, {110, 5, true}},
  {{2, 12, true}, {20, 12, true}, {38, 12, true}, {56, 12, true}, {74, 12, true}, {92, 12, true}, {110, 12, true}},
  {{2, 19, true}, {20, 19, true}, {38, 19, true}, {56, 19, false}, {74, 19, true}, {92, 19, true}, {110, 19, true}},
  {{2, 26, true}, {20, 26, true}, {38, 26, true}, {56, 26, false}, {74, 26, true}, {92, 26, true}, {110, 26, true}}
};

// birinci bolumun resetlenmesi icin tugla kontrolu //
// 1 : tugla var,  0 : tugla yok //
int level1Control[BRICK_ROWS][BRICK_COLS] {
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 0, 1, 1, 1,
  1, 1, 1, 0, 1, 1, 1
};

// ikinci bolumun tugla matrisi //
Brick level2[BRICK_ROWS][BRICK_COLS] = {
  {{2, 5, true}, {20, 5, false}, {38, 5, true}, {56, 5, false}, {74, 5, true}, {92, 5, false}, {110, 5, true}},
  {{2, 12, true}, {20, 12, false}, {38, 12, true}, {56, 12, false}, {74, 12, true}, {92, 12, false}, {110, 12, true}},
  {{2, 19, true}, {20, 19, false}, {38, 19, true}, {56, 19, false}, {74, 19, true}, {92, 19, false}, {110, 19, true}},
  {{2, 26, true}, {20, 26, false}, {38, 26, true}, {56, 26, false}, {74, 26, true}, {92, 26, false}, {110, 26, true}}
};

// ikinci bolumun resetlenmesi icin tugla kontrolu //
// 1 : tugla var,  0 : tugla yok //
int level2Control[BRICK_ROWS][BRICK_COLS] {
  1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1
};

// ucuncu bolumun tugla matrisi //
Brick level3[BRICK_ROWS][BRICK_COLS] = {
  {{2, 5, false}, {20, 5, true}, {38, 5, false}, {56, 5, true}, {74, 5, false}, {92, 5, true}, {110, 5, false}},
  {{2, 12, false}, {20, 12, true}, {38, 12, false}, {56, 12, true}, {74, 12, false}, {92, 12, true}, {110, 12, false}},
  {{2, 19, true}, {20, 19, false}, {38, 19, true}, {56, 19, false}, {74, 19, true}, {92, 19, false}, {110, 19, true}},
  {{2, 26, true}, {20, 26, false}, {38, 26, true}, {56, 26, false}, {74, 26, true}, {92, 26, false}, {110, 26, true}}
};

// ucuncu bolumun resetlenmesi icin tugla kontrolu //
// 1 : tugla var,  0 : tugla yok //
int level3Control[BRICK_ROWS][BRICK_COLS] {
  0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1
};

// dorduncu bolumun tugla matrisi //
Brick level4[BRICK_ROWS][BRICK_COLS] = {
  {{2, 5, true}, {20, 5, false}, {38, 5, true}, {56, 5, false}, {74, 5, true}, {92, 5, false}, {110, 5, true}},
  {{2, 12, false}, {20, 12, true}, {38, 12, false}, {56, 12, true}, {74, 12, false}, {92, 12, true}, {110, 12, false}},
  {{2, 19, false}, {20, 19, true}, {38, 19, false}, {56, 19, false}, {74, 19, false}, {92, 19, true}, {110, 19, false}},
  {{2, 26, true}, {20, 26, false}, {38, 26, true}, {56, 26, false}, {74, 26, true}, {92, 26, false}, {110, 26, true}}
};

// dorduncu bolumun resetlenmesi icin tugla kontrolu //
// 1 : tugla var,  0 : tugla yok //
int level4Control[BRICK_ROWS][BRICK_COLS] {
  1, 0, 1, 0, 1, 0, 1,
  0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1
};

// besinci bolumun tugla matrisi //
Brick level5[BRICK_ROWS][BRICK_COLS] = {
  {{2, 5, true}, {20, 5, true}, {38, 5, true}, {56, 5, true}, {74, 5, true}, {92, 5, true}, {110, 5, true}},
  {{2, 12, true}, {20, 12, false}, {38, 12, false}, {56, 12, false}, {74, 12, false}, {92, 12, false}, {110, 12, true}},
  {{2, 19, true}, {20, 19, false}, {38, 19, false}, {56, 19, false}, {74, 19, false}, {92, 19, false}, {110, 19, true}},
  {{2, 26, true}, {20, 26, true}, {38, 26, true}, {56, 26, true}, {74, 26, true}, {92, 26, true}, {110, 26, true}}
};

// besinci bolumun resetlenmesi icin tugla kontrolu //
// 1 : tugla var,  0 : tugla yok //
int level5Control[BRICK_ROWS][BRICK_COLS] {
  1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1
};

// bu metotlarin aciklamalari metotlarin ustunde yapilmistir //
void resetBricks(Brick bricks[BRICK_ROWS][BRICK_COLS], int bricks2[BRICK_ROWS][BRICK_COLS]);
void drawLevels(Brick bricks[BRICK_ROWS][BRICK_COLS]);
void controlBricks(Brick bricks[BRICK_ROWS][BRICK_COLS]);

// her program baslangicinda olmasini istedigimiz seyler //
void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();

  obj.x = 0;
  obj.y = 0;
  obj.active = false;

  randomSeed(analogRead(0));

  drawMenu();
}

// secimlerin yapılacagı menunun cizimi //
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("GAME MENU");

  if (menuState == START) {
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("> PLAY");

    display.setTextColor(WHITE);
    display.setCursor(0, 40);
    display.println("  QUIT");
  } else if (menuState == END) {
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println("  PLAY");

    display.setTextColor(WHITE);
    display.setCursor(0, 40);
    display.println("> QUIT");
  }

  display.display();
}

// her yukari-asagi buton hareketinde menunun guncellenmesi //
void updateMenu() {
  if (digitalRead(BUTTON_UP) == LOW) {
    menuState = START;
    delay(200);
    drawMenu();
  }

  if (digitalRead(BUTTON_DOWN) == LOW) {
    menuState = END;
    delay(200);
    drawMenu();
  }
}

// program calısırken surekli olarak calisan dongu //
void loop() {
  updateMenu();
  if (digitalRead(BUTTON_SELECT) == LOW) {
    if (menuState == START) {
      gameStarted = true;
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, HIGH);
      digitalWrite(ledPin3, HIGH);

      for (int i = 0; i < 7; i++) {
        pinMode(segmentPins1[i], OUTPUT);
        pinMode(segmentPins2[i], OUTPUT);
      }
      pinMode(commonPin1, OUTPUT);
      pinMode(commonPin2, OUTPUT);

      startGame();
      drawMenu();

      for (int i = 0; i < 7; i++) {
        pinMode(segmentPins1[i], INPUT);
        pinMode(segmentPins2[i], INPUT);
      }
      pinMode(commonPin1, INPUT);
      pinMode(commonPin2, INPUT);

      ledPin1Control = true;
      ledPin2Control = true;
      ledPin3Control = true;
      brokenBrickCount = 0;
      brickCount = 0;
      BALL_SPEED = 1.6;
    } else if (menuState == END) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("\n\nTHANK YOU FOR YOUR");
      display.println("INTEREST IN OUR GAME!");
      digitalWrite(ledPin1, LOW);
      digitalWrite(ledPin2, LOW);
      digitalWrite(ledPin3, LOW);
      display.display();
      delay(2000);
      while (true);
    }
  }
}

// oyunun baslamasi ve gerekli islemler //
void startGame() {

  while (gameStarted) {
    display.clearDisplay();

    int potValue = analogRead(potPin);
    paddleDirection = map(potValue, 0, 1023, -PADDLE_SPEED, PADDLE_SPEED);

    paddleX += paddleDirection;
    paddleX = constrain(paddleX, 0, SCREEN_WIDTH - PADDLE_WIDTH);

    int lightValue = analogRead(lightSensorPin);

    if (level == 1) {
      drawLevels(level1);
    }
    else if (level == 2) {
      drawLevels(level2);
    }
    else if (level == 3) {
      drawLevels(level3);
    }
    else if (level == 4) {
      drawLevels(level4);
    }
    else if (level == 5) {
      drawLevels(level5);
    }

    ballX += ballDX * BALL_SPEED;
    ballY += ballDY * BALL_SPEED;

    dropObject();
    catchObject();

    if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE) {
      ballDX = -ballDX;
    }

    if (ballY <= 0) {
      ballDY = -ballDY;
    }

    if (ballY >= SCREEN_HEIGHT - BALL_SIZE) {
      if (ballX + BALL_SIZE / 2 >= paddleX && ballX + BALL_SIZE / 2 <= paddleX + PADDLE_WIDTH) {
        ballDY = -ballDY;

        if (ballX + BALL_SIZE / 2 < PADDLE_BOUND_1) {
          ballDX = -BALL_SPEED;
        } else if (ballX + BALL_SIZE / 2 < PADDLE_BOUND_2) {
          ballDX = -BALL_SPEED / 2.0;
        } else if (ballX + BALL_SIZE / 2 < PADDLE_BOUND_3) {
          ballDX = 0;
        } else if (ballX + BALL_SIZE / 2 < PADDLE_BOUND_4) {
          ballDX = BALL_SPEED / 2.0;
        } else {
          ballDX = BALL_SPEED;
        }
      } else {
        ballX = paddleX + 16;
        ballY = 40;
        ballDX = 0;
        ballDY = 1;
        if (ledPin1Control) {
          digitalWrite(ledPin1, LOW);
          ledPin1Control = false;
          lives--;
        }
        else if (ledPin2Control) {
          digitalWrite(ledPin2, LOW);
          ledPin2Control = false;
          lives--;
        }
        else if (ledPin3Control) {
          digitalWrite(ledPin3, LOW);
          ledPin3Control = false;
          lives--;
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0, 0);
          display.println("\n      GAME OVER! ");
          display.println("\n\n      SCORE : " + (String)brokenBrickCount);
          level = 1;
          resetBricks(level1, level1Control);
          resetBricks(level2, level2Control);
          resetBricks(level3, level3Control);
          resetBricks(level4, level4Control);
          resetBricks(level5, level5Control);
          display.display();
          delay(3000);
          return;
        }
      }
    }

    if (level == 1) {
      controlBricks(level1);
    }
    else if (level == 2) {
      controlBricks(level2);
    }
    else if (level == 3) {
      controlBricks(level3);
    }
    else if (level == 4) {
      controlBricks(level4);
    }
    else if (level == 5) {
      controlBricks(level5);
    }

    showNumber(brokenBrickCount / 10, brokenBrickCount % 10);

    if (lightValue > 500) {
      display.fillRect(paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT, BLACK);
      display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, BLACK);
    }
    else {
      display.fillRect(paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);
      display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, WHITE);
    }


    if (brickCount == level1Count && level == 1) {
      completeLevel();
    }
    else if (brickCount == level2Count && level == 2) {
      completeLevel();
    }
    else if (brickCount == level3Count && level == 3) {
      completeLevel();
    }
    else if (brickCount == level4Count && level == 4) {
      completeLevel();
    }
    else if (brickCount == level5Count && level == 5) {
      completeLevel();
    }
    else if (level == 6) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("\n    OH MY LORD !!! ");
      display.println("\n    YOU ARE THE");
      display.println("\n  WORLD CHAMPION !!");
      display.display();
      resetBricks(level1, level1Control);
      resetBricks(level2, level2Control);
      resetBricks(level3, level3Control);
      resetBricks(level4, level4Control);
      resetBricks(level5, level5Control);
      delay(3000);
      return;
    }

    display.display();
    delay(10);
  }
}

// verilen bolum matrisine göre tuglalari cizdiren metot //
void drawLevels(Brick bricks[BRICK_ROWS][BRICK_COLS]) {

  int lightValue = analogRead(lightSensorPin);
  if (lightValue > 500) {
    display.clearDisplay();
    display.fillScreen(WHITE);
    for (int i = 0; i < BRICK_ROWS; i++) {
      for (int j = 0; j < BRICK_COLS; j++) {
        if (bricks[i][j].exists) {
          display.fillRect(bricks[i][j].x, bricks[i][j].y, BRICK_WIDTH, BRICK_HEIGHT, BLACK); // Tuğlaları siyah yap
        }
      }
    }
  }
  else {
    display.clearDisplay();
    display.fillScreen(BLACK);
    for (int i = 0; i < BRICK_ROWS; i++) {
      for (int j = 0; j < BRICK_COLS; j++) {
        if (bricks[i][j].exists) {
          display.fillRect(bricks[i][j].x, bricks[i][j].y, BRICK_WIDTH, BRICK_HEIGHT, WHITE);
        }
      }
    }
  }
}

// tuglalarin kirilip kirilmamasini kontrol eden metot //
void controlBricks(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLS; j++) {
      if (bricks[i][j].exists &&
          ballX + BALL_SIZE >= bricks[i][j].x &&
          ballX <= bricks[i][j].x + BRICK_WIDTH &&
          ballY + BALL_SIZE >= bricks[i][j].y &&
          ballY <= bricks[i][j].y + BRICK_HEIGHT) {
        ballDY = -ballDY;
        bricks[i][j].exists = false;
        if (random(100) < 10) {
          brickBreakingCoordinateX  = bricks[i][j].x;
          brickBreakingCoordinateY = bricks[i][j].y;
        }
        brokenBrickCount++;
        brickCount++;
      }
    }
  }
}

// oyun kaybedildiginde veya kazanıldıgında tugla matrislerini eski //
// hallerine getirmek icin tuglaları resetleyen metot //
void resetBricks(Brick bricks[BRICK_ROWS][BRICK_COLS], int bricks2[BRICK_ROWS][BRICK_COLS]) {
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLS; j++) {
      if (bricks2[i][j])
        bricks[i][j].exists = true;
    }
  }
}

// bolum tamamlanınca sonraki bolume gecmek icin kullanilan metot //
void completeLevel() {
  ballX = paddleX + 16;
  ballY = 40;
  ballDX = 0;
  ballDY = 1;
  BALL_SPEED *= 1.2;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("\n   LEVEL COMPLETED! ");
  display.display();
  delay(5000);
  brickCount = 0;
  level += 1;
  obj.active = false;
  obj.x = 0;
  obj.y = 0;
  brickBreakingCoordinateX = 0;
  brickBreakingCoordinateY = 0;
}

// 7 sekmentli gostericilerde kirilan tugla sayisini //
// gostermek icin kullanilan metot //
void showNumber(int num1, int num2) {
  displayNumber(num1, segmentPins1, commonPin1);
  displayNumber(num2, segmentPins2, commonPin2);
}

// 0-9 a kadar verilen sayilar icin pinlerin kontrolu //
void displayNumber(int num, int segmentPins[], int commonPin) {
  switch (num) {
    case 0:
      displaySegments(segmentPins, commonPin, 0, 0, 0, 0, 0, 0, 1);
      break;
    case 1:
      displaySegments(segmentPins, commonPin, 1, 0, 0, 1, 1, 1, 1);
      break;
    case 2:
      displaySegments(segmentPins, commonPin, 0, 0, 1, 0, 0, 1, 0);
      break;
    case 3:
      displaySegments(segmentPins, commonPin, 0, 0, 0, 0, 1, 1, 0);
      break;
    case 4:
      displaySegments(segmentPins, commonPin, 1, 0, 0, 1, 1, 0, 0);
      break;
    case 5:
      displaySegments(segmentPins, commonPin, 0, 1, 0, 0, 1, 0, 0);
      break;
    case 6:
      displaySegments(segmentPins, commonPin, 0, 1, 0, 0, 0, 0, 0);
      break;
    case 7:
      displaySegments(segmentPins, commonPin, 0, 0, 0, 1, 1, 1, 1);
      break;
    case 8:
      displaySegments(segmentPins, commonPin, 0, 0, 0, 0, 0, 0, 0);
      break;
    case 9:
      displaySegments(segmentPins, commonPin, 0, 0, 0, 0, 1, 0, 0);
      break;
    default:
      break;
  }
}

// pinlerin aktiflesmesi icin metot //
void displaySegments(int segmentPins[], int commonPin, int a, int b, int c, int d, int e, int f, int g) {
  digitalWrite(commonPin, HIGH);
  digitalWrite(segmentPins[0], a);
  digitalWrite(segmentPins[1], b);
  digitalWrite(segmentPins[2], c);
  digitalWrite(segmentPins[3], d);
  digitalWrite(segmentPins[4], e);
  digitalWrite(segmentPins[5], f);
  digitalWrite(segmentPins[6], g);
}

// objenin dusmesi icin metot //
void dropObject() {
  if ((brickBreakingCoordinateX != 0) && (brickBreakingCoordinateY != 0) && (obj.active == false)) {
    obj.active = true;
    obj.x = brickBreakingCoordinateX;
    obj.y = brickBreakingCoordinateY;
  }

  if (obj.active) {
    obj.y += 1;
    drawObject();
  }

  if (obj.y >= SCREEN_HEIGHT) {
    obj.active = false;
    obj.x = 0;
    obj.y = 0;
    brickBreakingCoordinateX = 0;
    brickBreakingCoordinateY = 0;
  }
}

// objenin cizilmesi icin metot //
void drawObject() {
  int lightValue = analogRead(lightSensorPin);
  if (lightValue > 500) {
    display.fillRect(obj.x, obj.y, OBJE_SIZE, OBJE_SIZE, BLACK);
  }
  else {
    display.fillRect(obj.x, obj.y, OBJE_SIZE, OBJE_SIZE, WHITE);
  }
}

// objenin yakalanip yakalandiktan sonra inaktif olmasi icin metot //
void catchObject() {
  if (obj.active && obj.x >= paddleX && obj.x <= paddleX + PADDLE_WIDTH && obj.y >= paddleY && obj.y <= paddleY + PADDLE_HEIGHT) {
    obj.active = false;
    obj.x = 0;
    obj.y = 0;
    brickBreakingCoordinateX = 0;
    brickBreakingCoordinateY = 0;
    if (lives < 3) {
      lives++;
      if (lives == 3) {
        digitalWrite(ledPin1, HIGH);
        ledPin1Control = true;
      } else if (lives == 2) {
        digitalWrite(ledPin2, HIGH);
        ledPin2Control = true;
      } else if (lives == 1) {
        digitalWrite(ledPin3, HIGH);
        ledPin3Control = true;
      }
    }
  }
}
