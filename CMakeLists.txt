# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(pico_uart_i2c_bridge)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/i2c_config_common.cmake)

set(UART_PIN_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/uart_config.txt" CACHE FILEPATH "UART pin configuration file")
message(STATUS "Using UART pin configuration: ${UART_PIN_CONFIG}")

file(READ "${UART_PIN_CONFIG}" UART_PIN_RAW_CONTENT)
string(REPLACE "\r\n" "\n" UART_PIN_RAW_CONTENT "${UART_PIN_RAW_CONTENT}")
string(REPLACE "\r" "\n" UART_PIN_RAW_CONTENT "${UART_PIN_RAW_CONTENT}")
string(REPLACE ";" "\\;" UART_PIN_RAW_CONTENT "${UART_PIN_RAW_CONTENT}")

if(UART_PIN_RAW_CONTENT STREQUAL "")
	set(UART_PIN_LINES "")
else()
	string(REPLACE "\n" ";" UART_PIN_LINES "${UART_PIN_RAW_CONTENT}")
endif()

set(_uart_channel_index 0)
set(_uart_next_sm 0)
set(_uart_uart0_free TRUE)
set(_uart_uart1_free TRUE)
set(_uart_used_pio_sms "")
foreach(_uart_line IN LISTS UART_PIN_LINES)
	string(STRIP "${_uart_line}" _uart_line)
	if(_uart_line STREQUAL "")
		continue()
	endif()
	if(_uart_line MATCHES "^#")
		continue()
	endif()
	math(EXPR _uart_channel_index "${_uart_channel_index} + 1")

	set(_uart_inst_desc "PIO SM")
	set(_uart_extra_desc "")
	set(_assignment_note "")

	if(_uart_line MATCHES "TX=([0-9]+)")
		set(_tx_pin "${CMAKE_MATCH_1}")
	else()
		message(WARNING "UART channel ${_uart_channel_index}: missing TX pin in '${_uart_line}'")
		set(_tx_pin "")
	endif()

	if(_uart_line MATCHES "RX=([0-9]+)")
		set(_rx_pin "${CMAKE_MATCH_1}")
	else()
		message(WARNING "UART channel ${_uart_channel_index}: missing RX pin in '${_uart_line}'")
		set(_rx_pin "")
	endif()

	if(_uart_line MATCHES "LED=([0-9]+)")
		set(_led_pin "${CMAKE_MATCH_1}")
	else()
		message(WARNING "UART channel ${_uart_channel_index}: missing LED pin in '${_uart_line}'")
		set(_led_pin "")
	endif()

	set(_explicit_uart "")
	if(_uart_line MATCHES "HW=([01])")
		set(_explicit_uart "${CMAKE_MATCH_1}")
	endif()
	if(_uart_line MATCHES "UART=([01])")
		if(_explicit_uart STREQUAL "" OR _explicit_uart STREQUAL CMAKE_MATCH_1)
			set(_explicit_uart "${CMAKE_MATCH_1}")
		else()
			message(FATAL_ERROR "UART channel ${_uart_channel_index}: conflicting HW/UART assignments in '${_uart_line}'")
		endif()
	endif()

	set(_explicit_pio "")
	if(_uart_line MATCHES "PIO=([0-9]+)")
		set(_explicit_pio "${CMAKE_MATCH_1}")
	endif()

	if(NOT _explicit_uart STREQUAL "" AND NOT _explicit_pio STREQUAL "")
		message(FATAL_ERROR "UART channel ${_uart_channel_index}: cannot specify both UART and PIO assignment ('${_uart_line}')")
	endif()

	if(NOT _explicit_uart STREQUAL "")
		if(_explicit_uart STREQUAL "0")
			if(NOT _uart_uart0_free)
				message(FATAL_ERROR "UART channel ${_uart_channel_index}: UART0 already assigned")
			endif()
			set(_uart_inst_desc "uart0")
			set(_uart_uart0_free FALSE)
		elseif(_explicit_uart STREQUAL "1")
			if(NOT _uart_uart1_free)
				message(FATAL_ERROR "UART channel ${_uart_channel_index}: UART1 already assigned")
			endif()
			set(_uart_inst_desc "uart1")
			set(_uart_uart1_free FALSE)
		else()
			message(FATAL_ERROR "UART channel ${_uart_channel_index}: unsupported UART index '${_explicit_uart}'")
		endif()
		set(_assignment_note " (forced)")
	elseif(NOT _explicit_pio STREQUAL "")
		list(FIND _uart_used_pio_sms "${_explicit_pio}" _pio_index)
		if(NOT _pio_index EQUAL -1)
			message(FATAL_ERROR "UART channel ${_uart_channel_index}: PIO state machine ${_explicit_pio} already assigned")
		endif()
		if(_explicit_pio GREATER 3)
			message(FATAL_ERROR "UART channel ${_uart_channel_index}: PIO state machine ${_explicit_pio} exceeds supported range (0-3)")
		endif()
		list(APPEND _uart_used_pio_sms "${_explicit_pio}")
		set(_uart_inst_desc "PIO SM")
		set(_uart_extra_desc "${_explicit_pio}")
		set(_assignment_note " (forced)")
	else()
		if(_uart_uart0_free)
			set(_uart_inst_desc "uart0")
			set(_uart_uart0_free FALSE)
		elseif(_uart_uart1_free)
			set(_uart_inst_desc "uart1")
			set(_uart_uart1_free FALSE)
		else()
			set(_selected_sm "")
			while(_selected_sm STREQUAL "")
				list(FIND _uart_used_pio_sms "${_uart_next_sm}" _pio_index)
				if(_pio_index EQUAL -1)
					if(_uart_next_sm GREATER 3)
						message(FATAL_ERROR "UART channel ${_uart_channel_index}: configuration requires more than 4 PIO UARTs")
					endif()
					set(_selected_sm "${_uart_next_sm}")
					list(APPEND _uart_used_pio_sms "${_selected_sm}")
				endif()
				math(EXPR _uart_next_sm "${_uart_next_sm} + 1")
			endwhile()
			set(_uart_inst_desc "PIO SM")
			set(_uart_extra_desc "${_selected_sm}")
		endif()
	endif()

	if(_uart_inst_desc STREQUAL "PIO SM")
		message(STATUS "UART channel ${_uart_channel_index}: ${_uart_line} -> ${_uart_inst_desc} ${_uart_extra_desc}${_assignment_note}")
	else()
		message(STATUS "UART channel ${_uart_channel_index}: ${_uart_line} -> ${_uart_inst_desc}${_assignment_note}")
	endif()

endforeach()
if(_uart_channel_index EQUAL 0)
	message(WARNING "UART pin configuration is empty; build may fail when generating uart_config.inc")
endif()

set(I2C_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/i2c_config.txt" CACHE FILEPATH "I2C bridge configuration file")
message(STATUS "Using I2C configuration: ${I2C_CONFIG_FILE}")
parse_i2c_config("${I2C_CONFIG_FILE}")

if(I2C_CFG_ENABLED)
	message(STATUS "I2C bridge: enabled (${I2C_CFG_INSTANCE_NAME}, SDA=${I2C_CFG_SDA}, SCL=${I2C_CFG_SCL}, LED=${I2C_CFG_LED})")
else()
	message(STATUS "I2C bridge: disabled")
endif()

unset(ENABLE_USB_I2C CACHE)
set(ENABLE_USB_I2C "${I2C_CFG_ENABLED}")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/tusb_config.h" TUSB_CONFIG_CONTENT)
string(REGEX MATCH "#define[ \t]+CFG_TUD_CDC[ \t]+([0-9]+)" _ "${TUSB_CONFIG_CONTENT}")
if(NOT CMAKE_MATCH_1)
	message(FATAL_ERROR "Unable to determine CFG_TUD_CDC from tusb_config.h")
endif()
set(CFG_TUD_CDC_VALUE "${CMAKE_MATCH_1}")
set(GENERATED_UART_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/generated/uart_config.inc")
set(GENERATED_I2C_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/generated/i2c_config.inc")

add_custom_command(
	OUTPUT "${GENERATED_UART_CONFIG}"
	COMMAND ${CMAKE_COMMAND}
		-DINPUT_FILE="${UART_PIN_CONFIG}"
		-DOUTPUT_FILE="${GENERATED_UART_CONFIG}"
		-DCFG_TUD_CDC_VALUE=${CFG_TUD_CDC_VALUE}
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_uart_config.cmake"
	DEPENDS "${UART_PIN_CONFIG}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_uart_config.cmake"
	COMMENT "Generating UART configuration from ${UART_PIN_CONFIG}"
)

add_custom_command(
	OUTPUT "${GENERATED_I2C_CONFIG}"
	COMMAND ${CMAKE_COMMAND}
		-DINPUT_FILE="${I2C_CONFIG_FILE}"
		-DOUTPUT_FILE="${GENERATED_I2C_CONFIG}"
		-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_i2c_config.cmake"
	DEPENDS "${I2C_CONFIG_FILE}"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_i2c_config.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/i2c_config_common.cmake"
	COMMENT "Generating I2C configuration from ${I2C_CONFIG_FILE}"
)

add_custom_target(generate_firmware_configs DEPENDS "${GENERATED_UART_CONFIG}" "${GENERATED_I2C_CONFIG}")

pico_sdk_init()
  
add_executable(uart_i2c_bridge uart-i2c-bridge.c usb-descriptors.c)

add_dependencies(uart_i2c_bridge generate_firmware_configs)

pico_generate_pio_header(uart_i2c_bridge ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)
pico_generate_pio_header(uart_i2c_bridge ${CMAKE_CURRENT_LIST_DIR}/uart_tx.pio)

target_include_directories(uart_i2c_bridge PUBLIC
	./
	../pico-sdk/lib/tinyusb/src
	${CMAKE_CURRENT_BINARY_DIR}/generated)

target_compile_definitions(uart_i2c_bridge PRIVATE ENABLE_USB_I2C=$<BOOL:${ENABLE_USB_I2C}>)

set(UART_I2C_BRIDGE_LIBS
	pico_multicore
	pico_stdlib
	hardware_flash
	tinyusb_device
	tinyusb_board
        hardware_pio)

if (ENABLE_USB_I2C)
	list(APPEND UART_I2C_BRIDGE_LIBS hardware_i2c)
endif()

target_link_libraries(uart_i2c_bridge ${UART_I2C_BRIDGE_LIBS})

pico_add_extra_outputs(uart_i2c_bridge)
