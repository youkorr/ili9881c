#include "ili9881c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

namespace esphome {
namespace ili9881c {

static const char *const TAG = "ili9881c";

void ILI9881C::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ILI9881C display...");
  
  // Configuration des pins
  if (this->dc_pin_ != nullptr) {
    this->dc_pin_->setup();
    this->dc_pin_->digital_write(false);
  }
  
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
  }
  
  if (this->backlight_pin_ != nullptr) {
    this->backlight_pin_->setup();
    this->backlight_pin_->digital_write(false); // Éteint au démarrage
  }
  
  // Appliquer la rotation pour calculer les bonnes dimensions
  this->apply_rotation_();
  
  // Si aucune séquence d'init personnalisée n'est fournie, utiliser celle par défaut
  if (this->init_commands_.empty()) {
    this->setup_default_init_sequence_();
  }
  
  // Calculer la taille du buffer selon le format de pixel (utiliser les dimensions physiques)
  size_t buffer_size;
  if (this->pixel_format_ == RGB565) {
    buffer_size = this->display_width_ * this->display_height_ * 2; // 16 bits par pixel
  } else {
    buffer_size = this->display_width_ * this->display_height_ * 3; // 24 bits par pixel
  }
  
  this->init_internal_(buffer_size);
  
  if (!this->init_display_()) {
    this->mark_failed();
    return;
  }
  
  ESP_LOGCONFIG(TAG, "ILI9881C display setup completed");
}

