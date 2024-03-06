#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>

#define BEGIN_X 0   // Begin position of plot on x-axis
#define SCREEN_WIDTH 128                // OLED display width
#define SCREEN_HEIGHT 64                // OLED display height
#define REC_LENG 200                    // size of wave data buffer
#define MIN_TRIG_SWING 5                // minimum trigger swing.(Display "Unsync" if swing smaller than this value

#define HOLD_BUTTON_PIN 2
#define Y_SCALE_BUTTON_PIN 10
#define X_SCALE_BUTTON_PIN 11
OneButton holdBtn(HOLD_BUTTON_PIN, true);
OneButton yScaleBtn(Y_SCALE_BUTTON_PIN, true);
OneButton xScaleBtn(X_SCALE_BUTTON_PIN, true);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1      // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);   // device name is oled

// Range name table (those are stored in flash memory)
#define VRANGE_MAX 10
const char vRangeName[10][5] PROGMEM = {"A50V", "A 5V", " 50V", " 20V", " 10V", "  5V", "  2V", "  1V", "0.5V", "0.2V"}; // Vertical display character (number of characters including \ 0 is required)
const char * const vstring_table[] PROGMEM = {vRangeName[0], vRangeName[1], vRangeName[2], vRangeName[3], vRangeName[4], vRangeName[5], vRangeName[6], vRangeName[7], vRangeName[8], vRangeName[9]};
const char hRangeName[10][6] PROGMEM = {"200ms", "100ms", " 50ms", " 20ms", " 10ms", "  5ms", "  2ms", "  1ms", "500us", "200us"};  //  Hrizontal display characters
const char * const hstring_table[] PROGMEM = {hRangeName[0], hRangeName[1], hRangeName[2], hRangeName[3], hRangeName[4], hRangeName[5], hRangeName[6], hRangeName[7], hRangeName[8], hRangeName[9]};


int dataOffset[VRANGE_MAX];    // Vertical offset for wave form.
int waveBuff[REC_LENG];        // Wave form buffer (RAM remaining capacity is barely)
char hScale[] = "xxxAs";       // Horizontal scale character
char vScale[] = "xxxx";        // Vertical scale

float lsb5V = 0.00566826;      // Sensivity coefficient of 5V range. std=0.00563965 1.1*630/(1024*120)
float lsb50V = 0.05243212;     // Sensivity coefficient of 50V range. std=0.0512898 1.1*520.91/(1024*10.91)

volatile int vRange = 3;           // V-range number 0:A50V,  1:A 5V,  2:50V,  3:20V,  4:10V,  5:5V,  6:2V,  7:1V,  8:0.5V,  9:0.2V
volatile int hRange = 3;           // H-range number 0:200ms, 1:100ms, 2:50ms, 3:20ms, 4:10ms, 5:5ms, 6;2ms, 7:1ms, 8:500us, 9;200us
volatile int trigD = 1;            // trigger slope flag,     0:positive 1:negative
volatile boolean hold = false; // Hold flag
volatile boolean switchPushed = false; // flag of switch pushed !
int timeExec;                  // Approx. execution time of current range setting (ms)

int dataMin;                   // Buffer minimum value (smallest=0)
int dataMax;                   //        maximum value (largest=1023)
int rangeMax;                  // Buffer value to graph full swing
int rangeMin;                  // Buffer value of graph botto
int rangeMaxDisp;              // Display value of max. (100x value)
int rangeMinDisp;              // Display value if min.
int trigP;                     // Trigger position pointer on data buffer
int att10x;                    // 10x attenuator ON (effective when 1)

void setup() {
  holdBtn.attachLongPressStart(handleHoldBtn);
  yScaleBtn.attachLongPressStart(handleYScaleBtn);
  xScaleBtn.attachLongPressStart(handleXScaleBtn);

  Serial.begin(9600);              // A lot of RAM is used when activating this. (may by crash!)
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // select 3C or 3D (set your OLED I2C address)
    for (;;);                           // loop forever
  }
  startScreen();                        // Display start message
}

void loop() {
  setConditions();                      // Set measurement conditions
  readWave();                           // Read wave form and store into buffer memory.
  dataAnalyze();                        // Analyze data.
  oled.clearDisplay();
  plotData();                           // Plot waveform (10-18ms).
  oled.display();                       // Send screen buffer to OLED (37ms).
  updateButtons();
  while (hold == true) {                // Wait if Hold flag ON.
    updateButtons();
    dispHold();
    delay(10);
  }                                     // Loop cycle speed = 60-470ms (buffer size = 200)
}

