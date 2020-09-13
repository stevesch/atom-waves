#include <Arduino.h>
#include <functional>
#include <FastLED.h>

// typedef EvalXy [](float x, float y) -> uint32_t;
typedef std::function<CRGB(float x, float y)> EvalXy;

// Helper class for evaluating a grid of colors:
// The EvalXy function passed to evalGrid is called for each pixel,
// with x and y normalized to a range of [-1.0, +1.0]
// (bottom-left is <-1.0, -1.0> and top-right is <+1.0, +1.0>)
class PixelGrid
{
  static constexpr uint8_t nx = 5;
  static constexpr uint8_t ny = 5;

public:
  PixelGrid() : mNx(nx), mNy(ny)
  {
    uint nn = nx * ny;
    for (uint n = 0; n < nn; ++n)
    {
      pixel[n] = 64;
    }
  }
  void evalGrid(EvalXy fn)
  {
    constexpr float x0 = -1.0f; // display is indexed left-to-right, so left is -1,
    constexpr float x1 = 1.0f;  // right is +1
    constexpr float y0 = 1.0f;  // display is indexed top-down, so reverse y so +y is up
    constexpr float y1 = -1.0f;
    const float dx = (x1 - x0) / (nx - 1);
    const float dy = (y1 - y0) / (ny - 1);
    int index = 0;
    float y = y0;
    for (int iy = ny; iy > 0; y += dy, --iy)
    {
      float x = x0;
      for (int ix = nx; ix > 0; x += dx, --ix)
      {
        CRGB color = fn(x, y);
        pixel[index++] = color;
      }
    }
  }
  const uint8_t *raw() const { return &mNx; }

protected:
  uint8_t mNx;
  uint8_t mNy;
  CRGB pixel[nx * ny];
};
