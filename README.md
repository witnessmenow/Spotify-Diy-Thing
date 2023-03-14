# Spotify-Diy-Thing
Something similar to the [Spotify Car Thing](https://carthing.spotify.com/), built with a cheap ESP32 Screen. Connects to your Spotify account and displays your currently playing song with its album art

![image](https://user-images.githubusercontent.com/1562562/221344692-7dd359d3-2e64-4a09-850b-b619477c5043.png)

This project is a Work in Progress!

## Help Support what I do!

[If you enjoy my work, please consider becoming a Github sponsor!](https://github.com/sponsors/witnessmenow/)

## Hardware Required

ESP32 With Built in 320x240 LCD with Touch Screen (ESP32-2432S028R), buy from wherever works out cheapest for you:
- [Aliexpress*](https://s.click.aliexpress.com/e/_DkSpIjB)
- [Aliexpress*](https://s.click.aliexpress.com/e/_DkcmuCh)
- [Aliexpress](https://www.aliexpress.com/item/1005004502250619.htm)
- [Makerfabs](https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html) - Makerfabs also sell what is required to make a [similar project with an RGB Matrix panel](https://github.com/witnessmenow/Spotify-NFC-Matrix-Display/blob/master/README.md)
    
 * = Affilate Link
 
 ### Alternate Hardware

There is nothing special about this particaulr display other than it is cheap! This project could be adapted to run on any ESP32 based display. If the display has a different resolution than 320x240 it will need to be modified.

## Software

The following libraries need to be installed for this project to work:

| Library Name/Link                                                                                 | Purpose                                     | Library manager                  |
| ------------------------------------------------------------------------------------------------- | ------------------------------------------- | ------------------------ |
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)                                                    | For controlling the LCD Display             | Yes ("tft_espi")             |
| [SpotifyArduino](https://github.com/witnessmenow/spotify-api-arduino)                             | For interacting with Spotify API            | No                                |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson)                                           | Dependancy of the Spotify API               | Yes ("Arduino Json")         |
| [JPEGDEC](https://github.com/bitbank2/JPEGDEC)                                                    | For decoding the album art images           | Yes ("JPEGDEC")              |
| [XPT2046 Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen)                      | For handling the touch screen               | Yes ("XPT2046")              |
| [WifiManager - By Tzapu](https://github.com/tzapu/WiFiManager)                                    | Captive portal for configuring the WiFi     | Yes ("WifiManager")              |
| [ESP_DoubleResetDetectorhttps://github.com/khoih-prog/ESP_DoubleResetDetector)                    | Detecting double pressing the reset button  | Yes ("ESP_DoubleResetDetector")              |

### Display Config

This project makes use of [TFT_eSPI library by Bodmer](https://github.com/Bodmer/TFT_eSPI).

TFT_eSPI is configured using a "User_Setup.h" file in the library folder, you will need to replace this file with the one in the `Display Config` folder of this repo.

## Project Setup

These steps only need to be run once.

### Spotify API

In order to use this project, you will need to create an application on the Spotify Developer Dashboard

- Sign into the [Spotify Developer page](https://developer.spotify.com/dashboard/login)
- Create a new application. (name it whatever you want)
- You will need the "client ID" and "client secret" from this page later in the steps
- You will also need to add a callback URI for authentication process by clicking "Edit Settings", what URI to use will be displayed on screen in a later step.

### Flash the Project

Flash the project directly from your browser [here](https://witnessmenow.github.io/Spotify-Diy-Thing/WebFlash/) (Chrome & Edge only)

or 

Install from the Arduino IDE, no changes are needed to the code

### WiFiManager

In order to enter your wifi details, the project will host it's own wifi network. Connect to it using your phone.
- SSID: SpotifyDiy
- Password: thing123

You should be automatically redirected to the config page. 
- Click Config
- Enter your WIfi details
- Enter the client Id and client secret from the earlier step
- You can leave refresh token blank
- Click save

Note: If you ever need to get back into WiFiManager, click reset button (the button closest to the side) twice.

### Authenticating the device with your Spotify account

The final step is to connect this device to your Spotify account. When the Wifi is configured correctly, it will enter "Refresh Token Mode".

- Go to the displayed address using your phone or PC
- Add the address displayed in bold to the callback URI section as mentioned in the Spotify API section
- Click the `Spotify Auth` URL
- You will need to give permision to the app you created to have access to your spotify account

Your project should now be setup and will start displaying your currently playing music!
