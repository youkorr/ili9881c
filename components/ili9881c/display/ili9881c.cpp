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
  
  // Séquence d'init par défaut si aucune fournie
  if (this->init_commands_.empty()) {
    this->setup_default_init_sequence_();
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
  
  // Calculer la taille du buffer (921,600 pixels * 3 bytes = 2,764,800 bytes)
  size_t buffer_size = this->get_buffer_length_internal_();
  this->init_internal_(buffer_size);
  
  ESP_LOGCONFIG(TAG, "ILI9881C display setup completed");
}

void ILI9881C::setup_default_init_sequence_() {
  ESP_LOGD(TAG, "Setting up default initialization sequence");
  
  this->clear_init_sequence();
  
  // Séquence minimale basée sur les commandes standards ILI9881C
  this->add_init_command(0x01, {}); // Software Reset
  this->add_init_delay(100);
  
  this->add_init_command(0x11, {}); // Sleep Out
  this->add_init_delay(120);
  
  this->add_init_command(0x3A, {0x77}); // Interface Pixel Format - 24bit/pixel RGB888
  
  this->add_init_command(0x29, {}); // Display On
  this->add_init_delay(20);
}

void ILI9881C::setup_mipi_dsi_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Configuring MIPI DSI bus...");
  
  // Configuration du bus DSI avec les paramètres utilisateur
  esp_lcd_dsi_bus_config_t dsi_bus_config = {
    .bus_id = 0,
    .num_data_lanes = this->data_lanes_,
    .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
    .lane_bit_rate_mbps = this->lane_bit_rate_mbps_,
  };
  
  esp_err_t ret = esp_lcd_new_dsi_bus(&dsi_bus_config, &this->dsi_bus_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create DSI bus: %s", esp_err_to_name(ret));
    return;
  }
  
  // Configuration DBI selon la macro officielle
  esp_lcd_dbi_io_config_t dbi_config = {
    .virtual_channel = 0,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
  };
  
  ret = esp_lcd_new_panel_io_dbi(this->dsi_bus_, &dbi_config, &this->io_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
    return;
  }
  
  ESP_LOGD(TAG, "MIPI DSI bus configured successfully");
#endif
}

void ILI9881C::setup_dpi_config_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Configuring DPI...");
  
  // Allouer la config DPI
  this->dpi_config_ = (esp_lcd_dpi_panel_config_t*)malloc(sizeof(esp_lcd_dpi_panel_config_t));
  if (!this->dpi_config_) {
    ESP_LOGE(TAG, "Failed to allocate DPI config");
    return;
  }
  
  // Configuration DPI - Structure simplifiée sans designated initializers
  memset(this->dpi_config_, 0, sizeof(esp_lcd_dpi_panel_config_t));
  this->dpi_config_->dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT;
  this->dpi_config_->dpi_clock_freq_mhz = this->dpi_clk_freq_mhz_;
  this->dpi_config_->virtual_channel = 0;
  this->dpi_config_->pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB888;
  this->dpi_config_->num_fbs = 1;
  
  // Video timings
  this->dpi_config_->video_timing.h_size = this->display_width_;
  this->dpi_config_->video_timing.v_size = this->display_height_;
  this->dpi_config_->video_timing.hsync_back_porch = this->hbp_;
  this->dpi_config_->video_timing.hsync_pulse_width = this->hsync_;
  this->dpi_config_->video_timing.hsync_front_porch = this->hfp_;
  this->dpi_config_->video_timing.vsync_back_porch = this->vbp_;
  this->dpi_config_->video_timing.vsync_pulse_width = this->vsync_;
  this->dpi_config_->video_timing.vsync_front_porch = this->vfp_;
  
  // Flags
  this->dpi_config_->flags.use_dma2d = true;
  
  ESP_LOGD(TAG, "DPI configured: %dx%d @ %d MHz", 
    this->display_width_, this->display_height_, this->dpi_clk_freq_mhz_);
#endif
}

