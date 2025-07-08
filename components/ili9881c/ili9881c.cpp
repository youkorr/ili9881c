#include "ili9881c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <driver/gpio.h>

namespace esphome {
namespace ili9881c {

static const char *const TAG = "ili9881c";

void ILI9881C::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ILI9881C display...");
  
  // Configuration des commandes d'initialisation basées sur ta config
  this->init_commands_ = {
    // Page 3
    {0xFF, {0x98, 0x81, 0x03}, 0},
    {0x01, {0x00}, 0},
    {0x02, {0x00}, 0},
    {0x03, {0x73}, 0},
    {0x04, {0x73}, 0},
    {0x05, {0x00}, 0},
    {0x06, {0x06}, 0},
    {0x07, {0x02}, 0},
    {0x08, {0x00}, 0},
    {0x09, {0x01}, 0},
    {0x0A, {0x00}, 0},
    {0x0B, {0x00}, 0},
    {0x0C, {0x01}, 0},
    {0x0D, {0x00}, 0},
    {0x0E, {0x00}, 0},
    {0x0F, {0x00}, 0},
    {0x10, {0x00}, 0},
    {0x11, {0x00}, 0},
    {0x12, {0x00}, 0},
    {0x13, {0x00}, 0},
    {0x14, {0x00}, 0},
    {0x15, {0x00}, 0},
    {0x16, {0x00}, 0},
    {0x17, {0x00}, 0},
    {0x18, {0x00}, 0},
    {0x19, {0x00}, 0},
    {0x1A, {0x00}, 0},
    {0x1B, {0x00}, 0},
    {0x1C, {0x00}, 0},
    {0x1D, {0x00}, 0},
    {0x1E, {0x40}, 0},
    {0x1F, {0x80}, 0},
    {0x20, {0x05}, 0},
    {0x21, {0x02}, 0},
    {0x22, {0x00}, 0},
    {0x23, {0x00}, 0},
    {0x24, {0x00}, 0},
    {0x25, {0x00}, 0},
    {0x26, {0x00}, 0},
    {0x27, {0x00}, 0},
    {0x28, {0x55}, 0},
    {0x29, {0x03}, 0},
    {0x2A, {0x00}, 0},
    {0x2B, {0x00}, 0},
    {0x2C, {0x00}, 0},
    {0x2D, {0x00}, 0},
    {0x2E, {0x00}, 0},
    {0x2F, {0x00}, 0},
    {0x30, {0x00}, 0},
    {0x31, {0x00}, 0},
    {0x32, {0x00}, 0},
    {0x33, {0x00}, 0},
    {0x34, {0x03}, 0},
    {0x35, {0x00}, 0},
    {0x36, {0x05}, 0},
    {0x37, {0x00}, 0},
    {0x38, {0x3C}, 0},
    {0x39, {0x00}, 0},
    {0x3A, {0x40}, 0},
    {0x3B, {0x40}, 0},
    {0x3C, {0x00}, 0},
    {0x3D, {0x00}, 0},
    {0x3E, {0x00}, 0},
    {0x3F, {0x00}, 0},
    {0x40, {0x00}, 0},
    {0x41, {0x00}, 0},
    {0x42, {0x00}, 0},
    {0x43, {0x00}, 0},
    {0x44, {0x00}, 0},
    
    // Configuration du mapping des pins
    {0x50, {0x01}, 0},
    {0x51, {0x23}, 0},
    {0x52, {0x45}, 0},
    {0x53, {0x67}, 0},
    {0x54, {0x89}, 0},
    {0x55, {0xAB}, 0},
    {0x56, {0x01}, 0},
    {0x57, {0x23}, 0},
    {0x58, {0x45}, 0},
    {0x59, {0x67}, 0},
    {0x5A, {0x89}, 0},
    {0x5B, {0xAB}, 0},
    {0x5C, {0xCD}, 0},
    {0x5D, {0xEF}, 0},
    {0x5E, {0x01}, 0},
    {0x5F, {0x14}, 0},
    {0x60, {0x15}, 0},
    {0x61, {0x0C}, 0},
    {0x62, {0x0D}, 0},
    {0x63, {0x0E}, 0},
    {0x64, {0x0F}, 0},
    {0x65, {0x10}, 0},
    {0x66, {0x11}, 0},
    {0x67, {0x08}, 0},
    {0x68, {0x02}, 0},
    {0x69, {0x0A}, 0},
    {0x6A, {0x02}, 0},
    {0x6B, {0x02}, 0},
    {0x6C, {0x02}, 0},
    {0x6D, {0x02}, 0},
    {0x6E, {0x02}, 0},
    {0x6F, {0x02}, 0},
    {0x70, {0x02}, 0},
    {0x71, {0x02}, 0},
    {0x72, {0x06}, 0},
    {0x73, {0x02}, 0},
    {0x74, {0x02}, 0},
    {0x75, {0x14}, 0},
    {0x76, {0x15}, 0},
    {0x77, {0x0F}, 0},
    {0x78, {0x0E}, 0},
    {0x79, {0x0D}, 0},
    {0x7A, {0x0C}, 0},
    {0x7B, {0x11}, 0},
    {0x7C, {0x10}, 0},
    {0x7D, {0x06}, 0},
    {0x7E, {0x02}, 0},
    {0x7F, {0x0A}, 0},
    {0x80, {0x02}, 0},
    {0x81, {0x02}, 0},
    {0x82, {0x02}, 0},
    {0x83, {0x02}, 0},
    {0x84, {0x02}, 0},
    {0x85, {0x02}, 0},
    {0x86, {0x02}, 0},
    {0x87, {0x02}, 0},
    {0x88, {0x08}, 0},
    {0x89, {0x02}, 0},
    {0x8A, {0x02}, 0},
    
    // Page 4 - Paramètres de puissance
    {0xFF, {0x98, 0x81, 0x04}, 0},
    {0x6C, {0x15}, 0},  // VCORE voltage = 1.5V
    {0x6E, {0x2A}, 0},  // VGH clamp 18V
    {0x6F, {0x33}, 0},  // pumping ratio VGH=5x VGL=-3x
    {0x8D, {0x1F}, 0},  // VGL clamp
    {0x87, {0xBA}, 0},  // ESD
    {0x26, {0x76}, 0},
    {0xB2, {0xD1}, 0},
    {0xB5, {0x27}, 0},
    
    // Page 1 - Paramètres gamma et couleur
    {0xFF, {0x98, 0x81, 0x01}, 0},
    {0x22, {0x0A}, 0},  // BGR, SS
    {0x31, {0x00}, 0},  // Inversion
    {0x53, {0x78}, 0},
    {0x55, {0x7B}, 0},
    {0x50, {0x87}, 0},
    {0x51, {0x82}, 0},
    {0x60, {0x15}, 0},
    {0x61, {0x01}, 0},
    {0x62, {0x0C}, 0},
    {0x63, {0x00}, 0},
    
    // Gamma positif
    {0xA0, {0x00}, 0},  // VP255
    {0xA1, {0x13}, 0},  // VP251
    {0xA2, {0x23}, 0},  // VP247
    {0xA3, {0x14}, 0},  // VP243
    {0xA4, {0x16}, 0},  // VP239
    {0xA5, {0x29}, 0},  // VP231
    {0xA6, {0x1E}, 0},  // VP219
    {0xA7, {0x1D}, 0},  // VP203
    {0xA8, {0x86}, 0},  // VP175
    {0xA9, {0x1E}, 0},  // VP144
    {0xAA, {0x29}, 0},  // VP111
    {0xAB, {0x74}, 0},  // VP80
    {0xAC, {0x19}, 0},  // VP52
    {0xAD, {0x17}, 0},  // VP36
    {0xAE, {0x4B}, 0},  // VP24
    {0xAF, {0x20}, 0},  // VP16
    
    // Gamma négatif
    {0xC0, {0x00}, 0},  // VN255
    {0xC1, {0x13}, 0},  // VN251
    {0xC2, {0x23}, 0},  // VN247
    {0xC3, {0x14}, 0},  // VN243
    {0xC4, {0x16}, 0},  // VN239
    {0xC5, {0x29}, 0},  // VN231
    {0xC6, {0x1E}, 0},  // VN219
    {0xC7, {0x1D}, 0},  // VN203
    {0xC8, {0x86}, 0},  // VN175
    {0xC9, {0x1E}, 0},  // VN144
    {0xCA, {0x29}, 0},  // VN111
    {0xCB, {0x74}, 0},  // VN80
    {0xCC, {0x19}, 0},  // VN52
    {0xCD, {0x17}, 0},  // VN36
    {0xCE, {0x4B}, 0},  // VN24
    {0xCF, {0x20}, 0},  // VN16
    
    // Retour à la page 0
    {0xFF, {0x98, 0x81, 0x00}, 0},
    
    // Commandes finales
    {0x11, {0x00}, 120},  // Sleep Out + délai 120ms
    {0x29, {0x00}, 20},   // Display On + délai 20ms
  };
  
