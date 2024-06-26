// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

#define TFT_GREY 0x2104 // Dark grey 16-bit colour

#include "Alert.h" // Out of range alert icon
#include "Noto_Sans_Bold_36.h"
#include "Final_Frontier_28.h"
#include "WiFiSetup.h"
#include "MqttSetup.h"

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

const char *AP_NAME = "G86-INFO-AP";
const char *WIFI_PASSWORD = "golf1986";
const char *PRIMARY_MQTT_CLIENT_NAME = "G86-TTF-INFO";
const char *SECONDARY_MQTT_CLIENT_NAME = "G86-TTF-INFO2";
const char *MQTT_TOPIC_BASE = "GOLF86";

const String MQTT_ECU_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/ECU/";
const String MQTT_GPS_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/GPS/";
const String MQTT_TIMER1_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/TM1/";
const String MQTT_TIMER2_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/TM2/";

const String MQTT_RPM_TOPIC = MQTT_ECU_TOPIC + "RPM";

char newMessage[128];

// Initialize WiFi and MQTT setup instances
WiFiSetup wifiSetup;
MqttSetup mqttSetup;

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height

uint32_t runTime = -99999; // time for next update

int reading = 0; // Value to be displayed
int d = 0;       // Variable used for the sinewave test waveform
bool range_error = 0;
int8_t ramp = 100;

void drawIcon(const unsigned short *icon, int16_t x, int16_t y, int8_t width, int8_t height);
float sineWave(int phase);
unsigned int rainbow(byte value);
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme);
void drawAlert(int x, int y, int side, bool draw);

void setup(void)
{
  tft.begin();
  Serial.begin(115200);

  // Initialize preferences for persistent storage
  wifiSetup.prefs.begin(PRIMARY_MQTT_CLIENT_NAME, false);

  // Initialize WiFi and MQTT setups
  wifiSetup.begin();
  mqttSetup.begin();
  // Serial.begin(9600);
  tft.setRotation(0);

  tft.fillScreen(TFT_BLACK);

  if (mqttSetup.mqtt.connect(PRIMARY_MQTT_CLIENT_NAME))
  {
    Serial.println("connected");
    mqttSetup.mqtt.subscribe(MQTT_RPM_TOPIC.c_str());
    Serial.println("\nSubscribed to primary topic: " + MQTT_RPM_TOPIC);
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(" try again in 5 seconds");
    delay(5000);
  }
}

void loop()
{

  mqttSetup.connect();

  // Set the the position, gap between meters, and inner radius of the meters
  int xpos = 480 / 2 - 230, ypos = 20, gap = 10, radius = 110;

  reading = atoi(newMessage);

  ringMeter(reading, 0, 7500, xpos, ypos, radius, " RPM", GREEN2RED); // Draw analogue meter
}

