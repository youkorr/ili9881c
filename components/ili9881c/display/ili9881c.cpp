#include "ili9881c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

namespace esphome {
namespace ili9881c {

static const char *const TAG = "ili9881c";

void ILI9881C::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ILI9881C display with official ESP-IDF driver...");
  
#if !SOC_MIPI_DSI_SUPPORTED
  ESP_LOGE(TAG, "MIPI DSI not supported on this ESP32 variant");
  Component::mark_failed();
  return;
#endif

  // Configuration des pins
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
  }
  
  // Configurer MIPI DSI
  this->setup_mipi_dsi_();
  
  // Configurer DPI
  this->setup_dpi_config_();
  
  // Convertir les commandes d'init ESPHome vers le format ESP-IDF
  this->convert_init_commands_();
  
  if (!this->init_display_()) {
    Component::mark_failed();
    return;
  }
  
  // Calculer la taille du buffer
  size_t buffer_size = this->display_width_ * this->display_height_ * 3; // RGB888
  this->init_internal_(buffer_size);
  
  ESP_LOGCONFIG(TAG, "ILI9881C display setup completed");
}

void ILI9881C::setup_mipi_dsi_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Configuring MIPI DSI bus...");
  
  // Configuration du bus DSI avec les macros officielles
  esp_lcd_dsi_bus_config_t dsi_bus_config = {
    .bus_id = 0,
    .num_data_lanes = this->data_lanes_,
    .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
    .lane_bit_rate_mbps = this->lane_bit_rate_mbps_,
  };
  
  ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&dsi_bus_config, &this->dsi_bus_));
  
  // Configuration DBI selon la macro officielle
  esp_lcd_dbi_io_config_t dbi_config = {
    .virtual_channel = 0,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
  };
  
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(this->dsi_bus_, &dbi_config, &this->io_handle_));
  
  ESP_LOGD(TAG, "MIPI DSI bus configured successfully");
#endif
}

void ILI9881C::setup_dpi_config_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Configuring DPI...");
  
  // Allouer la config DPI
  this->dpi_config_ = (esp_lcd_dpi_panel_config_t*)malloc(sizeof(esp_lcd_dpi_panel_config_t));
  
  // Configuration DPI basée sur la macro officielle mais adaptée à nos dimensions
  *this->dpi_config_ = {
    .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
    .dpi_clock_freq_mhz = this->dpi_clk_freq_mhz_,
    .virtual_channel = 0,
    .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB888, // ou RGB565
    .num_fbs = 1,
    .video_timing = {
      .h_size = this->display_width_,
      .v_size = this->display_height_,
      .hsync_back_porch = this->hbp_,
      .hsync_pulse_width = this->hsync_,
      .hsync_front_porch = this->hfp_,
      .vsync_back_porch = this->vbp_,
      .vsync_pulse_width = this->vsync_,
      .vsync_front_porch = this->vfp_,
    },
    .flags = {
      .use_dma2d = true,
    }
  };
  
  ESP_LOGD(TAG, "DPI configured: %dx%d @ %d MHz", 
    this->display_width_, this->display_height_, this->dpi_clk_freq_mhz_);
#endif
}

bool ILI9881C::init_display_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Initializing display with official driver...");
  
  // Configuration du vendor avec nos commandes d'init
  this->vendor_config_.init_cmds = this->esp_init_commands_.data();
  this->vendor_config_.init_cmds_size = this->esp_init_commands_.size();
  this->vendor_config_.mipi_config.dsi_bus = this->dsi_bus_;
  this->vendor_config_.mipi_config.dpi_config = this->dpi_config_;
  this->vendor_config_.mipi_config.lane_num = this->data_lanes_;
  
  // Configuration du panel
  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = this->reset_pin_ ? this->reset_pin_->get_pin() : -1,
    .rgb_ele_order = this->color_order_ == COLOR_ORDER_RGB ? LCD_RGB_ELEMENT_ORDER_RGB : LCD_RGB_ELEMENT_ORDER_BGR,
    .bits_per_pixel = 24, // RGB888
    .flags = {
      .reset_active_high = 0,
    },
    .vendor_config = &this->vendor_config_,
  };
  
  // Créer le panel avec le driver officiel ILI9881C
  esp_err_t ret = esp_lcd_new_panel_ili9881c(this->io_handle_, &panel_config, &this->panel_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create ILI9881C panel: %s", esp_err_to_name(ret));
    return false;
  }
  
  // Reset et initialisation
  ESP_ERROR_CHECK(esp_lcd_panel_reset(this->panel_handle_));
  ESP_ERROR_CHECK(esp_lcd_panel_init(this->panel_handle_));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(this->panel_handle_, true));
  
  this->initialized_ = true;
  ESP_LOGD(TAG, "Display initialized successfully with official driver");
  return true;
#else
  ESP_LOGE(TAG, "MIPI DSI not supported");
  return false;
#endif
}








