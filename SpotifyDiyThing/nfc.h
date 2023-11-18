#include <NfcAdapter.h>
#include <PN532/PN532/PN532.h>
#ifdef USE_I2C_NFC
#include <PN532/PN532_I2C/PN532_I2C.h>
#else
#include <PN532/PN532_SPI/PN532_SPI.h>
#endif
// Library for interfacing with the NFC Reader

// I modified the library to so it wouldn't lock if the NFC
// reader wasn't present

// Install from Github
// https://github.com/witnessmenow/Seeed_Arduino_NFC

// -------------------------------------
// -------   NFC Pin Config   ------
// -------------------------------------

// Using a MicroSD sniffer to connect a PN532 module

// SD -> NFC
// ------------
// DAT0 -> MISO
// CLK -> SCLK
// CMD -> MOSI
// CD -> CS

// If the module has a switch of jumpers to select the mode, make sure it's configured for SPI

// These should be the default SPI pins, but we'll call them out specficially
#if defined YELLOW_DISPLAY

#include "cheapYellowLCD.h"
#define NFC_SCLK 18
#define NFC_MISO 19
#define NFC_MOSI 23
#define NFC_SS 5

SPIClass nfcSpi = SPIClass(VSPI);

#elif defined MATRIX_DISPLAY

#define NFC_SCLK 33
#define NFC_MISO 32
#define NFC_MOSI 21
#define NFC_SS 22

SPIClass nfcSpi = SPIClass(HSPI);

#endif

PN532_SPI pn532spi(nfcSpi, NFC_SS);
NfcAdapter nfc = NfcAdapter(pn532spi);

SpotifyArduino *spotify_nfc;

unsigned long delayBetweenNfcReads = 200; // Time between NFC reads (.2 seconds)
unsigned long nfcDueTime;               //time when NFC read is due

bool forceUpdate = false;

bool nfcSetup(SpotifyArduino *spotifyObj) {

  spotify_nfc = spotifyObj;

#ifdef MATRIX_DISPLAY
  // matrix display uses custom pins so we need to specify them
  nfcSpi.begin(NFC_SCLK, NFC_MISO, NFC_MOSI, NFC_SS);
#endif
  nfc.begin();
  bool nfcStatus = !nfc.fail;
  if (nfcStatus) {
    // Could while loop here if you wanted
    //dma_display->print("NO!");
    Serial.println("NFC reader - OK!");
  } else {
    Serial.println("NFC reader - not working!!!");
    //dma_display->print("OK!");
  }

  return nfcStatus;
}

bool handleSpotifyUrl(char *tagContent) {
  char body[200];
  char contextUri[50];
  char trackUri[50];

  bool isTrack = false;

  //open.spotify.com/album/47lgREYotnsiuddvu6dXlk?si=F0r50tIETo-BffEB-HSpng&utm_source=copy-link&dl_branch=1
  // Note: the "https://" is stripped by the tag
  if (strncmp(tagContent, "open.spotify.com/album/", 23) == 0) {
    sprintf(contextUri, "spotify:album:%.*s", 22, tagContent + 23); // 22 is length of code, 23 is offset to get to the code (47lgREYotnsiuddvu6dXlk in above example)
  } else if (strncmp(tagContent, "open.spotify.com/playlist/", 26) == 0) {
    sprintf(contextUri, "spotify:playlist:%.*s", 22, tagContent + 26);
  } else if (strncmp(tagContent, "open.spotify.com/track/", 23) == 0) {
    isTrack = true;
    sprintf(trackUri, "spotify:track:%.*s", 22, tagContent + 23);
  } else {
    Serial.print("Unknown URL: ");
    Serial.println(tagContent);
    return false;
  }

  if (isTrack) {
    Serial.print("track: ");
    Serial.println(tagContent);
    sprintf(body, "{\"uris\" : [\"%s\"]}", trackUri);
  } else {
    Serial.print("context: ");
    Serial.println(tagContent);
    sprintf(body, "{\"context_uri\" : \"%s\"}", contextUri);
  }

  if (spotify_nfc->playAdvanced(body)) {
    Serial.println("done!");
    forceUpdate = true; // force it to update
    return true;
  }

  return false;
}

