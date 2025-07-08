import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import esp32

DEPENDENCIES = ['esp32']
CODEOWNERS = ['@your_username']

mipi_dsi_ns = cg.esphome_ns.namespace('mipi_dsi')
MIPIDSIComponent = mipi_dsi_ns.class_('MIPIDSIComponent', cg.Component)

CONF_NUMBER_OF_LANES = 'number_of_lanes'
CONF_BIT_RATE = 'bit_rate'
CONF_PHY_VOLTAGE = 'phy_voltage'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MIPIDSIComponent),
    cv.Required(CONF_NUMBER_OF_LANES): cv.int_range(min=1, max=4),
    cv.Required(CONF_BIT_RATE): cv.int_range(min=80000000, max=2500000000),  # 80Mbps à 2.5Gbps
    cv.Optional(CONF_PHY_VOLTAGE, default=1800): cv.int_range(min=1200, max=3300),  # 1.2V à 3.3V
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cg.add(var.set_number_of_lanes(config[CONF_NUMBER_OF_LANES]))
    cg.add(var.set_bit_rate(config[CONF_BIT_RATE]))
    cg.add(var.set_phy_voltage(config[CONF_PHY_VOLTAGE]))
