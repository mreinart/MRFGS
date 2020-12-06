/*
 * fanet_t9_thermal.c
 * 
 * Copyright 2019  <pi@pi4>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include "fanet_mac.h"
#include "fanet_struct.h"
#include "fanet_terminal.h"
#include "fanet_global.h"
#include "fanet_t8_hwinfo.h"

/*
 */

void type_8_hwinfo_decoder(sRawMessage *_rx_payload, sFanetMAC *_fanet_mac, sHardwareInfo *_rx_hwinfo) {
    memset(_rx_hwinfo, 0, sizeof(sHardwareInfo));
    _rx_payload->m_pointer = 0;
    _rx_hwinfo->s_address_manufactur_id = _fanet_mac->s_manufactur_id;
    _rx_hwinfo->s_address_unique_id = _fanet_mac->s_unique_id;

    uint16_t build_date;
    _rx_hwinfo->device_type = _rx_payload->message[_rx_payload->m_pointer] & 0xFF;
    _rx_payload->m_pointer += 1;
    // [Byte 1-2]	Firmware Build Date
    //  bit 15			0: Release 1: Develop/Experimental Mode
    //  bit 9-14		Year from 2019 (0 -> 2019, 1 -> 2020, ...)
    //  bit 5-8			Month (1-12)
    //  bit 0-4			Day (1-31)
    build_date = (_rx_payload->message[_rx_payload->m_pointer]   & 0xff) +
                ((_rx_payload->message[_rx_payload->m_pointer+1] & 0xff) << 8);
    _rx_payload->m_pointer += 2;
     if ((build_date >> 15) == 0x0001)
         _rx_hwinfo->experimental = true;
    _rx_hwinfo->fw_build_year  = ((build_date & 0x7E00) >> 9) + 2019;
    _rx_hwinfo->fw_build_month = (build_date & 0x00E0) >> 5;
    _rx_hwinfo->fw_build_day   = (build_date & 0x001F);

    _rx_hwinfo->add1 = _rx_payload->message[_rx_payload->m_pointer];
    _rx_payload->m_pointer += 1;
    _rx_hwinfo->add2 = _rx_payload->message[_rx_payload->m_pointer];
    _rx_payload->m_pointer += 1;

    // type text
    if (_rx_hwinfo->s_address_manufactur_id == 0x01) {
        if (_rx_hwinfo->device_type == 0x01) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Skytraxx", "Windstation");
        } else {
            sprintf(_rx_hwinfo->device_type_str, "%s unk: %d", "Skytraxx", _rx_hwinfo->device_type);
        }
    } else if (_rx_hwinfo->s_address_manufactur_id == 0x06) {
        if (_rx_hwinfo->device_type == 0x01) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Burnair", "Base Station Wifi");
        } else {
            sprintf(_rx_hwinfo->device_type_str, "%s unk: %d", "Burnair", _rx_hwinfo->device_type);
        }
    } else if (_rx_hwinfo->s_address_manufactur_id == 0x11) {
        if (_rx_hwinfo->device_type == 0x01) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Skytraxx", "3.0");
        } else if (_rx_hwinfo->device_type == 0x02) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Skytraxx", "2.1");
        } else if (_rx_hwinfo->device_type == 0x03) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Skytraxx", "Beacon");
        } else if (_rx_hwinfo->device_type == 0x10) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Naviter", "Oudie 5");
        } else if (_rx_hwinfo->device_type == 0x17) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "MR", "FGS");
        } else {
            sprintf(_rx_hwinfo->device_type_str, "%s unk: %d", "Skytraxx", _rx_hwinfo->device_type);
        }
    } else if (_rx_hwinfo->s_address_manufactur_id == 0xFB) {
        if (_rx_hwinfo->device_type == 0x01) {
            sprintf(_rx_hwinfo->device_type_str, "%s %s", "Skytraxx", "Wifi base station");
        } else {
            sprintf(_rx_hwinfo->device_type_str, "%s unk: %d", "Skytraxx", _rx_hwinfo->device_type);
        }
    } else {
        sprintf(_rx_hwinfo->device_type_str, "%s %d", "UNK Manuf. Type ", _rx_hwinfo->device_type);
    }
}

void type_8_hwinfo_coder(sRawMessage *_tx_payload, sHardwareInfo *_tx_hwinfo) {
    // assume MAC data will be encoded in message buffer
    memset(_tx_payload->message, 0, 256);
    _tx_payload->m_length = 0;
    _tx_payload->m_pointer = 0;

    // byte 0 - type
    _tx_payload->message[_tx_payload->m_pointer++] = _tx_hwinfo->device_type;

    // [Byte 1-2]	Firmware Build Date
    //bit 15		0: Release 1: Develop/Experimental Mode
    //bit 9-14		Year from 2019 (0 -> 2019, 1 -> 2020, ...)
    //bit 5-8		Month (1-12)
    //bit 0-4		Day (1-31)
    uint16_t  build_date = 0x0000;
    build_date |= _tx_hwinfo->experimental << 15;
    if (_tx_hwinfo->experimental)
        build_date |= 0x8000;
    build_date |= (_tx_hwinfo->fw_build_year - 2019) << 9;
    build_date |= (_tx_hwinfo->fw_build_month & 0x07) << 5;
    build_date |= (_tx_hwinfo->fw_build_day & 0x17);
    _tx_payload->message[_tx_payload->m_pointer++] = (build_date & 0x00ff);
    _tx_payload->message[_tx_payload->m_pointer++] = build_date >> 8;
    _tx_payload->message[_tx_payload->m_pointer++] = _tx_hwinfo->add1;
    _tx_payload->message[_tx_payload->m_pointer++] = _tx_hwinfo->add2;
    _tx_payload->m_length = _tx_payload->m_pointer;
}

void type_8_hwinfo_receiver(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_rx_payload) {
    sHardwareInfo _rx_hwinfo;

    type_8_hwinfo_decoder(_rx_payload, _fanet_mac, &_rx_hwinfo);
    terminal_message_8(0, 0, _radiodata, _fanet_mac, &_rx_hwinfo);
}
