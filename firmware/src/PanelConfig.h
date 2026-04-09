#pragma once
#include <stdint.h>

typedef struct
{
  uint8_t r1, r2;
  uint8_t g1, g2;
  uint8_t b1, b2;
  uint8_t a, b, c, d, e;
  uint8_t clk, lat;
} PanelPins;

#define HOME_PANEL     1
#define ALICAN_PANEL   2
#define OGUZALI_PANEL  3

static const PanelPins panelConfigs[] =
{
  // HOME_PANEL
  {
    25, 14,
    26, 12,
    27, 13,
    23, 19, 5, 17, 18,
    16, 4
  },

  // ALICAN_PANEL
  {
    25, 14,
    26, 12,
    27, 13,
    23, 19, 5, 17, 32,
    16, 4
  },

  // OGUZALI_PANEL
  {
    25, 14,
    27, 13,
    26, 12,
    23, 19, 5, 17, 18,
    //23, 19, 5, 17, 32,
    16, 4
  }
};
