// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2020 Damien P. George
 */

#if !defined(_TUSB_CONFIG_H_)
#define _TUSB_CONFIG_H_

#include <tusb_option.h>

#ifndef ENABLE_USB_I2C
#define ENABLE_USB_I2C 1
#endif

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

#ifndef CFG_TUD_CDC
#define CFG_TUD_CDC 6
#endif
#define CFG_TUD_CDC_RX_BUFSIZE 1024  // no harm making these bigger I guess
#define CFG_TUD_CDC_TX_BUFSIZE 1024

#if ENABLE_USB_I2C
#define CFG_TUD_VENDOR 1
#define CFG_TUD_VENDOR_EPSIZE     32
#define CFG_TUD_VENDOR_EP_BUFSIZE 512
#define CFG_TUD_VENDOR_RX_BUFSIZE 512
#define CFG_TUD_VENDOR_TX_BUFSIZE 512
#else
#define CFG_TUD_VENDOR 0
#define CFG_TUD_VENDOR_EPSIZE     32
#define CFG_TUD_VENDOR_EP_BUFSIZE 0
#define CFG_TUD_VENDOR_RX_BUFSIZE 0
#define CFG_TUD_VENDOR_TX_BUFSIZE 0
#endif

#endif /* _TUSB_CONFIG_H_ */
