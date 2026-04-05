
#include "Clockface.h"
#include <CWPreferences.h>

EventBus eventBus;

const char* FORMAT_TWO_DIGITS = "%02d";

// Graphical elements
Tile ground(GROUND, 8, 8); 

Object bush(bushNew, 24, 9);
Object cloud1(CLOUD1, 17, 12);
Object cloud2(CLOUD2, 16, 12);
Object hill(hillNew, 47, 18);
Object pipeObj(PIPE_SPRITE, 16, 12);


Mario mario(23, 40);
Block hourBlock(13, 8);
Block minuteBlock(32, 8);

unsigned long lastMillis = 0;
static const int16_t CLOUD_WRAP_GAP = 10;
static GFXcanvas16 skyStripCanvas(DISPLAY_WIDTH, Clockface::SKY_STRIP_HEIGHT);
static const int16_t VIRTUAL_DISPLAY_WIDTH = 160;
static const int16_t VIRTUAL_VISIBLE_OFFSET_X = 32;
static const int16_t CLOUD1_BASE_Y = 21;
static const int16_t CLOUD2_BASE_Y = 4;
static const int16_t PIPE_VIRTUAL_X = 75;
static const int16_t BUSH_VIRTUAL_X = 109;
static const int16_t BUSH_Y = 47;
static const int16_t PIPE_Y = 44;
static const int16_t HILL_BASELINE_Y = 56;
static const int16_t HILL_VIRTUAL_X = 5;
static const int16_t BUSH_SWAY_CLEAR_MARGIN = 2;
static const unsigned long GROUND_MOVE_INTERVAL = 200UL;
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
static int16_t g_lastMarioX = 23;
static int16_t g_lastMarioY = 40;
static uint8_t g_lastMarioWidth = 13;
static uint8_t g_lastMarioHeight = 16;
static uint16_t marioCompositeRow[DISPLAY_WIDTH];

static int16_t getVirtualScreenX(int16_t virtualX, uint8_t worldScrollOffset)
{
  return virtualX - VIRTUAL_VISIBLE_OFFSET_X - worldScrollOffset;
}

static bool intersectsScreen(int16_t x, int16_t width)
{
  return x < DISPLAY_WIDTH && (x + width) > 0;
}

static int16_t getHillY()
{
  return HILL_BASELINE_Y - hill._height;
}

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

  drawGround();
  drawHill();
  drawPipe();
  drawBush();
  drawClouds();
  mario.redraw();
  g_lastMarioX = mario.x();
  g_lastMarioY = mario.y();
  g_lastMarioWidth = mario.width();
  g_lastMarioHeight = mario.height();
}

