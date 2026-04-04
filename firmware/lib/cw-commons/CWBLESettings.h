#pragma once

#include <Arduino.h>

class CWBLESettings
{
  public:
    static CWBLESettings *getInstance();
    void begin();

  private:
    CWBLESettings() = default;
};
