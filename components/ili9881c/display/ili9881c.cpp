// Ajout de méthodes de conversion pour compatibilité ESPHome
void ILI9881C::set_rotation(int rotation) {
  switch (rotation) {
    case 0:
      this->set_rotation(ROTATION_0);
      break;
    case 90:
      this->set_rotation(ROTATION_90);
      break;
    case 180:
      this->set_rotation(ROTATION_180);
      break;
    case 270:
      this->set_rotation(ROTATION_270);
      break;
    default:
      ESP_LOGW(TAG, "Invalid rotation %d, defaulting to 0", rotation);
      this->set_rotation(ROTATION_0);
      break;
  }
}

void ILI9881C::set_pixel_format(int format) {
  switch (format) {
    case 0:
      this->set_pixel_format(RGB565);
      break;
    case 1:
      this->set_pixel_format(RGB888);
      break;
    default:
      ESP_LOGW(TAG, "Invalid pixel format %d, defaulting to RGB565", format);
      this->set_pixel_format(RGB565);
      break;
  }
}




