/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{ 
 * @addtogroup HomeLocation HomeLocation
 * @brief HomeLocation setting which contains the constants to tranlate from longitutde and latitude to NED reference frame.  Automatically set by @ref GPSModule after acquiring a 3D lock.  Used by @ref AHRSCommsModule.
 *
 * Autogenerated files and functions for HomeLocation Object
 * @{ 
 *
 * @file       homelocation.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of the HomeLocation object. This file has been 
 *             automatically generated by the UAVObjectGenerator.
 * 
 * @note       Object definition file: homelocation.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
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
#include "homelocation.h"

// Private variables
static UAVObjHandle handle;

// Private functions
static void setDefaults(UAVObjHandle obj, uint16_t instId);

/**
 * Initialize object.
 * \return 0 Success
 * \return -1 Failure
 */
int32_t HomeLocationInitialize()
{
	// Register object with the object manager
	handle = UAVObjRegister(HOMELOCATION_OBJID, HOMELOCATION_NAME, HOMELOCATION_METANAME, 0,
			HOMELOCATION_ISSINGLEINST, HOMELOCATION_ISSETTINGS, HOMELOCATION_NUMBYTES, &setDefaults);

	// Done
	if (handle != 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/**
 * Initialize object fields and metadata with the default values.
 * If a default value is not specified the object fields
 * will be initialized to zero.
 */
static void setDefaults(UAVObjHandle obj, uint16_t instId)
{
	HomeLocationData data;
	UAVObjMetadata metadata;

	// Initialize object fields to their default values
	UAVObjGetInstanceData(obj, instId, &data);
	memset(&data, 0, sizeof(HomeLocationData));
	data.Set = 0;
	data.Indoor = 0;
	data.Latitude = 0;
	data.Longitude = 0;
	data.Altitude = 0;
	data.ECEF[0] = 0;
	data.ECEF[1] = 0;
	data.ECEF[2] = 0;
	data.RNE[0] = 0;
	data.RNE[1] = 0;
	data.RNE[2] = 0;
	data.RNE[3] = 0;
	data.RNE[4] = 0;
	data.RNE[5] = 0;
	data.RNE[6] = 0;
	data.RNE[7] = 0;
	data.RNE[8] = 0;
	data.Be[0] = 0;
	data.Be[1] = 0;
	data.Be[2] = 0;

	UAVObjSetInstanceData(obj, instId, &data);

	// Initialize object metadata to their default values
	metadata.access = ACCESS_READWRITE;
	metadata.gcsAccess = ACCESS_READWRITE;
	metadata.telemetryAcked = 1;
	metadata.telemetryUpdateMode = UPDATEMODE_ONCHANGE;
	metadata.telemetryUpdatePeriod = 0;
	metadata.gcsTelemetryAcked = 1;
	metadata.gcsTelemetryUpdateMode = UPDATEMODE_ONCHANGE;
	metadata.gcsTelemetryUpdatePeriod = 0;
	metadata.loggingUpdateMode = UPDATEMODE_NEVER;
	metadata.loggingUpdatePeriod = 0;
	UAVObjSetMetadata(obj, &metadata);
}

/**
 * Get object handle
 */
UAVObjHandle HomeLocationHandle()
{
	return handle;
}

/**
 * @}
 */