  this->init_internal_(this->get_buffer_length_());
  
  if (!this->init_display_()) {
    this->mark_failed();
    return;
  }
  
  ESP_LOGCONFIG(TAG, "ILI9881C display setup completed");
}

void ILI9881C::dump_config() {
  ESP_LOGCONFIG(TAG, "ILI9881C Display:");
  ESP_LOGCONFIG(TAG, "  Width: %d", this->width_);
  ESP_LOGCONFIG(TAG, "  Height: %d", this->height_);
  ESP_LOGCONFIG(TAG, "  Data Lanes: %d", this->data_lanes_);
  ESP_LOGCONFIG(TAG, "  Pixel Format: %s", this->pixel_format_ == RGB565 ? "RGB565" : "RGB888");
  ESP_LOGCONFIG(TAG, "  Invert Colors: %s", YESNO(this->invert_colors_));
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  LOG_PIN("  Backlight Pin: ", this->backlight_pin_);
  
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to initialize display");
  }
}

void ILI9881C::update() {
  if (!this->initialized_) {
    return;
  }
  
  this->do_update_();
  this->write_display_data_();
}

void ILI9881C::loop() {
  // Rien à faire dans la boucle pour ce pilote
}

void ILI9881C::set_dimensions(uint16_t width, uint16_t height) {
  this->width_ = width;
  this->height_ = height;
}

