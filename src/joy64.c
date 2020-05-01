//
// libjoy64/include/os/si.h: SI helper functions.
//
// Copyright 2017 Fraser
//			https://sites.google.com/site/consoleprotocols/
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//
#include "joy64.h"

void joyInitTxBuffer()
{
	for(uint32_t idx = 0; idx < 16; idx++)
		joy_buffer_tx.raw.uints[idx] = 0x0;
}

void joyInitDevices()
{
	for(uint32_t idx = 0;idx < MAX_CONTROLLERS; idx++)
	{
		joy_console.port[idx].device.raw = 0x0;
		joy_console.port[idx].data.raw[0] = 0x0;
		joy_console.port[idx].data.raw[1] = 0x0;
	}
}

// enableBackground will enable the easy use of si accessories
// Example:
//		Identify Controller Accessories
//		Enable/Disable Rumble
//		Get Pulse Sensor Readings
//		Read the Table of Contents for Memory PAK's
//		Read the Header of Gameboy Carts
//		Initialize the VRU/VRS, except Word List
// If you are using any of these features this will make your work easier.
// The processing time taken for these actions is very minimal
void joyInit(uint32_t port_enable_count, uint32_t enableBackground)
{
	if (port_enable_count > MAX_CONTROLLERS)
		port_enable_count = 1;	

	joy_console.portcount = port_enable_count;
	
	if (enableBackground)
		joy_console.state = JOY_GSTAT_BACKGR;
	else
		joy_console.state = JOY_GSTAT_INIT;
	
	joy_buffer_tx_ram_addr = (((uint32_t)&joy_buffer_tx) & RDRAM_DIRECT_MASK);
	joy_buffer_rx_ram_addr = (((uint32_t)&joy_buffer_rx) & RDRAM_DIRECT_MASK);
	joy_controller_background_index = 0;
	joyInitTxBuffer();
	joyInitDevices();
}

void joyStatusZeroWait()
{
	uint32_t status = 0;
	do
	{
		status = joyStatus();
	}
	while (status > 0);
}

void joyStatusDmaIoWait()
{
	uint32_t status = 0;
	do
	{
		status = (joyStatus() & (JOY_STATUS_IO_BUSY | JOY_STATUS_DMA_BUSY));		
	}
	while (status > 0);
}

void joySendRequest(uint32_t buffer_addr)
{
	joySetDramAddr(buffer_addr);
	joyDmaWrite(PIF_RAM_ADDR);
}


void joyRecvResponse(uint32_t buffer_addr)
{  		
	joySetDramAddr(buffer_addr);
	joyDmaRead(PIF_RAM_ADDR);	
}