void ILI9881C::setup_default_init_sequence_() {
  ESP_LOGD(TAG, "Using default initialization sequence");
  
  // Séquence d'initialisation par défaut basée sur ta configuration
  this->clear_init_sequence();
  
  // Page 3
  this->add_init_command(0xFF, {0x98, 0x81, 0x03});
  this->add_init_command(0x01, {0x00});
  this->add_init_command(0x02, {0x00});
  this->add_init_command(0x03, {0x73});
  this->add_init_command(0x04, {0x73});
  this->add_init_command(0x05, {0x00});
  this->add_init_command(0x06, {0x06});
  this->add_init_command(0x07, {0x02});
  this->add_init_command(0x08, {0x00});
  this->add_init_command(0x09, {0x01});
  this->add_init_command(0x0A, {0x00});
  this->add_init_command(0x0B, {0x00});
  this->add_init_command(0x0C, {0x01});
  this->add_init_command(0x0D, {0x00});
  this->add_init_command(0x0E, {0x00});
  this->add_init_command(0x0F, {0x00});
  this->add_init_command(0x10, {0x00});
  this->add_init_command(0x11, {0x00});
  this->add_init_command(0x12, {0x00});
  this->add_init_command(0x13, {0x00});
  this->add_init_command(0x14, {0x00});
  this->add_init_command(0x15, {0x00});
  this->add_init_command(0x16, {0x00});
  this->add_init_command(0x17, {0x00});
  this->add_init_command(0x18, {0x00});
  this->add_init_command(0x19, {0x00});
  this->add_init_command(0x1A, {0x00});
  this->add_init_command(0x1B, {0x00});
  this->add_init_command(0x1C, {0x00});
  this->add_init_command(0x1D, {0x00});
  this->add_init_command(0x1E, {0x40});
  this->add_init_command(0x1F, {0x80});
  this->add_init_command(0x20, {0x05});
  this->add_init_command(0x21, {0x02});
  this->add_init_command(0x22, {0x00});
  this->add_init_command(0x23, {0x00});
  this->add_init_command(0x24, {0x00});
  this->add_init_command(0x25, {0x00});
  this->add_init_command(0x26, {0x00});
  this->add_init_command(0x27, {0x00});
  this->add_init_command(0x28, {0x55});
  this->add_init_command(0x29, {0x03});
  this->add_init_command(0x2A, {0x00});
  this->add_init_command(0x2B, {0x00});
  this->add_init_command(0x2C, {0x00});
  this->add_init_command(0x2D, {0x00});
  this->add_init_command(0x2E, {0x00});
  this->add_init_command(0x2F, {0x00});
  this->add_init_command(0x30, {0x00});
  this->add_init_command(0x31, {0x00});
  this->add_init_command(0x32, {0x00});
  this->add_init_command(0x33, {0x00});
  this->add_init_command(0x34, {0x03});
  this->add_init_command(0x35, {0x00});
  this->add_init_command(0x36, {0x05});
  this->add_init_command(0x37, {0x00});
  this->add_init_command(0x38, {0x3C});
  this->add_init_command(0x39, {0x00});
  this->add_init_command(0x3A, {0x40});
  this->add_init_command(0x3B, {0x40});
  this->add_init_command(0x3C, {0x00});
  this->add_init_command(0x3D, {0x00});
  this->add_init_command(0x3E, {0x00});
  this->add_init_command(0x3F, {0x00});
  this->add_init_command(0x40, {0x00});
  this->add_init_command(0x41, {0x00});
  this->add_init_command(0x42, {0x00});
  this->add_init_command(0x43, {0x00});
  this->add_init_command(0x44, {0x00});
  
  // Configuration du mapping des pins
  this->add_init_command(0x50, {0x01});
  this->add_init_command(0x51, {0x23});
  this->add_init_command(0x52, {0x45});
  this->add_init_command(0x53, {0x67});
  this->add_init_command(0x54, {0x89});
  this->add_init_command(0x55, {0xAB});
  this->add_init_command(0x56, {0x01});
  this->add_init_command(0x57, {0x23});
  this->add_init_command(0x58, {0x45});
  this->add_init_command(0x59, {0x67});
  this->add_init_command(0x5A, {0x89});
  this->add_init_command(0x5B, {0xAB});
  this->add_init_command(0x5C, {0xCD});
  this->add_init_command(0x5D, {0xEF});
  this->add_init_command(0x5E, {0x01});
  this->add_init_command(0x5F, {0x14});
  this->add_init_command(0x60, {0x15});
  this->add_init_command(0x61, {0x0C});
  this->add_init_command(0x62, {0x0D});
  this->add_init_command(0x63, {0x0E});
  this->add_init_command(0x64, {0x0F});
  this->add_init_command(0x65, {0x10});
  this->add_init_command(0x66, {0x11});
  this->add_init_command(0x67, {0x08});
  this->add_init_command(0x68, {0x02});
  this->add_init_command(0x69, {0x0A});
  this->add_init_command(0x6A, {0x02});
  this->add_init_command(0x6B, {0x02});
  this->add_init_command(0x6C, {0x02});
  this->add_init_command(0x6D, {0x02});
  this->add_init_command(0x6E, {0x02});
  this->add_init_command(0x6F, {0x02});
  this->add_init_command(0x70, {0x02});
  this->add_init_command(0x71, {0x02});
  this->add_init_command(0x72, {0x06});
  this->add_init_command(0x73, {0x02});
  this->add_init_command(0x74, {0x02});
  this->add_init_command(0x75, {0x14});
  this->add_init_command(0x76, {0x15});
  this->add_init_command(0x77, {0x0F});
  this->add_init_command(0x78, {0x0E});
  this->add_init_command(0x79, {0x0D});
  this->add_init_command(0x7A, {0x0C});
  this->add_init_command(0x7B, {0x11});
  this->add_init_command(0x7C, {0x10});
  this->add_init_command(0x7D, {0x06});
  this->add_init_command(0x7E, {0x02});
  this->add_init_command(0x7F, {0x0A});
  this->add_init_command(0x80, {0x02});
  this->add_init_command(0x81, {0x02});
  this->add_init_command(0x82, {0x02});
  this->add_init_command(0x83, {0x02});
  this->add_init_command(0x84, {0x02});
  this->add_init_command(0x85, {0x02});
  this->add_init_command(0x86, {0x02});
  this->add_init_command(0x87, {0x02});
  this->add_init_command(0x88, {0x08});
  this->add_init_command(0x89, {0x02});
  this->add_init_command(0x8A, {0x02});
  
  // Page 4 - Paramètres de puissance
  this->add_init_command(0xFF, {0x98, 0x81, 0x04});
  this->add_init_command(0x6C, {0x15}); // VCORE voltage = 1.5V
  this->add_init_command(0x6E, {0x2A}); // VGH clamp 18V
  this->add_init_command(0x6F, {0x33}); // pumping ratio VGH=5x VGL=-3x
  this->add_init_command(0x8D, {0x1F}); // VGL clamp
  this->add_init_command(0x87, {0xBA}); // ESD
  this->add_init_command(0x26, {0x76});
  this->add_init_command(0xB2, {0xD1});
  this->add_init_command(0xB5, {0x27});
  
  // Page 1 - Paramètres gamma et couleur
  this->add_init_command(0xFF, {0x98, 0x81, 0x01});
  this->add_init_command(0x22, {0x0A}); // BGR, SS
  this->add_init_command(0x31, {0x00}); // Inversion
  this->add_init_command(0x53, {0x78});
  this->add_init_command(0x55, {0x7B});
  this->add_init_command(0x50, {0x87});
  this->add_init_command(0x51, {0x82});
  this->add_init_command(0x60, {0x15});
  this->add_init_command(0x61, {0x01});
  this->add_init_command(0x62, {0x0C});
  this->add_init_command(0x63, {0x00});
  
  // Gamma positif
  this->add_init_command(0xA0, {0x00}); // VP255
  this->add_init_command(0xA1, {0x13}); // VP251
  this->add_init_command(0xA2, {0x23}); // VP247
  this->add_init_command(0xA3, {0x14}); // VP243
  this->add_init_command(0xA4, {0x16}); // VP239
  this->add_init_command(0xA5, {0x29}); // VP231
  this->add_init_command(0xA6, {0x1E}); // VP219
  this->add_init_command(0xA7, {0x1D}); // VP203
  this->add_init_command(0xA8, {0x86}); // VP175
  this->add_init_command(0xA9, {0x1E}); // VP144
  this->add_init_command(0xAA, {0x29}); // VP111
  this->add_init_command(0xAB, {0x74}); // VP80
  this->add_init_command(0xAC, {0x19}); // VP52
  this->add_init_command(0xAD, {0x17}); // VP36
  this->add_init_command(0xAE, {0x4B}); // VP24
  this->add_init_command(0xAF, {0x20}); // VP16
  
  // Gamma négatif
  this->add_init_command(0xC0, {0x00}); // VN255
  this->add_init_command(0xC1, {0x13}); // VN251
  this->add_init_command(0xC2, {0x23}); // VN247
  this->add_init_command(0xC3, {0x14}); // VN243
  this->add_init_command(0xC4, {0x16}); // VN239
  this->add_init_command(0xC5, {0x29}); // VN231
  this->add_init_command(0xC6, {0x1E}); // VN219
  this->add_init_command(0xC7, {0x1D}); // VN203
  this->add_init_command(0xC8, {0x86}); // VN175
  this->add_init_command(0xC9, {0x1E}); // VN144
  this->add_init_command(0xCA, {0x29}); // VN111
  this->add_init_command(0xCB, {0x74}); // VN80
  this->add_init_command(0xCC, {0x19}); // VN52
  this->add_init_command(0xCD, {0x17}); // VN36
  this->add_init_command(0xCE, {0x4B}); // VN24
  this->add_init_command(0xCF, {0x20}); // VN16
  
  // Retour à la page 0
  this->add_init_command(0xFF, {0x98, 0x81, 0x00});
  
  // Commandes finales
  this->add_init_command(0x11, {}); // Sleep Out
  this->add_init_delay(120); // Délai 120ms
  this->add_init_command(0x29, {}); // Display On
  this->add_init_delay(20); // Délai 20ms
}

