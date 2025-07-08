import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, spi
from esphome.const import (
    CONF_ID,
    CONF_MODEL,
    CONF_RESET_PIN,
    CONF_DIMENSIONS,
    CONF_HEIGHT,
    CONF_WIDTH,
    CONF_OFFSET_HEIGHT,
    CONF_OFFSET_WIDTH,
    CONF_INVERT_COLORS,
    CONF_AUTO_CLEAR_ENABLED,
)
from esphome import pins

CONF_DATA_LANES = "data_lanes"
CONF_PIXEL_FORMAT = "pixel_format"
CONF_BACKLIGHT_PIN = "backlight_pin"

DEPENDENCIES = ["esp32"]

ili9881c_ns = cg.esphome_ns.namespace("ili9881c")
ILI9881C = ili9881c_ns.class_("ILI9881C", cg.PollingComponent, display.DisplayBuffer)

MODELS = {
    "custom_720x1280": {
        "width": 720,
        "height": 1280,
    },
}

PIXEL_FORMATS = {
    "RGB565": 0,
    "RGB888": 1,
}

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ILI9881C),
        cv.Required(CONF_MODEL): cv.one_of(*MODELS, lower=True),
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_DATA_LANES, default=2): cv.int_range(min=1, max=4),
        cv.Optional(CONF_PIXEL_FORMAT, default="RGB565"): cv.enum(PIXEL_FORMATS),
        cv.Optional(CONF_BACKLIGHT_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_INVERT_COLORS, default=False): cv.boolean,
        cv.Optional(CONF_AUTO_CLEAR_ENABLED, default=True): cv.boolean,
        cv.Optional(CONF_DIMENSIONS): cv.Schema(
            {
                cv.Required(CONF_WIDTH): cv.positive_int,
                cv.Required(CONF_HEIGHT): cv.positive_int,
                cv.Optional(CONF_OFFSET_WIDTH, default=0): cv.int_,
                cv.Optional(CONF_OFFSET_HEIGHT, default=0): cv.int_,
            }
        ),
    }
).extend(cv.polling_component_schema("1s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)

    model = config[CONF_MODEL]
    if CONF_DIMENSIONS in config:
        dimensions = config[CONF_DIMENSIONS]
        cg.add(var.set_dimensions(dimensions[CONF_WIDTH], dimensions[CONF_HEIGHT]))
        cg.add(var.set_offsets(dimensions[CONF_OFFSET_WIDTH], dimensions[CONF_OFFSET_HEIGHT]))
    else:
        cg.add(var.set_dimensions(MODELS[model]["width"], MODELS[model]["height"]))

    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset_pin))

    if CONF_BACKLIGHT_PIN in config:
        backlight_pin = await cg.gpio_pin_expression(config[CONF_BACKLIGHT_PIN])
        cg.add(var.set_backlight_pin(backlight_pin))

    cg.add(var.set_data_lanes(config[CONF_DATA_LANES]))
    cg.add(var.set_pixel_format(config[CONF_PIXEL_FORMAT]))
    cg.add(var.set_invert_colors(config[CONF_INVERT_COLORS]))
    cg.add(var.set_auto_clear_enabled(config[CONF_AUTO_CLEAR_ENABLED]))
