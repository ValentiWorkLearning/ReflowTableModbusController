#include "modbus_server_task.hpp"
#include <ModbusSlave.h>

#include "system_config.h"

#define ETL_NO_STL
#include <etl/flat_map.h>
#include <etl/delegate.h>

u8 writeDigitalOutCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept;
u8 readHoldingRegistersCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept;
u8 writeHoldingRegistersCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept;

class ModbusServer::ModbusServerImpl
{
public:
    ModbusServerImpl() noexcept
        : m_slave(Serial, modbus::kDeviceId, modbus::krs485ControlPin)
    {
        m_modbusRequestsMultiplexer[modbus::address::kSurroundingTempAddr] =
            ModbusRegisterHandler{
                .readHandler = etl::delegate<u16()>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::getSurroundingTemperature>(*this)};

        m_modbusRequestsMultiplexer[modbus::address::kThermocoupleAddr] =
            ModbusRegisterHandler{
                .readHandler = etl::delegate<u16()>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::getReflowTableTemperature>(*this)};

        m_modbusRequestsMultiplexer[modbus::address::kFactorAddr] =
            ModbusRegisterHandler{
                .readHandler = etl::delegate<u16()>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::getRegulatorKFactor>(*this),
                .writeHandler = etl::delegate<void(u16)>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::setRegulatorKFactor>(*this)};

        m_modbusRequestsMultiplexer[modbus::address::kHysteresisAddr] =
            ModbusRegisterHandler{
                .readHandler = etl::delegate<u16()>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::getHysteresisValue>(*this),
                .writeHandler = etl::delegate<void(u16)>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::setHysteresisValue>(*this)};

        m_modbusRequestsMultiplexer[modbus::address::kTargetTemperatureAddr] =
            ModbusRegisterHandler{
                .readHandler = etl::delegate<u16()>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::getTargetTemperature>(*this),
                .writeHandler = etl::delegate<void(u16)>::create<ModbusServer::ModbusServerImpl, &ModbusServerImpl::setTargetTemperature>(*this)};
    }

    void pollRequests() noexcept
    {
        m_slave.poll();
    }

    void postSystemInit() noexcept
    {
        pinMode(LED_BUILTIN, OUTPUT);

        m_slave.cbVector[CB_WRITE_COILS] = writeDigitalOutCb;
        m_slave.cbVector[CB_READ_HOLDING_REGISTERS] = readHoldingRegistersCb;
        m_slave.cbVector[CB_WRITE_HOLDING_REGISTERS] = writeHoldingRegistersCb;

        m_slave.setCallbackContext(this);

        m_slave.begin(kSerialBaud);
    }

    void setSurroundingTemperature(int temperature) noexcept
    {
        m_lastSurroundingTemperature = temperature;
    }

    void setHeaterTemperature(int temperature) noexcept
    {
        m_heaterTemperature = temperature;
    }

    void handleCoilsWrite() noexcept
    {
        auto builtinLedValue = m_slave.readCoilFromBuffer(0);
        digitalWrite(LED_BUILTIN, builtinLedValue);
    }

    u8 handleReadHoldingRequest(u16 address, u16 length) noexcept
    {
        const u16 actualAddress = address;

        if (!checkAddress(actualAddress, length))
            return STATUS_ILLEGAL_DATA_ADDRESS;

        u16 arrayIdx{};
        for (u16 addrIt{actualAddress}; addrIt < actualAddress + length; ++addrIt)
        {
            u16 propertyValue = getProperyByAddress(addrIt);
            m_slave.writeRegisterToBuffer(arrayIdx, propertyValue);
            ++arrayIdx;
        }
        return STATUS_OK;
    }

    u8 handleWriteHoldingRegisterRequest(u16 address, u16 length) noexcept
    {
        const u16 actualAddress = address;

        if (!checkAddress(actualAddress, length))
            return STATUS_ILLEGAL_DATA_ADDRESS;

        u16 arrayIdx{};
        for (u16 addrIt{actualAddress}; addrIt < actualAddress + length; ++addrIt)
        {
            auto propertyAccessor = getPropertyWriteAccessor(addrIt);
            auto newPropertyValue = m_slave.readRegisterFromBuffer(arrayIdx);

            if(!propertyAccessor)
                return STATUS_ILLEGAL_FUNCTION;

            propertyAccessor(newPropertyValue);

            ++arrayIdx;
        }
        return STATUS_OK;
    }
    
    void registerRegulatorParamsGetter(ModbusServer::TParamsGetter getter)
    {
        m_regulatorParamsGetter = getter;
    }