bool handleSpotifyUri(char *tagContent) {
  char body[200];

  // First let's check if we have a comma
  char *commaLocation = NULL;
  commaLocation = strchr (tagContent, ',');

  if (commaLocation != NULL) {
    // We have a comma, this means its a track with a context.

    uint8_t lengthOfString = strlen(tagContent);
    uint8_t contextIndex = commaLocation - tagContent + 1; //don't want the comma
    uint8_t contextLength = lengthOfString - contextIndex;
    char context[contextLength + 1];
    strncpy(context, commaLocation + 1, contextLength);
    context[contextLength] = '\0';

    Serial.print("context: ");
    Serial.println(context);

    uint8_t trackLength = commaLocation - tagContent;
    char track[trackLength + 1];
    strncpy(track, tagContent, trackLength);
    track[trackLength] = '\0';

    Serial.print("track: ");
    Serial.println(track);

    sprintf(body, "{\"context_uri\" : \"%s\", \"offset\": {\"uri\": \"%s\"}}", context, track);

  } else {
    char *isTrack = NULL;
    isTrack = strstr (tagContent, "track");
    if (isTrack) {
      Serial.print("track: ");
      Serial.println(tagContent);
      sprintf(body, "{\"uris\" : [\"%s\"]}", tagContent);
    } else {
      Serial.print("context: ");
      Serial.println(tagContent);
      sprintf(body, "{\"context_uri\" : \"%s\"}", tagContent);
    }
  }

  if (spotify_nfc->playAdvanced(body)) {
    Serial.println("done!");
    forceUpdate = true; // force it to update
    return true;
  }

  return false;
}

bool updateSpotify(char *tagContent) {

  if (strncmp(tagContent + 1, "open.spotify.com", 16) == 0) { // The +1 is cause first charcter indicated protocol (I think), not needed anyways
    return handleSpotifyUrl(tagContent + 1);
  } else if (strncmp(tagContent + 8, "open.spotify.com", 16) == 0) { // In case it's written as plain text, skipping the "https://"
    return handleSpotifyUrl(tagContent + 8);
  } else if (strncmp(tagContent, "spotify:", 8) == 0) {
    // Probably in the format: spotify:track:4mCsFkDzm6z8j0glKdE164
    return handleSpotifyUri(tagContent);
  }

  // Not reconginized format.
  // Should maybe flash a square or something
  //refreshArt = true;
  return false;
}

void markDisplayAsTagRead() {
  //dma_display->drawRect(1, 1, dma_display->width() - 2, dma_display->height() - 2, dma_display->color444(0, 0, 255));
  //dma_display->drawRect(2, 2, dma_display->width() - 4, dma_display->height() - 4, dma_display->color444(255, 0, 0));
}

void markDisplayAsTagWritten() {
  //dma_display->drawRect(1, 1, dma_display->width() - 2, dma_display->height() - 2, dma_display->color444(255, 0, 255));
  //dma_display->drawRect(2, 2, dma_display->width() - 4, dma_display->height() - 4, dma_display->color444(0, 255, 0));
}

