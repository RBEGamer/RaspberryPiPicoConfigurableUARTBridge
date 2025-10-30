// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * This file is based on a file originally part of the
 * MicroPython project, http://micropython.org/
 *
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2019 Damien P. George
 */

#include <tusb.h>

#include "serial.h"

#ifndef ENABLE_USB_I2C
#define ENABLE_USB_I2C 1
#endif

#define DESC_STR_MAX 20

#define USBD_VID 0x3171 /* 8086 Consultancy */
#define USBD_PID 0x0060 /* PicoUART6 */

#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + (TUD_VENDOR_DESC_LEN * CFG_TUD_VENDOR) + (TUD_CDC_DESC_LEN * CFG_TUD_CDC))
#define USBD_MAX_POWER_MA 500

enum {
	USBD_ITF_CDC_0 = 0,
	USBD_ITF_CDC_1 = 2,
	USBD_ITF_CDC_2 = 4,
	USBD_ITF_CDC_3 = 6,
	USBD_ITF_CDC_4 = 8,
	USBD_ITF_CDC_5 = 10,
#if ENABLE_USB_I2C
	USBD_ITF_VENDOR_0 = 12,
	USBD_ITF_MAX = 13,
#else
	USBD_ITF_MAX = 12,
#endif
};

#define USBD_CDC_0_EP_CMD 0x81
#define USBD_CDC_1_EP_CMD 0x83
#define USBD_CDC_2_EP_CMD 0x85
#define USBD_CDC_3_EP_CMD 0x87
#define USBD_CDC_4_EP_CMD 0x89
#define USBD_CDC_5_EP_CMD 0x8B

#define USBD_CDC_0_EP_OUT 0x01
#define USBD_CDC_1_EP_OUT 0x03
#define USBD_CDC_2_EP_OUT 0x05
#define USBD_CDC_3_EP_OUT 0x07
#define USBD_CDC_4_EP_OUT 0x09
#define USBD_CDC_5_EP_OUT 0x0B

#define USBD_CDC_0_EP_IN 0x82
#define USBD_CDC_1_EP_IN 0x84
#define USBD_CDC_2_EP_IN 0x86
#define USBD_CDC_3_EP_IN 0x88
#define USBD_CDC_4_EP_IN 0x8E // 8D works at 9600, 8E seems to work best
#define USBD_CDC_5_EP_IN 0x8C 

#if ENABLE_USB_I2C
#define USBD_VENDOR_0_OUT 0x0D
#define USBD_VENDOR_0_IN 0x8D
#endif


#define USBD_CDC_CMD_MAX_SIZE 8
#define USBD_CDC_IN_OUT_MAX_SIZE 64

#define USBD_STR_0 0x00
#define USBD_STR_MANUF 0x01
#define USBD_STR_PRODUCT 0x02
#define USBD_STR_SERIAL 0x03
#define USBD_STR_CDC 0x04
#if ENABLE_USB_I2C
#define USBD_STR_VENDOR 0x05
#endif

static const tusb_desc_device_t usbd_desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = TUSB_CLASS_MISC,
	.bDeviceSubClass = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol = MISC_PROTOCOL_IAD,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor = USBD_VID,
	.idProduct = USBD_PID,
	.bcdDevice = 0x0100,
	.iManufacturer = USBD_STR_MANUF,
	.iProduct = USBD_STR_PRODUCT,
	.iSerialNumber = USBD_STR_SERIAL,
	.bNumConfigurations = 1,
};

static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
	TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN,
		TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, USBD_MAX_POWER_MA),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_0, USBD_STR_CDC, USBD_CDC_0_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_0_EP_OUT, USBD_CDC_0_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_1, USBD_STR_CDC, USBD_CDC_1_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_1_EP_OUT, USBD_CDC_1_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_2, USBD_STR_CDC, USBD_CDC_2_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_2_EP_OUT, USBD_CDC_2_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_3, USBD_STR_CDC, USBD_CDC_3_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_3_EP_OUT, USBD_CDC_3_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_4, USBD_STR_CDC, USBD_CDC_4_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_4_EP_OUT, USBD_CDC_4_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

	TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_5, USBD_STR_CDC, USBD_CDC_5_EP_CMD,
		USBD_CDC_CMD_MAX_SIZE, USBD_CDC_5_EP_OUT, USBD_CDC_5_EP_IN,
		USBD_CDC_IN_OUT_MAX_SIZE),

#if ENABLE_USB_I2C
	 TUD_VENDOR_DESCRIPTOR(USBD_ITF_VENDOR_0, USBD_STR_VENDOR, USBD_VENDOR_0_OUT, USBD_VENDOR_0_IN, 32),
#endif

};

char serial[17];

static char *const usbd_desc_str[] = {
	[USBD_STR_MANUF] = "8086 Consultancy",
	[USBD_STR_PRODUCT] = "PicoUART6",
	[USBD_STR_SERIAL] = serial,
	[USBD_STR_CDC] = "CDC Serial",
#if ENABLE_USB_I2C
	[USBD_STR_VENDOR] = "i2c-tiny-usb",
#endif
};

const uint8_t *tud_descriptor_device_cb(void)
{
	return (const uint8_t *) &usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
	return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	static uint16_t desc_str[DESC_STR_MAX];
	uint8_t len;

	if (index == 0) {
		desc_str[1] = 0x0409;
		len = 1;
	} else {
		const char *str;

		if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0]))
			return NULL;

		str = usbd_desc_str[index];
		for (len = 0; len < DESC_STR_MAX - 1 && str[len]; ++len)
			desc_str[1 + len] = str[len];
	}

	desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * len + 2);

	return desc_str;
}
