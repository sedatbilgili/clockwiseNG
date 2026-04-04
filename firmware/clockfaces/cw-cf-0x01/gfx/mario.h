#pragma once

#include <Arduino.h>
#include <Game.h>
#include <Locator.h>
#include <EventBus.h>
#include <ImageUtils.h>
#include <SkyBitmap.h>
#include "assets.h"


const uint8_t CHARACTER_MARIO = 0;
const uint8_t CHARACTER_LUIGI = 1;
const uint8_t MARIO_PACE = 3;
const uint8_t MARIO_JUMP_HEIGHT = 14;


class Mario: public Sprite, public EventTask {
  private:

    enum State {
      IDLE,
      WALKING,
      JUMPING
    };

    Direction direction;

    int _lastX;
    int _lastY;

    const unsigned short* _sprite;
    unsigned long lastMillis = 0;
    State _state = IDLE; 
    State _lastState = IDLE; 
    uint8_t _lastCharacterSelection = CHARACTER_MARIO;
    
    void idle();
    void drawSprite(const unsigned short* sprite, uint8_t width, uint8_t height);
    uint8_t getCharacterSelection();

  public:
    Mario(int x, int y);
    void init();
    void redraw();
    void move(Direction dir, int times);
    void jump();
    void update();
    const char* name();
    void execute(EventType event, Sprite* caller);
    
};
