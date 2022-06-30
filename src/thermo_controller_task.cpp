#include "thermo_controller_task.hpp"
#include <Arduino_FreeRTOS.h>
#include <Arduino.h>

#include "max6675.h"

#include "system_config.h"
#include <GyverRelay.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#include "system_queues.hpp"
#include "logger_task.hpp"

constexpr u8 MAX6675_READ_DELAY_MS = 250;

class ThermoController::ThermoControllerImpl
{
public:

    ThermoControllerImpl()noexcept
        : m_thermocouple{THERMO_CLK_PIN, THERMO_CS_PIN, THERMO_DO_PIN}
        , m_regulator{REVERSE}
        , m_oneWire{ONE_WIRE_BUS}
        , m_sensors{&m_oneWire}
    {
        m_regulator.setpoint = 30;
        m_regulator.hysteresis = 5;
        m_regulator.k = 0.5;
    }
        
    void processMessage(const ThermalControlMessage& message)noexcept
    {
        LOG_DEBUG("processMessage");
        switch (message.id)
        {
        case SET_TEMPERATURE:
            LOG_DEBUG("SET_TEMPERATURE_COMMAND");
            m_regulator.setpoint = message.payload;
            break;
        case EMERGENCY_STOP:
            LOG_DEBUG("EMERGENCY_STOP");
            m_regulator.setpoint = getSurroundingTemperature();
        break;
        case REQUEST_TEMPERATURE:
            LOG_DEBUG("REQUEST_TEMPERATURE");
        break;
        default:
            break;
        }
    }

    int getSurroundingTemperature()noexcept
    {   
        constexpr u8 kSensorIndex = 0;
        m_sensors.requestTemperatures();
        return m_sensors.getTempCByIndex(kSensorIndex);
    }

    int getThermocoupleTemperature()noexcept
    {
        return m_thermocouple.readCelsius();
    }

    void processTemperatureControl()noexcept
    {
        int temperatureValue = getThermocoupleTemperature();
        m_regulator.input = temperatureValue;
        digitalWrite(SYSTEM_RELAY_PIN, m_regulator.getResultTimer());
    }


    RegulatorParams getActualRegulatorParams()const noexcept
    {
        return RegulatorParams
        {
            .setpoint = m_regulator.setpoint,
            .hysteresis = m_regulator.hysteresis,
            .k = m_regulator.k
        };
    }
    void setActualRegulatorParams(const RegulatorParams& params)noexcept
    {
        m_regulator.setpoint = params.setpoint;
        m_regulator.hysteresis = params.hysteresis;
        m_regulator.k = params.k;
    }

private:
    MAX6675 m_thermocouple;
    GyverRelay m_regulator;
    OneWire m_oneWire;
    DallasTemperature m_sensors;
};

ThermoController::ThermoController()noexcept
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


int ThermoController::getSurroundingTemperature()noexcept
{
    return m_pImpl->getSurroundingTemperature();
}

int ThermoController::getThermocoupleTemperature()noexcept
{
    return m_pImpl->getThermocoupleTemperature();
}

ThermoController::RegulatorParams ThermoController::getActualRegulatorParams()const noexcept
{
    return m_pImpl->getActualRegulatorParams();
}
void ThermoController::setActualRegulatorParams(const RegulatorParams& params)noexcept
{
    return m_pImpl->setActualRegulatorParams(params);
}
