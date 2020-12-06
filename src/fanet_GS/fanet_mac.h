/***********************************************************************
 * fanet_mac.h
 * 
 * Copyright 2018  <christoph@betschart.ch>
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

#ifndef FANET_MAC_H
#define FANET_MAC_H

#include "fanet_struct.h"
#include "fanet_global.h"

void fanet_mac_coder(sRadioData *_radiodata, sFanetMAC *_fanet_mac, sRawMessage *_tx_message);

void fanet_mac_decoder(sFanetMAC *_fanet_mac, sRawMessage *_rx_message, sRawMessage *_rx_payload);

#endif
