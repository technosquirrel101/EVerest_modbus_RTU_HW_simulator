#include "responder.h"
#include <CRC.h>
#include <CRC16.h>

responder::responder() {
}


responder::~responder() {
}


void responder::run() {
    clean_arrays();

    //##############################################
    
    i = 0;
    while (Serial1.available()) {
        inputArray[i++] = Serial1.read();
    }

    //##############################################
    
    Serial.println(F("\n\nreceived message: "));
    j = i;
    while(i) {
        sprintf(bufferstr, "%02x ", inputArray[j-i]);
        Serial.print(bufferstr);
        i--;
    }
    sprintf(bufferstr, " \n");
    Serial.print(bufferstr);
    delay(100);

    //##############################################

    if (j > 3) {
        outputArray[0] = inputArray[0];
        outputArray[1] = inputArray[1];
        select_device_based_on_package(inputArray[0]);

        send_response = true;
        length_of_response = 10;

        if ( simulate_imd == true ) {
            Serial.println(F("Simulating IMD"));
            create_imd_response();
            
        } else if ( bauer_pm_test == true ) {
            Serial.println(F("Bauer powermeter test message detected!"));
            create_bauer_powermeter_response();
            
        } else if ( simulate_inverter == true ) {
            Serial.println(F("Simulating inverter"));
            create_inverter_response();

        } else if ( simulate_powermeter_dc == true ) {
            Serial.println(F("Simulating DC powermeter"));
            create_powermeter_dc_response();
            
        } else {
            create_random_response();
        }

        if (send_response == true) {
            send_modbus_response();
        }
        
    } else {
        sprintf(bufferstr, "message incomplete, aborting!\n");
        Serial.print(bufferstr);
    }
}


void responder::select_device_based_on_package(uint8_t selector) {
    if( selector == bauer_pm_modbus_id ) {
        bauer_pm_test          = true;
        simulate_inverter      = false;
        simulate_imd           = false;
        simulate_powermeter_dc = false;
    } else if( selector == imd_modbus_id ) {
        bauer_pm_test          = false;
        simulate_inverter      = true;
        simulate_imd           = false;
        simulate_powermeter_dc = false;
    } else if( selector == imd_modbus_id ) {
        bauer_pm_test          = false;
        simulate_inverter      = false;
        simulate_imd           = true;
        simulate_imd_error     = false;
        simulate_imd_not_ready = false;
        simulate_powermeter_dc = false;
    } else if( selector == imd_w_error_modbus_id ) {
        bauer_pm_test          = false;
        simulate_inverter      = false;
        simulate_imd           = true;
        simulate_imd_error     = true;
        simulate_imd_not_ready = false;
        simulate_powermeter_dc = false;
    } else if( selector == imd_not_ready_modbus_id ) {
        bauer_pm_test          = false;
        simulate_inverter      = false;
        simulate_imd           = true;
        simulate_imd_error     = false;
        simulate_imd_not_ready = true;
        simulate_powermeter_dc = false;
    } else if( selector == powermeter_dc_modbus_id ) {
        bauer_pm_test          = false;
        simulate_inverter      = false;
        simulate_imd           = false;
        simulate_imd_error     = false;
        simulate_imd_not_ready = false;
        simulate_powermeter_dc = true;
    } else {
        bauer_pm_test          = false;
        simulate_inverter      = false;
        simulate_imd           = false;
        simulate_imd_error     = false;
        simulate_imd_not_ready = false;
        simulate_powermeter_dc = false;
    }
}


