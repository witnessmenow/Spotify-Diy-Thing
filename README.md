# Spotify-Diy-Thing

Something similar to the [Spotify Car Thing](https://carthing.spotify.com/), built with a cheap ESP32 Screen. Connects to your Spotify account and displays your currently playing song with its album art

![image](https://user-images.githubusercontent.com/1562562/221344692-7dd359d3-2e64-4a09-850b-b619477c5043.png)

This project is a Work in Progress!

## Help Support what I do!

[If you enjoy my work, please consider becoming a Github sponsor!](https://github.com/sponsors/witnessmenow/)

## Hardware Required

This project is designed to make use of basically ready to go hardware, so is very easy to get up and running

Currently this project runs on two types of hardware:

### "Cheap Yellow Display" (CYD)

An ESP32 With Built in 320x240 LCD with Touch Screen (ESP32-2432S028R), buy from wherever works out cheapest for you:

- [Aliexpress\*](https://s.click.aliexpress.com/e/_DkSpIjB)
- [Aliexpress\*](https://s.click.aliexpress.com/e/_DkcmuCh)
- [Aliexpress](https://www.aliexpress.com/item/1005004502250619.htm)
- [Makerfabs](https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html)

### Matrix panel (ESP32 Trinity)

It's built to work with the [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity), an open source board I created for controlling Hub75 Matrix panels, but it will does work with any ESP32 that breaks out enough pins.

The display it uses is a 64x64 HUB75 Matrix Panel.

All the parts can be purchased from Makerfabs.com:

- [ESP32 Trinity](https://www.makerfabs.com/esp32-trinity.html)
- [64 x 64 Matrix Panel](https://www.makerfabs.com/64x64-rgb-led-matrix-3mm-pitch.html)
- Optional: [5V Power Supply](https://www.makerfabs.com/5v-6a-ac-dc-power-adapter-with-cable.html) - You can alternatively use a USB-C power supply

\* = Affilate Link

### BYOD (Bring your own display)

I've tried to design this project to be modular and have abstracted the display code behind an interface, so it should be pretty easy to get it up and running with a different type of display.

## NFC Tags (Optional)

One of the coolest parts about this project, in my opinion at least, is the ability to connect an NFC reader to control what songs/albums/playlists are being played.

You can write the spotify URI or URL of the song, album or playlist to an NFC tag and when you swipe it off the reader, the device will tell your spotify account to play what it reads.

If you aren't interested in this feature, you can just not connect it and the device will work without it.

### PN532 NFC reader and Tags

This code has been tested with these red PN532 NFC readers and cheap NFC stickers.

To use the PN532 as an SPI device, you need to configure the toggle switches so switch 1 is down and 2 is up. (You may need to remove the sticker on top of it)

#### Links

- [PN532 NFC reader - Aliexpress\*](https://s.click.aliexpress.com/e/_DCanbAB)
- [NFC Stickers - Aliexpress\*](https://s.click.aliexpress.com/e/_DkX2F5z)

### Hardware support

#### "Cheap Yellow Display" (CYD)

The CYD does not have enough free pins to use an SPI device by default, and the NFC reader is quite slow over i2c, so we need to get creative.

With the help of an "SD Card Sniffer", we can make use of the Micro SD slot of the CYD to connect the NFC reader to.

##### Connections

| Sniffer Board Label | ESP32 Pin | PN532 NFC |
| ------------------- | --------- | --------- |
| DAT2                | -         | -         |
| CD                  | IO5       | CS        |
| CMD                 | IO23      | DI / MOSI |
| GND                 | GND       | GND       |
| VCC                 | 3.3V      | VCC       |
| CLK                 | IO18      | SCLK      |
| DAT0                | IO19      | DO / MISO |
| DAT1                | -         | -         |

#### Links

- [Micro SD Card Sniffer - Aliexpress\*](https://s.click.aliexpress.com/e/_Ddwcy9h)

#### Matrix Panel (ESP32 Trinity)

Again, it is designed for the ESP32 Trinity, but can work with any ESP32 that breaks out the required pins.

The Trinity has some pins broken out that can be used for this purpose

#### Connections

| ESP32 Pin  | PN532 NFC |
| ---------- | --------- |
| IO22 (SCL) | CS        |
| IO21 (SDA) | DI / MOSI |
| GND        | GND       |
| 3.3V       | VCC       |
| IO33       | SCLK      |
| IO32       | DO / MISO |

## Project Setup

These steps only need to be run once.

### Step 1 - Spotify Dev Account

In order to use this project, you will need to create an application on the Spotify Developer Dashboard

- Sign into the [Spotify Developer page](https://developer.spotify.com/dashboard/login)
- Create a new application. (name it whatever you want). There is a section fo "callback URI" you can just make this "locahost" for now.
- You will need the "client ID" and "client secret" from this page later in the steps

You will need to add a callback URI for authentication process by clicking "Edit Settings", what URI to use will be displayed on screen in a later step.

### Step 2 - Flash the Project

Flash the project directly from your browser [here](https://witnessmenow.github.io/Spotify-Diy-Thing) (Chrome & Edge only)

or

Jump to the "code" section below

### Step 3 - Adding your Wifi and Spotify Details

The device is now flashed with the code, but it doesn't know what your Wifi or Spotify details are.

In order to enter your wifi details, the project will host it's own wifi network. Connect to it using your phone.

- SSID: SpotifyDiy
- Password: thing123

You should be automatically redirected to the config page.

- Click Config
- Enter your WIfi details (2.4Ghz only)
- Enter the client Id and client secret from the earlier step
- You can leave refresh token blank
- Click save

Note: If you ever need to get back into this config section, click reset button (the button closest to the side) twice.

### Step 4 - Authenticating the device with your Spotify account

The final step is to connect this device to your Spotify account. When the Wifi is configured correctly, it will enter "Refresh Token Mode".

- Your device will now be connected to the Wifi details you gave it.
- Go to the address displayed on screen using your phone or PC that is connected to the same network as your device.
- On the webpage that loads there will be an address displayed in bold, add this to the callback URI section as mentioned in the Spotify API section
- Click the `Spotify Auth` URL
- You will need to give permission to the app you created to have access to your spotify account

Your project should now be setup and will start displaying your currently playing music!

## Code

If you want to program this project manually, there are two options

### PlatformIO

PlatformIO is the easiest way to code this project.

In the [platformio.ini](platformio.ini), there are several environments defined for the different boards

| Environment | Description                                                                                                                  |
| ----------- | ---------------------------------------------------------------------------------------------------------------------------- |
| env:cyd     | For the [Cheap Yellow Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)                                   |
| env:cyd2usb | For the Cheap Yellow Display with two USB ports                                                                              |
| env:trinity | For the [ESP32 Trinity](https://github.com/witnessmenow/ESP32-Trinity) (or generic ESP32 wired to the matrix panel the same) |

When you select the environment, it will automatically install the right libraries and set the configurations in the code.

### Arduino IDE

If you want to use the Arduino IDE, you will need to do the following to get it working

The following libraries need to be installed for this project to work:

| Library Name/Link                                                                | Purpose                                    | Library manager                 |
| -------------------------------------------------------------------------------- | ------------------------------------------ | ------------------------------- |
| [SpotifyArduino](https://github.com/witnessmenow/spotify-api-arduino)            | For interacting with Spotify API           | No                              |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson)                          | Dependancy of the Spotify API              | Yes ("Arduino Json")            |
| [JPEGDEC](https://github.com/bitbank2/JPEGDEC)                                   | For decoding the album art images          | Yes ("JPEGDEC")                 |
| [WifiManager - By Tzapu](https://github.com/tzapu/WiFiManager)                   | Captive portal for configuring the WiFi    | Yes ("WifiManager")             |
| [ESP_DoubleResetDetector](https://github.com/khoih-prog/ESP_DoubleResetDetector) | Detecting double pressing the reset button | Yes ("ESP_DoubleResetDetector") |
| [Seeed_Arduino_NFC](https://github.com/witnessmenow/Seeed_Arduino_NFC)           | For the NFC reader                         | No (it's a modified version)    |

#### Cheap Yellow Display Specific libraries

| Library Name/Link                              | Purpose                         | Library manager  |
| ---------------------------------------------- | ------------------------------- | ---------------- |
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | For controlling the LCD Display | Yes ("tft_espi") |

#### Matrix Panel Specific libraries

| Library Name/Link                                                                                 | Purpose                          | Library manager          |
| ------------------------------------------------------------------------------------------------- | -------------------------------- | ------------------------ |
| [ESP32-HUB75-MatrixPanel-I2S-DMA](https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA) | For controlling the LED Matrix   | Yes ("ESP32 MATRIX DMA") |
| [Adafruit GFX library](https://github.com/adafruit/Adafruit-GFX-Library)                          | Dependency of the Matrix library | Yes ("Adafruit GFX")     |

#### Cheap Yellow Display Display Config

The CYD version of the project makes use of [TFT_eSPI library by Bodmer](https://github.com/Bodmer/TFT_eSPI).

TFT_eSPI is configured using a "User_Setup.h" file in the library folder, you will need to replace this file with the one in the `DisplayConfig` folder of this repo.

#### Display Selection

At the top of the `SpotifyDiyThing.ino` file, there is a section labeled "Display Type", follow the instructions there for how to enable the different displays.

Note: By default it will use the Cheap Yellow Display
