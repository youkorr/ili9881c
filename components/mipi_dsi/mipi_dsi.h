namespace esphome {
namespace mipi_dsi {

static const char *const TAG = "mipi_dsi";

class MIPIDSIComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void set_number_of_lanes(uint8_t lanes) { this->number_of_lanes_ = lanes; }
  void set_bit_rate(uint32_t bit_rate) { this->bit_rate_ = bit_rate; }
  void set_phy_voltage(uint16_t voltage) { this->phy_voltage_ = voltage; }

  // Méthodes pour l'envoi de commandes DCS (Display Command Set)
  bool send_dcs_command(uint8_t cmd, const uint8_t *data = nullptr, size_t len = 0);
  bool send_generic_command(uint8_t cmd, const uint8_t *data = nullptr, size_t len = 0);
  
  // Méthodes pour la gestion des paquets MIPI
  bool send_short_packet(uint8_t data_type, uint16_t data);
  bool send_long_packet(uint8_t data_type, const uint8_t *data, size_t len);
  
  // Contrôle du PHY
  bool initialize_phy();
  bool configure_lanes();
  bool set_hs_mode(bool enable);
  
  // Getters
  uint8_t get_number_of_lanes() const { return this->number_of_lanes_; }
  uint32_t get_bit_rate() const { return this->bit_rate_; }
  uint16_t get_phy_voltage() const { return this->phy_voltage_; }

 protected:
  uint8_t number_of_lanes_{2};
  uint32_t bit_rate_{730000000};  // 730Mbps par défaut
  uint16_t phy_voltage_{2500};    // 2.5V par défaut
  
  bool is_initialized_{false};
  bool hs_mode_enabled_{false};
  
  // Méthodes privées
  bool calculate_phy_timings();
  bool configure_clock_lane();
  bool configure_data_lanes();
  uint16_t calculate_checksum(const uint8_t *data, size_t len);
  
  // Structures pour les timings PHY
  struct phy_timings {
    uint16_t hs_prepare;
    uint16_t hs_zero;
    uint16_t hs_trail;
    uint16_t hs_exit;
    uint16_t clk_prepare;
    uint16_t clk_zero;
    uint16_t clk_trail;
    uint16_t clk_post;
    uint16_t clk_pre;
  } phy_timings_;
};

// Types de paquets MIPI DSI
enum MIPIPacketType {
  MIPI_DSI_V_SYNC_START = 0x01,
  MIPI_DSI_V_SYNC_END = 0x11,
  MIPI_DSI_H_SYNC_START = 0x21,
  MIPI_DSI_H_SYNC_END = 0x31,
  MIPI_DSI_COLOR_MODE_OFF = 0x02,
  MIPI_DSI_COLOR_MODE_ON = 0x12,
  MIPI_DSI_SHUTDOWN_PERIPHERAL = 0x22,
  MIPI_DSI_TURN_ON_PERIPHERAL = 0x32,
  MIPI_DSI_GENERIC_SHORT_WRITE_0 = 0x03,
  MIPI_DSI_GENERIC_SHORT_WRITE_1 = 0x13,
  MIPI_DSI_GENERIC_SHORT_WRITE_2 = 0x23,
  MIPI_DSI_GENERIC_READ_REQUEST_0 = 0x04,
  MIPI_DSI_GENERIC_READ_REQUEST_1 = 0x14,
  MIPI_DSI_GENERIC_READ_REQUEST_2 = 0x24,
  MIPI_DSI_DCS_SHORT_WRITE = 0x05,
  MIPI_DSI_DCS_SHORT_WRITE_PARAM = 0x15,
  MIPI_DSI_DCS_READ = 0x06,
  MIPI_DSI_EXECUTE_QUEUE = 0x16,
  MIPI_DSI_SET_MAXIMUM_RETURN_PACKET_SIZE = 0x37,
  MIPI_DSI_NULL_PACKET = 0x09,
  MIPI_DSI_BLANKING_PACKET = 0x19,
  MIPI_DSI_GENERIC_LONG_WRITE = 0x29,
  MIPI_DSI_DCS_LONG_WRITE = 0x39,
  MIPI_DSI_LOOSELY_PACKED_PIXEL_STREAM_YCBCR20 = 0x0c,
  MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR24 = 0x1c,
  MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR16 = 0x2c,
  MIPI_DSI_PACKED_PIXEL_STREAM_30 = 0x0d,
  MIPI_DSI_PACKED_PIXEL_STREAM_36 = 0x1d,
  MIPI_DSI_PACKED_PIXEL_STREAM_YCBCR12 = 0x3d,
  MIPI_DSI_PACKED_PIXEL_STREAM_16 = 0x0e,
  MIPI_DSI_PACKED_PIXEL_STREAM_18 = 0x1e,
  MIPI_DSI_PIXEL_STREAM_3BYTE_18 = 0x2e,
  MIPI_DSI_PACKED_PIXEL_STREAM_24 = 0x3e,
};

// Commandes DCS communes
enum DCSCommand {
  DCS_SOFT_RESET = 0x01,
  DCS_GET_DISPLAY_ID = 0x04,
  DCS_GET_RED_CHANNEL = 0x06,
  DCS_GET_GREEN_CHANNEL = 0x07,
  DCS_GET_BLUE_CHANNEL = 0x08,
  DCS_GET_DISPLAY_STATUS = 0x09,
  DCS_GET_POWER_MODE = 0x0A,
  DCS_GET_ADDRESS_MODE = 0x0B,
  DCS_GET_PIXEL_FORMAT = 0x0C,
  DCS_GET_DISPLAY_MODE = 0x0D,
  DCS_GET_SIGNAL_MODE = 0x0E,
  DCS_GET_DIAGNOSTIC_RESULT = 0x0F,
  DCS_ENTER_SLEEP_MODE = 0x10,
  DCS_EXIT_SLEEP_MODE = 0x11,
  DCS_ENTER_PARTIAL_MODE = 0x12,
  DCS_ENTER_NORMAL_MODE = 0x13,
  DCS_EXIT_INVERT_MODE = 0x20,
  DCS_ENTER_INVERT_MODE = 0x21,
  DCS_SET_GAMMA_CURVE = 0x26,
  DCS_SET_DISPLAY_OFF = 0x28,
  DCS_SET_DISPLAY_ON = 0x29,
  DCS_SET_COLUMN_ADDRESS = 0x2A,
  DCS_SET_PAGE_ADDRESS = 0x2B,
  DCS_WRITE_MEMORY_START = 0x2C,
  DCS_WRITE_LUT = 0x2D,
  DCS_READ_MEMORY_START = 0x2E,
  DCS_SET_PARTIAL_AREA = 0x30,
  DCS_SET_SCROLL_AREA = 0x33,
  DCS_SET_TEAR_OFF = 0x34,
  DCS_SET_TEAR_ON = 0x35,
  DCS_SET_ADDRESS_MODE = 0x36,
  DCS_SET_SCROLL_START = 0x37,
  DCS_EXIT_IDLE_MODE = 0x38,
  DCS_ENTER_IDLE_MODE = 0x39,
  DCS_SET_PIXEL_FORMAT = 0x3A,
  DCS_WRITE_MEMORY_CONTINUE = 0x3C,
  DCS_READ_MEMORY_CONTINUE = 0x3E,
  DCS_SET_TEAR_SCANLINE = 0x44,
  DCS_GET_SCANLINE = 0x45,
  DCS_READ_DDB_START = 0xA1,
  DCS_READ_DDB_CONTINUE = 0xA8,
};

}  // namespace mipi_dsi
}  // namespace esphome
