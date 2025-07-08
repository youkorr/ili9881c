#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/gpio.h"

#ifdef USE_ESP32

#include <vector>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_mipi_dsi.h>

namespace esphome {
namespace ili9881c {

enum PixelFormat : uint8_t {
  RGB565 = 0,
  RGB888 = 1,
};

enum Rotation : uint8_t {
  ROTATION_0 = 0,
  ROTATION_90 = 1,
  ROTATION_180 = 2,
  ROTATION_270 = 3,
};

struct InitCommand {
  uint8_t cmd;
  std::vector<uint8_t> data;
  uint16_t delay_ms;
  bool is_delay;
};

class ILI9881C : public display::DisplayBuffer {
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
  void set_rotation(Rotation rotation);
  
  // Surcharges pour compatibilité ESPHome
  void set_rotation(int rotation) { 
    this->set_rotation(static_cast<Rotation>(rotation)); 
  }
  void set_pixel_format(int format) { 
    this->set_pixel_format(static_cast<PixelFormat>(format)); 
  }
  
  // Méthodes pour la séquence d'init personnalisée
  void clear_init_sequence();
  void add_init_command(uint8_t cmd, const std::vector<uint8_t> &data);
  void add_init_delay(uint16_t delay_ms);

  int get_width_internal() override;
  int get_height_internal() override;
  
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  bool init_display_();
  void hard_reset_();
  void send_init_commands_();
  void write_command_(uint8_t cmd, const std::vector<uint8_t> &data = {});
  void setup_mipi_dsi_();
  void write_display_data_();
  void set_addr_window_(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
  void setup_default_init_sequence_();
  void apply_rotation_();
  void get_rotated_coordinates_(int x, int y, int &rotated_x, int &rotated_y);
  void set_hardware_rotation_();
  size_t get_buffer_length_internal_();
  void fill_screen_test_();
  
  // Callbacks MIPI DSI
  static bool mipi_dsi_callback_(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
  
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  GPIOPin *backlight_pin_{nullptr};
  
  uint16_t width_{720};
  uint16_t height_{1280};
  uint16_t display_width_{720};
  uint16_t display_height_{1280};
  uint16_t offset_x_{0};
  uint16_t offset_y_{0};
  uint8_t data_lanes_{2};
  PixelFormat pixel_format_{RGB565};
  bool invert_colors_{false};
  bool auto_clear_enabled_{true};
  Rotation rotation_{ROTATION_0};
  
  std::vector<InitCommand> init_commands_;
  bool initialized_{false};
  
  // Handles MIPI DSI
  esp_lcd_dsi_bus_handle_t dsi_bus_{nullptr};
  esp_lcd_panel_io_handle_t io_handle_{nullptr};
  esp_lcd_panel_handle_t panel_handle_{nullptr};
};

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32




