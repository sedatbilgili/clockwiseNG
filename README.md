> ![News 90s GIF](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/news.gif)[2024-04-21] [Version 1.4.2 released!](https://github.com/jnthas/clockwise/releases/tag/v1.4.2) Check the [change log](https://github.com/jnthas/clockwise/blob/main/CHANGELOG.md#142---2024-04-21) to see the fixes and new features added. Be part of the [Clock Club](https://github.com/jnthas/clock-club) and create your own clockface using Canvas.

[![Clockwise CI/CD](https://github.com/jnthas/clockwise/actions/workflows/clockwise-ci.yml/badge.svg)](https://github.com/jnthas/clockwise/actions/workflows/clockwise-ci.yml)

![Logo](https://github.com/jnthas/clockwise/blob/gh-pages/static/images/clockwise_logo.png "Logo")

> The DIY smart wall clock device

## ClockwiseNG

ClockwiseNG is a next-generation fork of the original [Clockwise](https://github.com/jnthas/clockwise) project.
This repository builds on the upstream work and will gradually document the changes, goals, and additions introduced in this fork.

Clockwise is a great idea to work with 64x64 LED matrices.
These displays are about the size of a wall clock and with the ESP32, besides controlling the content presented on the display we also gain the functionality of 
WiFi, Bluetooth, touch buttons and other sensors, which gives us basically a smart wall clock. 
From there I started to further develop the platform to create the _Clockfaces_, or skins that the clock can have. 

# ToDo:
 - Multilanguage support

## Changelog

# 2.8.3:

- A startup rendering bug affecting `PIPE_SPRITE` was fixed so the pipe now appears immediately on boot, including when animations are disabled.

# 2.8.2:

- The OTA workflow was improved for both ArduinoOTA and HTTP OTA paths.
- OTA preparation now more cleanly suspends conflicting runtime services before flashing.
- OTA progress state tracking was improved with more explicit runtime state and diagnostics.
- A dedicated OTA screen was added with update messaging and icon support.
- A progress bar was added to the OTA screen and it advances in `%10` steps.
- The OTA icon animation was redesigned to use fixed frames instead of runtime rotation for lower rendering cost.
- Support for multiple update icons was added, including correct handling of their different dimensions.
- OTA rendering was reworked so progress visualization does not unnecessarily slow down the update process.
- General display update timing and runtime behavior were tuned to better balance rendering, web activity, and OTA stability.

# 2.8.1:

- Timezone handling was made more deterministic by adding built-in POSIX mappings.
- `Asia/Istanbul` now uses a local POSIX timezone mapping instead of relying on remote lookup behavior.
- `UTC` and `Etc/UTC` were also mapped locally for consistency.

# 2.8.0:

- The Wi-Fi startup flow was fixed so a successful captive portal or alternative setup path now counts as a real successful connection.
- Uninitialized runtime flags in the Wi-Fi and web layers were corrected to avoid random restart and state issues.
- Preferences loading was optimized with an in-memory cache to reduce repeated NVS reads.
- The preferences namespace handling was corrected so the configured database name is actually used.
- The web server was refactored away from the old manual socket and HTTP parsing approach to the built-in ESP32 `WebServer` model.
- Several blocking request-handling paths were removed, improving general web responsiveness and reducing freeze risk.
- Request body handling and response generation were simplified to reduce `String` churn and heap pressure.
- The settings page script was moved into the firmware and is now served directly by the device instead of an external IP.
- Cache behavior for the settings UI and JSON endpoints was tightened to reduce stale frontend state.
- The settings page pin read flow was fixed so repeated reads work more reliably.
- Repeated `/read` requests were hardened against caching and overlapping XHR issues.
- The active firmware was simplified to the Mario clockface only.
- The unused canvas clockface and its related assets, settings, and helper code were removed.
- Unused canvas-related dependencies were cleaned out of the build.
- Display and clockface lifetime management was moved away from heap allocation to static storage with placement new.
  - This reduced fragmentation risk, especially around OTA and long-running uptime.

# 2.7.4:

- Rendering performance was adjusted to target a lower frame rate for improved stability.
- The display update loop was tuned from roughly `50 FPS` down to approximately `30 FPS`.
- This change was made to reduce rendering load while animations are active and to improve overall responsiveness.

# 2.7.3:

- The virtual scene width was expanded to allow a longer side-scrolling layout.
- A new pipe element was added to the scrolling scene.
- Scene object placement was adjusted to fit the new virtual world layout.
- Background composition was updated so the new pipe, hill, and bush elements all participate in the scrolling world consistently.

# 2.7.2:

- The Mario-themed scene has been significantly refreshed and made more dynamic.
- A new walking animation for Mario has been added.
- Mario’s classic jump animation at the start of each minute, where he hits the blocks, has been preserved and improved.
- The walking animation and the minute-change jump animation now work together more smoothly.
- Background scenery elements were also made animated to create a more lively moving scene.
- The overall background layout has been updated with new artwork.
- The scene composition was reworked around a wider virtual world layout.
- Layering between Mario, the background, and foreground elements has been improved.
- Masked and transparent-looking pixels now blend into the scene more correctly.
- Several rendering artifacts and layering issues around Mario were fixed.
- Hill rendering issues, including movement-related shifting and edge artifacts, were fixed.
- Small visual glitches in the block-hit animation were reduced.
- A new web setting was added to enable or disable Mario’s walking animation.
- Mario walking is now automatically unavailable when general animations are disabled.
- The OTA update flow was reworked and made more robust.
- During OTA updates, unnecessary runtime workload is suspended to keep the update process cleaner.
- The OTA screen was simplified into a cleaner update view.
- OTA failure handling was improved so the device can recover more reliably by restarting.
- Performance improvements were made to help the system stay more stable while animations and the web interface are active at the same time.
- The display update pipeline was optimized to reduce rendering load and limit freezes.
- Several redraw and compositing improvements were made to reduce flicker.
- Overall, the Mario-themed firmware experience is now more polished, more animated, and better controlled in terms of system behavior.



### ⏰ New Clockfaces
Create a new custom Clockface starting from [here](https://github.com/jnthas/cw-cf-0x00) or take a look at the [Clock Club](https://github.com/jnthas/clock-club) and discover how to create new ones using just a JSON file with no coding.


## Available clockfaces

Mario Bros. Clock | Time in Words
:----------------:|:------------:
![Mario Bros. Clockface](https://github.com/jnthas/cw-cf-0x01/blob/main/cf_0x01_thumb.jpg "Mario Bros. Clockface") | ![Time in Words Clockface](https://github.com/jnthas/cw-cf-0x02/blob/main/cf_0x02_thumb.jpg "Time in Words Clockface") 
https://github.com/jnthas/cw-cf-0x01 | https://github.com/jnthas/cw-cf-0x02

World Map Clock | Castlevania Clock Tower
:--------------:|:----------------------:
![World Map Clockface](https://github.com/jnthas/cw-cf-0x03/blob/main/cf_0x03_thumb.jpg "World Map Clockface") | ![Castlevania Clockface](https://github.com/jnthas/cw-cf-0x04/blob/main/cf_0x04_thumb.jpg "Castlevania Clockface") 
https://github.com/jnthas/cw-cf-0x03 | https://github.com/jnthas/cw-cf-0x04

Pacman | Pokedex
:-----:|:------:
![Pacman Clockface](https://github.com/jnthas/cw-cf-0x05/blob/main/cf_0x05_thumb.jpg "Pacman Clockface") | ![Pokedex Clockface](https://github.com/jnthas/cw-cf-0x06/blob/main/cf_0x06_thumb.jpg "Pokedex Clockface") 
https://github.com/jnthas/cw-cf-0x05 | https://github.com/jnthas/cw-cf-0x06

Canvas | Description
:-----:|:------:
<img id="cw-cf-0x07" src="https://github.com/jnthas/cw-cf-0x07/raw/main/cf_0x07_thumb.jpg" width="200" alt="Canvas Clockface"> | Canvas is a special type of Clockface<br>that is capable of rendering different<br>themes described in a JSON file.<br>Find out more [here](https://github.com/jnthas/clockwise/wiki/Canvas-Clockface).
https://github.com/jnthas/cw-cf-0x07 |


## Driving the led matrix

The three main hardware components of Clockwise are: 
- HUB75/HUB75E compatible LED matrix 64x64
- an ESP32; and 
- a power supply of 3A or more

With these components in hand, just follow the wiring instructions according to the library used, by default Clockwise uses the [ESP32-HUB75-MatrixPanel-I2S-DMA](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA#2-wiring-esp32-with-the-led-matrix-panel) but any Adafruit GFX compatible library should work. The default wiring connection is showed below.

![ESP32-HUB75-MatrixPanel-I2S-DMA wiring](https://github.com/jnthas/clockwise/blob/gh-pages/static/images/display_esp32_wiring_thumb.png "ESP32-HUB75-MatrixPanel-I2S-DMA wiring")

[Full size](https://github.com/jnthas/clockwise/blob/gh-pages/static/images/display_esp32_wiring_bb.png)

- In case you want something ready to use, I recommend Brian Lough's [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity), basically it's connecting the board and uploading the firmware, as simple as that.
- If you want a designed PCB, I recommend this project from @Alexvanheu. It's compatible with HUB75/HUB75E led matrices and already tested with Clockwise https://github.com/Alexvanheu/Mario-Clock-PCB-ESP32
- [ESP32 D1 Mini D1 RGB Matrix Shield](https://github.com/hallard/WeMos-Matrix-Shield-DMA) from @hallard is another option


## How to change the clockface (web flashing)

1) Go to https://clockwise.page/ and select the desired clockface
2) Connect the ESP32 device on your computer's USB port 
3) Click on the Flash button
4) A dialog will appear, select the correct USB port and click in Connect ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step1.png))
5) Select the INSTALL and INSTALL again ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step2.png))
6) Wait while the flash tool uploads the firmware and finish ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step3.png))
7) From the version 1.1.0, click in NEXT on step 6, Improv will start looking for available WiFi networks to connect
8) Select your local network (must be a 2.4GHz) and enter with your password ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step4.png))
9) If connection was successful, a message with button VISIT DEVICE will pop up and you can visit the Clockwise setting page  ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step5.png))


