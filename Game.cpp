#include "Game.h"

void Vec2::reflect(const Vec2 N) {
  const float d = x * N.x + y * N.y;
  x = x - (2.0f * d * N.x);
  y = y - (2.0f * d * N.y);
}

Vec2 rotateVec(Vec2 v, float theta) {
  return Vec2(v.x * cos(theta) - v.y * sin(theta), v.x * sin(theta) + v.y * cos(theta));
}

/////////////////////////////////////////////////////////////////////
void resolveCollision(Ball &b, Ball &bOther) {
  Vec2 res = Vec2(b.velocity_.x - bOther.velocity_.x, b.velocity_.y - bOther.velocity_.y);
  if (res.x * (bOther.pos_.x - b.pos_.x) + res.y * (bOther.pos_.y - b.pos_.y) >= 0.0f) {

    float m1 = 1.0f;
    float m2 = 1.0f;
    //Calculate the angle of rotation
    float theta = -atan2(bOther.pos_.y - b.pos_.y, bOther.pos_.x - b.pos_.x);
    Vec2 u1 = rotateVec(b.velocity_, theta);
    Vec2 u2 = rotateVec(bOther.velocity_, theta);
    //Apply 1-Dimensional Elastic Collision Formulas
    Vec2 v1 = rotateVec(Vec2(u1.x * (m1 - m2) / (m1 + m2) + u2.x * 2.0f * m2 / (m1 + m2), u1.y), -theta);
    Vec2 v2 = rotateVec(Vec2(u2.x * (m2 - m1) / (m1 + m2) + u1.x * 2.0f * m1 / (m1 + m2), u2.y), -theta);

    b.velocity_.x = v1.x;
    b.velocity_.y = v1.y;

    bOther.velocity_.x = v2.x;
    bOther.velocity_.y = v2.y;
  }
}

/////////////////////////////////////////////////////////////////////////////////

float getRand(const int maxValue) {
  return static_cast<float>(random(maxValue));
}

Game::Game() {

  balls_[0].color_ = TFT_LIGHTGREY;
  balls_[1].color_ = TFT_RED;
  balls_[2].color_ = TFT_BLUE;
  balls_[3].color_ = TFT_GREEN;
  balls_[4].color_ = TFT_YELLOW;
  balls_[5].color_ = TFT_ORANGE;
  balls_[6].color_ = TFT_BLACK;

  balls_[0].color32_ = LIGHTGREY;
  balls_[1].color32_ = RED;
  balls_[2].color32_ = BLUE;
  balls_[3].color32_ = GREEN;
  balls_[4].color32_ = YELLOW;
  balls_[5].color32_ = ORANGE;
  balls_[6].color32_ = BLACK;

  restartLevel();
}

bool Game::showTitle() {
  return gameTimer_ < 8.0f;
}

void Game::updateGameState(const float deltaTime) {
  if (gameState_ == GAME_WON) return;
  if (gameState_ == GAME_OVER) return;

  if (!gameWon()) {
    gameScore_ -= deltaTime * 4.0f;
    gameScore_ = max(0.0f, gameScore_);
  }

  // First test for game over, meaning either the main ball is in a hole or the black is in a hole with other active
  if (!balls_[0].isActive_)  // main ball in
  {
    gameState_ = GAME_OVER;
    return;
  }

  bool othersActive = false;
  if (!balls_[6].isActive_)  // Black ball in, check if other are active
  {
    for (int i = 1; i < 5; i++) {
      if (balls_[i].isActive_) {
        othersActive = true;
        break;
      }
    }

    if (othersActive) {  // one or more are active so the black ball is prematurely in
      gameState_ = GAME_OVER;
      return;
    }
  }

  for (int i = 1; i < NUMBER_OF_BALLS; i++) {
    if (balls_[i].isActive_) {
      return;
    }
  }

  gameState_ = GAME_WON;
}

Vec2 Game::getHighLightPosition(Ball &b) {
  Vec2 pos;
  Vec2 toLight = sub(mainLight_, b.pos_);
  const float l = min(b.radius_ * 0.6f, toLight.length());
  toLight.normalize();
  pos.x = b.pos_.x + toLight.x * l;
  pos.y = b.pos_.y + toLight.y * l;
  return pos;
}

