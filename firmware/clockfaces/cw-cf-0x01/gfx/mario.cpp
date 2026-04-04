#include "mario.h"
#include <CWPreferences.h>

static const uint16_t MARIO_MAX_PIXEL_COUNT = 17 * 16;
static const uint16_t LUIGI_RED = 0x0487;
static const uint16_t LUIGI_SHIRT = 0xa405;

Mario::Mario(int x, int y) {
  _x = x;
  _y = y;
}

uint8_t Mario::getCharacterSelection() {
  uint8_t characterSelection = ClockwiseParams::getInstance()->characterSelection;
  return characterSelection == CHARACTER_LUIGI ? CHARACTER_LUIGI : CHARACTER_MARIO;
}

void Mario::drawSprite(const unsigned short* sprite, uint8_t width, uint8_t height) {
  uint8_t characterSelection = getCharacterSelection();
  uint16_t pixelCount = width * height;

  if (characterSelection == CHARACTER_MARIO) {
    drawBitmapWithActiveSky(Locator::getDisplay(), _x, _y, sprite, width, height);
  } else {
    static uint16_t spriteBuffer[MARIO_MAX_PIXEL_COUNT];

    memcpy(spriteBuffer, sprite, pixelCount * sizeof(uint16_t));

    for (uint16_t i = 0; i < pixelCount; i++) {
      if (spriteBuffer[i] == SKY_PIXEL) {
        spriteBuffer[i] = getActiveSkyColor();
      } else if (spriteBuffer[i] == M_RED) {
        spriteBuffer[i] = LUIGI_RED;
      } else if (spriteBuffer[i] == M_SHIRT) {
        spriteBuffer[i] = LUIGI_SHIRT;
      }
    }

    Locator::getDisplay()->drawRGBBitmap(_x, _y, spriteBuffer, width, height);
  }

  _lastCharacterSelection = characterSelection;
}

void Mario::move(Direction dir, int times) {
  
  if (dir == RIGHT) {
    _x += MARIO_PACE;
  } else if (dir == LEFT) {
    _x -= MARIO_PACE;
  }  

}

void Mario::jump() {
  if (_state != JUMPING && (millis() - lastMillis > 500) ) {
    // Serial.println("Jump - Start");

    _lastState = _state;
    _state = JUMPING;

    Locator::getDisplay()->fillRect(_x, _y, _width, _height, getActiveSkyColor());
    
    _width = MARIO_JUMP_SIZE[0];
    _height = MARIO_JUMP_SIZE[1];
    _sprite = MARIO_JUMP;

    direction = UP;

    _lastY = _y;
    _lastX = _x;
  }  
}

void Mario::idle() {
  if (_state != IDLE) {
    // Serial.println("Idle - Start");

    _lastState = _state;
    _state = IDLE;

    Locator::getDisplay()->fillRect(_x, _y, _width, _height, getActiveSkyColor());

    _width = MARIO_IDLE_SIZE[0];
    _height = MARIO_IDLE_SIZE[1];
    _sprite = MARIO_IDLE;
  }
}


void Mario::init() {
  Locator::getEventBus()->subscribe(this);
  _width = MARIO_IDLE_SIZE[0];
  _height = MARIO_IDLE_SIZE[1];
  _sprite = MARIO_IDLE;
  drawSprite(MARIO_IDLE, MARIO_IDLE_SIZE[0], MARIO_IDLE_SIZE[1]);
}

void Mario::redraw() {
  drawSprite(_sprite, _width, _height);
}

void Mario::update() {
  if (_state == IDLE && _state != _lastState) {
    drawSprite(MARIO_IDLE, MARIO_IDLE_SIZE[0], MARIO_IDLE_SIZE[1]);
  } else if (_state == IDLE && getCharacterSelection() != _lastCharacterSelection) {
    drawSprite(MARIO_IDLE, MARIO_IDLE_SIZE[0], MARIO_IDLE_SIZE[1]);
  } else if (_state == JUMPING) {
    
    if (millis() - lastMillis >= 50) {

      //Serial.println(_y);
      
      Locator::getDisplay()->fillRect(_x, _y, _width, _height, getActiveSkyColor());
      
      _y = _y + (MARIO_PACE * (direction == UP ? -1 : 1));

      drawSprite(_sprite, _width, _height);
      
      Locator::getEventBus()->broadcast(MOVE, this);

     
      if (floor(_lastY - _y) >= MARIO_JUMP_HEIGHT) {
        direction = DOWN;
      }

      if (_y+_height >= 56) {
        idle();
      }

      lastMillis = millis();
    }

  }
}

void Mario::execute(EventType event, Sprite* caller) {
  if (event == EventType::COLLISION) {
    //Serial.println("MARIO - Collision detected");
    direction = DOWN;
  }
}

const char* Mario::name() {
  return "MARIO";
}
