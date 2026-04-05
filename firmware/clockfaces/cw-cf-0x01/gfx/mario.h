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
const uint8_t MARIO_WALK_FRAME_COUNT = 6;
const unsigned long MARIO_WALK_FRAME_INTERVAL = 200UL;


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
    unsigned long _lastWalkFrameMillis = 0;
    State _state = IDLE; 
    State _lastState = IDLE; 
    uint8_t _walkFrame = 0;
    uint8_t _lastCharacterSelection = CHARACTER_MARIO;
    bool _dirty = false;
    
    void idle();
    void walk();
    void drawSprite(const unsigned short* sprite, uint8_t width, uint8_t height);
    uint8_t getCharacterSelection();

  public:
    Mario(int x, int y);
    void init();
    void redraw();
    void move(Direction dir, int times);
    void jump();
    void update();
    bool consumeDirty();
    bool pixelAt(int16_t screenX, int16_t screenY, uint16_t &pixel) const;
    int16_t x() const;
    int16_t y() const;
    uint8_t width() const;
    uint8_t height() const;
    const char* name();
    void execute(EventType event, Sprite* caller);
    
};