void ILI9881C::dump_config() {
  ESP_LOGCONFIG(TAG, "ILI9881C Display:");
  ESP_LOGCONFIG(TAG, "  Physical Size: %dx%d", this->display_width_, this->display_height_);
  ESP_LOGCONFIG(TAG, "  Effective Size: %dx%d", this->width_, this->height_);
  ESP_LOGCONFIG(TAG, "  Rotation: %d°", (int)this->rotation_ * 90);
  ESP_LOGCONFIG(TAG, "  Offset X: %d", this->offset_x_);
  ESP_LOGCONFIG(TAG, "  Offset Y: %d", this->offset_y_);
  ESP_LOGCONFIG(TAG, "  Data Lanes: %d", this->data_lanes_);
  ESP_LOGCONFIG(TAG, "  Pixel Format: %s", this->pixel_format_ == RGB565 ? "RGB565" : "RGB888");
  ESP_LOGCONFIG(TAG, "  Invert Colors: %s", YESNO(this->invert_colors_));
  ESP_LOGCONFIG(TAG, "  Auto Clear Enabled: %s", YESNO(this->auto_clear_enabled_));
  ESP_LOGCONFIG(TAG, "  Init Commands: %d", this->init_commands_.size());
  LOG_PIN("  DC Pin: ", this->dc_pin_);
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

// Méthodes pour rotation
void ILI9881C::set_rotation(Rotation rotation) {
  this->rotation_ = rotation;
  this->apply_rotation_();
}

void ILI9881C::apply_rotation_() {
  // Calculer les nouvelles dimensions selon la rotation
  if (this->rotation_ == ROTATION_90 || this->rotation_ == ROTATION_270) {
    // Rotation de 90° ou 270° : inverser largeur et hauteur
    this->width_ = this->display_height_;
    this->height_ = this->display_width_;
  } else {
    // Rotation de 0° ou 180° : garder les dimensions originales
    this->width_ = this->display_width_;
    this->height_ = this->display_height_;
  }
  
  ESP_LOGD(TAG, "Applied rotation %d°: %dx%d", (int)this->rotation_ * 90, this->width_, this->height_);
}

int ILI9881C::get_width_internal() {
  return this->width_;
}

int ILI9881C::get_height_internal() {
  return this->height_;
}

void ILI9881C::get_rotated_coordinates_(int x, int y, int &rotated_x, int &rotated_y) {
  switch (this->rotation_) {
    case ROTATION_0:
      rotated_x = x;
      rotated_y = y;
      break;
    case ROTATION_90:
      rotated_x = y;
      rotated_y = this->display_width_ - 1 - x;
      break;
    case ROTATION_180:
      rotated_x = this->display_width_ - 1 - x;
      rotated_y = this->display_height_ - 1 - y;
      break;
    case ROTATION_270:
      rotated_x = this->display_height_ - 1 - y;
      rotated_y = x;
      break;
  }
}

void ILI9881C::set_dimensions(uint16_t width, uint16_t height) {
  this->display_width_ = width;
  this->display_height_ = height;
  this->apply_rotation_(); // Recalculer les dimensions avec la rotation actuelle
}

void ILI9881C::set_offsets(uint16_t offset_x, uint16_t offset_y) {
  this->offset_x_ = offset_x;
  this->offset_y_ = offset_y;
}

// Méthodes pour la séquence d'init
void ILI9881C::add_init_command(uint8_t cmd, const std::vector<uint8_t> &data) {
  InitCommand init_cmd;
  init_cmd.cmd = cmd;
  init_cmd.data = data;
  init_cmd.delay_ms = 0;
  init_cmd.is_delay = false;
  this->init_commands_.push_back(init_cmd);
}

void ILI9881C::add_init_delay(uint16_t delay_ms) {
  InitCommand init_cmd;
  init_cmd.cmd = 0;
  init_cmd.data = {};
  init_cmd.delay_ms = delay_ms;
  init_cmd.is_delay = true;
  this->init_commands_.push_back(init_cmd);
}

void ILI9881C::clear_init_sequence() {
  this->init_commands_.clear();
}

bool ILI9881C::init_display_() {
  ESP_LOGD(TAG, "Initializing display...");
  
  // Reset matériel
  this->hard_reset_();
  
  // Envoi des commandes d'initialisation
  this->send_init_commands_();
  
  // Configuration de la zone d'affichage
  this->set_addr_window_(0, 0, this->display_width_ - 1, this->display_height_ - 1);
  
  // Activation du rétroéclairage si configuré
  if (this->backlight_pin_ != nullptr) {
    ESP_LOGD(TAG, "Enabling backlight");
    this->backlight_pin_->digital_write(true);
  }
  
  this->initialized_ = true;
  ESP_LOGD(TAG, "Display initialization completed");
  
  return true;
}

void ILI9881C::hard_reset_() {
  if (this->reset_pin_ != nullptr) {
    ESP_LOGD(TAG, "Performing hardware reset");
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(120);
  }
}

void ILI9881C::send_init_commands_() {
  ESP_LOGD(TAG, "Sending %d initialization commands...", this->init_commands_.size());
  
  for (const auto &cmd : this->init_commands_) {
    if (cmd.is_delay) {
      ESP_LOGVV(TAG, "Delay: %dms", cmd.delay_ms);
      delay(cmd.delay_ms);
    } else {
      ESP_LOGVV(TAG, "Command: 0x%02X with %d data bytes", cmd.cmd, cmd.data.size());
      this->write_command_(cmd.cmd, cmd.data);
    }
  }
  
  // Configurer la rotation matérielle après les commandes d'init
  this->set_hardware_rotation_();
}

void ILI9881C::write_command_(uint8_t cmd, const std::vector<uint8_t> &data) {
  // Set DC pin low for command
  if (this->dc_pin_ != nullptr) {
    this->dc_pin_->digital_write(false);
  }
  
  // Pour une implémentation SPI simple (à adapter pour MIPI DSI sur ESP32-P4)
  this->write_byte_(cmd);
  
  if (!data.empty()) {
    // Set DC pin high for data
    if (this->dc_pin_ != nullptr) {
      this->dc_pin_->digital_write(true);
    }
    
    for (uint8_t byte : data) {
      this->write_byte_(byte);
    }
  }
}

void ILI9881C::write_byte_(uint8_t data) {
  // TODO: Implémenter l'écriture via SPI ou MIPI DSI
  // Pour l'instant, c'est un placeholder
  ESP_LOGVV(TAG, "Writing byte: 0x%02X", data);
  
  // Pour SPI, vous utiliseriez quelque chose comme :
  // spi_device_transmit() ou équivalent
  
  // Pour MIPI DSI sur ESP32-P4, vous utiliseriez :
  // esp_lcd_panel_io_tx_param() ou équivalent
}

void ILI9881C::set_addr_window_(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  // Ajuster avec les offsets
  x1 += this->offset_x_;
  x2 += this->offset_x_;
  y1 += this->offset_y_;
  y2 += this->offset_y_;
  
  // CASET - Column Address Set
  this->write_command_(0x2A, {
    static_cast<uint8_t>(x1 >> 8),
    static_cast<uint8_t>(x1 & 0xFF),
    static_cast<uint8_t>(x2 >> 8),
    static_cast<uint8_t>(x2 & 0xFF)
  });
  
  // RASET - Row Address Set
  this->write_command_(0x2B, {
    static_cast<uint8_t>(y1 >> 8),
    static_cast<uint8_t>(y1 & 0xFF),
    static_cast<uint8_t>(y2 >> 8),
    static_cast<uint8_t>(y2 & 0xFF)
  });
  
  // RAMWR - Memory Write
  this->write_command_(0x2C, {});
}

void ILI9881C::write_display_data_() {
  if (!this->initialized_) {
    return;
  }
  
  ESP_LOGVV(TAG, "Writing display data...");
  
  // Configurer la zone d'écriture
  this->set_addr_window_(0, 0, this->display_width_ - 1, this->display_height_ - 1);
  
  // Set DC pin high for data
  if (this->dc_pin_ != nullptr) {
    this->dc_pin_->digital_write(true);
  }
  
  // Écrire les données du buffer
  size_t buffer_size = this->get_buffer_length_internal_();
  
  // TODO: Optimiser l'écriture par blocs
  for (size_t i = 0; i < buffer_size; i++) {
    this->write_byte_(this->buffer_[i]);
  }
}

void ILI9881C::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0) {
    return;
  }
  
  // Appliquer la rotation
  int rotated_x, rotated_y;
  this->get_rotated_coordinates_(x, y, rotated_x, rotated_y);
  
  if (this->pixel_format_ == RGB565) {
    // 16-bit per pixel
    uint32_t color565 = display::ColorUtil::color_to_565(color);
    if (this->invert_colors_) {
      color565 = ~color565;
    }
    
    size_t pos = (rotated_y * this->display_width_ + rotated_x) * 2;
    if (pos + 1 < this->get_buffer_length_internal_()) {
      this->buffer_[pos] = (color565 >> 8) & 0xFF;
      this->buffer_[pos + 1] = color565 & 0xFF;
    }
  } else {
    // RGB888 - 24-bit per pixel  
    size_t pos = (rotated_y * this->display_width_ + rotated_x) * 3;
    if (pos + 2 < this->get_buffer_length_internal_()) {
      if (this->invert_colors_) {
        this->buffer_[pos] = 255 - color.red;
        this->buffer_[pos + 1] = 255 - color.green;
        this->buffer_[pos + 2] = 255 - color.blue;
      } else {
        this->buffer_[pos] = color.red;
        this->buffer_[pos + 1] = color.green;
        this->buffer_[pos + 2] = color.blue;
      }
    }
  }
}