void ILI9881C::set_offsets(uint16_t offset_x, uint16_t offset_y) {
  this->offset_x_ = offset_x;
  this->offset_y_ = offset_y;
}

bool ILI9881C::init_display_() {
  ESP_LOGD(TAG, "Initializing display...");
  
  // Reset matériel
  this->hard_reset_();
  
  // Envoi des commandes d'initialisation
  this->send_init_commands_();
  
  // Activation du rétroéclairage si configuré
  if (this->backlight_pin_ != nullptr) {
    this->backlight_pin_->setup();
    this->backlight_pin_->digital_write(true);
  }
  
  this->initialized_ = true;
  ESP_LOGD(TAG, "Display initialization completed");
  
  return true;
}

void ILI9881C::hard_reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(false);
    delay(20);
    this->reset_pin_->digital_write(true);
    delay(120);
  }
}

void ILI9881C::send_init_commands_() {
  ESP_LOGD(TAG, "Sending %d initialization commands...", this->init_commands_.size());
  
  for (const auto &cmd : this->init_commands_) {
    this->write_command_(cmd.cmd, cmd.data);
    if (cmd.delay_ms > 0) {
      delay(cmd.delay_ms);
    }
  }
}

void ILI9881C::write_command_(uint8_t cmd, const std::vector<uint8_t> &data) {
  // Pour l'ESP32-P4 avec MIPI DSI, cette fonction devrait utiliser
  // les API ESP-IDF appropriées pour envoyer les commandes via DSI
  // Pour l'instant, c'est un placeholder
  
  ESP_LOGVV(TAG, "Sending command 0x%02X with %d data bytes", cmd, data.size());
  
  // TODO: Implémenter l'envoi via MIPI DSI
  // esp_lcd_panel_io_tx_param() ou équivalent pour DSI
}

void ILI9881C::switch_page_(uint8_t page) {
  this->write_command_(0xFF, {0x98, 0x81, page});
}

void ILI9881C::write_display_data_() {
  if (!this->initialized_) {
    return;
  }
  
  // TODO: Implémenter l'écriture des données pixel via MIPI DSI
  ESP_LOGVV(TAG, "Writing display data...");
}

void ILI9881C::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0) {
    return;
  }
  
  uint32_t color565 = display::ColorUtil::color_to_565(color);
  
  if (this->pixel_format_ == RGB565) {
    // 16-bit per pixel
    size_t pos = (y * this->width_ + x) * 2;
    this->buffer_[pos] = (color565 >> 8) & 0xFF;
    this->buffer_[pos + 1] = color565 & 0xFF;
  } else {
    // RGB888 - 24-bit per pixel  
    size_t pos = (y * this->width_ + x) * 3;
    this->buffer_[pos] = color.red;
    this->buffer_[pos + 1] = color.green;
    this->buffer_[pos + 2] = color.blue;
  }
}

void ILI9881C::fill_internal(Color color) {
  uint32_t color565 = display::ColorUtil::color_to_565(color);
  
  if (this->pixel_format_ == RGB565) {
    uint16_t color_packed = color565;
    for (size_t i = 0; i < this->get_buffer_length_internal_(); i += 2) {
      this->buffer_[i] = (color_packed >> 8) & 0xFF;
      this->buffer_[i + 1] = color_packed & 0xFF;
    }
  } else {
    for (size_t i = 0; i < this->get_buffer_length_internal_(); i += 3) {
      this->buffer_[i] = color.red;
      this->buffer_[i + 1] = color.green;
      this->buffer_[i + 2] = color.blue;
    }
  }
}

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32
