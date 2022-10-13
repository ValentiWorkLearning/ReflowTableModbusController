#include "thermo_controller_task.hpp"
#include <Arduino.h>

#include "max6675.h"

#include "system_config.h"
#include <GyverRelay.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "settings_storage.hpp"

constexpr u8 MAX6675_READ_DELAY_MS = 250;

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
    }

    int getSurroundingTemperature() noexcept
    {
        constexpr u8 kSensorIndex = 0;
        m_sensors.requestTemperatures();
        return m_sensors.getTempCByIndex(kSensorIndex);
    }

    int getThermocoupleTemperature() noexcept
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
