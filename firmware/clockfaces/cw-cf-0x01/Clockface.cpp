
#include "Clockface.h"
#include <CWPreferences.h>

EventBus eventBus;

const char* FORMAT_TWO_DIGITS = "%02d";

// Graphical elements
Tile ground(GROUND, 8, 8); 

Object bush(BUSH, 21, 9);
Object cloud1(CLOUD1, 17, 12);
Object cloud2(CLOUD2, 16, 12);
Object hill(HILL, 20, 22);


Mario mario(23, 40);
Block hourBlock(13, 8);
Block minuteBlock(32, 8);

unsigned long lastMillis = 0;
static const int16_t CLOUD_WRAP_GAP = 10;
static GFXcanvas16 skyStripCanvas(DISPLAY_WIDTH, Clockface::SKY_STRIP_HEIGHT);
static const int16_t CLOUD1_BASE_Y = 21;
static const int16_t CLOUD2_BASE_Y = 4;
static const int16_t BUSH_X = 43;
static const int16_t BUSH_Y = 47;
static const int16_t BUSH_SWAY_CLEAR_MARGIN = 2;
static const uint8_t STAR_COUNT = 22;
static const uint8_t STAR_X[STAR_COUNT] = {
  2, 5, 11, 7, 14, 4, 9, 17, 23, 29, 34,
  41, 47, 52, 55, 59, 62, 57, 53, 26, 40, 44
};
static const uint8_t STAR_Y[STAR_COUNT] = {
  4, 9, 5, 17, 30, 28, 32, 3, 7, 2, 5,
  4, 6, 3, 10, 15, 8, 27, 31, 30, 30, 33
};
static const uint16_t STAR_COLORS[] = {
  0x8D1F, 0xC67F, 0xE73F, 0xF5B2, 0xFDF2, 0xFEF7, 0xFF7B, 0xFFDF, 0xFFFF
};

static void drawWrappedObject(Adafruit_GFX &display, Object &object, int16_t x, int16_t y)
{
  drawBitmapWithActiveSky(&display, x, y, object._image, object._width, object._height);

  if (x < 0)
  {
    drawBitmapWithActiveSky(&display, x + DISPLAY_WIDTH + CLOUD_WRAP_GAP, y, object._image, object._width, object._height);
  }
}

Clockface::Clockface(Adafruit_GFX* display) {
  _display = display;

  Locator::provide(display);
  Locator::provide(&eventBus);
}

uint16_t Clockface::resolveSkyColor() const
{
  uint8_t dynamicSkyMode = ClockwiseParams::getInstance()->dynamicSkyMode;

  if (dynamicSkyMode == 0)
    return SKY_DAY;
  if (dynamicSkyMode == 1)
    return SKY_NIGHT;
  if (_dateTime == nullptr)
    return SKY_DAY;

  uint8_t hour = _dateTime->getHour();
  return (hour >= 8 && hour < 20) ? SKY_DAY : SKY_NIGHT;
}

bool Clockface::isNightSky() const
{
  return resolveSkyColor() == SKY_NIGHT;
}

void Clockface::redrawScene()
{
  Locator::getDisplay()->setFont(&Super_Mario_Bros__24pt7b);
  Locator::getDisplay()->fillRect(0, 0, 64, 64, getActiveSkyColor());

  ground.fillRow(DISPLAY_HEIGHT - ground._height);
  hill.draw(0, 34);
  drawBush();
  drawClouds();
  mario.redraw();
}

void Clockface::updateSkyTheme()
{
  uint16_t desiredSkyColor = resolveSkyColor();
  if (desiredSkyColor == _lastSkyColor)
    return;

  setActiveSkyColor(desiredSkyColor);
  _lastSkyColor = desiredSkyColor;
  redrawScene();
}

void Clockface::drawStars()
{
  if (!isNightSky())
    return;

  for (uint8_t i = 0; i < STAR_COUNT; i++)
  {
    skyStripCanvas.drawPixel(STAR_X[i], STAR_Y[i], STAR_COLORS[_starPaletteIndex[i]]);
  }
}

