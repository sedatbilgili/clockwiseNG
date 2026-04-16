## ClockwiseNG

ClockwiseNG is a next-generation fork of the original [Clockwise](https://github.com/jnthas/clockwise) project.
This repository builds on the upstream work and will gradually document the changes, goals, and additions introduced in this fork.



## Changelog

Changelog is available at: https://github.com/sedatbilgili/clockwiseNG/blob/main/CHANGELOG.md



## Driving the led matrix

The three main hardware components of Clockwise are: 
- HUB75/HUB75E compatible LED matrix 64x64
- an ESP32; and 
- a power supply of 3A or more

Note: The HUB75 panels used in this fork were purchased from AliExpress. However, since the pin configurations were found to vary across different panels, multiple configurations (hop, abp, acp, oap) have been created for these panels.

For HUB75 connection / hardware details: https://github.com/jnthas/clockwise

## Clockfaces

This fork may affect clockface switching compatibility. It has not been tested with any clockface other than the default.

## How to change the clockface (web flashing)



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

