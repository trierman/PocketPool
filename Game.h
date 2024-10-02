#include <string>
#pragma once
#include <M5StickCPlus.h>

class Vec2 {
public:
  Vec2() {}
  Vec2(float xf, float yf)
    : x(xf), y(yf) {}
  float x{ 0.0f };
  float y{ 0.0f };
  void reflect(const Vec2 N);
  float length() {
    return sqrt(x * x + y * y);
  }
  void normalize() {
    const float len = max(0.0001f, length());
    x /= len;
    y /= len;
  }
};

inline float dot(const Vec2 v0, const Vec2 v1) {
  return (v0.x * v1.x) + (v0.y * v1.y);
}

inline Vec2 add(const Vec2 v0, const Vec2 v1) {
  return Vec2(v0.x + v1.x, v0.y + v1.y);
}

inline Vec2 sub(const Vec2 v0, const Vec2 v1) {
  return Vec2(v0.x - v1.x, v0.y - v1.y);
}

// v - 2.0 * dot(n, v) * n.
inline Vec2 reflect(const Vec2 v, const Vec2 n) {
  const float d = dot(v, n);
  const float x = v.x - 2.0f * d * n.x;
  const float y = v.y - 2.0f * d * n.y;
  return Vec2(x, y);
}

inline float distance(const Vec2 v0, const Vec2 v1) {
  const float dx = v1.x - v0.x;
  const float dy = v1.y - v0.y;
  return sqrt(dx * dx + dy * dy);
}

struct Ball {
  Vec2 pos_;
  Vec2 velocity_;
  float radius_{ 12.0f };
  int16_t color_;
  uint32_t color32_;
  bool isActive_{ true };
};

#define NUMBER_OF_BALLS 7
const int GAME_RUNNING = 0;
const int GAME_OVER = 1;
const int GAME_WON = 2;


class Game {
public:
  Game();

  bool showTitle();
  void updateGameState(const float deltaTime);
  bool gameOver() {
    return gameState_ == GAME_OVER;
  }
  bool gameWon() {
    return gameState_ == GAME_WON;
  }

  std::string getGameScore() {
    return std::to_string(static_cast<int>(gameScore_));
  }

  Vec2 getHighLightPosition(Ball &b);

  void setTableSize(const float w, const float h) {
    width_ = w;
    height_ = h;
    mainLight_.x = width_ * 0.5f;
    mainLight_.y = height_ * 0.4f;
  }
  void restartLevel();
  void update(const float theta, const float phi, const float deltaTime);

  Ball balls_[NUMBER_OF_BALLS];
  Vec2 holes_[4];

  Vec2 mainLight_;

  int gameState_{ 0 };  // 0 == is running, 1 == game over, 2 == game won;

  const float maxGameScore_{ 1000.0f };
  float gameScore_{ maxGameScore_ };
  float gameTimer_{ 0.0f };
  float width_{ 0.0f };
  float height_{ 0.0f };
  float damping_{ 0.99f };
  float holesRadius_{ 25.0f };
};