void responder::create_imd_response() {
    if ((inputArray[1] == 0x02) &&
        (inputArray[2] == 0x00) &&
        (inputArray[3] == 0x02) &&
        (inputArray[4] == 0x10) &&
        (inputArray[5] == 0x20)
    ) {
        Serial.println(F("no response expected, skipping!"));
        send_response = false;
    } else if ( (inputArray[1] == 0x01) &&
                (inputArray[2] == 0x02) &&
                (inputArray[3] == 0x03) &&
                (inputArray[4] == 0x04) &&
                (inputArray[5] == 0x05)
    ) {
        const unsigned char grounding_relay_status_mask        = 0x20;
        const unsigned char insulation_measurement_status_mask = 0x10;
        const unsigned char se_reverse_conn_fault_mask         = 0x02;
        const unsigned char car_reverse_conn_fault_mask        = 0x01;

        outputArray[1] = 0x00;
        if (simulate_imd_error == false) {
            outputArray[1] |= grounding_relay_status_mask;
        }
        if (simulate_imd_not_ready == false) {
            outputArray[1] |= insulation_measurement_status_mask;
        }
        
        length_of_response = 8;
        for (i = 2; i < length_of_response; i++) {
            outputArray[i] = random(1, 200);
        }
    }
}


void responder::create_bauer_powermeter_response() {
    outputArray[2] = 0x84;
    length_of_response = outputArray[2] + 3; 
    for (i = 3; i < length_of_response; i++) {
        outputArray[i] = stripped_incomming_rtu_response[i - 3];
    }
}


void responder::create_inverter_response() {
    if ((inputArray[2] == 0x04) &&
        (inputArray[3] == 0x40)
    ) {
        length_of_response = inputArray[5] * 2;
        outputArray[2] = length_of_response;
        for (i = 3; i < length_of_response; i++) {
            outputArray[i] = 0;
        }
        
        for (i = 3; i < 12; i++) {
            outputArray[i] = random(1, 200);
        }
        // set "Serialnumber"
        outputArray[13] = 'H';
        outputArray[15] = 'a';
        outputArray[17] = 'l';
        outputArray[19] = 'l';
        outputArray[21] = 'o';
        outputArray[23] = ' ';
        outputArray[25] = 'W';
        outputArray[27] = 'e';
        outputArray[29] = 'l';
        outputArray[31] = 't';
        outputArray[33] = '!';

    } else {
        length_of_response = inputArray[5] * 2;
        outputArray[2] = length_of_response;
        for (i = 3; i < length_of_response; i++) {
            outputArray[i] = random(1, 200);
        }
    }
}


void responder::create_powermeter_dc_response() {
    length_of_response = 40;
    outputArray[2] = length_of_response;
    
    for (i = 3; i < length_of_response; i++) {
        outputArray[i] = random(1, 200);
    }

    // voltage V
    outputArray[3] = 0;
    outputArray[4] = 222;

    // voltage exponent ( * 10^(-1 * x) )
    outputArray[5] = 0;
    outputArray[6] = 0;

    // current A
    outputArray[7] = 0;
    outputArray[8] = 33;

    // current exponent ( * 10^(-1 * x) )
    outputArray[9]  = 0;
    outputArray[10] = 0;

    // power W
    outputArray[19] = 0;
    outputArray[20] = 55;

    // power exponent ( * 10^(-1 * x) )
    outputArray[21] = 0;
    outputArray[22] = 0;
}


void responder::create_random_response() {
    outputArray[2] = 10;
    length_of_response = outputArray[2] + 3;
    for (i = 3; i < length_of_response; i++) {
        outputArray[i] = random(1, 200);
    }
}


void responder::send_modbus_response() {
    uint16_t CRCx16 = crc16(outputArray, length_of_response, 0x8005, 0xFFFF, 0, true, true); 

    outputArray[  length_of_response ]    = lowByte(CRCx16);
    outputArray[ (length_of_response+1) ] = highByte(CRCx16);

    Serial.println(F("\n\n---> response message:"));
    i = length_of_response + 1 + 1;
    j = i;
    sprintf(bufferstr, "length: %d \n", j);
    Serial.print(bufferstr);
    delay(20);
    while(i) {
        sprintf(bufferstr, "%02x ", outputArray[j - i]);
        Serial.print(bufferstr);
        Serial1.write(uint8_t(outputArray[j - i]));
        i--;
    }
}


void responder::clean_arrays() {
    i = 0;
    for (i = 0; i < 200; i++) {
        inputArray[i]  = 0;
        outputArray[i] = 0;
    }
}
