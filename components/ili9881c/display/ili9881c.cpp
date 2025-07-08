#include "ili9881c.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

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
  
  // Appliquer la rotation pour calculer les bonnes dimensions
  this->apply_rotation_();
  
  // Utiliser la séquence d'init fournie ou celle par défaut
  if (this->init_commands_.empty()) {
    this->setup_default_init_sequence_();
  }
  
  // Buffer 24-bit RGB (3 bytes par pixel) pour tous les pixels
  size_t buffer_size = this->display_width_ * this->display_height_ * 3;
  this->init_internal_(buffer_size);
  
  if (!this->init_display_()) {
    Component::mark_failed();
    return;
  }
  
  ESP_LOGCONFIG(TAG, "ILI9881C display setup completed");
}

void ILI9881C::setup_default_init_sequence_() {
  ESP_LOGD(TAG, "Setting up minimal initialization sequence");
  
  this->clear_init_sequence();
  
  // Séquence minimale pour test
  this->add_init_command(0x01, {}); // Software Reset
  this->add_init_delay(100); 
  
  this->add_init_command(0x11, {}); // Sleep Out
  this->add_init_delay(120);
  
  this->add_init_command(0x3A, {0x66}); // Interface Pixel Format - 18bit/pixel RGB666
  
  this->add_init_command(0x29, {}); // Display On
  this->add_init_delay(20);
}

bool ILI9881C::init_display_() {
  ESP_LOGD(TAG, "Initializing display...");
  
  // Reset matériel
  this->hard_reset_();
  
  // Envoyer la séquence d'initialisation
  this->send_init_commands_();
  
  // Test simple : remplir l'écran avec un pattern de couleurs
  this->fill_screen_test_();
  
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
  // Set DC low pour commande
  if (this->dc_pin_ != nullptr) {
    this->dc_pin_->digital_write(false);
  }
  
  this->write_byte_(cmd);
  
  if (!data.empty()) {
    // Set DC high pour données
    if (this->dc_pin_ != nullptr) {
      this->dc_pin_->digital_write(true);
    }
    
    for (uint8_t byte : data) {
      this->write_byte_(byte);
    }
  }
}

void ILI9881C::write_byte_(uint8_t data) {
  // Implementation simple pour test - à remplacer par SPI ou MIPI DSI
  ESP_LOGVV(TAG, "Writing byte: 0x%02X", data);
  // TODO: Implémenter la vraie communication
  delayMicroseconds(1);
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

void ILI9881C::apply_color_order_(uint8_t &r, uint8_t &g, uint8_t &b) {
  if (this->color_order_ == COLOR_ORDER_BGR) {
    // Échanger R et B pour BGR
    uint8_t temp = r;
    r = b;
    b = temp;
  }
  // Pour RGB, pas de changement nécessaire
}

void ILI9881C::fill_screen_test_() {
  ESP_LOGD(TAG, "Filling screen with colored test pattern");
  
  // Créer un pattern de test avec différentes couleurs
  size_t pixels_total = this->display_width_ * this->display_height_;
  size_t pixels_per_band = pixels_total / 4;
  
  for (size_t i = 0; i < pixels_total; i++) {
    uint8_t r, g, b;
    
    // 4 bandes de couleurs
    if (i < pixels_per_band) {
      r = 255; g = 0; b = 0;    // Rouge
    } else if (i < 2 * pixels_per_band) {
      r = 0; g = 255; b = 0;    // Vert
    } else if (i < 3 * pixels_per_band) {
      r = 0; g = 0; b = 255;    // Bleu
    } else {
      r = 255; g = 255; b = 255; // Blanc
    }
    
    // Appliquer l'ordre des couleurs
    this->apply_color_order_(r, g, b);
    
    // Écrire dans le buffer
    size_t pos = i * 3;
    if (pos + 2 < this->get_buffer_length_internal_()) {
      this->buffer_[pos] = r;
      this->buffer_[pos + 1] = g;
      this->buffer_[pos + 2] = b;
    }
  }
  
  this->write_display_data_();
}

void ILI9881C::write_display_data_() {
  if (!this->initialized_) {
    return;
  }
  
  ESP_LOGVV(TAG, "Writing display data...");
  
  // Configuration de la zone d'écriture
  this->set_addr_window_(0, 0, this->display_width_ - 1, this->display_height_ - 1);
  
  // Set DC high pour données
  if (this->dc_pin_ != nullptr) {
    this->dc_pin_->digital_write(true);
  }
  
  // Écriture du buffer (optimisé par blocs plus tard)
  size_t buffer_size = this->get_buffer_length_internal_();
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
  
  // Extraire les composantes de couleur
  uint8_t r = color.red;
  uint8_t g = color.green;
  uint8_t b = color.blue;
  
  // Appliquer l'inversion des couleurs
  if (this->invert_colors_) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  
  // Appliquer l'ordre des couleurs (RGB vs BGR)
  this->apply_color_order_(r, g, b);
  
  // RGB888 - 24-bit per pixel  
  size_t pos = (rotated_y * this->display_width_ + rotated_x) * 3;
  if (pos + 2 < this->get_buffer_length_internal_()) {
    this->buffer_[pos] = r;
    this->buffer_[pos + 1] = g;
    this->buffer_[pos + 2] = b;
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

void ILI9881C::dump_config() {
  ESP_LOGCONFIG(TAG, "ILI9881C Display:");
  ESP_LOGCONFIG(TAG, "  Physical Size: %dx%d", this->display_width_, this->display_height_);
  ESP_LOGCONFIG(TAG, "  Effective Size: %dx%d", this->width_, this->height_);
  ESP_LOGCONFIG(TAG, "  Rotation: %d°", (int)this->rotation_);
  ESP_LOGCONFIG(TAG, "  Color Order: %s", this->color_order_ == COLOR_ORDER_RGB ? "RGB" : "BGR");
  ESP_LOGCONFIG(TAG, "  Offset X: %d", this->offset_x_);
  ESP_LOGCONFIG(TAG, "  Offset Y: %d", this->offset_y_);
  ESP_LOGCONFIG(TAG, "  Invert Colors: %s", YESNO(this->invert_colors_));
  ESP_LOGCONFIG(TAG, "  Auto Clear Enabled: %s", YESNO(this->auto_clear_enabled_));
  ESP_LOGCONFIG(TAG, "  Init Commands: %d", this->init_commands_.size());
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  
  if (Component::is_failed()) {
    ESP_LOGE(TAG, "Failed to initialize display");
  }
}

// Méthodes de gestion de la rotation
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
  
  ESP_LOGD(TAG, "Applied rotation %d°: %dx%d", (int)this->rotation_, this->width_, this->height_);
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

// Méthodes pour la séquence d'init personnalisée
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
  
  // Ajuster selon l'ordre des couleurs
  if (this->color_order_ == COLOR_ORDER_BGR) {
    madctl |= 0x08; // BGR bit
  }
  
  ESP_LOGD(TAG, "Setting hardware rotation: MADCTL = 0x%02X", madctl);
  this->write_command_(0x36, {madctl});
}

size_t ILI9881C::get_buffer_length_internal_() {
  return this->display_width_ * this->display_height_ * 3; // RGB888 = 3 bytes par pixel
}

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32