bool handleTag() {
  NfcTag tag = nfc.read();

  bool writeTag = false;
  bool formatTag = false;
  Serial.println(tag.getTagType());
  Serial.print("UID: "); Serial.println(tag.getUidString());

  if (!tag.isFormatted) {
    writeTag = true;
    formatTag = true;
  } else if (tag.hasNdefMessage()) { // every tag won't have a message

    NdefMessage message = tag.getNdefMessage();
    Serial.print("\nThis NFC Tag contains an NDEF Message with ");
    Serial.print(message.getRecordCount());
    Serial.print(" NDEF Record");
    if (message.getRecordCount() != 1) {
      Serial.print("s");
    }
    Serial.println(".");

    // cycle through the records, printing some info from each
    int recordCount = message.getRecordCount();
    if (recordCount > 0) {
      for (int i = 0; i < recordCount; i++) {
        Serial.print("\nNDEF Record "); Serial.println(i + 1);
        NdefRecord record = message.getRecord(i);

        Serial.print("  TNF: "); Serial.println(record.getTnf());
        Serial.print("  Type: "); Serial.println(record.getType()); // will be "" for TNF_EMPTY

        // The TNF and Type should be used to determine how your application processes the payload
        // There's no generic processing for the payload, it's returned as a byte[]
        int payloadLength = record.getPayloadLength();
        if (payloadLength > 0) {
          byte payload[payloadLength];
          record.getPayload(payload);

          // Print the Hex and Printable Characters
          Serial.print("  Payload (HEX): ");
          PrintHexChar(payload, payloadLength);

          // id is probably blank and will return ""
          String uid = record.getId();
          if (uid != "") {
            Serial.print("  ID: "); Serial.println(uid);
          }

          // Force the data into a String (might work depending on the content)
          // Real code should use smarter processing
          char payloadAsString[payloadLength + 1];
          int numChars = 0;
          for (int c = 0; c < payloadLength; c++) {
            if ((char)payload[c] != '\0') {
              payloadAsString[numChars] = (char)payload[c];
              numChars++;
            }
          }

          payloadAsString[numChars] = '\0';
          markDisplayAsTagRead();
          //refreshArt = true; // update the art to remove the mark, even if the art doesnt change.
          Serial.print("  Payload (String): ");
          Serial.println(payloadAsString);
          return updateSpotify(payloadAsString);
        } else {
          //At least one of the records we had was not valid
          writeTag = true;
        }
      }
    } else {
      //Card has no records
      writeTag = true;
    }
  }

  if (formatTag && nfc.tagPresent()) {
    bool success = nfc.format();
    if (success) {
      SERIAL.println("\nSuccess, tag formatted as NDEF.");
    } else {
      SERIAL.println("\nFormat failed.");
    }
    delay(100);
  }

//  if (writeTag && nfc.tagPresent()) {
//
//    NdefMessage message = NdefMessage();
//    //This seems to be a blank card, lets write to it
//    NdefRecord r = NdefRecord();
//    r.setTnf(TNF_WELL_KNOWN);
//
//    String mimeType = "U";
//    byte type[mimeType.length() + 1];
//    mimeType.getBytes(type, sizeof(type));
//    r.setType(type, mimeType.length());
//
//    // One for new line, one for the 0x00 needed at the start
//    byte payloadBytes[currentTrackUri.length() + 2];
//    //Write to the new buffer offset by one
//    currentTrackUri.getBytes(&payloadBytes[1], currentTrackUri.length() + 1);
//    payloadBytes[0] = 0;
//
//    r.setPayload(payloadBytes, currentTrackUri.length() + 1);
//
//    message.addRecord(r);
//    boolean success = nfc.write(message);
//    if (success) {
//      markDisplayAsTagWritten();
//      Serial.println("Success. Try reading this tag with your phone.");
//    } else {
//      Serial.println("Write failed");
//    }
//
//    return true;
//  }

  return false;
}

bool nfcLoop() {
  forceUpdate = false;
  if (millis() > nfcDueTime)
  {
    if (nfc.tagPresent() && handleTag()) {
      Serial.println("Succesful Read - Back to loop:");
      nfcDueTime = millis() + 5000; // 5 second cool down on NFC tag if succesful
    } else {
      nfcDueTime = millis() + delayBetweenNfcReads;
      //Serial.println("Failed - Back to loop:");
    }
  }
  return forceUpdate;
}
