#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

using namespace esphome;

class JuncTekKGF
  : public esphome::Component
  , public uart::UARTDevice
{
public:
  JuncTekKGF(unsigned address = 1, bool invert_current=false);

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature) { temperature_ = temperature; }
  void set_update_settings_interval(uint32_t interval) { update_settings_interval_ = interval; }
  void set_update_stats_interval(uint32_t interval) { update_stats_interval_ = interval; }
  void set_current_direction_sensor(sensor::Sensor *current_direction_sensor) { current_direction_sensor_ = current_direction_sensor; }
  void set_battery_level_sensor(sensor::Sensor *battery_level_sensor) { battery_level_sensor_ = battery_level_sensor; }
  void set_amp_hour_remain_sensor(sensor::Sensor *amp_hour_remain_sensor) { amp_hour_remain_sensor_ = amp_hour_remain_sensor; }
  
  // Энергия (кВт*ч)
  void set_energy_total_discharged_sensor(sensor::Sensor *energy_sensor) { energy_total_discharged_sensor_ = energy_sensor; }
  void set_energy_total_charged_sensor(sensor::Sensor *energy_sensor) { energy_total_charged_sensor_ = energy_sensor; }
  
  // Мощность (Вт)
  void set_battery_charged_power_sensor(sensor::Sensor *power_sensor) { battery_charged_power_sensor_ = power_sensor; }
  void set_battery_discharged_power_sensor(sensor::Sensor *power_sensor) { battery_discharged_power_sensor_ = power_sensor; }
  
  void set_output_status_sensor(sensor::Sensor *output_status_sensor) { output_status_sensor_ = output_status_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_battery_life_sensor(sensor::Sensor *battery_life_sensor) { battery_life_sensor_ = battery_life_sensor; }

  void dump_config() override;
  void loop() override;
  float get_setup_priority() const;

protected:
  bool readline();
  void handle_line();
  void handle_status(const char* buffer);
  void handle_settings(const char* buffer);
  bool verify_checksum(int checksum, const char* buffer);

  const unsigned address_;
  uint32_t update_settings_interval_ = 30000;
  uint32_t update_stats_interval_ = 1000;

  sensor::Sensor* voltage_sensor_{nullptr};
  sensor::Sensor* current_sensor_{nullptr};
  sensor::Sensor* temperature_{nullptr};
  sensor::Sensor* current_direction_sensor_{nullptr};
  sensor::Sensor* battery_level_sensor_{nullptr};
  sensor::Sensor* amp_hour_remain_sensor_{nullptr};

  sensor::Sensor* energy_total_discharged_sensor_{nullptr};
  sensor::Sensor* energy_total_charged_sensor_{nullptr};
  sensor::Sensor* battery_charged_power_sensor_{nullptr};
  sensor::Sensor* battery_discharged_power_sensor_{nullptr};

  sensor::Sensor* output_status_sensor_{nullptr};
  sensor::Sensor* power_sensor_{nullptr};
  sensor::Sensor* battery_life_sensor_{nullptr};

  static constexpr int MAX_LINE_LEN = 120;
  char line_buffer_[MAX_LINE_LEN];
  size_t line_pos_ = 0;

  optional<float> battery_capacity_;
  optional<unsigned long> last_settings_;
  optional<unsigned long> last_stats_;
  bool invert_current_;
};