void setConditions() {           // measuring condition setting
  // get range name from PROGMEM
  strcpy_P(hScale, (char*)pgm_read_word(&(hstring_table[hRange])));  // H range name
  strcpy_P(vScale, (char*)pgm_read_word(&(vstring_table[vRange])));  // V range name

  switch (vRange) {              // setting of Vrange
    case 0: {                    // Auto50V range
        att10x = 1;              // use input attenuator
        break;
      }
    case 1: {                    // Auto 5V range
        att10x = 0;              // no attenuator
        break;
      }
    case 2: {                    // 50V range
        rangeMax = 50 / lsb50V;  // Set full scale pixcel count number
        rangeMaxDisp = 5000;     // Vertical scale (set100x value)
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 1;              // Use input attenuator
        break;
      }
    case 3: {                    // 20V range
        rangeMax = 20 / lsb50V;  // Set full scale pixcel count number
        rangeMaxDisp = 2000;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 1;              // Use input attenuator
        break;
      }
    case 4: {                    // 10V range
        rangeMax = 10 / lsb50V;  // Set full scale pixcel count number
        rangeMaxDisp = 1000;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 1;              // Use input attenuator
        break;
      }
    case 5: {                    // 5V range
        rangeMax = 5 / lsb5V;    // Set full scale pixcel count number
        rangeMaxDisp = 500;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 0;              // No input attenuator
        break;
      }
    case 6: {                    // 2V range
        rangeMax = 2 / lsb5V;    // Set full scale pixcel count number
        rangeMaxDisp = 200;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 0;              // No input attenuator
        break;
      }
    case 7: {                    // 1V range
        rangeMax = 1 / lsb5V;    // Set full scale pixcel count number
        rangeMaxDisp = 100;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 0;              // No input attenuator
        break;
      }
    case 8: {                    // 0.5V range
        rangeMax = 0.5 / lsb5V;  // Set full scale pixcel count number
        rangeMaxDisp = 50;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 0;              // No input attenuator
        break;
      }
    case 9: {                    // 0.5V range
        rangeMax = 0.2 / lsb5V;  // set full scale pixcel count number
        rangeMaxDisp = 20;
        rangeMin = 0;
        rangeMinDisp = 0;
        att10x = 0;              // No input attenuator
        break;
      }
  }
}

void readWave() {                            // Record waveform
  switchPushed = false;                      // Clear switch operation flag

  switch (hRange) {                          // Set recording conditions in accordance with the range number
    case 0: {                                // 200ms range.
        timeExec = 1600 + 60;                // Approximate execution time(ms) Used for countdown until saving to EEPROM.
        ADCSRA = ADCSRA & 0xf8;              // Clear bottom 3bit.
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) { // Up to rec buffer size
          waveBuff[i] = analogRead(A0);       // read and save approx 112us
          delayMicroseconds(7888);           // timing adjustment
          if (switchPushed == true) {        // If any switch touched then
            switchPushed = false;
            break;                           // abandon record (this improves response time).
          }
        }
        break;
      }
    case 1: {                                // 100ms range
        timeExec = 800 + 60;                 // Approximate execution time(ms) Used for countdown until saving to EEPROM
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) {  // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 112us
          // delayMicroseconds(3888);           // timing adjustment
          delayMicroseconds(3860);           // timing adjustment tuned
          if (switchPushed == true) {        // if any switch touched
            switchPushed = false;
            break;                           // abandon record(this improve response)
          }
        }
        break;
      }
    case 2: {                                // 50ms range
        timeExec = 400 + 60;                 // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 112us
          // delayMicroseconds(1888);           // timing adjustment
          delayMicroseconds(1880);           // timing adjustment tuned
          if (switchPushed == true) {        // if any switch touched
            break;                           // abandon record(this improve response)
          }
        }
        break;
      }
    case 3: {                                // 20ms range
        timeExec = 160 + 60;                 // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 112us
          // delayMicroseconds(688);            // timing adjustment
          delayMicroseconds(686);            // timing adjustment tuned
          if (switchPushed == true) {        // if any switch touched
            break;                           // abandon record(this improve response)
          }
        }
        break;
      }
    case 4: {                                // 10ms range
        timeExec = 80 + 60;                  // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 112us
          // delayMicroseconds(288);            // timing adjustment
          delayMicroseconds(287);            // timing adjustment tuned
          if (switchPushed == true) {        // if any switch touched
            break;                           // abandon record(this improve response)
          }
        }
        break;
      }
    case 5: {                                // 5ms range
        timeExec = 40 + 60;                  // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x07;              // dividing ratio = 128 (default of Arduino）
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 112μs
          // delayMicroseconds(88);             // timing adjustment
          delayMicroseconds(87);             // timing adjustment tuned
          if (switchPushed == true) {        // if any switch touched
            break;                           // abandon record(this improve response)
          }
        }
        break;
      }
    case 6: {                                // 2ms range
        timeExec = 16 + 60;                  // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x06;              // dividing ratio = 64 (0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 56us
          // delayMicroseconds(24);             // timing adjustment
          delayMicroseconds(23);             // timing adjustment tuned
        }
        break;
      }
    case 7: {                                // 1ms range
        timeExec = 8 + 60;                   // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x05;              // dividing ratio = 16 (0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 28us
          // delayMicroseconds(12);             // timing adjustment
          delayMicroseconds(10);             // timing adjustment tuned
        }
        break;
      }
    case 8: {                                // 500us range
        timeExec = 4 + 60;                   // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x04;              // dividing ratio = 16(0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 16us
          delayMicroseconds(4);              // timing adjustment
          // time fine adjustment 0.0625 x 8 = 0.5us（nop=0.0625us @16MHz)
          asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
        }
        break;
      }
    case 9: {                                // 200us range
        timeExec = 2 + 60;                   // Approximate execution time(ms)
        ADCSRA = ADCSRA & 0xf8;              // clear bottom 3bit
        ADCSRA = ADCSRA | 0x02;              // dividing ratio = 4(0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
        for (int i = 0; i < REC_LENG; i++) { // up to rec buffer size
          waveBuff[i] = analogRead(0);       // read and save approx 6us
          // time fine adjustment 0.0625 * 20 = 1.25us (nop=0.0625us @16MHz)
          asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
          asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
        }
        break;
      }
  }
}

