import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_BATTERY_LEVEL,
    CONF_DIRECTION,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_VOLT,
    UNIT_CELSIUS,
    UNIT_AMPERE,
    UNIT_WATT,
    UNIT_PERCENT,
    UNIT_KILOWATT_HOURS,
    UNIT_MINUTE,
    ICON_FLASH,
    ICON_PERCENT,
    ICON_THERMOMETER,
    ICON_BATTERY,
    ICON_TIMER,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_DURATION,
)

UNIT_AMPER_HOURS = 'Ah'
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

# Новые названия констант для соответствия логике
CONF_ENERGY_TOTAL_DISCHARGED = "energy_total_discharged"
CONF_ENERGY_TOTAL_CHARGED = "energy_total_charged"
CONF_BATTERY_CHARGED_POWER = 'battery_charged_power'
CONF_BATTERY_DISCHARGED_POWER = 'battery_discharged_power'

CONF_AMP_HOUR_REMAIN = "amp_hour_remain"
CONF_BATTERY_LIFE = 'battery_life'
CONF_OUTPUT_STATUS = "output_status"
CONF_POWER = "power"
CONF_CURRENT_DIRECTION = "current_direction"

TYPES = {
    CONF_VOLTAGE: "set_voltage_sensor",
    CONF_CURRENT: "set_current_sensor",
    CONF_BATTERY_LEVEL: "set_battery_level_sensor",
    CONF_TEMPERATURE: "set_temperature_sensor",
    CONF_DIRECTION: "set_current_direction_sensor",
    CONF_POWER: "set_power_sensor",
    CONF_BATTERY_LIFE: "set_battery_life_sensor",
    CONF_OUTPUT_STATUS: "set_output_status_sensor",
    CONF_AMP_HOUR_REMAIN: "set_amp_hour_remain_sensor",
    # Переименованные связки с C++
    CONF_ENERGY_TOTAL_DISCHARGED: "set_energy_total_discharged_sensor",
    CONF_ENERGY_TOTAL_CHARGED: "set_energy_total_charged_sensor",
    CONF_BATTERY_CHARGED_POWER: "set_battery_charged_power_sensor",
    CONF_BATTERY_DISCHARGED_POWER: "set_battery_discharged_power_sensor",
}

CONF_INVERT_CURRENT = "invert_current"
CONF_UPDATE_SETTINGS_INTERVAL = "update_settings_interval"
CONF_UPDATE_STATS_INTERVAL = "update_stats_interval"

JuncTekKGF = cg.global_ns.class_("JuncTekKGF", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JuncTekKGF),
            cv.Optional(CONF_ADDRESS, default=1): cv.int_range(1, 255),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT                
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon="mdi:current-dc",
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
             cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DIRECTION): sensor.sensor_schema(
                accuracy_decimals=0,
                icon="mdi:directions"
            ),
            cv.Optional(CONF_OUTPUT_STATUS): sensor.sensor_schema(
                accuracy_decimals=0,
                icon="mdi:list-status"
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                icon="mdi:flash",
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_LIFE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MINUTE,
                icon=ICON_TIMER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DURATION,
                state_class=STATE_CLASS_MEASUREMENT,
             ),
            # Мгновенная мощность заряда/разряда
            cv.Optional(CONF_BATTERY_CHARGED_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                icon="mdi:battery-arrow-up",
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_DISCHARGED_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                icon="mdi:battery-arrow-down",
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AMP_HOUR_REMAIN): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPER_HOURS,
                icon=ICON_BATTERY,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            # Накопленная энергия в кВт*ч
            cv.Optional(CONF_ENERGY_TOTAL_DISCHARGED): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                icon="mdi:lightning-bolt",
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_TOTAL_INCREASING,
            ),
            cv.Optional(CONF_ENERGY_TOTAL_CHARGED): sensor.sensor_schema(
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                icon="mdi:lightning-bolt-outline",
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_TOTAL_INCREASING,
            ),

            cv.Optional(CONF_INVERT_CURRENT, default=False): cv.boolean,
            cv.Optional(CONF_UPDATE_SETTINGS_INTERVAL, default=30000): cv.int_,
            cv.Optional(CONF_UPDATE_STATS_INTERVAL, default=1000): cv.int_,
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_ADDRESS], config[CONF_INVERT_CURRENT])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    
    for key, method_name in TYPES.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, method_name)(sens))

    cg.add(var.set_update_settings_interval(config[CONF_UPDATE_SETTINGS_INTERVAL]))
    cg.add(var.set_update_stats_interval(config[CONF_UPDATE_STATS_INTERVAL]))
