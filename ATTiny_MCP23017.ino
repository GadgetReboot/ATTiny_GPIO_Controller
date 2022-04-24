// MCP23017 GPIO Expander Demo
//
// Target Hardware:  ATTiny1604 (using megaTinyCore board support), MCP23017 (I2C version)
//
// Arduino library pin numbers to use for MCP23017 input/output pins
// mcp23017 pin    pin name       library pin #
//     21            GPA0             0
//     22            GPA1             1
//     23            GPA2             2
//     24            GPA3             3
//     25            GPA4             4
//     26            GPA5             5
//     27            GPA6             6
//     28            GPA7             7
//      1            GPB0             8
//      2            GPB1             9
//      3            GPB2             10
//      4            GPB3             11
//      5            GPB4             12
//      6            GPB5             13
//      7            GPB6             14
//      8            GPB7             15
//
// Tested with Arduino IDE 1.8.13
// Gadget Reboot
// https://www.youtube.com/gadgetreboot


#include <Adafruit_MCP23X17.h>

#define addr 0x20     // mcp23017 address

// map switches and leds to gpio pin numbers used in library
#define sw1     0   // GPA0 button
#define sw2     1   // GPA1 button
#define sw3     2   // GPA2 button
#define sw4     3   // GPA3 button
#define sw5     4   // GPA4 button
#define sw6     5   // GPA5 button
#define sw7     6   // GPA6 button
#define sw8     7   // GPA7 button

#define led1    8   // GPB0 led
#define led2    9   // GPB1 led
#define led3    10  // GPB2 led
#define led4    11  // GPB3 led
#define led5    12  // GPB4 led
#define led6    13  // GPB5 led
#define led7    14  // GPB6 led
#define led8    15  // GPB7 led

Adafruit_MCP23X17 mcp;  // MCP23017 object to communicate with

// OLED display *******************************************
// graphic demo from http://www.technoblogy.com/show?2CFT
// SSD1304 128x64 connected to I2C header
int const address = 0x3C;
int const commands = 0x00;
int const onecommand = 0x80;
int const data = 0x40;
int const onedata = 0xC0;

// Init sequence for OLED module
int const InitLen = 10;
const uint8_t Init[InitLen] PROGMEM = {
  0x8D, // Charge pump
  0x14,
  0x20, // Memory mode
  0x01, // Vertical addressing
  0xA1, // Flip horizontally
  0x81, // Set contrast
  0xCF, // brighter
  0xDB, // Set vcom detect
  0x40, // brighter
  0xAF  // Display on
};

void setup() {
  Serial.begin(9600);
  Serial.println("ATTiny mcp23017and OLED demo");

  // init device
  if (!mcp.begin_I2C(addr)) {
    Serial.println("Can't initialize mcp23017");
    while (1);
  }

  // configure led pins for output
  mcp.pinMode(led1, OUTPUT);
  mcp.pinMode(led2, OUTPUT);
  mcp.pinMode(led3, OUTPUT);
  mcp.pinMode(led4, OUTPUT);
  mcp.pinMode(led5, OUTPUT);
  mcp.pinMode(led6, OUTPUT);
  mcp.pinMode(led7, OUTPUT);
  mcp.pinMode(led8, OUTPUT);

  // configure button pins for input with pull up
  mcp.pinMode(sw1, INPUT_PULLUP);
  mcp.pinMode(sw2, INPUT_PULLUP);
  mcp.pinMode(sw3, INPUT_PULLUP);
  mcp.pinMode(sw4, INPUT_PULLUP);
  mcp.pinMode(sw5, INPUT_PULLUP);
  mcp.pinMode(sw6, INPUT_PULLUP);
  mcp.pinMode(sw7, INPUT_PULLUP);
  mcp.pinMode(sw8, INPUT_PULLUP);

  // turn off leds as an entire port at once
  mcp.writeGPIO(0x00, 1);  // set all of port B to "0"

  // display led pattern scanning back and forth a few times
  scanLeds();
  scanLeds();
  scanLeds();

  // turn off leds as an entire port at once
  mcp.writeGPIO(0x00, 1);  // set all of port B to "0"

  // OLED init
  InitDisplay();
  drawDemo1();   // plot a demo curve
}