void ILI9881C::convert_init_commands_() {
  this->esp_init_commands_.clear();
  
  for (const auto &cmd : this->init_commands_) {
    if (cmd.is_delay) {
      // Pour les délais, créer une commande avec delay_ms défini
      ili9881c_lcd_init_cmd_t esp_cmd;
      esp_cmd.cmd = -1; // Indicateur de délai
      esp_cmd.data = nullptr;
      esp_cmd.data_bytes = 0;
      esp_cmd.delay_ms = cmd.delay_ms;
      this->esp_init_commands_.push_back(esp_cmd);
    } else {
      // Pour les vraies commandes
      ili9881c_lcd_init_cmd_t esp_cmd;
      esp_cmd.cmd = cmd.cmd;
      esp_cmd.data = cmd.data.empty() ? nullptr : cmd.data.data();
      esp_cmd.data_bytes = cmd.data.size();
      esp_cmd.delay_ms = 0;
      this->esp_init_commands_.push_back(esp_cmd);
    }
  }
  
  ESP_LOGD(TAG, "Converted %d init commands to ESP-IDF format", this->esp_init_commands_.size());
}

bool ILI9881C::init_display_() {
#if SOC_MIPI_DSI_SUPPORTED
  ESP_LOGD(TAG, "Initializing display with official driver...");
  
  // Configuration du vendor avec nos commandes d'init
  this->vendor_config_.init_cmds = this->esp_init_commands_.empty() ? nullptr : this->esp_init_commands_.data();
  this->vendor_config_.init_cmds_size = this->esp_init_commands_.size();
  this->vendor_config_.mipi_config.dsi_bus = this->dsi_bus_;
  this->vendor_config_.mipi_config.dpi_config = this->dpi_config_;
  this->vendor_config_.mipi_config.lane_num = this->data_lanes_;
  
  // Configuration du panel - Gérer le reset manuellement
  esp_lcd_panel_dev_config_t panel_config;
  memset(&panel_config, 0, sizeof(panel_config));
  
  // Désactiver le reset GPIO automatique et le gérer manuellement
  panel_config.reset_gpio_num = -1;
  panel_config.rgb_ele_order = this->color_order_ == COLOR_ORDER_RGB ? LCD_RGB_ELEMENT_ORDER_RGB : LCD_RGB_ELEMENT_ORDER_BGR;
  panel_config.bits_per_pixel = 24; // RGB888
  panel_config.flags.reset_active_high = 0;
  panel_config.vendor_config = &this->vendor_config_;
  
  // Reset manuel avant de créer le panel
  if (this->reset_pin_ != nullptr) {
    ESP_LOGD(TAG, "Performing manual hardware reset...");
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(120);
  }
  
  // Créer le panel avec le driver officiel ILI9881C
  esp_err_t ret = esp_lcd_new_panel_ili9881c(this->io_handle_, &panel_config, &this->panel_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create ILI9881C panel: %s", esp_err_to_name(ret));
    return false;
  }
  
  // Initialisation (sans reset automatique puisqu'on l'a fait manuellement)
  ret = esp_lcd_panel_init(this->panel_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Panel init failed: %s", esp_err_to_name(ret));
    return false;
  }
  
  ret = esp_lcd_panel_disp_on_off(this->panel_handle_, true);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Panel display on failed: %s", esp_err_to_name(ret));
    return false;
  }
  
  this->initialized_ = true;
  ESP_LOGD(TAG, "Display initialized successfully with official driver");
  return true;
#else
  ESP_LOGE(TAG, "MIPI DSI not supported");
  return false;
#endif
}

void ILI9881C::update() {
  if (!this->initialized_) {
    return;
  }
  
  this->do_update_();
  this->send_display_buffer_();
}

void ILI9881C::send_display_buffer_() {
#if SOC_MIPI_DSI_SUPPORTED
  if (!this->panel_handle_ || !this->initialized_) {
    return;
  }
  
  ESP_LOGVV(TAG, "Sending display buffer...");
  
  // Envoyer le buffer complet à l'écran
  esp_err_t ret = esp_lcd_panel_draw_bitmap(this->panel_handle_, 
    0, 0, this->display_width_, this->display_height_, this->buffer_);
  
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to draw bitmap: %s", esp_err_to_name(ret));
  }
#endif
}

void ILI9881C::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0) {
    return;
  }
  
  // Appliquer l'offset et les coordonnées
  int rotated_x = x + this->offset_x_;
  int rotated_y = y + this->offset_y_;
  
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
  
  // Le driver officiel gère automatiquement l'ordre des couleurs via rgb_ele_order
  
  // RGB888 - 24-bit per pixel  
  size_t pos = (rotated_y * this->display_width_ + rotated_x) * 3;
  if (pos + 2 < this->get_buffer_length_internal_()) {
    this->buffer_[pos] = r;
    this->buffer_[pos + 1] = g;
    this->buffer_[pos + 2] = b;
  }
}

