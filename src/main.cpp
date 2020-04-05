#include <Arduino.h>
#include <EmonLib.h>
#include <RFM69_ATC.h>
#include <RFM69.h>

// allow filter to settle down - we'll wait this much before we send any data (in ms)
#define FILTERSETTLETIME 5000
// mark that we've settled or not
bool settled = false;

// how often we take measurements (in ms)
#define MEASUREINTERVAL 10000
unsigned long lastRepPeriod = 0xffff;

// enable single/double/quad measurements
#define CT1
//#define CT2
//#define CT3CT4

// enable this if you have Vref (AC-AC adapter) to calculate actual voltage
#define HAS_VREF
// set this if no AC-AC ref voltage provided
#define MAINS_VOLTAGE 240

// RF parameters
#define NODEID      2
#define NETWORKID   89
#define GATEWAYID   1
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY  "sampleEncryptKey" // has to be same 16 characters
// enable Auto Transmission Control
#define ENABLE_ATC
// enable high power - only for RFM69HW/HCW
//#define IS_RFM69HW_HCW

// TODO: this will need to go to a central place, where all the node functions are defined
#define NODEFUNC_POWER_SINGLE   1
#define NODEFUNC_POWER_DOUBLE   2
#define NODEFUNC_POWER_QUAD     3

// create energy monitor object(s)
#ifdef CT1
  EnergyMonitor emon1;
#endif
#ifdef CT2
  EnergyMonitor emon2;
#endif
#ifdef CT3CT4
  EnergyMonitor emon3;
  EnergyMonitor emon4;
#endif

// create radio object
#ifdef ENABLE_ATC
  RFM69_ATC radio(10, 2);
#else
  RFM69 radio(10, 2);
#endif

// data to send
typedef struct {
  uint16_t  nodeId;
  uint8_t   nodeFunction;
  uint16_t  power1;
  uint16_t  Vrms;
} __attribute__((packed)) PayloadPowerSingle;
PayloadPowerSingle payloadPowerSingle;

typedef struct {
  uint16_t  nodeId;
  uint8_t   nodeFunction;
  uint16_t  power1;
  uint16_t  power2;
  uint16_t  Vrms;
} __attribute__((packed)) PayloadPowerDouble;
PayloadPowerDouble payloadPowerDouble;

typedef struct {
  uint16_t  nodeId;
  uint8_t   nodeFunction;
  uint16_t  power1;
  uint16_t  power2;
  uint16_t  power3;
  uint16_t  power4;
  uint16_t  Vrms;
} __attribute__((packed)) PayloadPowerQuad;
PayloadPowerQuad payloadPowerQuad;

void setup() {
  // fire up the serial port
  Serial.begin(9600);

  Serial.println("[SYS ] emonTx-shield-RFM69 starting up");

  // set up energy monitor(s) for power and voltage monitoring
  #ifdef CT1
    emon1.current(1, 60.606);
    #ifdef HAS_VREF
      emon1.voltage(0, 256, 1.7);
    #endif
  #endif

  #ifdef CT2
    emon2.current(1, 60.606);
    #ifdef HAS_VREF
      emon2.voltage(0, 256, 1.7);
    #endif
  #endif

  #ifdef CT3CT4
    emon3.current(1, 60.606);
    emon4.current(1, 60.606);
    #ifdef HAS_VREF
      emon3.voltage(0, 256, 1.7);
      emon4.voltage(0, 256, 1.7);
    #endif
  #endif

  // initialise radio
  Serial.println("[RF69] Radio init start");
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  #ifdef IS_RFM69HW_HCW
    radio.setHighPower(); // only for RFM69HW/HCW
  #endif
  radio.encrypt(ENCRYPTKEY);
  Serial.println("[RF69] Radio init done");
  Serial.println("[SYS ] Init done");
}

