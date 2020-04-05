#include <Arduino.h>
#include <EmonLib.h>

// allow filter to settle down - we'll wait this much before we send any data
#define FILTERSETTLETIME 5000
// mark that we've settled or not
bool settled = false;

#define CT1
//#define CT2
//#define CT3
//#define CT4

// create energy monitor object(s)
#ifdef CT1
  EnergyMonitor emon1;
#endif
#ifdef CT2
  EnergyMonitor emon2;
#endif
#ifdef CT3
  EnergyMonitor emon3;
#endif
#ifdef CT4
  EnergyMonitor emon4;
#endif

// enable this if you have Vref to calculate actual voltage
#define HAS_VREF
// set this if no AC-AC ref voltage provided
#define MAINS_VOLTAGE 240

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

  #ifdef CT3
    emon3.current(1, 60.606);
    #ifdef HAS_VREF
      emon3.voltage(0, 256, 1.7);
    #endif
  #endif

  #ifdef CT4
    emon4.current(1, 60.606);
    #ifdef HAS_VREF
      emon4.voltage(0, 256, 1.7);
    #endif
  #endif

  // TODO: initialise radio

  Serial.println("[SYS ] Init done");
}

void loop() {
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

  #ifdef CT3
    #ifdef HAS_VREF
      emon3.calcVI(20, 2000);
      Serial.print("[CT3 ] Power: ");
      Serial.print(emon3.realPower);
      Serial.print("W , VRms: ");
      Serial.println(emon3.Vrms);
    #else
      emon3.calcIrms(1480);
      Serial.print("CT3 ] Power: ");
      Serial.print(emon3.Irms * MAINS_VOLTAGE);
    #endif
  #endif

  #ifdef CT4
    #ifdef HAS_VREF
      emon4.calcVI(20, 2000);
      Serial.print("[CT4 ] Power: ");
      Serial.print(emon4.realPower);
      Serial.print("W , VRms: ");
      Serial.println(emon4.Vrms);
    #else
      emon4.calcIrms(1480);
      Serial.print("CT4 ] Power: ");
      Serial.print(emon4.Irms * MAINS_VOLTAGE);
    #endif
  #endif

  // check if we're settled
  // extra check due to millis resetting every 50 days
  if (!settled && millis() > FILTERSETTLETIME) {
    settled = true;
  }

  // now send the data over radio
  if (settled) {
    // TODO: send data
  }
}