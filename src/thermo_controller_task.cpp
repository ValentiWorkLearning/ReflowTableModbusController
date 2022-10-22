#include "thermo_controller_task.hpp"
#include <Arduino.h>

#include "max6675.h"

#include "system_config.h"
#include <GyverRelay.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "settings_storage.hpp"

#define ETL_NO_STL
#include <etl/flat_map.h>
#include <etl/delegate.h>

constexpr u8 MAX6675_READ_DELAY_MS = 250;

template<typename LazyType>
struct LazySensorAccessor
{
public:


    LazySensorAccessor() = default;

    LazySensorAccessor(u32 accessPeriod,etl::delegate<int()> accessor)
        :   m_accessPeriod{accessPeriod},
            m_accessor{accessor}
    {
    }

    LazyType getValue() noexcept
    {
        auto current = millis();
        if(current - m_lastAccessTime < m_accessPeriod)
            return m_cachedValue;

        m_lastAccessTime = current;
        m_cachedValue = m_accessor();

        return m_cachedValue;
    }
    
private:
    uint32_t m_accessPeriod;
    etl::delegate<int()> m_accessor;
    uint32_t m_lastAccessTime = 0x00;

    LazyType m_cachedValue;
};


class ThermoController::ThermoControllerImpl
{
public:
    ThermoControllerImpl() noexcept
        : m_thermocouple{kThermoClockPin, kThermoCsPin, kThermoDoPin}, m_regulator{REVERSE}, m_oneWire{kOneWireBusPin}, m_sensors{&m_oneWire}
    {
        
        if(settings::SettingsStorage::Instance().canRestore()){
            RegulatorParams regulatorSettings{settings::SettingsStorage::Instance().restore()};
            m_regulator.hysteresis = regulatorSettings.hysteresis;
            m_regulator.k = regulatorSettings.k;
        }
        else{
            m_regulator.setpoint = 30;
            m_regulator.hysteresis = 5;
            m_regulator.k = 0.5;
        }

        constexpr u32 kSurroundingAccessMs{10000};
        constexpr u32 kTableAccessMs{250};

        m_surroundingLazyAccessor = LazySensorAccessor<int>{kSurroundingAccessMs,
            etl::delegate<int()>::create<ThermoController::ThermoControllerImpl, &ThermoController::ThermoControllerImpl::getSurroundingTemperatureImpl>(*this)
        };

        m_tableLazyAccessor = LazySensorAccessor<int>{kTableAccessMs,
            etl::delegate<int()>::create<ThermoController::ThermoControllerImpl, &ThermoController::ThermoControllerImpl::getThermocoupleTemperatureImpl>(*this)
        };
    }


    int getSurroundingTemperature() noexcept
    {
        return m_surroundingLazyAccessor.getValue();
    }

    int getSurroundingTemperatureImpl() noexcept
    {
        constexpr u8 kSensorIndex = 0;
        m_sensors.requestTemperatures();
        return m_sensors.getTempCByIndex(kSensorIndex);
    }

    int getThermocoupleTemperature() noexcept
    {
        return m_tableLazyAccessor.getValue();
    }

    int getThermocoupleTemperatureImpl() noexcept
    {
        return m_thermocouple.readCelsius();
    }

    void processTemperatureControl() noexcept
    {
        int temperatureValue = getThermocoupleTemperature();
        m_regulator.input = temperatureValue;
        digitalWrite(kSystemRelayPin, m_regulator.getResultTimer());
    }

    RegulatorParams getActualRegulatorParams() const noexcept
    {
        return RegulatorParams{
            .setpoint = m_regulator.setpoint,
            .hysteresis = m_regulator.hysteresis,
            .k = m_regulator.k};
    }
    void setActualRegulatorParams(const RegulatorParams &params) noexcept
    {
        m_regulator.setpoint = params.setpoint;
        m_regulator.hysteresis = params.hysteresis;
        m_regulator.k = params.k;

        settings::SettingsStorage::Instance().store(params);
    }

private:
    LazySensorAccessor<int> m_surroundingLazyAccessor;
    LazySensorAccessor<int> m_tableLazyAccessor;

    MAX6675 m_thermocouple;
    GyverRelay m_regulator;
    OneWire m_oneWire;
    DallasTemperature m_sensors;
};

ThermoController::ThermoController() noexcept
    : m_pImpl{new ThermoControllerImpl()}
{
}

ThermoController::~ThermoController()
{
    delete m_pImpl;
}

void ThermoController::processTemperatureControl()
{
    m_pImpl->processTemperatureControl();
}

int ThermoController::getSurroundingTemperature() noexcept
{
    return m_pImpl->getSurroundingTemperature();
}

int ThermoController::getThermocoupleTemperature() noexcept
{
    return m_pImpl->getThermocoupleTemperature();
}

ThermoController::RegulatorParams ThermoController::getActualRegulatorParams() const noexcept
{
    return m_pImpl->getActualRegulatorParams();
}
void ThermoController::setActualRegulatorParams(const RegulatorParams &params) noexcept
{
    return m_pImpl->setActualRegulatorParams(params);
}
