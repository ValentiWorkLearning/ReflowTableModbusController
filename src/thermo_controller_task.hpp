#ifndef THERMO_CONTROLLER_TASK_HPP
#define THERMO_CONTROLLER_TASK_HPP

class ThermoController
{
public:

    struct RegulatorParams
    {
        float setpoint;
        float hysteresis;
        float k;
    };

public:
    ThermoController()noexcept;
    ~ThermoController();

public:
    void processTemperatureControl()noexcept;
    int getSurroundingTemperature()noexcept;
    int getThermocoupleTemperature()noexcept;

    RegulatorParams getActualRegulatorParams()const noexcept;
    void setActualRegulatorParams(const RegulatorParams& params)noexcept;

private:
    class ThermoControllerImpl;
    ThermoControllerImpl* m_pImpl;
};

#endif