void Clockface::drawBush()
{
  Locator::getDisplay()->fillRect(
    BUSH_X - BUSH_SWAY_CLEAR_MARGIN,
    BUSH_Y,
    bush._width + (BUSH_SWAY_CLEAR_MARGIN * 2),
    bush._height,
    getActiveSkyColor()
  );

  for (int16_t row = 0; row < bush._height; row++)
  {
    int16_t swayOffset = 0;

    if (row <= 1)
      swayOffset = _bushSway;
    else if (row <= 3)
      swayOffset = _bushSway > 0 ? 1 : (_bushSway < 0 ? -1 : 0);

    drawBitmapWithActiveSky(
      Locator::getDisplay(),
      BUSH_X + swayOffset,
      BUSH_Y + row,
      bush._image + (row * bush._width),
      bush._width,
      1
    );
  }
}

void Clockface::updateBush()
{
  if (!_animationEnabled)
    return;

  uint8_t cloudSpeed = ClockwiseParams::getInstance()->cloudSpeed;
  if (cloudSpeed < 1)
    cloudSpeed = 1;
  if (cloudSpeed > 30)
    cloudSpeed = 30;

  if (millis() < _nextBushSwayMillis)
    return;

  int8_t nextSway = _bushSway;
  while (nextSway == _bushSway)
  {
    nextSway = random(-1, 2);
  }
  _bushSway = nextSway;
  _nextBushSwayMillis = millis() + (random(2, 6) * cloudSpeed * 100UL);

  drawBush();
}

void Clockface::drawClouds()
{
  skyStripCanvas.fillScreen(getActiveSkyColor());
  skyStripCanvas.setFont(&Super_Mario_Bros__24pt7b);

  if (!isNightSky())
  {
    drawWrappedObject(skyStripCanvas, cloud1, _cloud1X, (CLOUD1_BASE_Y + _cloud1YOffset) - SKY_STRIP_Y);
    drawWrappedObject(skyStripCanvas, cloud2, _cloud2X, (CLOUD2_BASE_Y + _cloud2YOffset) - SKY_STRIP_Y);
  }

  hourBlock.renderTo(&skyStripCanvas, 0, -SKY_STRIP_Y);
  minuteBlock.renderTo(&skyStripCanvas, 0, -SKY_STRIP_Y);

  if (isNightSky())
  {
    drawStars();
  }

  Locator::getDisplay()->drawRGBBitmap(0, SKY_STRIP_Y, skyStripCanvas.getBuffer(), DISPLAY_WIDTH, SKY_STRIP_HEIGHT);
}

void Clockface::updateStars()
{
  if (!isNightSky())
    return;

  if (!_animationEnabled)
    return;

  if (millis() < _nextStarTwinkleMillis)
    return;

  uint8_t starIndex = random(0, STAR_COUNT);

  int8_t nextDirection = _starTwinkleDirection[starIndex];
  uint8_t maxPaletteIndex = (sizeof(STAR_COLORS) / sizeof(STAR_COLORS[0])) - 1;

  if (_starPaletteIndex[starIndex] == 0)
    nextDirection = 1;
  else if (_starPaletteIndex[starIndex] >= maxPaletteIndex)
    nextDirection = -1;
  else if (random(0, 4) == 0)
    nextDirection *= -1;

  _starTwinkleDirection[starIndex] = nextDirection;
  _starPaletteIndex[starIndex] = constrain(
    _starPaletteIndex[starIndex] + nextDirection,
    0,
    maxPaletteIndex
  );

  uint8_t secondStarIndex = random(0, STAR_COUNT);
  if (secondStarIndex != starIndex && random(0, 3) == 0)
  {
    int8_t secondDirection = _starTwinkleDirection[secondStarIndex];
    if (_starPaletteIndex[secondStarIndex] == 0)
      secondDirection = 1;
    else if (_starPaletteIndex[secondStarIndex] >= maxPaletteIndex)
      secondDirection = -1;

    _starTwinkleDirection[secondStarIndex] = secondDirection;
    _starPaletteIndex[secondStarIndex] = constrain(
      _starPaletteIndex[secondStarIndex] + secondDirection,
      0,
      maxPaletteIndex
    );
  }
  _nextStarTwinkleMillis = millis() + random(180UL, 650UL);

  drawClouds();
}

