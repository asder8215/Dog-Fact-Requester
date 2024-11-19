/**************************************************************************
HTTP Request Example
Fetch a random color palette from colormind.io and draw the colors on the display

To fetch a new color, press either button on the LILYGO (GPIO 0 or 35)     
**************************************************************************/
#include "TFT_eSPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h>
#include <Arduino_JSON.h>

// TODO: replace with your own SSID & Password
const char* ssid = "Columbia University";
const char* password = "";

#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

volatile bool leftButtonPressed = false;
volatile bool rightButtonPressed = false;

TFT_eSPI tft = TFT_eSPI();

// Buffer size (adjust if necessary based on available memory)
#define IMAGE_BUFFER_SIZE 40000 // 40 KB buffer for JPEG data

// Buffer to hold the image
uint8_t imageBuffer[IMAGE_BUFFER_SIZE];

// Callback function for TJpg_Decoder
bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.height()) return 0; // Out of bounds
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void setup() {
  Serial.begin(115200);
  
  // setup our display
  tft.init();
  tft.setRotation(1);  // 1 = landscape, 2 = portrait
  tft.fillScreen(TFT_BLACK);

  Serial.print("display dimensions: ");
  Serial.print(tft.width());
  Serial.print(" x ");
  Serial.println(tft.height());

  // connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Send the HTTP POST request
  if (WiFi.status() == WL_CONNECTED) {
    fetchDogFact();
  } else {
    Serial.println("WiFi not connected");
  }

  // Initialize TJpg_Decoder
  TJpgDec.setCallback(tftOutput);
  TJpgDec.setJpgScale(1); // Scale factor (1 = full size)

  randomSeed(analogRead(0));

  // setup our buttons
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT), pressedLeftButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), pressedRightButton, FALLING);
}


// Function to wrap and draw text on the TFT display
void drawWrappedText(String text, int x, int y, int maxWidth) {
    int cursorX = x;          // Current x position
    int cursorY = y;          // Current y position
    int lineHeight = 20;      // Adjust based on font size
    String currentLine = "";  // Temporary buffer for the current line

    for (unsigned int i = 0; i < text.length(); i++) {
        char c = text[i];
        String tempLine = currentLine + c;

        // Check if the new line fits within the max width
        if (tft.textWidth(tempLine) > maxWidth) {
            // Draw the current line and move to the next line
            tft.drawString(currentLine, cursorX, cursorY);
            cursorY += lineHeight;

            // Start a new line with the current character
            currentLine = String(c);
        } else {
            // Add character to the current line
            currentLine = tempLine;
        }
    }

    // Draw the last line
    if (currentLine.length() > 0) {
        tft.drawString(currentLine, cursorX, cursorY);
    }
}

void fetchDogFact() {
    HTTPClient http;

    // dog api
    String url = "https://dogapi.dog/api/v2/facts?limit=1";
    url = url + String(random(129000, 130000));
    http.begin(url);

    // Set the content type to JSON
    http.addHeader("Content-Type", "application/json");

    // JSON data to send in the request
    // String jsonData = "accept: application/json";

    // Send the request
    int httpResponseCode = http.GET();

    // Check the response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
      JSONVar responseJSON = JSON.parse(response);

      if (JSON.typeof(responseJSON) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      Serial.println(responseJSON);
      String body = JSON.stringify(responseJSON["data"][0]["attributes"]["body"]);

      if (body) {
          Serial.println(F("Extracted body:"));
          Serial.println(body);
          // tft.drawString(body, 10, 10);
          // Draw wrapped text on the TFT
          drawWrappedText(body, 10, 10, tft.width() - 20); // Adjust margins if needed

      } else {
        Serial.println(F("Body field not found."));
        // tft.drawString("Body field not found", 10, 10);
        // Draw wrapped text on the TFT
        drawWrappedText(body, 10, 10, tft.width() - 20); // Adjust margins if needed
      }
    } else {
      Serial.println("Error on sending GET request");
      // tft.drawString("Failed to request dog fact", 10, 10);
      // Draw wrapped text on the TFT
      drawWrappedText("Failed to request dog fact", 10, 10, tft.width() - 20); // Adjust margins if needed
    }

    // Free resources
    http.end();
}


void pressedLeftButton() {
  leftButtonPressed = true;
}

void pressedRightButton() {
  rightButtonPressed = true;
}

void loop() {
  // fetch colors when either button is pressed
  if (leftButtonPressed) {
    tft.fillScreen(TFT_BLACK);
    fetchDogFact();
    leftButtonPressed = false;
  }
  if (rightButtonPressed) {
    tft.fillScreen(TFT_BLACK);
    fetchDogFact();
    rightButtonPressed = false;
  }
}
