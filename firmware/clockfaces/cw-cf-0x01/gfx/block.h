#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Game.h>
#include <Locator.h>
#include <EventTask.h>
#include <SkyBitmap.h>
#include "assets.h"

const uint8_t MOVE_PACE = 2;
const uint8_t MAX_MOVE_HEIGHT = 4;

class Block: public Sprite, public EventTask
{
  private:
    enum State
    {
      IDLE,
      HIT
    };   

    Direction direction; 

    String _text;

    unsigned long lastMillis = 0;
    State _state = IDLE; 
    State _lastState = IDLE; 
    uint8_t _lastY;
    uint8_t _firstY;
    
    void idle();
    void hit();
    void setTextBlock(Adafruit_GFX *display);

  public:
    Block(int x, int y);
    void setText(String text);
    void init();
    void update();
    void redraw();
    void renderTo(Adafruit_GFX *display, int16_t offsetX = 0, int16_t offsetY = 0);
    const char* name();
    void execute(EventType event, Sprite* caller);

};