    void registerRegulatorParamsChangeHandler(ModbusServer::TParamsObserver paramsObserver)
    {
        m_paramsObserver = paramsObserver;
    }

private:
    bool checkAddress(u16 address, u16 length) const noexcept
    {
        for (u16 addrIt{address}; addrIt < address + length; ++addrIt)
        {
            auto it = m_modbusRequestsMultiplexer.find(addrIt);
            if (it == m_modbusRequestsMultiplexer.end())
                return false;
        }
        return true;
    }

    u16 getProperyByAddress(u16 address) const noexcept
    {
        auto it = m_modbusRequestsMultiplexer.find(address);
        return it->second.readHandler();
    }


    etl::delegate<void(u16)> getPropertyWriteAccessor(u16 address)
    {
        auto it = m_modbusRequestsMultiplexer.find(address);
        return it->second.writeHandler;
    }

    u16 getSurroundingTemperature() const noexcept
    {
        return m_lastSurroundingTemperature;
    }

    u16 getReflowTableTemperature() const noexcept
    {
        return m_heaterTemperature;
    }

    u16 getRegulatorKFactor() const noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        return currentRegulatorParams.k * 10;
    }

    void setRegulatorKFactor(u16 factor) noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        currentRegulatorParams.k = static_cast<float>(factor) / 10.f;
        m_paramsObserver(currentRegulatorParams);
    }

    u16 getHysteresisValue() const noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        return currentRegulatorParams.hysteresis;
    }

    void setHysteresisValue(u16 newHysteresis) noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        currentRegulatorParams.hysteresis= newHysteresis;
        m_paramsObserver(currentRegulatorParams);
    }

    u16 getTargetTemperature() const noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        return currentRegulatorParams.setpoint;
    }

    void setTargetTemperature(u16 newSetpoint) noexcept
    {
        auto currentRegulatorParams = m_regulatorParamsGetter();
        currentRegulatorParams.setpoint= newSetpoint;
        m_paramsObserver(currentRegulatorParams);
    }

private:
    struct ModbusRegisterHandler
    {
        etl::delegate<u16()> readHandler;
        etl::delegate<void(u16)> writeHandler;
    };

private:
    int m_lastSurroundingTemperature;
    int m_heaterTemperature;

    ModbusServer::TParamsGetter m_regulatorParamsGetter;
    ModbusServer::TParamsObserver m_paramsObserver;

    etl::flat_map<u16, ModbusRegisterHandler, modbus::address::kModbusRegistersCount> m_modbusRequestsMultiplexer;
    Modbus m_slave;
};

ModbusServer::ModbusServer() noexcept
    : m_pImpl{new ModbusServerImpl()}
{
}

ModbusServer::~ModbusServer()
{
    delete m_pImpl;
}

void ModbusServer::postSystemInit() noexcept
{
    m_pImpl->postSystemInit();
}

void ModbusServer::pollRequests() noexcept
{
    m_pImpl->pollRequests();
}
void ModbusServer::setSurroundingTemperature(int temperature) noexcept
{
    m_pImpl->setSurroundingTemperature(temperature);
}

void ModbusServer::setHeaterTemperature(int temperature) noexcept
{
    m_pImpl->setHeaterTemperature(temperature);
}

void ModbusServer::registerRegulatorParamsGetter(TParamsGetter getter)
{
    m_pImpl->registerRegulatorParamsGetter(getter);
}

void ModbusServer::registerRegulatorParamsChangeHandler(TParamsObserver paramsObserver)
{
    m_pImpl->registerRegulatorParamsChangeHandler(paramsObserver);
}

u8 writeDigitalOutCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept
{
    const bool isAddrValid = (address == modbus::LED_COIL_ADDR);
    if (!isAddrValid)
    {
        return STATUS_ILLEGAL_DATA_ADDRESS;
    }

    auto modbusServerImpl = reinterpret_cast<ModbusServer::ModbusServerImpl *>(callbackContext);
    modbusServerImpl->handleCoilsWrite();
    return STATUS_OK;
}

u8 readHoldingRegistersCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept
{
    auto modbusServerImpl = reinterpret_cast<ModbusServer::ModbusServerImpl *>(callbackContext);
    return modbusServerImpl->handleReadHoldingRequest(address, length);
}

u8 writeHoldingRegistersCb(u8 fc, u16 address, u16 length, void *callbackContext) noexcept
{
    auto modbusServerImpl = reinterpret_cast<ModbusServer::ModbusServerImpl *>(callbackContext);
    return modbusServerImpl->handleWriteHoldingRegisterRequest(address, length);

}