void ILI9881C::loop() {
  // Rien à faire dans la boucle pour ce pilote
}

void ILI9881C::dump_config() {
  ESP_LOGCONFIG(TAG, "ILI9881C Display (Official ESP-IDF Driver):");
  ESP_LOGCONFIG(TAG, "  Physical Size: %dx%d", this->display_width_, this->display_height_);
  ESP_LOGCONFIG(TAG, "  Effective Size: %dx%d", this->get_width_internal(), this->get_height_internal());
  
  // Convertir l'enum en degrés pour l'affichage
  int rotation_degrees = 0;
  switch (this->rotation_) {
    case ROTATION_0: rotation_degrees = 0; break;
    case ROTATION_90: rotation_degrees = 90; break;
    case ROTATION_180: rotation_degrees = 180; break;
    case ROTATION_270: rotation_degrees = 270; break;
  }
  ESP_LOGCONFIG(TAG, "  Rotation: %d°", rotation_degrees);
  
  ESP_LOGCONFIG(TAG, "  Color Order: %s", this->color_order_ == COLOR_ORDER_RGB ? "RGB" : "BGR");
  ESP_LOGCONFIG(TAG, "  Offset X: %d", this->offset_x_);
  ESP_LOGCONFIG(TAG, "  Offset Y: %d", this->offset_y_);
  ESP_LOGCONFIG(TAG, "  Invert Colors: %s", YESNO(this->invert_colors_));
  ESP_LOGCONFIG(TAG, "  Auto Clear Enabled: %s", YESNO(this->auto_clear_enabled_));
  
  // Affichage des paramètres MIPI DSI
  ESP_LOGCONFIG(TAG, "  MIPI DSI Configuration:");
  ESP_LOGCONFIG(TAG, "    Data Lanes: %d", this->data_lanes_);
  ESP_LOGCONFIG(TAG, "    Lane Bit Rate: %d Mbps", this->lane_bit_rate_mbps_);
  ESP_LOGCONFIG(TAG, "    DPI Clock: %d MHz", this->dpi_clk_freq_mhz_);
  
  // Affichage des timings DPI
  ESP_LOGCONFIG(TAG, "  DPI Video Timings:");
  ESP_LOGCONFIG(TAG, "    HSYNC: %d", this->hsync_);
  ESP_LOGCONFIG(TAG, "    HBP: %d", this->hbp_);
  ESP_LOGCONFIG(TAG, "    HFP: %d", this->hfp_);
  ESP_LOGCONFIG(TAG, "    VSYNC: %d", this->vsync_);
  ESP_LOGCONFIG(TAG, "    VBP: %d", this->vbp_);
  ESP_LOGCONFIG(TAG, "    VFP: %d", this->vfp_);
  
  ESP_LOGCONFIG(TAG, "  Buffer Size: %zu bytes (%.2f MB)", 
    this->get_buffer_length_internal_(), this->get_buffer_length_internal_() / (1024.0 * 1024.0));
  ESP_LOGCONFIG(TAG, "  Init Commands: %d", this->init_commands_.size());
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  
#if !SOC_MIPI_DSI_SUPPORTED
  ESP_LOGE(TAG, "MIPI DSI not supported on this ESP32 variant");
#endif
  
  if (Component::is_failed()) {
    ESP_LOGE(TAG, "Failed to initialize display");
  }
}

// Méthodes de configuration
void ILI9881C::set_dimensions(uint16_t width, uint16_t height) {
  this->display_width_ = width;
  this->display_height_ = height;
}

void ILI9881C::set_offsets(uint16_t offset_x, uint16_t offset_y) {
  this->offset_x_ = offset_x;
  this->offset_y_ = offset_y;
}

void ILI9881C::set_rotation(Rotation rotation) {
  this->rotation_ = rotation;
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

int ILI9881C::get_width_internal() {
  return this->display_width_;
}

int ILI9881C::get_height_internal() {
  return this->display_height_;
}

size_t ILI9881C::get_buffer_length_internal_() {
  return this->display_width_ * this->display_height_ * 3; // RGB888 = 3 bytes par pixel
}

}  // namespace ili9881c
}  // namespace esphome

#endif  // USE_ESP32











