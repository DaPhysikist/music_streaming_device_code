#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define CLK 4
#define DT 2
#define SW 15

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t volume;
int currentStateCLK;
int lastStateCLK;
unsigned  long lastButtonPress = 0;
bool buttonState;

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

char title[64] = "";
char artist[64] = "";
char album[64] = "";

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
  // Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);

  switch (data1) {
    case 0x01:
      strncpy(title, (char *)data2, sizeof(title) - 1);
      title[sizeof(title) - 1] = '\0'; // Ensure null termination
      // Serial.printf("Title: %s\n", title);
      break;
    case 0x02:
      strncpy(artist, (char *)data2, sizeof(artist) - 1);
      artist[sizeof(artist) - 1] = '\0';
      // Serial.printf("Artist: %s\n", artist);
      break;
    case 0x04:
      strncpy(album, (char *)data2, sizeof(album) - 1);
      album[sizeof(album) - 1] = '\0';
      // Serial.printf("Album: %s\n", album);
      break;
    default:
      break;
  }

  // Update OLED display
  display.clearDisplay();                                       
  display.setTextSize(1); // Smaller text to fit all info
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print(F("Title: "));
  display.println(title);

  display.print( F("Artist: "));
  display.println(artist);

  display.print(F("Album: "));
  display.println(album);

  display.display();
}

void setup() {
    Serial.begin(115200);

    pinMode (CLK,INPUT);
    pinMode (DT,INPUT);
    pinMode(SW, INPUT_PULLUP);
    lastStateCLK = digitalRead(CLK);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();

    auto cfg = i2s.defaultConfig();
    cfg.pin_bck = 18;
    cfg.pin_ws = 5;
    cfg.pin_data = 19;
    i2s.begin(cfg);

    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.start("MyMusic");
}

void loop() {
  currentStateCLK = digitalRead(CLK); //CLK current value
  
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){  //CLK change  && CLK only 1state change
		if (digitalRead(DT) != currentStateCLK) { //Encoder  CCW Rotation
     if (volume < 127){
      volume++;
     }
      Serial.println("Volume Up");
      delay(2);
		} else {			// Encoder CW Rotation
      if (volume > 0){
        volume--;
      }
      Serial.println("Volume Down");
      delay(2);
		}
	}

	lastStateCLK  = currentStateCLK; // Last CLK Value

  int btnState = digitalRead(SW);  // Button Value
  if (btnState == LOW) { // Switch pushed
    if  (millis() - lastButtonPress > 1000) { // Over 50ms
      Serial.println("Button press");
      buttonState = !buttonState;

      if(buttonState){
        a2dp_sink.pause();
      }
      else {
        a2dp_sink.play();
      }

      lastButtonPress = millis();
    }
  }

  a2dp_sink.set_volume(volume);
}
