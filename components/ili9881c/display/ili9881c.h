#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/gpio.h"

#ifdef USE_ESP32

#include <vector>

#if SOC_MIPI_DSI_SUPPORTED
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#endif

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
  
  void set_data_lanes(uint8_t lanes) { this->data_lanes_ = lanes; }
  void set_lane_bit_rate_mbps(uint16_t rate) { this->lane_bit_rate_mbps_ = rate; }
  void set_dpi_clk_freq_mhz(uint8_t freq) { this->dpi_clk_freq_mhz_ = freq; }
  
  void set_hsync(uint16_t hsync) { this->hsync_ = hsync; }
  void set_hbp(uint16_t hbp) { this->hbp_ = hbp; }
  void set_hfp(uint16_t hfp) { this->hfp_ = hfp; }
  void set_vsync(uint16_t vsync) { this->vsync_ = vsync; }
  void set_vbp(uint16_t vbp) { this->vbp_ = vbp; }
  void set_vfp(uint16_t vfp) { this->vfp_ = vfp; }
  
  void set_rotation(int rotation) { 
    this->set_rotation(static_cast<Rotation>(rotation)); 
  }
  
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
  void setup_default_init_sequence_();
  void setup_mipi_dsi_();
  void setup_dpi_config_();
  void send_display_buffer_();
  size_t get_buffer_length_internal_();
  
  GPIOPin *dc_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  
  uint16_t display_width_{720};
  uint16_t display_height_{1280};
  uint16_t offset_x_{0};
  uint16_t offset_y_{0};
  bool invert_colors_{false};
  bool auto_clear_enabled_{true};
  Rotation rotation_{ROTATION_0};
  ColorOrder color_order_{COLOR_ORDER_RGB};
  
  uint8_t data_lanes_{2};
  uint16_t lane_bit_rate_mbps_{1000};
  uint8_t dpi_clk_freq_mhz_{80};
  
  uint16_t hsync_{40};
  uint16_t hbp_{140};
  uint16_t hfp_{40};
  uint16_t vsync_{4};
  uint16_t vbp_{16};
  uint16_t vfp_{16};
  
  std::vector<InitCommand> init_commands_;
  
#if SOC_MIPI_DSI_SUPPORTED
  esp_lcd_dsi_bus_handle_t dsi_bus_{nullptr};
  esp_lcd_panel_io_handle_t io_handle_{nullptr};
  esp_lcd_panel_handle_t dpi_panel_{nullptr};
  esp_lcd_dpi_panel_config_t *dpi_config_{nullptr};
#endif
  
  bool initialized_{false};
};

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32






