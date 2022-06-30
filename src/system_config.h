#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H


constexpr u8 THERMO_DO_PIN  = 4;
constexpr u8 THERMO_CS_PIN  = 5;
constexpr u8 THERMO_CLK_PIN = 6;

constexpr u32 SERIAL_BAUDRATE = 9600;
constexpr u8 SYSTEM_RELAY_PIN = 3;


constexpr u8 ONE_WIRE_BUS = 7;

constexpr u8 LOG_MESSAGE_SIZE = 25;
constexpr u8 LOG_TASK_DELAY = 10;

namespace modbus
{
    constexpr u8 kDeviceId = 1;
    constexpr u8 krs485ControlPin = 2;

    //http://support.innon.com/eTactica/Meters/Power_Meter_EM/Manual/EB-Modbus-register-map-0200.pdf
    
    constexpr u16 LED_COIL_ADDR=0x2001;

namespace address{
    constexpr u16 kSurroundingTempAddr=0x200F;
    constexpr u16 kThermocoupleAddr=0x2010;
    constexpr u16 kFactorAddr=0x2011;
    constexpr u16 kFeedbackAddr=0x2012;
    constexpr u16 kTargetTemperatureAddr=0x2013;

    constexpr u16 MODBUS_REGISTERS_COUNT = 5;
}

}
 #endif