void loop() {

  mcp.digitalWrite(led1, !mcp.digitalRead(sw1));  // led1 is on when sw1 is pressed, off when released
  mcp.digitalWrite(led2, !mcp.digitalRead(sw2));  // led2 is on when sw2 is pressed, off when released
  mcp.digitalWrite(led3, !mcp.digitalRead(sw3));  // led3 is on when sw3 is pressed, off when released
  mcp.digitalWrite(led4, !mcp.digitalRead(sw4));  // led4 is on when sw4 is pressed, off when released

  mcp.digitalWrite(led5, !mcp.digitalRead(sw5));  // led5 is on when sw5 is pressed, off when released
  mcp.digitalWrite(led6, !mcp.digitalRead(sw6));  // led6 is on when sw6 is pressed, off when released
  mcp.digitalWrite(led7, !mcp.digitalRead(sw7));  // led7 is on when sw7 is pressed, off when released

  // force all leds on for 1 second and then off, if sw8 is pressed
  if (!mcp.digitalRead(sw8)) {
    mcp.writeGPIO(0xff, 1);  // turn on all leds (write 0xff to port b)
    drawDemo2();             // plot an alternate graph on the OLED
    delay (1000);            // wait 1 second
    mcp.writeGPIO(0x00, 1);  // turn off all leds (write 0x00 to port b)
    drawDemo1();             // plot original demo graph on OLED
  }
}

// turn on leds sequentially, scanning back and forth once
void scanLeds() {
  byte leds = 0b10000000;    // one led turned on, seven turned off
  for (int i = 0; i < 8; i++) {
    mcp.writeGPIO(leds, 1);  // write byte to port A =0 B =1
    leds = leds >> 1;        // shift on-led by one position
    delay(60);
  }
  leds = 0b00000001;
  for (int i = 0; i < 8; i++) {
    mcp.writeGPIO(leds, 1);  // write byte to port A =0 B =1
    leds = leds << 1;        // shift on-led by one position
    delay(60);
  }
}

// OLED functions from http://www.technoblogy.com/show?2CFT
void InitDisplay () {
  Wire.beginTransmission(address);
  Wire.write(commands);
  for (uint8_t i = 0; i < InitLen; i++) Wire.write(pgm_read_byte(&Init[i]));
  Wire.endTransmission();
}

// Plot point (SSD1306): x (0 to 127), y (0 to 63), mode (0 = point, 1 = histogram)
void PlotPoint (int x, int y, int mode) {
  Wire.beginTransmission(address);
  Wire.write(commands);
  Wire.write(0x21); Wire.write(x); Wire.write(x);  // Column range
  Wire.write(0x22); Wire.write(0); Wire.write(7);  // Page range
  Wire.endTransmission();
  Wire.beginTransmission(address);
  Wire.write(data);
  for (int i = 0; i < 8; i++) {
    if (y > 7) Wire.write(- mode);
    else if (y < 0) Wire.write(0);
    else Wire.write((1 << y) - mode);
    y = y - 8;
  }
  Wire.endTransmission();
}

void ClearDisplay () {
  for (int x = 0; x < 128; x++) PlotPoint(x, 0, 1);
}

// OLED graph demo plots

// Gaussian approximation
int e (int x, int f, int m) {
  return (f * 256) / (256 + ((x - m) * (x - m)));
}
void drawDemo1 () {
  for (int x = 0; x < 128; x++) {
    int y = e(x, 40, 24) + e(x, 68, 64) + e(x, 30, 104) - 14;
    PlotPoint(x, y, 0);
  }
}
void drawDemo2 () {
  for (int x = 0; x < 128; x++) {
    int y = e(x, 30, 14) + e(x, 58, 44) + e(x, 15, 84) - 14;
    PlotPoint(x, y, 0);
  }
}


/*
     //mcp23017 port read/write examples
     //read port A or port B into a byte
     byte test = mcp.readGPIO(0); // read port A =0 B =1

     //write a byte to port A or port B
     mcp.writeGPIO(test, 1);  // write byte to port A =0 B =1

     // read or write both ports as 16 bits all at once
     uint16_t data16 = mcp.readGPIOAB();  // read both ports into uint16_t
     mcp.writeGPIOAB(data16);             // write both ports from uint16_t
*/