void loop() {
  // check if we're settled
  // extra check due to millis resetting every 50 days
  if (!settled && millis() > FILTERSETTLETIME) {
    settled = true;
  }

  // check for any received packets
  // (might be a ping)
  if (radio.receiveDone())
  {
    Serial.print("[RF69] Received from [");
    Serial.print(radio.SENDERID, DEC);
    Serial.print("], Data [");
    for (byte i = 0; i < radio.DATALEN; i++) {
      Serial.print((char)radio.DATA[i]);
    }
    Serial.print("],  RSSI ");
    Serial.print(radio.readRSSI());

    // check if we need to ACK
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print("; ACK sent");
      delay(10);
    }
    Serial.println();
  }

  unsigned long curRepPeriod = millis() / MEASUREINTERVAL;
  if (curRepPeriod != lastRepPeriod) {
    // update the last report period
    lastRepPeriod = curRepPeriod;
    // measure power and voltage
    #ifdef CT1
      #ifdef HAS_VREF
        emon1.calcVI(20, 2000);
        Serial.print("[CT1 ] Power: ");
        Serial.print(emon1.realPower);
        Serial.print("W , VRms: ");
        Serial.println(emon1.Vrms);
      #else
        emon1.calcIrms(1480);
        Serial.print("CT1 ] Power: ");
        Serial.print(emon1.Irms * MAINS_VOLTAGE);
      #endif
    #endif

    #ifdef CT2
      #ifdef HAS_VREF
        emon2.calcVI(20, 2000);
        Serial.print("[CT2 ] Power: ");
        Serial.print(emon2.realPower);
        Serial.print("W , VRms: ");
        Serial.println(emon2.Vrms);
      #else
        emon2.calcIrms(1480);
        Serial.print("CT2 ] Power: ");
        Serial.print(emon2.Irms * MAINS_VOLTAGE);
      #endif
    #endif

    #ifdef CT3CT4
      #ifdef HAS_VREF
        emon3.calcVI(20, 2000);
        Serial.print("[CT3 ] Power: ");
        Serial.print(emon3.realPower);
        Serial.print("W , VRms: ");
        Serial.println(emon3.Vrms);

        emon4.calcVI(20, 2000);
        Serial.print("[CT4 ] Power: ");
        Serial.print(emon4.realPower);
        Serial.print("W , VRms: ");
        Serial.println(emon4.Vrms);
      #else
        emon3.calcIrms(1480);
        Serial.print("CT3 ] Power: ");
        Serial.print(emon3.Irms * MAINS_VOLTAGE);

        emon4.calcIrms(1480);
        Serial.print("CT4 ] Power: ");
        Serial.print(emon4.Irms * MAINS_VOLTAGE);
      #endif
    #endif

    // now send the data over radio
    if (settled) {
      // prep data for sending
      #ifdef CT3CT4
        payloadPowerQuad.nodeId = NODEID;
        payloadPowerQuad.nodeFunction = NODEFUNC_POWER_QUAD;
        #ifdef HAS_VREF
          payloadPowerQuad.Vrms = emon1.Vrms * 10;
          payloadPowerQuad.power1 = emon1.realPower;
          payloadPowerQuad.power2 = emon2.realPower;
          payloadPowerQuad.power3 = emon3.realPower;
          payloadPowerQuad.power4 = emon4.realPower;
        #else
          payloadPowerQuad.Vrms = MAINS_VOLTAGE * 10;
          payloadPowerQuad.power1 = emon1.Irms * MAINS_VOLTAGE;
          payloadPowerQuad.power2 = emon2.Irms * MAINS_VOLTAGE;
          payloadPowerQuad.power3 = emon3.Irms * MAINS_VOLTAGE;
          payloadPowerQuad.power4 = emon4.Irms * MAINS_VOLTAGE;
        #endif
      #elif defined(CT2)
        payloadPowerDouble.nodeId = NODEID;
        payloadPowerDouble.nodeFunction = NODEFUNC_POWER_DOUBLE;
        #ifdef HAS_VREF
          payloadPowerDouble.Vrms = emon1.Vrms * 10;
          payloadPowerDouble.power1 = emon1.realPower;
          payloadPowerDouble.power2 = emon2.realPower;
        #else
          payloadPowerDouble.Vrms = MAINS_VOLTAGE * 10;
          payloadPowerDouble.power1 = emon1.Irms * MAINS_VOLTAGE;
          payloadPowerDouble.power2 = emon2.Irms * MAINS_VOLTAGE;
        #endif
      #else
        payloadPowerSingle.nodeId = NODEID;
        payloadPowerSingle.nodeFunction = NODEFUNC_POWER_SINGLE;
        #ifdef HAS_VREF
          payloadPowerSingle.Vrms = emon1.Vrms * 10;
          payloadPowerSingle.power1 = emon1.realPower;
        #else
          payloadPowerSingle.Vrms = MAINS_VOLTAGE * 10;
          payloadPowerSingle.power1 = emon1.Irms * MAINS_VOLTAGE;
        #endif
      #endif

      // now send the data
      Serial.print("[RF69] Sending data - ");
      #ifdef CT3CT4
        if (radio.sendWithRetry(GATEWAYID, (const void*)(&payloadPowerQuad), sizeof(payloadPowerQuad))) {
      #elif defined(CT2)
        if (radio.sendWithRetry(GATEWAYID, (const void*)(&payloadPowerDouble), sizeof(payloadPowerDouble))) {
      #else
        if (radio.sendWithRetry(GATEWAYID, (const void*)(&payloadPowerSingle), sizeof(payloadPowerSingle))) {
      #endif
          Serial.println("OK");
        }
        else
        {
          Serial.println("Failed");
        }
    }
  }
}