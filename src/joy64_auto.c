//
// libjoy64/include/os/si.h: SI helper functions.
//
// Copyright 2017 Fraser
//			https://sites.google.com/site/consoleprotocols/
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//
#include "joy64.h"

uint32_t joyAutoBeforeVi() // 0x76 CPU Cycles aka 118 (Excluding background)
{
	if (joy_console.state & JOY_GSTAT_BACKGR)
	{
		// Parse Background SI Request
		joyStatusZeroWait(); // Should be ZERO delay, but make sure

		joyProcessBackground();
	}
	joyPreparePort();

	joyStatusDmaIoWait();
	
	joySendRequest(joy_buffer_tx_ram_addr);  // Time 0x06			
	
	joyStatusDmaIoWait();
	
	joyRecvResponse(joy_buffer_rx_ram_addr); // Time 0x06		
	return 0;
}

uint32_t joyAutoAfterVi() // 0x0A CPU Cycles aka 10 (Excluding background)
{
	uint32_t result = 0;
	joyStatusZeroWait(); // Longest delay is potentially right here.
	
	result = joyProcessPort(); // Process Player Input
	
	if (joy_console.state & JOY_GSTAT_BACKGR)
	{
		// Create Background SI Request
		if (joyPrepareBackground())
		{
			joyStatusDmaIoWait();		
			joySendRequest(joy_buffer_tx_ram_addr);
		}
	}

	return result;
}
/*
while(1) {
	
	if (requested)
	{
		recv_io();
	}
	prepare_port();
	prepare_io();
	
	send_port();
	video_interrupt();
	recv_port();
	send_io();

}
	dma wait (minimal)
controller dma tx
	dma wait (0x76)
controller dma rx
	dma wait
io dma tx
	dma wait
io dma rx

ms:us
6:60
7:210
940+210=

1150us ON
 546us OFF
 
Max bitrate 31,250 bytes / second
"Easy" 19,200 bytes / second
*/