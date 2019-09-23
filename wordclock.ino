//
/*
 Libraries:
  ESP8266WiFi, WifiUDP: https://github.com/ekstrand/ESP8266wifi
  Time, TimeLib:  https://github.com/PaulStoffregen/Time
  Timezone: https://github.com/JChristensen/Timezone
  NTPClient: https://github.com/arduino-libraries/NTPClient
*/

#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <String.h>
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

const char* ssid = "your ssid";       // insert your own ssid 
const char* password = "your passwd"; // and password

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

    public: 
    Word(int _from, int _to) {
      from = _from;
      to = _to;
    }
  
    void on() 
    { 
      for (int c=from; c<=to;c++) 
      {
        strip.SetPixelColor(c, red);
      }
    } 
}; 

Word uhr(8,10);
Word sechs(1,5);
Word zehn(13,16);
Word acht(17,20);
Word elf(22,24);
Word neun(25,28);
Word vier(29,32);
Word fuenf(33,36);
Word drei(39,42);
Word zwei(44, 47);
Word eins(46, 49);
Word sieben(49,54);
Word zwoelf(56, 60);
Word halb(62, 65);
Word nach(68, 71);
Word vor(72,74);
Word viertel(77,83);
Word dreiviertel(77, 87);
Word zehnm(88,91);
Word zwanzigm(92,98);
Word fuenfm(99, 102);
Word ist(104,106);
Word es(108, 109);
Word none(0,-1);

Word hours[] = {eins, zwei, drei, vier, fuenf, sechs, sieben, acht, neun, zehn, elf, zwoelf };
Word min5[13][3] = {
  { none, none, none},
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
    if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
    {
      updateSystemTimeFromNtp();   
    }
    else
    {
      WiFi.begin(ssid, password);
    }
  }
  diplayTimeOnWordClock();
  delay(60000); // One minute
}

void connectWifi() {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected to WiFi at ");
    Serial.print(WiFi.localIP());
    Serial.println();
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
