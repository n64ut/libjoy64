//
// libjoy64/si_port.c: controller input functions.
//
// Copyright 2017 Fraser
//			https://sites.google.com/site/consoleprotocols/
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//
#include "joy64.h"
uint32_t joyPrepareBackground()
{
	// The trick here is that each of these are 32 byte reads/writes.
	// Each must be done one at a time
	//	The programmer initializing with an appropriate controller count will really help.
	
	// We kind of have to assume if the gamer is "hot swapping" they are ok with 
	//	several frames (to several seconds) of lag before something works.
	//	The number of controllers will also affect when the controller data is ready.
	// Examples if Single Controller initialized:
	//		Controllers (1 frame identify, next frame data) Main thread.
	// 		Rumble (2 frame Identify, next frame enable)
	// 		Pulse Sensor (2 frame Identify, next frame enable)
	//		Transfer PAK (6 frame Identify, 3 frame load header)
	//		Controller PAK (2 frame Identify, 40 frame load Table of Contents)	
	//  The max lag would be 4 controllers with 4 controller pak's (168 frames or 2.8 seconds)
	//	
	//		This might not seem "fast enough" to some, I'd ask how long does it take a player
	// 	to insert any of these accessories and look at the screen. The accessory will be identified 
	//  before they look and the background data will already be loaded by the time they are ready
	//  to take their next action.
	//		Loading an entire controller PAK or Transfer PAK will have their own api's.
	
	// 1. identify the accessories
	//		If an accessory is removed or swapped in always do identify
	// 2. Identify EEPROM
	// 3. Load/Manage devices
	// 		1. Read GB/GBC Header (2 reads)
	// 		2. retrieve input data (i.e. Pulse Sensor)
	// 		3. manage rumble pak
	// 		4. Controller PAK ToC
	uint32_t buffer_offset = 0;	// uints so 4 bytes / offset	
	uint32_t controllers_checked = 0;
	uint32_t work_queued = 0;
	do
	{
		if (joy_console.port[joy_controller_background_index].device.dev_type.id == N64_CONTROLLER)
		{		
			if (joy_console.port[joy_controller_background_index].device.dev_type.acc == JOY_ACC_UNKNOWN)
			{
				// Send Write 0xFE to 0x8000
				joy_buffer_tx.raw.uints[buffer_offset++] = 0x0;
				joy_console.port[joy_controller_background_index].device.dev_type.acc = JOY_ACC_REQUESTW;
				work_queued++;
			}
			else if (joy_console.port[joy_controller_background_index].device.dev_type.acc == JOY_ACC_REQUESTW)
			{
				// Send Read to 0x8000
				joy_buffer_tx.raw.uints[buffer_offset++] = 0x0;
				// Response: 0xE1 with memory pak, 0x1E without.
				joy_console.port[joy_controller_background_index].device.dev_type.acc = JOY_ACC_REQUESTR;
				work_queued++;
			}
			else
			{
				switch(joy_console.port[joy_controller_background_index].device.dev_type.acc)
				{				
					case JOY_ACC_RUMB_PAK:
					{
						if (joy_console.port[joy_controller_background_index].device.dev_type.state & JOY_GSTAT_ACC_ON)
						{
							// Send Command to Turn ON Rumble
							// Send Write 0x01 to 0xC000
						}
						else
						{
							// Send Command to Turn OFF Rumble
							// Send Write 0x00 to 0xC000
						}
					}
					break;
					case JOY_ACC_PULS_PAK:
					{
						// Send command to take pulse reading
					}
					break;
					case JOY_ACC_CONT_PAK:
					{
						// Load Table of Contents.
					}
					break;
					case JOY_ACC_TXFR_PAK:
					{
						// Load GB/GBC Header
					}
					break;
				}
			}
		}
		if (!work_queued)
		{
			// si_controller_background_index Increment
		}
	}
	while (work_queued == 0 && controllers_checked < joy_console.portcount);
	if (work_queued)
	{
		joy_buffer_tx.raw.uints[buffer_offset] = JOY_REQ_FINL_CH;
		joy_buffer_tx.raw.bytes[63] = 0x01;
	}
	return work_queued;
}

