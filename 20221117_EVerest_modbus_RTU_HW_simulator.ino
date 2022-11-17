/*
 *   Hardware simulator for ModbusRTU devices
 *   ----------------------------------------
 *      built on Arduino Mega 2560 (or any other Arduino with at least two Serial device ports)
 *      
 *      uses the following libraries:
 *      -----------------------------
 *          - CRC by Rob Tillaart (robtillaart/CRC@^0.3.2)
 * 
 *      responds to ModbusRTU messages incoming from Serial1 and creates a (semi-correct but valid) output message on Serial1
 * 
 *      debug messages will be displayed on Serial 
 * 
 *      !!! set Modbus baudrate and PARITY before use !!!
 * 
 *      currently supported devices:
 *      ----------------------------
 *          - Insulation measurement device (IMD)                               on incoming Modbus ID (0x37)
 *          - Insulation measurement device (IMD) with insulation fault/error   on incoming Modbus ID (0x38)
 *          - Insulation measurement device (IMD) in busy/not ready state       on incoming Modbus ID (0x39)
 *          - Bauer reference powermeter                                        on incoming Modbus ID (0x2A)
 *          - Inverter device                                                   on incoming Modbus ID (0x21)
 *          - DC powermeter                                                     on incoming Modbus ID (0x4D)
 *          - a default device which responds with a random value series        on any other Modbus ID
 * 
 */

#include <Arduino.h>
#include "responder.h"

#define MODBUS_RTU_BAUDRATE (19200)
// #define MODBUS_RTU_BAUDRATE  (9600)

// uncomment ONLY ONE at a time, use PARITY_EVEN for Bauer PM
#define PARITY_EVEN
// #define PARITY_ODD


responder modbus_handler;

void setup() {
    Serial.begin(115200);
    Serial.println(F("Modbus RTU simulator start."));

#ifdef PARITY_EVEN
    Serial1.begin(MODBUS_RTU_BAUDRATE, SERIAL_8E1);
#elif PARITY_ODD
    Serial1.begin(MODBUS_RTU_BAUDRATE, SERIAL_8O1);
#else
    Serial1.begin(MODBUS_RTU_BAUDRATE);
#endif // PARITY

    randomSeed(analogRead(0));
}

void loop() {
    if (Serial1.available()) {
        modbus_handler.run();
    }
    delay(50);
}
