#include <M5StickCPlus.h>
#include "Game.h"

// Globals /////////////////////////////////////////////////////////////////////////
#define LOOP_WAIT 30
Game poolGame;
TFT_eSprite *displayBuffer;
long last_millis = 0;
float anim = 0.0f;
// Input ///////////////////////////////////////////////////////////////////////////
float accX = 0.0f;
float accY = 0.0f;
float accZ = 0.0f;
double theta = 0.0;
double last_theta = 0.0;
double phi = 0.0;
double last_phi = 0.0;
double alpha = 0.2;
////////////////////////////////////////////////////////////////////////////////////

void updateOrientation() {
  M5.Imu.getAccelData(&accX, &accY, &accZ);
  if ((accX < 1.0) && (accX > -1.0)) {
    theta = asin(-accX) * 57.295;
  }
  if (accZ != 0) {
    phi = atan(accY / accZ) * 57.295;
  }

  theta = alpha * theta + (1.0 - alpha) * last_theta;
  phi = alpha * phi + (1.0 - alpha) * last_phi;
}

void startupScreen(const char *string) {
  displayBuffer->fillScreen(TFT_BLACK);
  displayBuffer->setTextSize(1);
  displayBuffer->setTextColor(TFT_WHITE, TFT_BLACK);
  displayBuffer->setTextDatum(TC_DATUM);
  displayBuffer->drawString(string, 120, 2, 4);
  displayBuffer->pushSprite(0, 0);
  delay(500);
}

void restartGame() {
  poolGame.setTableSize(M5.Lcd.width(), M5.Lcd.height());
  poolGame.restartLevel();
  M5.Axp.ScreenBreath(99);
  delay(100);
}

void setup() {
  M5.begin(true, true, true);
  M5.Imu.Init(); // Input init
  M5.Lcd.setRotation(3);

  // Setup the sprite graphics
  displayBuffer = new TFT_eSprite(&M5.Lcd);
  displayBuffer->setSwapBytes(false);
  displayBuffer->createSprite(M5.Lcd.width(), M5.Lcd.height());
  startupScreen("Trierlab.com");
  restartGame();
}

void loop() {
  M5.update();  // Read the press state of the key.
  updateOrientation();

  if (M5.BtnA.wasPressed())  // Restart game
  {
    restartGame();
  }

  // Update and render the game
  anim += 0.25f;
  poolGame.update(theta, phi, static_cast<float>(LOOP_WAIT) * 0.01f);
  displayBuffer->fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height(), TFT_DARKGREEN);

  // Render holes
  for (int i = 0; i < 4; i++) {
    Vec2 h = poolGame.holes_[i];
    displayBuffer->fillCircle(h.x, h.y, poolGame.holesRadius_, TFT_BLACK);
  }

  // Render Pool balls
  for (int i = 0; i < NUMBER_OF_BALLS; i++) {
    Ball &ball = poolGame.balls_[i];
    if (!ball.isActive_) continue;
    displayBuffer->fillCircle(ball.pos_.x, ball.pos_.y, ball.radius_, ball.color32_);
    Vec2 hl = poolGame.getHighLightPosition(ball);
    displayBuffer->fillCircle(hl.x, hl.y, 1.75, TFT_WHITE);
  }

  // Draw bounds
  const int32_t b = 2;
  const int32_t textOffset = M5.Lcd.width() / 2;

  displayBuffer->fillRect(0, 0, M5.Lcd.width(), b, 0x01E0);                    // bottom
  displayBuffer->fillRect(0, 0, b, M5.Lcd.height(), 0x01E0);                   // left
  displayBuffer->fillRect(0, M5.Lcd.height() - b, M5.Lcd.width(), b, 0x01E0);  // top
  displayBuffer->fillRect(M5.Lcd.width() - b, 0, b, M5.Lcd.height(), 0x01E0);  // right

  // Game logic
  if (poolGame.gameOver()) {
    displayBuffer->setTextColor(TFT_RED, TFT_DARKGREEN);
    displayBuffer->drawString("Game Over!", textOffset + cos(anim) * 15.0f, (M5.Lcd.height() / 2) + sin(anim) * 10.0f - 5, 4);
  }

  if (poolGame.gameWon()) {
    displayBuffer->setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    std::string tmpStr = std::string("Win score: ") + poolGame.getGameScore();
    displayBuffer->drawString(tmpStr.c_str(), textOffset + cos(anim) * 15.0f, (M5.Lcd.height() / 2) + sin(anim) * 10.0f - 5, 4);
  }

  if (poolGame.showTitle()) {
    displayBuffer->setTextColor(TFT_WHITE, TFT_DARKGREEN);
    displayBuffer->drawString("Pocket Pool", textOffset + cos(anim) * 15.0f, (M5.Lcd.height() / 2) + sin(anim) * 10.0f - 25, 4);
    displayBuffer->drawString("Made by Trier", textOffset + cos(anim) * 15.0f, (M5.Lcd.height() / 2) + sin(anim) * 10.0f + 5, 4);
  }
  displayBuffer->pushSprite(0, 0);
  delay(LOOP_WAIT);
}
