#pragma once

#include <Arduino.h>

#include "gfx/Super_Mario_Bros__24pt7b.h"

#include <Adafruit_GFX.h>
#include <Tile.h>
#include <Locator.h>
#include <Game.h>
#include <Object.h>
#include <ImageUtils.h>
// Commons
#include <IClockface.h>
#include <CWDateTime.h>

#include "gfx/assets.h"
#include "gfx/mario.h"
#include "gfx/block.h"

#define CLOCKFACE_NAME "cw-cf-0x01"

class Clockface: public IClockface {
  public:
    static const int16_t SKY_STRIP_Y = 0;
    static const int16_t SKY_STRIP_HEIGHT = 35;

    Clockface(Adafruit_GFX* display);
    void setup(CWDateTime *dateTime);
    void update();
    void externalEvent(int type);

  private:
    Adafruit_GFX* _display;
    CWDateTime* _dateTime;
    bool _animationEnabled = true;
    int16_t _cloud1X = 0;
    int16_t _cloud2X = 51;
    int8_t _cloud1YOffset = 0;
    int8_t _cloud2YOffset = 0;
    int8_t _bushSway = 0;
    uint8_t _cloud2MoveTick = 0;
    uint8_t _starPaletteIndex[22] = {0};
    int8_t _starTwinkleDirection[22] = {1};
    uint8_t _groundScrollOffset = 0;
    uint8_t _worldScrollOffset = 0;
    unsigned long _lastCloudMoveMillis = 0;
    unsigned long _lastGroundMoveMillis = 0;
    unsigned long _lastGoombaFrameMillis = 0;
    uint16_t _lastSkyColor = SKY_DAY;
    unsigned long _nextCloud1YOffsetMillis = 0;
    unsigned long _nextBushSwayMillis = 0;
    unsigned long _nextStarTwinkleMillis = 0;
    bool _skyStripDirty = false;
    bool _landscapeDirty = false;
    uint8_t _goombaWalkFrame = 0;
    bool _lastGoombaEnabled = true;
    bool isNightSky() const;
    uint16_t resolveSkyColor() const;
    void redrawScene();
    void updateSkyTheme();
    void drawStars();
    void updateStars();
    void drawBush();
    void drawHill();
    void drawPipe();
    void updateBush();
    void drawGround();
    void drawClouds();
    void composeSceneRow(int16_t row, uint16_t *rowBuffer, bool includeMario);
    void redrawLandscapeBand();
    void redrawMarioArea(int16_t x, int16_t y, int16_t width, int16_t height);
    void updateClouds();
    void updateGround();
    bool updateGoombaAnimation();
    void updateTime();

};