size_t ILI9881C::get_buffer_length_internal_() {
  if (this->pixel_format_ == RGB565) {
    return this->display_width_ * this->display_height_ * 2;
  } else {
    return this->display_width_ * this->display_height_ * 3;
  }
}

void ILI9881C::switch_page_(uint8_t page) {
  this->write_command_(0xFF, {0x98, 0x81, page});
}

void ILI9881C::set_hardware_rotation_() {
  // Configurer le registre MADCTL (0x36) pour la rotation matérielle
  uint8_t madctl = 0x00;
  
  switch (this->rotation_) {
    case ROTATION_0:
      madctl = 0x00;  // Normal
      break;
    case ROTATION_90:
      madctl = 0x60;  // Row/Col exchange + Col reverse
      break;
    case ROTATION_180:
      madctl = 0xC0;  // Row reverse + Col reverse
      break;
    case ROTATION_270:
      madctl = 0xA0;  // Row/Col exchange + Row reverse
      break;
  }
  
  // Ajuster selon l'inversion des couleurs
  if (this->invert_colors_) {
    madctl |= 0x08; // BGR bit
  }
  
  ESP_LOGD(TAG, "Setting hardware rotation: MADCTL = 0x%02X", madctl);
  this->write_command_(0x36, {madctl});
}

// Méthodes utilitaires pour l'affichage
void ILI9881C::display_off_() {
  this->write_command_(0x28, {}); // Display OFF
}

void ILI9881C::display_on_() {
  this->write_command_(0x29, {}); // Display ON
}

void ILI9881C::sleep_mode_() {
  this->write_command_(0x10, {}); // Enter Sleep Mode
}

void ILI9881C::wake_up_() {
  this->write_command_(0x11, {}); // Sleep Out
  delay(120); // Délai requis après Sleep Out
}

void ILI9881C::set_brightness_(uint8_t brightness) {
  // Ajuster la luminosité via le rétroéclairage ou les registres de l'écran
  this->write_command_(0x51, {brightness}); // Write Display Brightness
}

void ILI9881C::invert_display_(bool invert) {
  if (invert) {
    this->write_command_(0x21, {}); // Display Inversion ON
  } else {
    this->write_command_(0x20, {}); // Display Inversion OFF
  }
}

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32


