#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/core/gpio.h"

#ifdef USE_ESP32

#include <vector>

// Inclure l'API officielle ESP-IDF
#if SOC_MIPI_DSI_SUPPORTED
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"

// Prototypes pour le driver ILI9881C officiel
typedef struct {
    int cmd;
    const void *data;
    size_t data_bytes;
    unsigned int delay_ms;
} ili9881c_lcd_init_cmd_t;

typedef struct {
    const ili9881c_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    struct {
        esp_lcd_dsi_bus_handle_t dsi_bus;
        const esp_lcd_dpi_panel_config_t *dpi_config;
        uint8_t lane_num;
    } mipi_config;
} ili9881c_vendor_config_t;

extern "C" esp_err_t esp_lcd_new_panel_ili9881c(const esp_lcd_panel_io_handle_t io, 
    const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);
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
  
  // Méthodes pour les timings MIPI DSI
  void set_hsync(uint16_t hsync) { this->hsync_ = hsync; }
  void set_hbp(uint16_t hbp) { this->hbp_ = hbp; }
  void set_hfp(uint16_t hfp) { this->hfp_ = hfp; }
  void set_vsync(uint16_t vsync) { this->vsync_ = vsync; }
  void set_vbp(uint16_t vbp) { this->vbp_ = vbp; }
  void set_vfp(uint16_t vfp) { this->vfp_ = vfp; }
  void set_data_lanes(uint8_t lanes) { this->data_lanes_ = lanes; }
  void set_lane_bit_rate_mbps(uint16_t rate) { this->lane_bit_rate_mbps_ = rate; }
  void set_dpi_clk_freq_mhz(uint8_t freq) { this->dpi_clk_freq_mhz_ = freq; }
  
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
  void setup_mipi_dsi_();
  void setup_dpi_config_();
  void convert_init_commands_();
  void send_display_buffer_();
  
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
  
  // Paramètres MIPI DSI
  uint8_t data_lanes_{2};
  uint16_t lane_bit_rate_mbps_{1000};
  uint8_t dpi_clk_freq_mhz_{80};
  
  // Timings MIPI DSI/DPI
  uint16_t hsync_{40};    // hsync_pulse_width
  uint16_t hbp_{140};     // hsync_back_porch  
  uint16_t hfp_{40};      // hsync_front_porch
  uint16_t vsync_{4};     // vsync_pulse_width
  uint16_t vbp_{16};      // vsync_back_porch
  uint16_t vfp_{16};      // vsync_front_porch
  
  std::vector<InitCommand> init_commands_;
  std::vector<ili9881c_lcd_init_cmd_t> esp_init_commands_;
  
#if SOC_MIPI_DSI_SUPPORTED
  esp_lcd_dsi_bus_handle_t dsi_bus_{nullptr};
  esp_lcd_panel_io_handle_t io_handle_{nullptr};
  esp_lcd_panel_handle_t panel_handle_{nullptr};
  esp_lcd_dpi_panel_config_t *dpi_config_{nullptr};
  ili9881c_vendor_config_t vendor_config_{};
#endif
  
  bool initialized_{false};
};

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32




