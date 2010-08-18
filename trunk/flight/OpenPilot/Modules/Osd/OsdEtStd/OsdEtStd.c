/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup OSDModule OSD Module
 * @brief On screen display support
 * @{ 
 *
 * @file       OsdEtStd.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Interfacing with EagleTree OSD Std module
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "openpilot.h"

#include "flightbatterystate.h"
#include "positionactual.h"



//
// Configuration
//
#define DEBUG_PORT		PIOS_COM_GPS
#define STACK_SIZE		1024
#define TASK_PRIORITY	(tskIDLE_PRIORITY + 3)
#define ENABLE_DEBUG_MSG
//#define DUMP_CONFIG		// Enable this do read and dump the OSD config
//#define WRITE_CONFIG		// Enable this do write and verify the OSD config
//#define DO_PAR_SEEK		// Enable this to start a tool to find where parameters are encoded


//
// Private constants
//

#ifdef ENABLE_DEBUG_MSG
	#define DEBUG_MSG(format, ...) PIOS_COM_SendFormattedString(DEBUG_PORT, format, ## __VA_ARGS__)
#else
	#define DEBUG_MSG(format, ...)
#endif

#define CONFIG_LENGTH 6726
#define MIN(a,b) ((a)<(b)?(a):(b))

#define OSDMSG_V_LS_IDX		10
#define OSDMSG_A_LS_IDX		17
#define OSDMSG_VA_MS_IDX	18
#define OSDMSG_LAT_IDX		33
#define OSDMSG_LON_IDX		37
#define OSDMSG_HOME_IDX		47
#define OSDMSG_ALT_IDX		49
#define OSDMSG_NB_SATS		58
#define OSDMSG_GPS_STAT		59

#define OSDMSG_GPS_STAT_NOFIX	0x03
#define OSDMSG_GPS_STAT_FIX		0x2B
#define OSDMSG_GPS_STAT_HB_FLAG	0x10



//
// Private types
//

//
// Private variables
//

//	                  | Header / cmd?|                                  | V  |                                  |  V |                                                                     | LAT: 37 57.0000   | LONG: 24 00.4590  |                             | Hom<-179| Alt 545.4 m       |                        |#sat|stat|
//	                     00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40   41   42   43   44   45   46   47   48   49   50   51   52   53   54   55   56   57   58   59   60   61   62
	uint8_t msg[63] = {0x03,0x3F,0x03,0x00,0x00,0x00,0x00,0x00,0x90,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x60,0x10,0x02,0x00,0x00,0x90,0x00,0x54,0x54,0x00,0x00,0x33,0x28,0x13,0x00,0x00,0x08,0x00,0x00,0x90,0x0A};
static volatile bool newPosData=FALSE;
static volatile bool newBattData=FALSE;



//
// Private functions
//
static void WriteToMsg8(uint8_t index, uint8_t value)
{
	if (value>100)
		value = 100;

	msg[index] = ((value/10) << 4) + (value%10);
}

static void WriteToMsg16(uint8_t index, uint16_t value)
{
	WriteToMsg8(index, value % 100);
	WriteToMsg8(index+1, value / 100);
}

static void WriteToMsg24(uint8_t index, uint32_t value)
{
	WriteToMsg16(index, value % 10000);
	WriteToMsg8(index+2, value / 10000);
}

static void WriteToMsg32(uint8_t index, uint32_t value)
{
	WriteToMsg16(index, value % 10000);
	WriteToMsg16(index+2, value / 10000);
}

static void SetCoord(uint8_t index, float coord)
{
	uint32_t deg = (uint32_t)coord;
	float sec = (coord-deg)*60;

	WriteToMsg24(index, sec*10000);
	WriteToMsg8(index+3, deg);
}

static void SetCourse(uint16_t dir)
{
	WriteToMsg16(OSDMSG_HOME_IDX, dir);
}

static void SetAltitude(uint32_t altitudeMeter)
{
	WriteToMsg32(OSDMSG_ALT_IDX, altitudeMeter*10);
}

static void SetVoltage(uint32_t milliVolt)
{
	msg[OSDMSG_VA_MS_IDX] &= 0x0F;
	msg[OSDMSG_VA_MS_IDX] |= (milliVolt / 6444)<<4;
	msg[OSDMSG_V_LS_IDX] = (milliVolt % 6444)*256/6444;
}

static void SetCurrent(uint32_t milliAmp)
{
	uint32_t value = (milliAmp*16570/1000000) + 0x7FA;
	msg[OSDMSG_VA_MS_IDX] &= 0xF0;
	msg[OSDMSG_VA_MS_IDX] |= ((value >> 8) & 0x0F);
	msg[OSDMSG_A_LS_IDX] = (value & 0xFF);
}


static void SetNbSats(uint8_t nb)
{
	msg[OSDMSG_NB_SATS] = nb;
}

static void FlightBatteryStateUpdatedCb(UAVObjEvent* ev)
{
	newBattData = TRUE;
}

static void PositionActualUpdatedCb(UAVObjEvent* ev)
{
	newPosData = TRUE;
}

static void Task(void* parameters)
{
	uint32_t cnt = 0;

#ifdef ENABLE_DEBUG_MSG
	PIOS_COM_ChangeBaud(DEBUG_PORT, 57600);
#endif

	PositionActualConnectCallback(PositionActualUpdatedCb);
	FlightBatteryStateConnectCallback(FlightBatteryStateUpdatedCb);


	DEBUG_MSG("OSD ET Std Started\n");
	vTaskDelay(2000 / portTICK_RATE_MS);

	while (1)
	{
		DEBUG_MSG("%d\n\r", cnt);
#if 1
		if ( newBattData )
		{
			FlightBatteryStateData flightBatteryData;

			FlightBatteryStateGet(&flightBatteryData);

			DEBUG_MSG("%5d Batt: V=%dmV\n\r", cnt, (uint32_t)(flightBatteryData.Voltage*1000));

			SetVoltage((uint32_t)(flightBatteryData.Voltage*1000));
			SetCurrent((uint32_t)(flightBatteryData.Current*1000));
			newBattData = FALSE;
		}


		if (newPosData)
		{
			PositionActualData positionData;

			PositionActualGet(&positionData);

			DEBUG_MSG("%5d Pos: #stat=%d #sats=%d alt=%d\n\r", cnt,
					positionData.Status, positionData.Satellites, (uint32_t)positionData.Altitude);

			// GPS Status
			if (positionData.Status == POSITIONACTUAL_STATUS_FIX3D)
				msg[OSDMSG_GPS_STAT] = OSDMSG_GPS_STAT_FIX;
			else
				msg[OSDMSG_GPS_STAT] = OSDMSG_GPS_STAT_NOFIX;
			msg[OSDMSG_GPS_STAT] |= OSDMSG_GPS_STAT_HB_FLAG;


			// GPS info
			SetCoord(OSDMSG_LAT_IDX, positionData.Latitude);
			SetCoord(OSDMSG_LON_IDX, positionData.Longitude);
			SetAltitude(positionData.Altitude);
			SetNbSats(positionData.Satellites);
			SetCourse(positionData.Heading);

			newPosData = FALSE;
		}
		else
		{
			msg[OSDMSG_GPS_STAT] &= ~OSDMSG_GPS_STAT_HB_FLAG;
		}
#endif

		DEBUG_MSG("SendMsg .");
		{
			const struct pios_i2c_txn txn_list[] = {
				{
				  .addr = 0x30,
				  .rw   = PIOS_I2C_TXN_WRITE,
				  .len  = sizeof(msg),
				  .buf  = msg,
				},
			  };

			PIOS_I2C_Transfer(PIOS_I2C_MAIN_ADAPTER, txn_list, NELEMENTS(txn_list));
		}

		DEBUG_MSG("\n\r");

		cnt++;

		vTaskDelay(100 / portTICK_RATE_MS);
	}
}


//
// Public functions
//

/**
 * Initialise the module
 * \return -1 if initialisation failed
 * \return 0 on success
 */
int32_t OsdEtStdInitialize(void)
{
	// Start gps task
	xTaskCreate(Task, (signed char*) "Osd", STACK_SIZE, NULL, TASK_PRIORITY, NULL);

	return 0;
}
