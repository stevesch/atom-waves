# Atom Waves

Displays a 2D traveling sine wave in each color channel of the 5x5 LED display of an [M5 ATOM ESP32 microcontroller](https://m5stack.com/collections/m5-atom).

Each wave moves independently and has independent parameters for wavelength and velocity.

Wave velocities are also continuously rotated over time, adding to a more chaotic yet fluid movement:

![Waves Animation](docs/m5-atom-waves-anim.gif)

(The M5 Atom is only 24mm square-- note the size of the USB-C plug in the image)

# Building and Running

To get up-and-running for M5 ATOM on Arduino:
- Set board to "ESP32 Pico Kit"
- Install "M5Atom" and "FastLED" libraries
- Select serial port connected to your board
- Set upload speed to 115200

Platform.io users:
- Choose "build and upload" from the tasks.
(All configuration is already be set in platformio.ini.  "M5Atom" and "FastLED" libraries are still required but should be automatically installed according to workspace settings)
