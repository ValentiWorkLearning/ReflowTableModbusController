#ifndef SETTINGS_STORAGE_HPP
#define SETTINGS_STORAGE_HPP
#include "thermo_controller_task.hpp"
#include <EEPROM.h>

namespace settings
{

class SettingsStorage
{
public:

    void store(const ThermoController::RegulatorParams& params)const noexcept
    {
        EEPROM.put(kEepromSettingsAddr,params);

        constexpr bool kSettingsWritingHasDone{true};
        EEPROM.put(kEepromSettingsFlag, kSettingsWritingHasDone);
    }

    ThermoController::RegulatorParams restore()noexcept
    {
        ThermoController::RegulatorParams outParam{};
        EEPROM.get(kEepromSettingsAddr,outParam);
        return outParam;
    }

    bool canRestore() const noexcept
    {
        bool haveSettingsBeenWritten{false};
        EEPROM.get(kEepromSettingsFlag,haveSettingsBeenWritten);
        return haveSettingsBeenWritten;
    }
    static SettingsStorage& Instance()
    {
        static SettingsStorage instance{};
        return instance;
    }

private:

    static const u8 kEepromSettingsAddr = 0x00;
    static const u16 kEepromSettingsFlag = 0x244;
private:

    SettingsStorage() = default;

};

}

#endif