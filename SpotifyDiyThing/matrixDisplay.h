#include "spotifyDisplay.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
// This is the library for interfacing with the display

// Can be installed from the library manager (Search for "ESP32 MATRIX DMA")
// https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

#include <JPEGDEC.h>
// Library for decoding Jpegs from the API responses
//
// Can be installed from the library manager (Search for "JPEGDEC")
// https://github.com/bitbank2/JPEGDEC

// ----------------------------
// Dependency Libraries - each one of these will need to be installed.
// ----------------------------

// Adafruit GFX library is a dependency for the matrix Library
// Can be installed from the library manager
// https://github.com/adafruit/Adafruit-GFX-Library

// -------------------------------------
// -------   Matrix Config   ------
// -------------------------------------

const int panelResX = 64;      // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 64;     // Number of pixels tall of each INDIVIDUAL panel module.
const int panel_chain = 1;      // Total number of panels chained one to another

// -------------------------------
// Putting this stuff outside the class because
// I can't easily pass member functions in as callbacks for jpegdec

// -------------------------------

MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);

JPEGDEC jpeg;

const char* ALBUM_ART = "/album.jpg";

// This next function will be called during decoding of the jpeg file to
// render each block to the Matrix.  If you use a different display
// you will need to adapt this function to suit.
int JPEGDraw(JPEGDRAW *pDraw)
{
  // Stop further decoding as image is running off bottom of screen
  if (  pDraw->y >= dma_display->height() ) return 1;

  dma_display->drawRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

fs::File myfile;

void * myOpen(const char *filename, int32_t *size) {
  myfile = SPIFFS.open(filename);
  *size = myfile.size();
  return &myfile;
}
void myClose(void *handle) {
  if (myfile) myfile.close();
}
int32_t myRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
int32_t mySeek(JPEGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

class MatrixDisplay: public SpotifyDisplay {
  public:

    void displaySetup(SpotifyArduino *spotifyObj) {

      spotify_display = spotifyObj;

      Serial.println("matrix display setup");
      setWidth(panelResX * panel_chain);
      setHeight(panelResY);

      setImageHeight(64);
      setImageWidth(64);

      HUB75_I2S_CFG mxconfig(
        panelResX,   // module width
        panelResY,   // module height
        panel_chain    // Chain length
      );

      // If you are using a 64x64 matrix you need to pass a value for the E pin
      // The trinity connects GPIO 18 to E.
      // This can be commented out for any smaller displays (but should work fine with it)
      mxconfig.gpio.e = 18;

      // May or may not be needed depending on your matrix
      // Example of what needing it looks like:
      // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
      mxconfig.clkphase = false;

      // Some matrix panels use different ICs for driving them and some of them have strange quirks.
      // If the display is not working right, try this.
      //mxconfig.driver = HUB75_I2S_CFG::FM6126A;

      dma_display = new MatrixPanel_I2S_DMA(mxconfig);
      dma_display->begin();
    }

    void showDefaultScreen() {
      dma_display->fillScreen(myBLACK);
    }

    void displayTrackProgress(long progress, long duration) {
      //Nothing to do for Matrix
    }

    void printCurrentlyPlayingToScreen(CurrentlyPlaying currentlyPlaying) {
      //Nothing to do for Matrix
    }

    void checkForInput() {
      //Nothing to do for Matrix
    }

    //Image Related
    void clearImage() {
      dma_display->fillScreen(myBLACK);
    }

    boolean processImageInfo(CurrentlyPlaying currentlyPlaying) {
      SpotifyImage currentlyPlayingSmallImage = currentlyPlaying.albumImages[currentlyPlaying.numImages - 1];
      if (!albumDisplayed || !isSameAlbum(currentlyPlayingSmallImage.url)) {
        // We have a differenent album than we currently have displayed
        albumDisplayed = false;
        setImageHeight(currentlyPlayingSmallImage.height);
        setImageWidth(currentlyPlayingSmallImage.width);
        setAlbumArtUrl(currentlyPlayingSmallImage.url);
        return true;
      }

      return false;
    }

    int displayImage() {
      int imageStatus = displayImageUsingFile(_albumArtUrl);
      Serial.print("imageStatus: ");
      Serial.println(imageStatus);
      if (imageStatus) {
        albumDisplayed = true;
        return imageStatus;
      }

      return imageStatus;
    }



    //NFC tag messages
    void markDisplayAsTagRead() {
      dma_display->drawRect(1, 1, dma_display->width() - 2, dma_display->height() - 2, dma_display->color444(0, 0, 255));
      dma_display->drawRect(2, 2, dma_display->width() - 4, dma_display->height() - 4, dma_display->color444(255, 0, 0));
    }
    void markDisplayAsTagWritten() {
      dma_display->drawRect(1, 1, dma_display->width() - 2, dma_display->height() - 2, dma_display->color444(255, 0, 255));
      dma_display->drawRect(2, 2, dma_display->width() - 4, dma_display->height() - 4, dma_display->color444(0, 255, 0));
    }

    void drawWifiManagerMessage(WiFiManager *myWiFiManager) {
      Serial.println("Entered Conf Mode");
      dma_display->fillScreen(myBLACK);
      dma_display->setTextSize(1);     // size 1 == 8 pixels high
      dma_display->setTextWrap(false);
      dma_display->setTextColor(myBLUE);
      dma_display->setCursor(0, 0);
      dma_display->print(myWiFiManager->getConfigPortalSSID());

      dma_display->setTextWrap(true);
      dma_display->setTextColor(myRED);
      dma_display->setCursor(0, 8);
      dma_display->print(WiFi.softAPIP());
    }

    void drawRefreshTokenMessage() {
      Serial.println("Refresh Token Mode");
      Serial.println("Entered Conf Mode");
      dma_display->fillScreen(myBLACK);
      dma_display->setTextSize(1);     // size 1 == 8 pixels high
      dma_display->setTextWrap(false);
      dma_display->setTextColor(myBLUE);
      dma_display->setCursor(0, 0);
      dma_display->print("Refresh Token");

      dma_display->setTextWrap(true);
      dma_display->setTextColor(myRED);
      dma_display->setCursor(0, 8);
      dma_display->print(WiFi.localIP());

    }

  private:

    int displayImageUsingFile(char *albumArtUrl)
    {

      // In this example I reuse the same filename
      // over and over, maybe saving the art using
      // the album URI as the name would be better
      // as you could save having to download them each
      // time, but this seems to work fine.
      if (SPIFFS.exists(ALBUM_ART) == true)
      {
        Serial.println("Removing existing image");
        SPIFFS.remove(ALBUM_ART);
      }

      fs::File f = SPIFFS.open(ALBUM_ART, "w+");
      if (!f)
      {
        Serial.println("file open failed");
        return -1;
      }

      // Spotify uses a different cert for the Image server, so we need to swap to that for the call
      client.setCACert(spotify_image_server_cert);
      bool gotImage = spotify_display->getImage(albumArtUrl, &f);

      // Swapping back to the main spotify cert
      client.setCACert(spotify_server_cert);

      // Make sure to close the file!
      f.close();

      if (gotImage)
      {
        return drawImagefromFile(ALBUM_ART);
      }
      else
      {
        return -2;
      }
    }

    int drawImagefromFile(const char *imageFileUri) {
      unsigned long lTime = millis();
      lTime = millis();
      jpeg.open((const char *) imageFileUri, myOpen, myClose, myRead, mySeek, JPEGDraw);
      int decodeStatus = jpeg.decode(0, 0, 0);
      jpeg.close();
      Serial.print("Time taken to decode and display Image (ms): ");
      Serial.println(millis() - lTime);

      return decodeStatus;
    }

};