void dataAnalyze() {                       // get various information from wave form
  int d;
  long sum = 0;

  // search max and min value
  dataMin = 1023;                          // min value initialize to big number
  dataMax = 0;                             // max value initialize to small number
  for (int i = 0; i < REC_LENG; i++) {     // serach max min value
    d = waveBuff[i];
    sum = sum + d;
    if (d < dataMin) {                     // update min
      dataMin = d;
    }
    if (d > dataMax) {                     // updata max
      dataMax = d;
    }
  }

  // Decide display's max min value
  if (vRange <= 1) {                       // if Autorange(Range number <= 1）
    rangeMin = dataMin - 20;               // maintain bottom margin 20
    rangeMin = (rangeMin / 10) * 10;       // round 10
    if (rangeMin < 0) {
      rangeMin = 0;                        // no smaller than 0
    }
    rangeMax = dataMax + 20;               // Set display top at  data max +20
    rangeMax = ((rangeMax / 10) + 1) * 10; // Round up 10
    if (rangeMax > 1020) {
      rangeMax = 1023;                     // If more than 1020, hold down at 1023
    }

    if (att10x == 1) {                            // if 10x attenuator used
      rangeMaxDisp = 100 * (rangeMax * lsb50V);   // display range is determined by the data.(the upper limit is up to the full scale of the ADC)
      rangeMinDisp = 100 * (rangeMin * lsb50V);   // lower depend on data, but zero or more
    } else {                                      // if no attenuator used
      rangeMaxDisp = 100 * (rangeMax * lsb5V);
      rangeMinDisp = 100 * (rangeMin * lsb5V);
    }
  } else {                                        // if fix range
    // Write necessary code here (none for now)
  }

  // Trigger position search
  for (trigP = ((REC_LENG / 2) - 51); trigP < ((REC_LENG / 2) + 50); trigP++) { 
    // Find the points that straddle the median at the center ± 50 of the data range
    if (trigD == 0) {                             // if trigger direction is positive
      if ((waveBuff[trigP - 1] < (dataMax + dataMin) / 2) && (waveBuff[trigP] >= (dataMax + dataMin) / 2)) {
        break;                                    // Positive trigger position found!
      }
    } else {                                      // Trigger direction is negative.
      if ((waveBuff[trigP - 1] > (dataMax + dataMin) / 2) && (waveBuff[trigP] <= (dataMax + dataMin) / 2)) {
        break;
      }                                           // Negative trigger position found!
    }
  }
}

void startScreen() {                      // Start up screen
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(25, 15);
  oled.println(F("Notes & Volts"));          // Title(Poor Man's Osilloscope, RadioPench 1)  
  oled.setCursor(50, 35);
  oled.println(F("V1.0"));
  oled.display();                         // Actual display here.
  delay(3000);
  oled.clearDisplay();
}

void dispHold() {                         // Display "Hold".
  oled.setCursor(50, 11);
  oled.print(F("Hold"));                  // Hold
  oled.display();                         //
}

void plotData() { // Plot wave form on OLED over full screen width.
  long y1, y2;
  for (int x = 0; x <= 98; x++) {
    y1 = map(waveBuff[x + trigP - 50] + dataOffset[vRange], rangeMin, rangeMax, 63, 9); // Convert to plot address
    y1 = constrain(y1, 9, 63);                                                  // Crush(Saturate) the protruding part
    y2 = map(waveBuff[x + trigP - 49] + dataOffset[vRange], rangeMin, rangeMax, 63, 9); // Convert to plot address
    y2 = constrain(y2, 9, 63);                                                  // Limit y2 to the range [9...63]
    int xx = map(x, 0, 98, 0, SCREEN_WIDTH - 1);
    oled.drawLine(xx + BEGIN_X + 3, y1, xx + BEGIN_X + 4, y2, WHITE);// connect between point
  }
}

void updateButtons(){
  holdBtn.tick();
  yScaleBtn.tick();
  xScaleBtn.tick();
}

static void handleHoldBtn(){
  hold = !hold;
}

static void handleYScaleBtn(){
  if (vRange == 8){
    vRange = 0;
  }else {
    vRange++;
  }
}

static void handleXScaleBtn(){
  if(hRange == 6){
    hRange = 3;
  }else {
    hRange++;
  }
}