### Configuring only WiFi
After flashing your clockface, you will have a step to configure the WiFi. But in case you change your access point or password, you can set up just the WiFi connecting the Clockwise on USB, opening https://clockwise.page and clicking in Flash button, a window will pop up with a few options where you can re-configure your WiFi network ([screenshot](https://github.com/jnthas/clockwise/raw/gh-pages/static/images/usb-step6.png)) as well as open the Settings page to change preferences using button VISIT DEVICE. Remember: it is important to use a 2.4GHz WiFi, it will not work on 5GHz.


### Settings page
The settings page have the following options
- *Timezone*: The timezone must be in the format America/New_York, America/Sao_Paulo, Europe/Paris, Asia/Dubai, etc. so that the clock can connect to an NTP server to get the correct time.
- *NTP Server*: By default the clock will sync with `pool.ntp.org`, but you can configure your own (local) NTP server to be used.
- *Swap Blue/Green pins*: Some displays have the RGB order different, in this case RBG. You can use this options to change the order.
- *Display Bright*: Change the display bright.
- *Use 24h format*: You can choose between 20:00 or 8:00PM in your device.
- *Automatic Bright*: Once you connect a LDR in the ESP32, Clockwise will be able to control the display bright based on the ambient light. Check the [Wiki](
https://github.com/jnthas/clockwise/wiki/Connecting-the-LDR) about that.
- *NTP Server*: Configure your prefered NTP Server. You can use one of the [NTP Pool Project](https://www.ntppool.org/) pools or a local one. Default is `time.google.com`.
- *LDR Pin*: The ESP32 GPIO pin where the LDR is connected to. The default is 35. There is a link there where you can read the current value of LDR and test if it's working.
- *Posix Timezone String*: To avoid remote lookups of ezTime, provide a Posix string that corresponds to your timezone ([explanation](https://github.com/ropg/ezTime#timezones-1)). Leave empty to obtain this automatically from the server. 
- *Display Rotation*: Allows you to rotate the display. This is useful if you need to adjust the direction in which cables protrude relative to the displayed image.

## How to change the clockface (PlatformIO)

Clockwise uses PlatformIO as IDE, so the configuration is already done if you use the same. The Clockwise structure consists mainly of three folders
- clockfaces: contains the collection of available clockfaces. This folder is not included when compiling
- lib: contains the basic code for Clockwise to work and in addition a symbolic link to the current clockface
- src: contains the entry point for the clock code

```
.
├── clockfaces
│   ├── cw-cf-0x01
│   ├── cw-cf-0x02
│   └── cw-cf-0x03
├── lib
│   ├── cw-commons
│   ├── cw-gfx-engine
│   └── timeinwords -> ../clockfaces/cw-cf-0x02/
└── src
    └── main.cpp

```
Clone this repository and then run the following command to clone the clockface submodules 

``.../clockwise$ git submodule update --init firmware/clockfaces``

To create the symbolic link run the following command inside lib/ folder:

``.../clockwise/firmware/lib$ ln -s ../clockfaces/cw-cf-0x02/ timeinwords``

Or, if you prefer, you can get the same result by copying the desired clockface folder into lib/

The same way as web flashing, when connecting for the first time you will have to configure the wifi, follow the instructions in Configuring WiFi section above. 

## How to change the clockface (esp-idf)

You can use the [official Esspressif IoT Development Framekwork (aka esp-idf)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) to build and upload this project to an ESP32 device, including the [ESP32-Trinity board](https://esp32trinity.com/).

### Install esp-idf
Follow the [Step By Step installation instructions](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/#installation-step-by-step).

### Setup the environment variables
Follow the [instructions here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/#step-4-set-up-the-environment-variables).

### Clone and build this project
* `git clone --recurse-submodules https://github.com/jnthas/clockwise.git`
* `idf.py reconfigure`
* `idf.py menuconfig` (select `Clockwise Configuration` and choose the clockface)
* `idf.py flash`
* `idf.py monitor`
