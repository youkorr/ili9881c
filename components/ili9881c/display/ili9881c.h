#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/gpio.h"

#ifdef USE_ESP32

#include <vector>

namespace esphome {
namespace ili9881c {

enum PixelFormat {
  RGB565 = 0,
  RGB888 = 1,
};

struct InitCommand {
  uint8_t cmd;
  std::vector<uint8_t> data;
  uint16_t delay_ms;
  bool is_delay;
};

class ILI9881C : public PollingComponent, public display::DisplayBuffer {
 public:
  void setup() override;
  void dump_config() override;
  void update() override;
  void loop() override;
  
  void set_dc_pin(GPIOPin *dc_pin) { this->dc_pin_ = dc_pin; }
  void set_reset_pin(GPIOPin *reset_pin) { this->reset_pin_ = reset_pin; }
  void set_backlight_pin(GPIOPin *backlight_pin) { this->backlight_pin_ = backlight_pin; }
  void set_dimensions(uint16_t width, uint16_t height);
  void set_offsets(uint16_t offset_x, uint16_t offset_y);
  void set_data_lanes(uint8_t lanes) { this->data_lanes_ = lanes; }
  void set_pixel_format(PixelFormat format) { this->pixel_format_ = format; }
  void set_invert_colors(bool invert) { this->invert_colors_ = invert; }
  void set_auto_clear_enabled(bool enable) { this->auto_clear_enabled_ = enable; }
  
  // Méthodes pour la séquence d'init personnalisée
  void clear_init_sequence() { this->init_commands_.clear(); }
  void add_init_command(uint8_t cmd, const std::vector<uint8_t> &data);
  void add_init_delay(uint16_t delay_ms);

  int get_width_internal() override { return this->width_; }
  int get_height_internal() override { return this->height_; }
  
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  void fill_internal(Color color) override;

  bool init_display_();
  void hard_reset_();
  void send_init_commands_();
  void write_command_(uint8_t cmd, const std::vector<uint8_t> &data = {});
  void switch_page_(uint8_t page);
  void write_display_data_();
  
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *backlight_pin_{nullptr};
  
  uint16_t width_{720};
  uint16_t height_{1280};
  uint16_t offset_x_{0};
  uint16_t offset_y_{0};
  uint8_t data_lanes_{2};
  PixelFormat pixel_format_{RGB565};
  bool invert_colors_{false};
  bool auto_clear_enabled_{true};
  
  std::vector<InitCommand> init_commands_;
  bool initialized_{false};
};

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32
