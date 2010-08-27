/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{ 
 * @addtogroup BaroAltitude BaroAltitude 
 * @brief The raw data from the barometric sensor with pressure, temperature and altitude estimate.
 *
 * Autogenerated files and functions for BaroAltitude Object
 
 * @{ 
 *
 * @file       baroaltitude.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of the BaroAltitude object. This file has been 
 *             automatically generated by the UAVObjectGenerator.
 * 
 * @note       Object definition file: baroaltitude.xml. 
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

#ifndef BAROALTITUDE_H
#define BAROALTITUDE_H

// Object constants
#define BAROALTITUDE_OBJID 3980666102U
#define BAROALTITUDE_NAME "BaroAltitude"
#define BAROALTITUDE_METANAME "BaroAltitudeMeta"
#define BAROALTITUDE_ISSINGLEINST 1
#define BAROALTITUDE_ISSETTINGS 0
#define BAROALTITUDE_NUMBYTES sizeof(BaroAltitudeData)

// Object access macros
/**
 * @function BaroAltitudeGet(dataOut)
 * @brief Populate a BaroAltitudeData object
 * @param[out] dataOut 
 */
#define BaroAltitudeGet(dataOut) UAVObjGetData(BaroAltitudeHandle(), dataOut)
#define BaroAltitudeSet(dataIn) UAVObjSetData(BaroAltitudeHandle(), dataIn)
#define BaroAltitudeInstGet(instId, dataOut) UAVObjGetInstanceData(BaroAltitudeHandle(), instId, dataOut)
#define BaroAltitudeInstSet(instId, dataIn) UAVObjSetInstanceData(BaroAltitudeHandle(), instId, dataIn)
#define BaroAltitudeConnectQueue(queue) UAVObjConnectQueue(BaroAltitudeHandle(), queue, EV_MASK_ALL_UPDATES)
#define BaroAltitudeConnectCallback(cb) UAVObjConnectCallback(BaroAltitudeHandle(), cb, EV_MASK_ALL_UPDATES)
#define BaroAltitudeCreateInstance() UAVObjCreateInstance(BaroAltitudeHandle())
#define BaroAltitudeRequestUpdate() UAVObjRequestUpdate(BaroAltitudeHandle())
#define BaroAltitudeRequestInstUpdate(instId) UAVObjRequestInstanceUpdate(BaroAltitudeHandle(), instId)
#define BaroAltitudeUpdated() UAVObjUpdated(BaroAltitudeHandle())
#define BaroAltitudeInstUpdated(instId) UAVObjUpdated(BaroAltitudeHandle(), instId)
#define BaroAltitudeGetMetadata(dataOut) UAVObjGetMetadata(BaroAltitudeHandle(), dataOut)
#define BaroAltitudeSetMetadata(dataIn) UAVObjSetMetadata(BaroAltitudeHandle(), dataIn)
#define BaroAltitudeReadOnly(dataIn) UAVObjReadOnly(BaroAltitudeHandle(), dataIn)

// Object data
typedef struct {
    float Altitude;
    float Temperature;
    float Pressure;

} __attribute__((packed)) BaroAltitudeData;

// Field information
// Field Altitude information
// Field Temperature information
// Field Pressure information


// Generic interface functions
int32_t BaroAltitudeInitialize();
UAVObjHandle BaroAltitudeHandle();

#endif // BAROALTITUDE_H

/**
 * @}
 * @}
 */