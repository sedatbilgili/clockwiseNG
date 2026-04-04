#pragma once
#include "Locator.h"
#include "SkyBitmap.h"

struct Object {
  const unsigned short *_image;
  int _width;
  int _height;

  Object(const unsigned short *image, int width, int height) {
    this->_image = image;
    this->_width = width;
    this->_height = height;
  }
  
  void draw(int x, int y) {
    drawBitmapWithActiveSky(Locator::getDisplay(), x, y, _image, _width, _height);
  }
};
