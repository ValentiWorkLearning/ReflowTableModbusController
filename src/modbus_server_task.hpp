#ifndef MODBUS_SERVER_TASK_HPP
#define MODBUS_SERVER_TASK_HPP

#define ETL_NO_STL
#include <thermo_controller_task.hpp>
#include <etl/delegate.h>

class ModbusServer
{
public:
    ModbusServer() noexcept;
    ~ModbusServer() noexcept;

public:
    void postSystemInit() noexcept;
    void pollRequests() noexcept;
    void setSurroundingTemperature(int temperature) noexcept;
    void setHeaterTemperature(int temperature) noexcept;

    using TParamsGetter = etl::delegate<ThermoController::RegulatorParams()>;
    void registerRegulatorParamsGetter(TParamsGetter getter);

    using TParamsObserver = etl::delegate<void(const ThermoController::RegulatorParams &)>;
    void registerRegulatorParamsChangeHandler(TParamsObserver paramsObserver);

public:
    class ModbusServerImpl;
    ModbusServerImpl *m_pImpl;
};

#endif