void Clockface::drawHill()
{
  int16_t hillY = getHillY();
  int16_t baseX = getVirtualScreenX(HILL_VIRTUAL_X, _worldScrollOffset);
  int16_t candidateX[3] = {baseX, (int16_t)(baseX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(baseX + VIRTUAL_DISPLAY_WIDTH)};

  for (uint8_t i = 0; i < 3; i++)
  {
    if (!intersectsScreen(candidateX[i], hill._width))
      continue;

    drawBitmapWithActiveSky(Locator::getDisplay(), candidateX[i], hillY, hill._image, hill._width, hill._height);
  }
}

void Clockface::drawPipe()
{
  int16_t pipeX = getVirtualScreenX(PIPE_VIRTUAL_X, _worldScrollOffset);
  int16_t pipeCandidateX[3] = {pipeX, (int16_t)(pipeX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(pipeX + VIRTUAL_DISPLAY_WIDTH)};

  for (uint8_t i = 0; i < 3; i++)
  {
    if (!intersectsScreen(pipeCandidateX[i], pipeObj._width))
      continue;

    drawBitmapWithActiveSky(Locator::getDisplay(), pipeCandidateX[i], PIPE_Y, pipeObj._image, pipeObj._width, pipeObj._height);
  }
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
  int16_t baseX = getVirtualScreenX(BUSH_VIRTUAL_X, _worldScrollOffset);
  int16_t candidateX[3] = {baseX, (int16_t)(baseX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(baseX + VIRTUAL_DISPLAY_WIDTH)};

  for (uint8_t candidate = 0; candidate < 3; candidate++)
  {
    if (!intersectsScreen(candidateX[candidate], bush._width))
      continue;

    Locator::getDisplay()->fillRect(
      candidateX[candidate] - BUSH_SWAY_CLEAR_MARGIN,
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
        candidateX[candidate] + swayOffset,
        BUSH_Y + row,
        bush._image + (row * bush._width),
        bush._width,
        1
      );
    }
  }
}

void Clockface::drawGround()
{
  const int16_t groundY = DISPLAY_HEIGHT - ground._height;
  const uint8_t normalizedOffset = _groundScrollOffset % ground._width;

  for (int16_t row = 0; row < ground._height; row++)
  {
    uint16_t rowBuffer[DISPLAY_WIDTH];
    const unsigned short *sourceRow = ground._image + (row * ground._width);

    for (int16_t xx = 0; xx < DISPLAY_WIDTH; xx++)
    {
      rowBuffer[xx] = sourceRow[(xx + normalizedOffset) % ground._width];
    }

    Locator::getDisplay()->drawRGBBitmap(0, groundY + row, rowBuffer, DISPLAY_WIDTH, 1);
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
  _landscapeDirty = true;
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

void Clockface::composeSceneRow(int16_t row, uint16_t *rowBuffer, bool includeMario)
{
  int16_t hillBaseX = getVirtualScreenX(HILL_VIRTUAL_X, _worldScrollOffset);
  int16_t hillY = getHillY();
  int16_t groundY = DISPLAY_HEIGHT - ground._height;
  int16_t pipeBaseX = getVirtualScreenX(PIPE_VIRTUAL_X, _worldScrollOffset);
  int16_t bushBaseX = getVirtualScreenX(BUSH_VIRTUAL_X, _worldScrollOffset);

  for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
    rowBuffer[col] = getActiveSkyColor();
  }

  if (row < SKY_STRIP_HEIGHT) {
    const uint16_t *skyBuffer = skyStripCanvas.getBuffer();
    for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
      rowBuffer[col] = skyBuffer[(row * DISPLAY_WIDTH) + col];
    }
  }

  if (row >= hillY && row < hillY + hill._height) {
    int16_t hillRow = row - hillY;
    int16_t hillCandidateX[3] = {hillBaseX, (int16_t)(hillBaseX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(hillBaseX + VIRTUAL_DISPLAY_WIDTH)};

    for (uint8_t candidate = 0; candidate < 3; candidate++) {
      for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
        int16_t hillCol = col - hillCandidateX[candidate];
        if (hillCol < 0 || hillCol >= hill._width) {
          continue;
        }

        uint16_t pixel = hill._image[(hillRow * hill._width) + hillCol];
        if (pixel == SKY_PIXEL) {
          continue;
        }
        rowBuffer[col] = pixel;
      }
    }
  }

  if (row >= PIPE_Y && row < PIPE_Y + pipeObj._height) {
    int16_t pipeRow = row - PIPE_Y;
    int16_t pipeCandidateX[3] = {pipeBaseX, (int16_t)(pipeBaseX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(pipeBaseX + VIRTUAL_DISPLAY_WIDTH)};

    for (uint8_t candidate = 0; candidate < 3; candidate++) {
      for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
        int16_t pipeCol = col - pipeCandidateX[candidate];
        if (pipeCol < 0 || pipeCol >= pipeObj._width) {
          continue;
        }

        uint16_t pixel = pipeObj._image[(pipeRow * pipeObj._width) + pipeCol];
        if (pixel == SKY_PIXEL) {
          continue;
        }
        rowBuffer[col] = pixel;
      }
    }
  }

  if (row >= BUSH_Y && row < BUSH_Y + bush._height) {
    int16_t bushRow = row - BUSH_Y;
    int16_t swayOffset = 0;

    if (bushRow <= 1)
      swayOffset = _bushSway;
    else if (bushRow <= 3)
      swayOffset = _bushSway > 0 ? 1 : (_bushSway < 0 ? -1 : 0);

    int16_t bushCandidateX[3] = {bushBaseX, (int16_t)(bushBaseX - VIRTUAL_DISPLAY_WIDTH), (int16_t)(bushBaseX + VIRTUAL_DISPLAY_WIDTH)};

    for (uint8_t candidate = 0; candidate < 3; candidate++) {
      for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
        int16_t bushCol = col - (bushCandidateX[candidate] + swayOffset);
        if (bushCol < 0 || bushCol >= bush._width) {
          continue;
        }

        uint16_t pixel = bush._image[(bushRow * bush._width) + bushCol];
        if (pixel == SKY_PIXEL) {
          continue;
        }
        rowBuffer[col] = pixel;
      }
    }
  }

  if (row >= groundY && row < groundY + ground._height) {
    int16_t groundRow = row - groundY;
    const unsigned short *sourceRow = ground._image + (groundRow * ground._width);
    uint8_t normalizedOffset = _groundScrollOffset % ground._width;

    for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
      rowBuffer[col] = sourceRow[(col + normalizedOffset) % ground._width];
    }
  }

  if (includeMario) {
    for (int16_t col = 0; col < DISPLAY_WIDTH; col++) {
      uint16_t marioPixel;
      if (mario.pixelAt(col, row, marioPixel)) {
        rowBuffer[col] = marioPixel;
      }
    }
  }
}

