//
/*
 Libraries:
  ESP8266WiFi, WifiUDP: https://github.com/ekstrand/ESP8266wifi
  Time, TimeLib:  https://github.com/PaulStoffregen/Time
  Timezone: https://github.com/JChristensen/Timezone
  NTPClient: https://github.com/arduino-libraries/NTPClient
*/
#include <String.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <NeoPixelBus.h>

// Define NTP properties
#define NTP_OFFSET   0      // s
#define NTP_INTERVAL 60 * 1000 // ms
#define NTP_ADDRESS  "europe.pool.ntp.org"  // see ntp.org for ntp pools

// Set up the NTP UDP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

const uint16_t PixelCount = 110;

// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
const uint8_t PixelPin = 2;

#define colorSaturation 128

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

HslColor hslRed(red);
HslColor hslGreen(green);
HslColor hslBlue(blue);
HslColor hslWhite(white);
HslColor hslBlack(black);

class Word 
{ 
    int from;
    int to;
    String word;

    public: 
    Word(int _from, String _word) {
      from = _from;
      to = _from + _word.length() -1;
      word = _word;
    }
  
    void on() 
    { 
      for (int c=from; c<=to;c++) 
      {
        strip.SetPixelColor(c, red);
      }
      Serial.print(word);
      Serial.print(" ");
    } 
}; 

Word uhr(8,"Uhr");
Word sechs(1,"sechs");
Word zehn(13, "zehn");
Word acht(17, "acht");
Word elf(22, "elf");
Word neun(25, "neun");
Word vier(29, "vier");
Word fuenf(33, "fünf");
Word drei(39, "drei");
Word zwei(44, "zwei");
Word eins(46, "eins");
Word sieben(49, "sieben");
Word zwoelf(56, "zwölf");
Word halb(62, "halb");
Word nach(68, "nach");
Word vor(72, "vor");
Word viertel(77, "viertel");
Word dreiviertel(77, "dreiviertel");
Word zehnm(88, "zehn");
Word zwanzigm(92, "zwanzig");
Word fuenfm(99, "fünf");
Word ist(104, "ist");
Word es(108, "es");
Word none(0, "");

Word hours[] = {eins, zwei, drei, vier, fuenf, sechs, sieben, acht, neun, zehn, elf, zwoelf };
Word min5[13][3] = {
  { uhr, none, none},
  { fuenfm, nach, none },
  { zehnm, nach, none },
  { viertel, nach, none },
  { zwanzigm, nach, none },
  { fuenfm, vor, halb },
  { halb, none, none },
  { fuenfm, nach, halb },
  { zwanzigm, vor, none },
  { dreiviertel, none, none },
  { zehnm, vor, none },
  { fuenfm, vor, none },
  { none, none, none},
};

int minutesSinceLastUpdate;

void setup()
{
    Serial.begin(115200);
    
    strip.Begin();
    strip.ClearTo(black);
    strip.Show();
    timeClient.begin();

    connectWifi();
    updateSystemTimeFromNtp();
}

void loop()
{
  minutesSinceLastUpdate ++;
  if (minutesSinceLastUpdate > 10) {
    updateSystemTimeFromNtp();   
  }
  diplayTimeOnWordClock();
  delay(60000); // One minute
}

void connectWifi() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.autoConnect("Wordclock");
}

void diplayTimeOnWordClock() {
  time_t local = now();
  int curHour = hourFormat12(local);
  int curMinute = minute(local);
  Serial.print(curHour);
  Serial.print(":");
  Serial.println(curMinute);

  int minute5 = (curMinute  + 2) / 5;
  strip.ClearTo(black);
  es.on();
  ist.on();
  Word minuteWords[3] = min5[minute5];
  for (int c=0; c < 3; c++)
  {
    minuteWords[c].on();
  }
  int displayHour = minute5 >= 5 ? curHour + 1 : curHour;
  hours[displayHour-1].on();
  strip.Show();
  Serial.println();
}

void updateSystemTimeFromNtp() {
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();
    time_t utc = epochTime;

    // Central European Time (Frankfurt, Paris)
    TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
    TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
    Timezone CE(CEST, CET);
    time_t local = CE.toLocal(utc);
    setTime(local);
    minutesSinceLastUpdate = 0;
}
