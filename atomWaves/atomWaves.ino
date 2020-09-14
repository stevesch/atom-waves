#include <Arduino.h>
#include "M5Atom.h"

#include "PixelGrid.h"

///////////////////////////////////////

// To get up-and-running for M5Atom on Arduino:
// - Set board to "ESP32 Pico Kit"
// - Install "M5Atom" and "FastLED" libraries
// - Select serial port connected to your board
// - Set upload speed to 115200

///////////////////////////////////////

// #include <numbers>
// constexpr float kPi = std::numbers::pi;
#define _USE_MATH_DEFINES
#include <cmath>
constexpr float kPi = M_PI;
constexpr float k2Pi = M_TWOPI;

// Traveling wave display: 2D traveling sine wave in each color.
//
// Each wave moves independently and has independent parameters.
// Wave parameters (wavelength, speed of travel, direction of travel) are
// controlled by values in the TravelingWave structs, wave_r, wave_g, and
// wave_b.
//
// Separate variables, angle_r_speed, angle_g_speed, angle_b_speed, specify
// a continuous rotation rate for the direction of travel for each wave.
// i.e. each wave's direction of travel rotates at a constant speed specified
// by these variables.

void updateFrame(float dt);
void changeSpeed();

///////////////////////////////////////

// variables for keeping track of our delta time:
long tLastFrameUpdate = 0;

// We don't bother refeshing display faster than 100fps (1/0.01):
constexpr float kMinFrameTime = 0.01f;

///////////////////////////////////////

void setup()
{
  M5.begin(true, false, true);
}

void loop()
{
  // compute a delta time since last loop
  long tNow = micros();

  M5.update();
  if (M5.Btn.wasPressed())
  {
    changeSpeed();
  }

  long delta = tNow - tLastFrameUpdate;
  float dt = 1.0e-6f * delta;
  if (dt >= kMinFrameTime)
  {
    tLastFrameUpdate = tNow;
    updateFrame(dt);
  }
  else
  {
    // yield a bit of time to the system (we don't need to update the display
    // every millisecond, but we like to poll our buttons frequently for
    // responsive input, so we loop frequently, but we yield/delay so we
    // don't hog CPU in our update loop)
    delay(1);
  }
}

///////////////////////////////////////

PixelGrid grid;

// button press cycles through these speed factors:
float paramSpeeds[]{
    1.0f,
    -1.0f,
    5.0f,
    -5.0f,
    10.0f,
    -10.0f,
    0.0f,
};
constexpr int paramSpeedCount = sizeof(paramSpeeds) / sizeof(paramSpeeds[0]);

int paramSpeedIndex = 0; // current speed factor being used
float param = 0.0f;

void changeSpeed()
{
  paramSpeedIndex = (paramSpeedIndex + 1) % paramSpeedCount;
}

///////////////////////////////////////

struct TravelingWave
{
  float k;     // spatial frequency
  float w;     // frequency
  float phase; // phase offset

  // spatial direction of travel (vx=cos(angle), vy=sin(angle))
  float vx;
  float vy;
};

void setAngleOfTravel(struct TravelingWave &wave, float angle)
{
  // This occurs outside of the main pixel loop, so
  // it is not a huge speed hit, but a slight speedup
  // could be achieved via use of the FastLED cos8 and sin8 functions:
  wave.vx = cosf(angle);
  wave.vy = sinf(angle);
}

// k = 2pi/lambda;  w = 2pi/f
float travelingCos(float x, float y, float t, const TravelingWave &wave)
{
  // For simplicity, we're operating entirely with floating-point operations here.
  // We only have 25 LEDs on the Atom M5, so this isn't much of an issue, but
  // if we want a faster evaluation (like for a larger display), we can use
  // the FastLED cos8 function (would need to )
  const float pxx = wave.vx;
  const float pxy = wave.vy;
  const float tx = x * pxx + y * pxy;
  float value = cosf(wave.k * tx - wave.w * t + wave.phase);
  return value;
}

///////////////////////////////////////
// Wave parameters for wavelength, etc.

// Red wave
TravelingWave wave_r{
    .k = k2Pi * 0.5f,
    .w = k2Pi * 1.0f,
    .phase = 0.0f,
};
float angle_r = 0.0f;
const float angle_r_speed = k2Pi * 0.125f; // rotational frequency

// Green wave
TravelingWave wave_g{
    .k = k2Pi * 0.3f,
    .w = k2Pi * 0.8f,
    .phase = 0.0f,
};
float angle_g = 0.0f;
const float angle_g_speed = k2Pi * -0.125f; // rotational frequency

// Blue wave
TravelingWave wave_b{
    .k = k2Pi * 0.10f,
    .w = k2Pi * 0.05f,
    .phase = kPi,
};
float angle_b = 0.0f;
const float angle_b_speed = k2Pi * 0.107f; // rotational frequency

///////////////////////////////////////

void updateFrame(float dt)
{
  float paramSpeed = paramSpeeds[paramSpeedIndex];
  param += dt * paramSpeed;

  // update direction of travel for each wave:
  angle_r += angle_r_speed * dt;
  setAngleOfTravel(wave_r, angle_r);
  angle_g += angle_g_speed * dt;
  setAngleOfTravel(wave_g, angle_g);
  angle_b += angle_b_speed * dt;
  setAngleOfTravel(wave_b, angle_b);

  const float t = param;

  // The grid "evaluator" evalGrid is called for each pixel, with x and y
  // normalized to a range ot [-1.0, +1.0] for the entire screen.
  // (bottom-left is <-1.0, -1.0>, and top-right is <+1.0, +1.0>)
  // The return value from the evaluator is the color for the pixel at
  // point <x, y>.

  // // Debug: set all pixels to green
  // grid.evalGrid([=](float x, float y) {
  //     uint8_t r = 0;
  //     uint8_t g = 255;
  //     uint8_t b = 0;
  //     CRGB color (r, g, b);
  //     return color;
  // });

  // // Debug: draw red gradient in x, green gradient in y
  // grid.evalGrid([=](float x, float y) {
  //     uint8_t r = 0;
  //     uint8_t g = 0;
  //     uint8_t b = 0;
  //     r = (uint8_t)floorf( (0.5*(x + 1.0f)) * 255.0f + 0.5f);
  //     g = (uint8_t)floorf( (0.5*(y + 1.0f)) * 255.0f + 0.5f);
  //     return CRGB(r, g, b);
  // });

  grid.evalGrid([=](float x, float y) {
    // evaluate a traveling wave to each color.  Remap the range
    // ([-1.0, +1.0]) of cosine to a range of [0, 255] for
    // color intensity.

    float vr = travelingCos(x, y, t, wave_r);
    uint8_t r = (uint8_t)floorf((0.5 * (vr + 1.0f)) * 255.0f + 0.5f);
    // uint8_t r = 0; // Debug: zero out this channel

    float vg = travelingCos(x, y, t, wave_g);
    uint8_t g = (uint8_t)floorf((0.5 * (vg + 1.0f)) * 255.0f + 0.5f);
    // uint8_t g = 0; // Debug: zero out this channel

    float vb = travelingCos(x, y, t, wave_b);
    uint8_t b = (uint8_t)floorf((0.5 * (vb + 1.0f)) * 255.0f + 0.5f);
    // uint8_t b = 0; // Debug: zero out this channel

    return CRGB(r, g, b);
  });

  // Grab the raw data from the grid and send it off to the display:
  // (must be a 5x5 grid for the M5 Atom)
  M5.dis.displaybuff(const_cast<uint8_t *>(grid.raw()), 0, 0);
}