void Clockface::updateClouds()
{
  if (!_animationEnabled)
    return;

  if (isNightSky())
    return;

  uint8_t cloudSpeed = ClockwiseParams::getInstance()->cloudSpeed;
  if (cloudSpeed < 1)
    cloudSpeed = 1;
  if (cloudSpeed > 30)
    cloudSpeed = 30;

  unsigned long cloudMoveInterval = cloudSpeed * 100UL;
  if (millis() - _lastCloudMoveMillis < cloudMoveInterval)
    return;

  _lastCloudMoveMillis = millis();
  _cloud1X--;

  if (millis() >= _nextCloud1YOffsetMillis)
  {
    _cloud1YOffset = constrain(_cloud1YOffset + random(-1, 2), -1, 1);
    _nextCloud1YOffsetMillis = millis() + (random(1, 7) * 1000UL);
  }

  _cloud2MoveTick = (_cloud2MoveTick + 1) % 2;
  if (_cloud2MoveTick == 0)
  {
    _cloud2X--;
    _cloud2YOffset = constrain(_cloud2YOffset + random(-1, 2), -2, 2);
  }

  if (_cloud1X <= -(cloud1._width + CLOUD_WRAP_GAP))
    _cloud1X += DISPLAY_WIDTH + CLOUD_WRAP_GAP;

  if (_cloud2X <= -(cloud2._width + CLOUD_WRAP_GAP))
    _cloud2X += DISPLAY_WIDTH + CLOUD_WRAP_GAP;

  drawClouds();
}

void Clockface::setup(CWDateTime *dateTime) {
  _dateTime = dateTime;
  _animationEnabled = ClockwiseParams::getInstance()->animationEnabled;
  _lastSkyColor = resolveSkyColor();
  setActiveSkyColor(_lastSkyColor);
  _cloud1X = 0;
  _cloud2X = 51;
  _cloud1YOffset = 0;
  _cloud2YOffset = 0;
  _bushSway = 0;
  _cloud2MoveTick = 0;
  for (uint8_t i = 0; i < STAR_COUNT; i++)
  {
    _starPaletteIndex[i] = random(0, sizeof(STAR_COLORS) / sizeof(STAR_COLORS[0]));
    _starTwinkleDirection[i] = random(0, 2) == 0 ? -1 : 1;
  }
  _lastCloudMoveMillis = millis();
  _nextCloud1YOffsetMillis = millis() + (random(1, 7) * 1000UL);
  _nextBushSwayMillis = millis() + (random(2, 6) * ClockwiseParams::getInstance()->cloudSpeed * 100UL);
  _nextStarTwinkleMillis = millis() + random(250UL, 900UL);

  Locator::getDisplay()->setFont(&Super_Mario_Bros__24pt7b);
  Locator::getDisplay()->fillRect(0, 0, 64, 64, getActiveSkyColor());

  ground.fillRow(DISPLAY_HEIGHT - ground._height);

  drawBush();
  hill.draw(0, 34);

  updateTime();

  hourBlock.init();
  minuteBlock.init();
  mario.init();
  drawClouds();
}

void Clockface::update()
{
  hourBlock.update();
  minuteBlock.update();
  mario.update();
  updateSkyTheme();
  updateStars();
  updateBush();
  updateClouds();

  if (_dateTime->getSecond() == 0 && millis() - lastMillis > 1000)
  {
    mario.jump();
    updateTime();
    lastMillis = millis();

    //Serial.println(_dateTime->getFormattedTime());
  }
}

void Clockface::updateTime()
{
  //hourBlock.setText(String(_dateTime->getHour()));
  hourBlock.setText(String(_dateTime->getHour(FORMAT_TWO_DIGITS)));
  minuteBlock.setText(String(_dateTime->getMinute(FORMAT_TWO_DIGITS)));
}

void Clockface::externalEvent(int type)
{
  if (type == 0) {  //TODO create an enum
    mario.jump();
    updateTime();
  }
}
