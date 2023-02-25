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
- [Makerfabs](https://www.makerfabs.com/sunton-esp32-2-8-inch-tft-with-touch.html)
    
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