void Game::restartLevel() {

  const float border = 35.0f;
  const float pStart = 0.4f;
  const float off = balls_[0].radius_ * 1.5f;
  const int rMax = 25;
  gameTimer_ = 0.0f;
  gameScore_ = maxGameScore_;
  gameState_ = GAME_RUNNING;

  // Balls
  balls_[0].pos_ = Vec2(width_ * 0.8f, height_ * 0.5f);

  // Create simple pyramid
  balls_[1].pos_ = Vec2(width_ * pStart, height_ * 0.5f);

  balls_[2].pos_ = Vec2(width_ * pStart - off * 1.5f, height_ * 0.5f - off);
  balls_[3].pos_ = Vec2(width_ * pStart - off * 1.5f, height_ * 0.5f + off);

  balls_[4].pos_ = Vec2(width_ * pStart - off * 3.0f, height_ * 0.5f - off * 1.5f);
  balls_[6].pos_ = Vec2(width_ * pStart - off * 3.0f, height_ * 0.5f);
  balls_[5].pos_ = Vec2(width_ * pStart - off * 3.0f, height_ * 0.5f + off * 1.5f);

  for (int i = 0; i < NUMBER_OF_BALLS; i++) {
    balls_[i].velocity_.x = 0.0f;
    balls_[i].velocity_.y = 0.0f;
    balls_[i].isActive_ = true;
  }

  holes_[0] = Vec2(0.0f, 0.0f);
  holes_[1] = Vec2(width_, 0.0f);
  holes_[2] = Vec2(width_, height_);
  holes_[3] = Vec2(0.0f, height_);
}

void Game::update(const float theta, const float phi, const float deltaTime) {

  updateGameState(deltaTime);

  gameTimer_ += deltaTime;
  // first update the movement
  for (int i = 0; i < NUMBER_OF_BALLS; i++) {
    Ball &ball = balls_[i];

    if (!ball.isActive_) continue;

    if (i == 0) {
      ball.velocity_.x -= phi * deltaTime;
      ball.velocity_.y += theta * deltaTime;
    }

    ball.velocity_.x *= damping_;
    ball.velocity_.y *= damping_;

    ball.pos_.x += ball.velocity_.x * deltaTime;
    ball.pos_.y += ball.velocity_.y * deltaTime;
  }

  // update ball ball collisions ugly N^2
  for (int i = 0; i < NUMBER_OF_BALLS; i++) {
    for (int j = 0; j < NUMBER_OF_BALLS; j++) {

      if (i == j) continue;

      Ball &bi = balls_[i];
      Ball &bj = balls_[j];

      if (!bi.isActive_ || !bj.isActive_) continue;

      const float dist = distance(bi.pos_, bj.pos_);

      if (dist < bi.radius_ + bj.radius_) {
        resolveCollision(bi, bj);
      }
    }
  }

  // update bounds collision
  for (int i = 0; i < NUMBER_OF_BALLS; i++) {

    Ball &ball = balls_[i];
    if (!ball.isActive_) continue;
    if (ball.pos_.x < 1.0f + ball.radius_)  // collision with left side
    {
      ball.velocity_.reflect(Vec2(1.0f, 0.0f));
      ball.pos_.x = 1.01f + ball.radius_;
    }

    if (ball.pos_.x >= width_ - 1.0f - ball.radius_) {
      ball.velocity_.reflect(Vec2(-1.0f, 0.0f));
      ball.pos_.x = width_ - 1.01f - ball.radius_;
    }

    if (ball.pos_.y < 1.0f + ball.radius_)  // collision with left side
    {
      ball.velocity_.reflect(Vec2(0.0f, 1.0f));
      ball.pos_.y = 1.01f + ball.radius_;
    }

    if (ball.pos_.y >= height_ - 1.0f - ball.radius_) {
      ball.velocity_.reflect(Vec2(0.0f, -1.0f));
      ball.pos_.y > height_ - 1.01f - ball.radius_;
    }
  }

  // update for ball in holes
  for (int i = 0; i < NUMBER_OF_BALLS; i++) {
    Ball &ball = balls_[i];
    if (!ball.isActive_) continue;

    for (int j = 0; j < 4; j++) {
      float dist = distance(ball.pos_, holes_[j]);
      if (dist < holesRadius_ * 1.1f) {
        ball.isActive_ = false;
      }
    }
  }
}