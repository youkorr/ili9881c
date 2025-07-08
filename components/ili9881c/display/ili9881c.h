#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/gpio.h"

#ifdef USE_ESP32

#include <vector>

namespace esphome {
namespace ili9881c {

enum Rotation : uint8_t {
  ROTATION_0 = 0,
  ROTATION_90 = 1,
  ROTATION_180 = 2,
  ROTATION_270 = 3,
};

enum ColorOrder : uint8_t {
  COLOR_ORDER_RGB = 0,
  COLOR_ORDER_BGR = 1,
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
  void set_dimensions(uint16_t width, uint16_t height);
  void set_offsets(uint16_t offset_x, uint16_t offset_y);
  void set_invert_colors(bool invert) { this->invert_colors_ = invert; }
  void set_auto_clear_enabled(bool enable) { this->auto_clear_enabled_ = enable; }
  void set_rotation(Rotation rotation);
  void set_color_order(ColorOrder color_order) { this->color_order_ = color_order; }
  
  // Méthodes pour les timings MIPI DSI
  void set_hsync(uint16_t hsync) { this->hsync_ = hsync; }
  void set_hbp(uint16_t hbp) { this->hbp_ = hbp; }
  void set_hfp(uint16_t hfp) { this->hfp_ = hfp; }
  void set_vsync(uint16_t vsync) { this->vsync_ = vsync; }
  void set_vbp(uint16_t vbp) { this->vbp_ = vbp; }
  void set_vfp(uint16_t vfp) { this->vfp_ = vfp; }
  
  // Surcharge pour compatibilité ESPHome
  void set_rotation(int rotation) { 
    this->set_rotation(static_cast<Rotation>(rotation)); 
  }
  
  // Méthodes pour la séquence d'init personnalisée
  void clear_init_sequence();
  void add_init_command(uint8_t cmd, const std::vector<uint8_t> &data);
  void add_init_delay(uint16_t delay_ms);

  int get_width_internal() override;
  int get_height_internal() override;
  
  display::DisplayType get_display_type() override { 
    return display::DisplayType::DISPLAY_TYPE_COLOR; 
  }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  bool init_display_();
  void hard_reset_();
  void send_init_commands_();
  void write_command_(uint8_t cmd, const std::vector<uint8_t> &data = {});
  void write_display_data_();
  void set_addr_window_(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
  void write_byte_(uint8_t data);
  void setup_default_init_sequence_();
  void apply_rotation_();
  void get_rotated_coordinates_(int x, int y, int &rotated_x, int &rotated_y);
  void set_hardware_rotation_();
  size_t get_buffer_length_internal_();
  void fill_screen_test_();
  void apply_color_order_(uint8_t &r, uint8_t &g, uint8_t &b);
  
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  
  uint16_t width_{720};
  uint16_t height_{1280};
  uint16_t display_width_{720};
  uint16_t display_height_{1280};
  uint16_t offset_x_{0};
  uint16_t offset_y_{0};
  bool invert_colors_{false};
  bool auto_clear_enabled_{true};
  Rotation rotation_{ROTATION_0};
  ColorOrder color_order_{COLOR_ORDER_RGB};
  
  // Timings MIPI DSI
  uint16_t hsync_{10};   // BSP_LCD_MIPI_DSI_LCD_HSYNC
  uint16_t hbp_{40};     // BSP_LCD_MIPI_DSI_LCD_HBP  
  uint16_t hfp_{40};     // BSP_LCD_MIPI_DSI_LCD_HFP
  uint16_t vsync_{4};    // BSP_LCD_MIPI_DSI_LCD_VSYNC
  uint16_t vbp_{16};     // BSP_LCD_MIPI_DSI_LCD_VBP
  uint16_t vfp_{16};     // BSP_LCD_MIPI_DSI_LCD_VFP
  
  std::vector<InitCommand> init_commands_;
  bool initialized_{false};
};

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32



