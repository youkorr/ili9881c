import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
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
    CONF_ROTATION,
)
from esphome import pins

CONF_DC_PIN = "dc_pin"
CONF_INIT_SEQUENCE = "init_sequence"
CONF_DELAY = "delay"
CONF_COLOR_ORDER = "color_order"

# Nouveaux paramètres MIPI DSI
CONF_DATA_LANES = "data_lanes"
CONF_LANE_BIT_RATE_MBPS = "lane_bit_rate_mbps"
CONF_DPI_CLK_FREQ_MHZ = "dpi_clk_freq_mhz"

# Paramètres de timing MIPI DPI
CONF_HSYNC = "hsync"
CONF_HBP = "hbp" 
CONF_HFP = "hfp"
CONF_VSYNC = "vsync"
CONF_VBP = "vbp"
CONF_VFP = "vfp"

DEPENDENCIES = ["esp32"]

ili9881c_ns = cg.esphome_ns.namespace("ili9881c")
ILI9881C = ili9881c_ns.class_("ILI9881C", display.DisplayBuffer)

# Énumérations pour la rotation
Rotation = ili9881c_ns.enum("Rotation")
ROTATIONS = {
    0: Rotation.ROTATION_0,
    90: Rotation.ROTATION_90,
    180: Rotation.ROTATION_180,
    270: Rotation.ROTATION_270,
}

# Énumérations pour l'ordre des couleurs
ColorOrder = ili9881c_ns.enum("ColorOrder")
COLOR_ORDERS = {
    "rgb": ColorOrder.COLOR_ORDER_RGB,
    "bgr": ColorOrder.COLOR_ORDER_BGR,
}

MODELS = {
    "custom": {
        "width": 720,
        "height": 1280,
    },
}

def validate_init_sequence(value):
    """Valide la séquence d'initialisation."""
    if not isinstance(value, list):
        raise cv.Invalid("init_sequence must be a list")
    
    validated = []
    for item in value:
        if isinstance(item, list):
            if len(item) < 1:
                raise cv.Invalid("Command must have at least one byte")
            validated.append([int(x) for x in item])
        elif isinstance(item, dict):
            if CONF_DELAY in item:
                delay_str = str(item[CONF_DELAY])
                if delay_str.endswith('ms'):
                    delay_ms = int(delay_str[:-2])
                elif delay_str.endswith('s'):
                    delay_ms = int(float(delay_str[:-1]) * 1000)
                else:
                    delay_ms = int(delay_str)
                validated.append({CONF_DELAY: delay_ms})
            else:
                raise cv.Invalid("Unknown command format in init_sequence")
        else:
            raise cv.Invalid("Invalid init_sequence item")
    
    return validated

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ILI9881C),
        cv.Required(CONF_MODEL): cv.one_of(*MODELS, lower=True),
        cv.Optional(CONF_DC_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
        cv.Optional(CONF_INVERT_COLORS, default=False): cv.boolean,
        cv.Optional(CONF_AUTO_CLEAR_ENABLED, default=True): cv.boolean,
        cv.Optional(CONF_ROTATION, default=0): cv.enum(ROTATIONS, int=True),
        cv.Optional(CONF_COLOR_ORDER, default="rgb"): cv.enum(COLOR_ORDERS, lower=True),
        cv.Optional(CONF_INIT_SEQUENCE): validate_init_sequence,
        
        # Paramètres MIPI DSI
        cv.Optional(CONF_DATA_LANES, default=2): cv.int_range(min=1, max=4),
        cv.Optional(CONF_LANE_BIT_RATE_MBPS, default=1000): cv.int_range(min=100, max=2000),
        cv.Optional(CONF_DPI_CLK_FREQ_MHZ, default=80): cv.int_range(min=10, max=200),
        
        # Paramètres de timing DPI
        cv.Optional(CONF_HSYNC, default=40): cv.positive_int,
        cv.Optional(CONF_HBP, default=140): cv.positive_int,
        cv.Optional(CONF_HFP, default=40): cv.positive_int,
        cv.Optional(CONF_VSYNC, default=4): cv.positive_int,
        cv.Optional(CONF_VBP, default=16): cv.positive_int,
        cv.Optional(CONF_VFP, default=16): cv.positive_int,
        
        cv.Optional(CONF_DIMENSIONS): cv.Schema(
            {
                cv.Required(CONF_WIDTH): cv.positive_int,
                cv.Required(CONF_HEIGHT): cv.positive_int,
                cv.Optional(CONF_OFFSET_WIDTH, default=0): cv.int_,
                cv.Optional(CONF_OFFSET_HEIGHT, default=0): cv.int_,
            }
        ),
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    model = config[CONF_MODEL]
    if CONF_DIMENSIONS in config:
        dimensions = config[CONF_DIMENSIONS]
        cg.add(var.set_dimensions(dimensions[CONF_WIDTH], dimensions[CONF_HEIGHT]))
        cg.add(var.set_offsets(dimensions[CONF_OFFSET_WIDTH], dimensions[CONF_OFFSET_HEIGHT]))
    else:
        cg.add(var.set_dimensions(MODELS[model]["width"], MODELS[model]["height"]))

    if CONF_DC_PIN in config:
        dc_pin = await cg.gpio_pin_expression(config[CONF_DC_PIN])
        cg.add(var.set_dc_pin(dc_pin))

    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset_pin))

    cg.add(var.set_invert_colors(config[CONF_INVERT_COLORS]))
    cg.add(var.set_auto_clear_enabled(config[CONF_AUTO_CLEAR_ENABLED]))
    cg.add(var.set_rotation(config[CONF_ROTATION]))
    cg.add(var.set_color_order(config[CONF_COLOR_ORDER]))

    # Configuration des paramètres MIPI DSI
    cg.add(var.set_data_lanes(config[CONF_DATA_LANES]))
    cg.add(var.set_lane_bit_rate_mbps(config[CONF_LANE_BIT_RATE_MBPS]))
    cg.add(var.set_dpi_clk_freq_mhz(config[CONF_DPI_CLK_FREQ_MHZ]))

    # Configuration des timings DPI
    cg.add(var.set_hsync(config[CONF_HSYNC]))
    cg.add(var.set_hbp(config[CONF_HBP]))
    cg.add(var.set_hfp(config[CONF_HFP]))
    cg.add(var.set_vsync(config[CONF_VSYNC]))
    cg.add(var.set_vbp(config[CONF_VBP]))
    cg.add(var.set_vfp(config[CONF_VFP]))

    # Gestion de la séquence d'initialisation
    if CONF_INIT_SEQUENCE in config:
        init_seq = config[CONF_INIT_SEQUENCE]
        
        cg.add(var.clear_init_sequence())
        
        for item in init_seq:
            if isinstance(item, list):
                cmd = item[0]
                data = item[1:] if len(item) > 1 else []
                cg.add(var.add_init_command(cmd, data))
            elif isinstance(item, dict) and CONF_DELAY in item:
                delay_ms = item[CONF_DELAY]
                cg.add(var.add_init_delay(delay_ms))







