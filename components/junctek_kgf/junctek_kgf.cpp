#include "junctek_kgf.h"
#include "esphome/core/log.h"
#include "esphome/core/optional.h"
#include <string>
#include <string.h>
#include <setjmp.h>
#include <cmath>

static jmp_buf parsing_failed;
static const char *const TAG = "JunkTek KG-F";

esphome::optional<int> try_getval(const char*& cursor)
{
  long val;
  const char* pos = cursor;
  char* end = nullptr;
  val = strtoll(pos, &end, 10);
  if (end == pos || end == nullptr)
  {
    return nullopt;
  }
  if (*end != ',' && *end != '.')
  {
    return nullopt;
  }
  cursor = end + 1; // Пропускаем запятую или точку
  return (int)val;
}

int getval(const char*& cursor)
{
  auto val = try_getval(cursor);
  if (!val)
  {
    longjmp(parsing_failed, 1);
  }
  return *val;
}

JuncTekKGF::JuncTekKGF(unsigned address, bool invert_current)
  : address_(address)
  , invert_current_(invert_current)
{
}

void JuncTekKGF::dump_config()
{
  ESP_LOGCONFIG(TAG, "junctek_kgf:");
  ESP_LOGCONFIG(TAG, "  Address: %d", this->address_);
  ESP_LOGCONFIG(TAG, "  Invert Current: %s", this->invert_current_ ? "True" : "False");
}

void JuncTekKGF::handle_settings(const char* buffer)
{
  const char* cursor = buffer;
  const int address = getval(cursor);

  if (address != this->address_)
    return;

  const int checksum = getval(cursor);
  if (! verify_checksum(checksum, cursor))
    return;

  // Пропускаем ненужные настройки до емкости батареи
  for(int i=0; i<8; i++) getval(cursor); 

  const float batteryAmpHourCapacity = getval(cursor) / 10.0;
  
  this->battery_capacity_ = batteryAmpHourCapacity;
  this->last_settings_ = esphome::millis();
}

void JuncTekKGF::handle_status(const char* buffer)
{
  ESP_LOGV(TAG, "Status: %s", buffer);
  const char* cursor = buffer;
  
  const int address = getval(cursor); // 1. Адрес
  if (address != this->address_) return;
 
  const int checksum = getval(cursor); // 2. Чексумма
  if (! verify_checksum(checksum, cursor)) return;

  const float voltage = getval(cursor) / 100.00; // 3. Напряжение (В)
  const float amps = getval(cursor) / 100.00; // 4. Ток (А)
  const float ampHourRemaining = getval(cursor) / 1000.0; // 5. Остаток емкости (А*ч)
  const float energyDischarged = getval(cursor) / 100000.0; // 6. Энергия разряда (кВт*ч)
  const float energyCharged = getval(cursor) / 100000.0; // 7. Энергия заряда (кВт*ч)
  
  getval(cursor); // 8. operational record value (пропускаем)
  
  const float temperature = getval(cursor) - 100.0; // 9. Температура (°C)
  
  getval(cursor); // 10. reserved (пропускаем)
  
  const int outputStatus = getval(cursor); // 11. Код статуса
  if (outputStatus != 99) {
    ESP_LOGW(TAG, "Junctek abnormal status: %d. Sensors update skipped.", outputStatus);
    // Обновляем только сенсор статуса, чтобы видеть ошибку в HA
    if (output_status_sensor_) this->output_status_sensor_->publish_state(outputStatus);
    return; 
  }
  
  const int direction = getval(cursor); // 12. Направление (0-разряд, 1-заряд)
  const int batteryLifeMinutes = getval(cursor); // 13. Оставшееся время (мин)
  
  getval(cursor); // 14. time adjustment (пропускаем)

  // --- ПУБЛИКАЦИЯ ДАННЫХ ---

  if (voltage_sensor_) 
    this->voltage_sensor_->publish_state(voltage);

  if (battery_level_sensor_ && this->battery_capacity_) {
    float battLvl = ampHourRemaining * 100.0 / *this->battery_capacity_;
    if(battLvl <= 100 && battLvl > 0)
      this->battery_level_sensor_->publish_state(battLvl);
  }

  // Расчет тока с учетом знака
  float adjustedCurrent = (direction == 0) ? amps : -amps;
  if (invert_current_) adjustedCurrent *= -1;

  if (current_sensor_) 
    this->current_sensor_->publish_state(adjustedCurrent);

  if (current_direction_sensor_)
    this->current_direction_sensor_->publish_state(direction == 0);

  if (energy_total_discharged_sensor_)
    this->energy_total_discharged_sensor_->publish_state(energyDischarged);

  if (energy_total_charged_sensor_)
    this->energy_total_charged_sensor_->publish_state(energyCharged);

  if (amp_hour_remain_sensor_)
    this->amp_hour_remain_sensor_->publish_state(ampHourRemaining);

  if (temperature_)
    this->temperature_->publish_state(temperature);

  if (output_status_sensor_)
    this->output_status_sensor_->publish_state(outputStatus);

  if (battery_life_sensor_)
    this->battery_life_sensor_->publish_state(batteryLifeMinutes);

  // Мощность (P = U * I)
  float watts = voltage * adjustedCurrent;
  if (power_sensor_) 
    this->power_sensor_->publish_state(watts);

  // Разделение мощности на заряд/разряд (Вт)
  if (battery_charged_power_sensor_)
    this->battery_charged_power_sensor_->publish_state((direction == 1) ? std::abs(watts) : 0.0f);

  if (battery_discharged_power_sensor_)
    this->battery_discharged_power_sensor_->publish_state((direction == 0) ? std::abs(watts) : 0.0f);

  this->last_stats_ = esphome::millis();
}

void JuncTekKGF::handle_line()
{
  if (setjmp(parsing_failed)){
    ESP_LOGE(TAG, "Parsing failed for line: %s", this->line_buffer_);
    return;
  }

  const char* buffer = &this->line_buffer_[0];
  if (buffer[0] != ':' || buffer[1] != 'r') return;

  if (strncmp(&buffer[2], "50=", 3) == 0)
    handle_status(&buffer[5]);
  else if (strncmp(&buffer[2], "51=", 3) == 0)
    handle_settings(&buffer[5]);
}

bool JuncTekKGF::verify_checksum(int checksum, const char* buffer)
{
  long total = 0;
  const char* temp_cursor = buffer;
  while (auto val = try_getval(temp_cursor))
  {
    total += *val;
  }
  return (total % 255) + 1 == checksum;
}

void JuncTekKGF::loop()
{
  while (available()) {
    if (this->readline()) {
      this->handle_line();
    }
  }
}

bool JuncTekKGF::readline()
{
  while (available()) {
    const char readch = read();
    if (readch > 0) {
      if (readch == '\n') {
        this->line_buffer_[this->line_pos_] = 0;
        this->line_pos_ = 0;
        return true;
      } else if (readch != '\r') {
        if (this->line_pos_ < MAX_LINE_LEN - 1) {
          this->line_buffer_[this->line_pos_++] = readch;
          this->line_buffer_[this->line_pos_] = 0;
        } else {
          this->line_pos_ = 0;
        }
      }
    }
  }
  return false;
}

float JuncTekKGF::get_setup_priority() const
{
  return setup_priority::DATA;
}
