#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#define I2S_DOUT     15
#define I2S_BCLK      16
#define I2S_LRC        17
Audio audio;
String ssid =    "mirkwood";
String password = "eyeofsauron";
void setup() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED)
  delay(1500);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(100);
  audio.connecttohost("http://vis.media-ice.musicradio.com/CapitalMP3");
}
void loop()
{
  audio.loop();
}