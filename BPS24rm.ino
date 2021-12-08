#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"

#include <Wire.h>
#include <MCP342x.h>
#include <Preferences.h>


void loadPrefs(void);
void initPrefs(void);
void RM_Setup(void);
bool ADC_Setup(void);
void readVoltage(void);
int cellState(void);
void cutoffs(int);

/**
   Dallas family thermometers access example.
*/
#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"

/*
   Default Settings
*/
#define DEFAULT_VOLTAGE 24.113
#define DEFAULT_MIN_VOLTAGE 22.0
#define DEFAULT_MAX_VOLTAGE 28.0
#define DEFAULT_MAX_DELTA 250
#define DEFAULT_CHARGE_ENABLE true
#define DEFAULT_LOAD_ENABLE true

#define CHARGE_ENABLE 19
#define LOAD_ENABLE 18

#define ADC_ON 1
//#define PREFS 1

/*
   Init control variables
*/
static float minVoltage = DEFAULT_MIN_VOLTAGE;
static float maxVoltage = DEFAULT_MAX_VOLTAGE;
static bool chargeEnable = DEFAULT_CHARGE_ENABLE;
static bool loadEnable = DEFAULT_LOAD_ENABLE;
static int maxDelta = DEFAULT_MAX_DELTA;
static char bankStatus[10] = "Normal";
static char cellStatus[50] = "";
bool aadc = false;
/*
    Pins
*/
static int pchargeEnable = 12;
static int ploadEnable = 21;//19
//GPIO for push button reset
static int gpio_0 = 0;

const int mySCL = 23;
const int mySDA = 22;//21
/*
   Provisioning Service Name
*/
const char *service_name = "PROV_1234";
const char *pop = "abcd1234";

/*
   Create space for the Custom Device
*/
static Device battery("Battery", "custom.device.battery");

/*
   Now for the App itself
*/
unsigned long mcounter = 0;
volatile bool ADCinterruptCounter;

bool prefState = false;

hw_timer_t * ADCtimer = NULL;
portMUX_TYPE ADCtimerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onADCTimer() {
  portENTER_CRITICAL_ISR(&ADCtimerMux);
  ADCinterruptCounter = true;
  portEXIT_CRITICAL_ISR(&ADCtimerMux);
}

//#ifdef ADC
uint8_t MCPaddress = 0x6E;
uint8_t MCPaddress2 = 0x6A;

MCP342x adc = MCP342x(MCPaddress);
MCP342x adc02 = MCP342x(MCPaddress2);
//#endif


int oldbank = 26800;
int olddelta = 0;
int change = 0;
float bank = 26.800;
int highcell = 0;
int lowcell = 0;
int16_t cellsum = 0;
int highcellv = 0;
int lowcellv = 0;
int delta = 0;
int Cell[] = {3350, 3350, 3350, 3350, 3350, 3350, 3350, 3350};
int cellAve[] = {3350, 3350, 3350, 3350, 3350, 3350, 3350, 3350};
int deltaSum = 0;
float testbank = 28;
float dd = 0.01;

Preferences prefs;

void setup()
{
  Serial.begin(115200);

  pinMode(gpio_0, INPUT);
  pinMode(ploadEnable, OUTPUT);
  pinMode(pchargeEnable, OUTPUT);

  /*
     Setup the default Pin states
  */

  Serial.println("Pref Setup");
  initPrefs();
  loadPrefs();
  Serial.println("Pref Finished");

  digitalWrite(ploadEnable, loadEnable);
  digitalWrite(pchargeEnable, chargeEnable);

  RM_Setup();

  ADCtimer = timerBegin(0, 80, true);
  timerAttachInterrupt(ADCtimer, &onADCTimer, true);
  timerAlarmWrite(ADCtimer, 10 * 1000000, true);
  timerAlarmEnable(ADCtimer);

  //#ifdef ADC
  Serial.printf("ADC Setup Start\n");
  bool badc = ADC_Setup();
  Serial.printf("ADC Setup Finished\n");
  //#endif
  Serial.print(bankStatus);
  Serial.print(" : ");
  Serial.println(sizeof(bankStatus));

  Serial.printf("Setup Completed\n");

}

void loop()
{

  if (digitalRead(gpio_0) == LOW) { //Push button pressed

    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(gpio_0) == LOW) delay(50);
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi - Fi.\n");
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    } else {
      readVoltage();
    }
  }

  if (ADCinterruptCounter) {
    portENTER_CRITICAL(&ADCtimerMux);
    ADCinterruptCounter = false;
    portEXIT_CRITICAL(&ADCtimerMux);

    readVoltage();
    cutoffs(cellState());

    battery.updateAndReportParam("Voltage (V)", bank);
    battery.updateAndReportParam("Delta (mV)", delta);
    battery.updateAndReportParam("Status", &bankStatus[0]);
    battery.updateAndReportParam("CellStatus", &cellStatus[0]);
    battery.updateAndReportParam("Hicell", highcell);
    battery.updateAndReportParam("Locell", lowcell);

    battery.updateAndReportParam("Low Voltage", minVoltage);

    Serial.println(highcell);
    Serial.println(lowcell);
    Serial.print("cellStatus: ");
    Serial.println(&cellStatus[0]);

    Serial.print(&bankStatus[0]);
    Serial.print(" : ");
    Serial.println(sizeof(bankStatus));

    delay (100);
  }
}
