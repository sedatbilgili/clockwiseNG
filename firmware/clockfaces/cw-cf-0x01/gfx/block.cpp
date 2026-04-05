#include "block.h"

//String &Block::_text;

Block::Block(int x, int y)
{
  _x = x;
  _y = y;
  _firstY = y;
  _width = 19;
  _height = 19;
}

void Block::idle()
{
  if (_state != IDLE) {
    // Serial.println("Block - Idle - Start");

    _lastState = _state;
    _state = IDLE;

    _y = _firstY;
    _dirty = true;
  }
} 

void Block::hit()
{
  if (_state != HIT) {
    // Serial.println("Hit - Start");

    _lastState = _state;
    _state = HIT;

    _lastY = _y;

    direction = UP;
    _dirty = true;
  }
}

void Block::setTextBlock(Adafruit_GFX *display)
{
  display->setTextColor(0x0000);
  
  if (_text.length() == 1)
  {
    display->setCursor(_x + 6, _y + 12);
  }
  else
  {
    display->setCursor(_x + 2, _y + 12);
  }

  display->print(_text);
}

void Block::setText(String text)
{
  _text = text;
}

void Block::init()
{
  Locator::getEventBus()->subscribe(this);
  _dirty = true;
  redraw();
}

void Block::redraw()
{
  renderTo(Locator::getDisplay());
}

void Block::renderTo(Adafruit_GFX *display, int16_t offsetX, int16_t offsetY)
{
  drawBitmapWithActiveSky(display, _x + offsetX, _y + offsetY, BLOCK, _width, _height);

  int16_t originalX = _x;
  int16_t originalY = _y;
  _x += offsetX;
  _y += offsetY;
  setTextBlock(display);
  _x = originalX;
  _y = originalY;
}

void Block::update()
{

  if (_state == IDLE && _lastState != _state)
  {
    _dirty = true;
    _lastState= _state;

  } else if (_state == HIT)
  {
    
    if (millis() - lastMillis >= 60)
    {

      // Serial.print("BLOCK Y = ");
      // Serial.println(_y);
      
      _y = _y + (MOVE_PACE * (direction == UP ? -1 : 1));
      _dirty = true;
                 
      if (floor(_firstY - _y) >= MAX_MOVE_HEIGHT)
      {
        // Serial.println("DOWN");
        direction = DOWN;
      }

      if (_y >= _firstY && direction == DOWN)
      {
        idle();
      }

      lastMillis = millis();
    }

  }
}

bool Block::consumeDirty()
{
  bool wasDirty = _dirty;
  _dirty = false;
  return wasDirty;
}


void Block::execute(EventType event, Sprite* caller)
{
  //Serial.println("Checking collision");

  if (event == EventType::MOVE)
  {
    if (this->collidedWith(caller))
    {
      hit();
      Locator::getEventBus()->broadcast(EventType::COLLISION, this);
    }
  }
  
}


const char* Block::name()
{
  return "BLOCK";
}