void Clockface::redrawLandscapeBand()
{
  int16_t startY = getHillY();
  int16_t endY = DISPLAY_HEIGHT;

  for (int16_t row = startY; row < endY; row++) {
    composeSceneRow(row, marioCompositeRow, true);
    Locator::getDisplay()->drawRGBBitmap(0, row, marioCompositeRow, DISPLAY_WIDTH, 1);
  }
}

void Clockface::redrawMarioArea(int16_t x, int16_t y, int16_t width, int16_t height)
{
  int16_t clampedX = max<int16_t>(0, x);
  int16_t clampedY = max<int16_t>(0, y);
  int16_t maxX = min<int16_t>(DISPLAY_WIDTH, x + width);
  int16_t maxY = min<int16_t>(DISPLAY_HEIGHT, y + height);

  if (maxX <= clampedX || maxY <= clampedY)
    return;

  for (int16_t row = clampedY; row < maxY; row++) {
    composeSceneRow(row, marioCompositeRow, true);

    Locator::getDisplay()->drawRGBBitmap(clampedX, row, marioCompositeRow + clampedX, maxX - clampedX, 1);
  }
}

void Clockface::updateGround()
{
  if (!_animationEnabled)
    return;

  if (!ClockwiseParams::getInstance()->walkingMario)
    return;

  if (millis() - _lastGroundMoveMillis < GROUND_MOVE_INTERVAL)
    return;

  _lastGroundMoveMillis = millis();
  _groundScrollOffset = (_groundScrollOffset + 1) % ground._width;
  _worldScrollOffset = (_worldScrollOffset + 1) % VIRTUAL_DISPLAY_WIDTH;
  _landscapeDirty = true;
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
  _skyStripDirty = true;
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
  _skyStripDirty = true;
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
  _groundScrollOffset = 0;
  _worldScrollOffset = 0;
  for (uint8_t i = 0; i < STAR_COUNT; i++)
  {
    _starPaletteIndex[i] = random(0, sizeof(STAR_COLORS) / sizeof(STAR_COLORS[0]));
    _starTwinkleDirection[i] = random(0, 2) == 0 ? -1 : 1;
  }
  _lastCloudMoveMillis = millis();
  _lastGroundMoveMillis = millis();
  _nextCloud1YOffsetMillis = millis() + (random(1, 7) * 1000UL);
  _nextBushSwayMillis = millis() + (random(2, 6) * ClockwiseParams::getInstance()->cloudSpeed * 100UL);
  _nextStarTwinkleMillis = millis() + random(250UL, 900UL);
  _skyStripDirty = false;
  _landscapeDirty = false;

  Locator::getDisplay()->setFont(&Super_Mario_Bros__24pt7b);
  Locator::getDisplay()->fillRect(0, 0, 64, 64, getActiveSkyColor());

  drawGround();

  drawBush();
  drawHill();
  drawPipe();

  updateTime();

  hourBlock.init();
  minuteBlock.init();
  mario.init();
  drawClouds();
}

void Clockface::update()
{
  bool shouldRedrawMario = false;
  bool shouldRedrawSkyStrip = false;

  hourBlock.update();
  minuteBlock.update();
  shouldRedrawSkyStrip = hourBlock.consumeDirty() || minuteBlock.consumeDirty();
  mario.update();
  shouldRedrawMario = mario.consumeDirty();
  updateSkyTheme();
  updateStars();
  updateBush();
  updateClouds();
  updateGround();

  if (_dateTime->getSecond() == 0 && millis() - lastMillis > 1000)
  {
    mario.jump();
    updateTime();
    lastMillis = millis();
    shouldRedrawMario = true;
    shouldRedrawSkyStrip = true;

    //Serial.println(_dateTime->getFormattedTime());
  }

  if (shouldRedrawSkyStrip)
  {
    drawClouds();
  }

  if (_skyStripDirty)
  {
    drawClouds();
    _skyStripDirty = false;
  }

  if (_landscapeDirty)
  {
    redrawLandscapeBand();
    _landscapeDirty = false;
  }

  if (shouldRedrawMario)
  {
    int16_t currentMarioX = mario.x();
    int16_t currentMarioY = mario.y();
    uint8_t currentMarioWidth = mario.width();
    uint8_t currentMarioHeight = mario.height();

    int16_t redrawX = min<int16_t>(g_lastMarioX, currentMarioX);
    int16_t redrawY = min<int16_t>(g_lastMarioY, currentMarioY);
    int16_t redrawRight = max<int16_t>(g_lastMarioX + g_lastMarioWidth, currentMarioX + currentMarioWidth);
    int16_t redrawBottom = max<int16_t>(g_lastMarioY + g_lastMarioHeight, currentMarioY + currentMarioHeight);

    redrawMarioArea(redrawX, redrawY, redrawRight - redrawX, redrawBottom - redrawY);

    g_lastMarioX = currentMarioX;
    g_lastMarioY = currentMarioY;
    g_lastMarioWidth = currentMarioWidth;
    g_lastMarioHeight = currentMarioHeight;
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