// #########################################################################
//  Draw the meter on the screen, returns x coord of righthand side
// #########################################################################
int ringMeter(int value, int vmin, int vmax, int x, int y, int r, const char *units, byte scheme)
{
  // Minimum value of r is about 52 before value text intrudes on ring
  // drawing the text first is an option

  x += r;
  y += r; // Calculate coords of centre of ring

  int w = r / 3; // Width of outer ring is 1/4 of radius

  int angle = 110; // Half the sweep angle of meter (300 degrees)

  int v = map(value, vmin, vmax, -angle, angle); // Map the value to an angle v

  byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
  byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;

  // Draw colour blocks every inc degrees
  for (int i = -angle + inc / 2; i < angle - inc / 2; i += inc)
  {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * 0.0174532925);
    float sy = sin((i - 90) * 0.0174532925);
    uint16_t x0 = sx * (r - w) + x;
    uint16_t y0 = sy * (r - w) + y;
    uint16_t x1 = sx * r + x;
    uint16_t y1 = sy * r + y;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + seg - 90) * 0.0174532925);
    float sy2 = sin((i + seg - 90) * 0.0174532925);
    int x2 = sx2 * (r - w) + x;
    int y2 = sy2 * (r - w) + y;
    int x3 = sx2 * r + x;
    int y3 = sy2 * r + y;

    if (i < v)
    { // Fill in coloured segments with 2 triangles
      switch (scheme)
      {
      case 0:
        colour = TFT_RED;
        break; // Fixed colour
      case 1:
        colour = TFT_GREEN;
        break; // Fixed colour
      case 2:
        colour = TFT_BLUE;
        break; // Fixed colour
      case 3:
        colour = rainbow(map(i, -angle, angle, 0, 127));
        break; // Full spectrum blue to red
      case 4:
        colour = rainbow(map(i, -angle, angle, 70, 127));
        break; // Green to red (high temperature etc.)
      case 5:
        colour = rainbow(map(i, -angle, angle, 127, 63));
        break; // Red to green (low battery etc.)
      default:
        colour = TFT_BLUE;
        break; // Fixed colour
      }
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
      // text_colour = colour; // Save the last colour drawn
    }
    else // Fill in blank segments
    {
      tft.fillTriangle(x0, y0, x1, y1, x2, y2, TFT_GREY);
      tft.fillTriangle(x1, y1, x2, y2, x3, y3, TFT_GREY);
    }
  }
  // Convert value to a string
  char buf[10];
  byte len = 3;
  if (value > 999)
    len = 5;
  dtostrf(value, len, 0, buf);
  buf[len] = ' ';
  buf[len + 1] = 0; // Add blanking space and terminator, helps to centre text too!
  // Set the text colour to default
  tft.setTextSize(5);
  tft.loadFont(NotoSansBold36);

  if (value < vmin || value > vmax)
  {
    drawAlert(x, y + 90, 50, 1);
  }
  else
  {
    drawAlert(x, y + 90, 50, 0);
  }

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Uncomment next line to set the text colour to the last segment value!
  tft.setTextColor(colour, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  // Print value, if the meter is large then use big font 8, othewise use 4
  if (r > 84)
  {
    tft.setTextPadding(40 * 3);   // Allow for 3 digits each 55 pixels wide
    tft.drawString(buf, x, y, 2); // Value in middle
  }
  else
  {
    tft.setTextPadding(3 * 16);   // Allow for 3 digits each 14 pixels wide
    tft.drawString(buf, x, y, 1); // Value in middle
  }
  tft.setTextSize(1);
  tft.setTextPadding(0);
  tft.loadFont(Final_Frontier_28);
  // Print units, if the meter is large then use big font 4, othewise use 2
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (r > 84)
    tft.drawString(units, x, y + 60, 6); // Units display
  else
    tft.drawString(units, x, y + 15, 4); // Units display

  // Calculate and return right hand side x coordinate
  return x + r;
}

void drawAlert(int x, int y, int side, bool draw)
{
  if (draw && !range_error)
  {
    drawIcon(alert, x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight);
    range_error = 1;
  }
  else if (!draw)
  {
    tft.fillRect(x - alertWidth / 2, y - alertHeight / 2, alertWidth, alertHeight, TFT_BLACK);
    range_error = 0;
  }
}

// #########################################################################
// Return a 16-bit rainbow colour
// #########################################################################
unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0;   // Red is the top 5 bits of a 16-bit colour value
  byte green = 0; // Green is the middle 6 bits
  byte blue = 0;  // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0)
  {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1)
  {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2)
  {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3)
  {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
float sineWave(int phase)
{
  return sin(phase * 0.0174532925);
}

//====================================================================================
// This is the function to draw the icon stored as an array in program memory (FLASH)
//====================================================================================

// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64

// Draw array "icon" of defined width and height at coordinate x,y
// Maximum icon size is 255x255 pixels to avoid integer overflow

void drawIcon(const unsigned short *icon, int16_t x, int16_t y, int8_t width, int8_t height)
{

  uint16_t pix_buffer[BUFF_SIZE]; // Pixel buffer (16 bits per pixel)

  tft.startWrite();

  // Set up a window the right size to stream pixels into
  tft.setAddrWindow(x, y, width, height);

  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++)
  {
    for (int j = 0; j < BUFF_SIZE; j++)
    {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np)
  {
    for (int i = 0; i < np; i++)
      pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }

  tft.endWrite();
}
