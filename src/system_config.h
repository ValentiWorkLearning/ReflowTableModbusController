#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

constexpr u8 kThermoDoPin = 4;
constexpr u8 kThermoCsPin = 5;
constexpr u8 kThermoClockPin = 6;

constexpr u32 kSerialBaud = 9600;
constexpr u8 kSystemRelayPin = 8;

constexpr u8 kOneWireBusPin = 7;

namespace modbus
{
    constexpr u8 kDeviceId = 1;
    constexpr u8 krs485ControlPin = 2;

    // http://support.innon.com/eTactica/Meters/Power_Meter_EM/Manual/EB-Modbus-register-map-0200.pdf

    constexpr u16 LED_COIL_ADDR = 0x2001;

    namespace address
    {
        constexpr u16 kSurroundingTempAddr = 0x200F;
        constexpr u16 kThermocoupleAddr = 0x2010;
        constexpr u16 kFactorAddr = 0x2011;
        constexpr u16 kFeedbackAddr = 0x2012;
        constexpr u16 kTargetTemperatureAddr = 0x2013;

        constexpr u16 kModbusRegistersCount = 5;
    }

}
#endif