void joyPreparePort()
{
	uint32_t buffer_offset = 0;	// uints so 4 bytes / offset
	uint32_t controller_count = joy_console.portcount;
	for( uint32_t idx = 0; idx < controller_count; idx++)
	{						
		switch(joy_console.port[idx].device.dev_type.id)
		{
			case N64_CONTROLLER:
			case N64_MOUSE:
			case N64_DENSHA: // Query Controller Data			
			{		
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_CONT_ST;															
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_FILL_FF;												
			}
			break;	
			case N64_KEYBOARD:
			{
				//buffer_offset+=4;
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_KYBD_ST;
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_FILL_FF;
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_FILL_FF;
			}
			break;
			default: // Type Query or Reset
			{				
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_CONT_QY;				
				joy_buffer_tx.raw.uints[buffer_offset++] = JOY_REQ_FILL_FF;								
			}
			break;
		}
	}
	joy_buffer_tx.raw.uints[buffer_offset] = JOY_REQ_FINL_CH;
	joy_buffer_tx.raw.bytes[63] = 0x01;
}

uint32_t joyProcessBackground()
{
	return 0;
}

uint32_t joyProcessPort()
{
	uint32_t port_idx = 0;
	
	uint32_t result = NO_ERROR;
	for(uint32_t rx_uint_idx = 0;rx_uint_idx < 16; rx_uint_idx++)
	{		
		uint32_t requestCommand = joy_buffer_rx.raw.uints[rx_uint_idx];
		uint32_t error = (requestCommand & 0xC000) >> 8;
		rx_uint_idx++;
		if (error > 0)
		{
			joy_console.port[port_idx].device.raw = 0xFFFFFF00 | error | JOY_PSTAT_CHANG;
			joy_console.port[port_idx].data.raw[0] = 0x0;
			joy_console.port[port_idx].data.raw[1] = 0x0;
		}
		else if (requestCommand == JOY_REQ_CONT_QY)
		{
			uint32_t responseValue = joy_buffer_rx.raw.uints[rx_uint_idx];
			// TODO: Process Held & Released Properties
			joy_console.port[port_idx].device.raw = (responseValue & 0xFFFFFF00) | JOY_PSTAT_CHANG;
		}
		else if (requestCommand == JOY_REQ_CONT_ST)
		{			
			joy_console.port[port_idx].data.raw[0] = joy_buffer_rx.raw.uints[rx_uint_idx];			
			result = JOY_REQ_CONT_ST;
		}		
		else if (requestCommand == JOY_REQ_KYBD_ST)
		{
			// TODO: Convert Keys
			joy_console.port[port_idx].data.raw[0] = joy_buffer_rx.raw.uints[rx_uint_idx++];			
			joy_console.port[port_idx].data.raw[1] = joy_buffer_rx.raw.uints[rx_uint_idx++];
			result = JOY_REQ_KYBD_ST;
		}
		else if (requestCommand == JOY_REQ_FINL_CH)
		{
			break; // exit for loop when 0xFE000000 is found. 
		}	
		else if (requestCommand == 0x0)
		{
			// Ignored
		}
		else
		{
			result = requestCommand;
		}
		port_idx++;
		if (port_idx > (MAX_CONTROLLERS - 1))
			break;
	}
	return result;
}

// If you want to offer the use of Non-Standard controllers
// 1. Prompt user to Hold "Start + A" buttons
// 2. Call in a loop for 5-10 seconds or whatever you think your user needs.
void joyControllerManualIdentify()
{
	for(uint32_t idx = 0; idx < MAX_CONTROLLERS; idx++)
	{
		uint32_t buttons = (joy_console.port[idx].data.controller.pressed);
		if ((buttons & DENSHA_TYPE_TEST) == DENSHA_TYPE_TEST)
			joy_console.port[idx].device.raw = ((N64_DENSHA << 16) & 0x0200);
		else if ((buttons & STANDARD_TYPE_TEST) == STANDARD_TYPE_TEST)
			joy_console.port[idx].device.raw = ((N64_CONTROLLER << 16) & 0x0200);
	}
}