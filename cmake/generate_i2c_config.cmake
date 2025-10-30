if(NOT DEFINED INPUT_FILE)
  message(FATAL_ERROR "INPUT_FILE is not defined")
endif()

if(NOT DEFINED OUTPUT_FILE)
  message(FATAL_ERROR "OUTPUT_FILE is not defined")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/i2c_config_common.cmake")

parse_i2c_config("${INPUT_FILE}")

get_filename_component(_output_dir "${OUTPUT_FILE}" DIRECTORY)
if(NOT EXISTS "${_output_dir}")
  file(MAKE_DIRECTORY "${_output_dir}")
endif()

set(_inst_macro "${I2C_CFG_INSTANCE_NAME}")
if(I2C_CFG_ENABLED EQUAL 0)
  # Provide a safe default instance when the bridge is disabled.
  set(_inst_macro "i2c0")
endif()

if(I2C_CFG_ENABLED EQUAL 0)
  set(_sda_macro -1)
  set(_scl_macro -1)
  set(_led_macro -1)
else()
  set(_sda_macro "${I2C_CFG_SDA}")
  set(_scl_macro "${I2C_CFG_SCL}")
  set(_led_macro "${I2C_CFG_LED}")
endif()

set(_content "/* Auto-generated from ${INPUT_FILE}. Do not edit directly. */\n")
string(APPEND _content "#define I2C_BRIDGE_ENABLED ${I2C_CFG_ENABLED}\n")
string(APPEND _content "#define I2C_INST ${_inst_macro}\n")
string(APPEND _content "#define I2C_SDA ${_sda_macro}\n")
string(APPEND _content "#define I2C_SCL ${_scl_macro}\n")
string(APPEND _content "#define I2C_LED_PIN ${_led_macro}\n")

file(WRITE "${OUTPUT_FILE}" "${_content}")
