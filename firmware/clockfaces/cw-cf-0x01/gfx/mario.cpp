#include "mario.h"
#include <CWPreferences.h>

static const uint16_t MARIO_MAX_PIXEL_COUNT = 17 * 16;
static const uint16_t LUIGI_RED = 0x0487;
static const uint16_t LUIGI_SHIRT = 0xa405;
static const unsigned short* const MARIO_WALK_FRAMES[MARIO_WALK_FRAME_COUNT] = {
  mario001,
  mario002,
  mario003,
  mario004,
  mario005,
  mario006
};

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
    for (uint8_t row = 0; row < height; row++) {
      for (uint8_t col = 0; col < width; col++) {
        uint16_t pixel = sprite[(row * width) + col];
        if (pixel == SKY_PIXEL) {
          continue;
        }
        Locator::getDisplay()->drawPixel(_x + col, _y + row, pixel);
      }
    }
  } else {
    static uint16_t spriteBuffer[MARIO_MAX_PIXEL_COUNT];

    memcpy(spriteBuffer, sprite, pixelCount * sizeof(uint16_t));

    for (uint16_t i = 0; i < pixelCount; i++) {
      if (spriteBuffer[i] == SKY_PIXEL) {
        continue;
      } else if (spriteBuffer[i] == M_RED) {
        spriteBuffer[i] = LUIGI_RED;
      } else if (spriteBuffer[i] == M_SHIRT) {
        spriteBuffer[i] = LUIGI_SHIRT;
      }
    }

    for (uint8_t row = 0; row < height; row++) {
      for (uint8_t col = 0; col < width; col++) {
        uint16_t pixel = spriteBuffer[(row * width) + col];
        if (pixel == SKY_PIXEL) {
          continue;
        }
        Locator::getDisplay()->drawPixel(_x + col, _y + row, pixel);
      }
    }
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
    
    _width = MARIO_JUMP_SIZE[0];
    _height = MARIO_JUMP_SIZE[1];
    _sprite = MARIO_JUMP;
    _dirty = true;

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

    _width = MARIO_IDLE_SIZE[0];
    _height = MARIO_IDLE_SIZE[1];
    _sprite = MARIO_IDLE;
    _dirty = true;
  }
}

void Mario::walk() {
  if (_state != WALKING) {
    _lastState = _state;
    _state = WALKING;
    _walkFrame = 0;
    _lastWalkFrameMillis = millis();
    _width = MARIO_ANIM_SIZE[0];
    _height = MARIO_ANIM_SIZE[1];
    _sprite = MARIO_WALK_FRAMES[_walkFrame];
    _dirty = true;
    return;
  }

  if (millis() - _lastWalkFrameMillis < MARIO_WALK_FRAME_INTERVAL) {
    return;
  }

  _walkFrame = (_walkFrame + 1) % MARIO_WALK_FRAME_COUNT;
  _lastWalkFrameMillis = millis();
  _width = MARIO_ANIM_SIZE[0];
  _height = MARIO_ANIM_SIZE[1];
  _sprite = MARIO_WALK_FRAMES[_walkFrame];
  _dirty = true;
}

void Mario::init() {
  Locator::getEventBus()->subscribe(this);
  _width = MARIO_IDLE_SIZE[0];
  _height = MARIO_IDLE_SIZE[1];
  _sprite = MARIO_IDLE;
  _walkFrame = 0;
  _lastWalkFrameMillis = millis();
  _dirty = true;
  drawSprite(MARIO_IDLE, MARIO_IDLE_SIZE[0], MARIO_IDLE_SIZE[1]);
}

void Mario::redraw() {
  drawSprite(_sprite, _width, _height);
}

void Mario::update() {
  if (_state == JUMPING) {
    
    if (millis() - lastMillis >= 50) {
      _y = _y + (MARIO_PACE * (direction == UP ? -1 : 1));
      _dirty = true;

      Locator::getEventBus()->broadcast(MOVE, this);

     
      if (floor(_lastY - _y) >= MARIO_JUMP_HEIGHT) {
        direction = DOWN;
      }

      if (_y+_height >= 56) {
        idle();
      }

      lastMillis = millis();
    }
  } else if (ClockwiseParams::getInstance()->walkingMario) {
    walk();
  } else if (_state == IDLE && _state != _lastState) {
    _dirty = true;
  } else if (_state == IDLE && getCharacterSelection() != _lastCharacterSelection) {
    _dirty = true;
  } else if (_state == WALKING) {
    idle();
  }
}

bool Mario::consumeDirty() {
  bool wasDirty = _dirty;
  _dirty = false;
  return wasDirty;
}

bool Mario::pixelAt(int16_t screenX, int16_t screenY, uint16_t &pixel) const {
  if (screenX < _x || screenY < _y || screenX >= (_x + _width) || screenY >= (_y + _height)) {
    return false;
  }

  uint16_t spritePixel = _sprite[((screenY - _y) * _width) + (screenX - _x)];
  if (spritePixel == SKY_PIXEL) {
    return false;
  }

  uint8_t characterSelection = ClockwiseParams::getInstance()->characterSelection;
  if (characterSelection == CHARACTER_LUIGI) {
    if (spritePixel == M_RED) {
      spritePixel = LUIGI_RED;
    } else if (spritePixel == M_SHIRT) {
      spritePixel = LUIGI_SHIRT;
    }
  }

  pixel = spritePixel;
  return true;
}

int16_t Mario::x() const {
  return _x;
}

int16_t Mario::y() const {
  return _y;
}

uint8_t Mario::width() const {
  return _width;
}

uint8_t Mario::height() const {
  return _height;
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
