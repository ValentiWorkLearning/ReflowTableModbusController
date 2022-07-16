#include "thermo_controller_task.hpp"
#include "system_init.hpp"
#include "modbus_server_task.hpp"

ModbusServer modbusServer;
ThermoController thermoController;

void setup()
{
  systemns::initPeripheral();
  modbusServer.postSystemInit();
  modbusServer.registerRegulatorParamsChangeHandler(
      etl::delegate<void(const ThermoController::RegulatorParams &)>::create<ThermoController, &ThermoController::setActualRegulatorParams>(thermoController));
  modbusServer.registerRegulatorParamsGetter(
      etl::delegate<ThermoController::RegulatorParams()>::create<ThermoController, &ThermoController::getActualRegulatorParams>(thermoController));
}

void loop()
{
  modbusServer.pollRequests();
  modbusServer.setHeaterTemperature(thermoController.getThermocoupleTemperature());
  modbusServer.setSurroundingTemperature(thermoController.getSurroundingTemperature());
  thermoController.processTemperatureControl();
  systemns::heartbeat();
}
