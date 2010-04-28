/**
 ******************************************************************************
 *
 * @file       examplesettings.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of the ExampleSettings object. This file has been 
 *             automatically generated by the UAVObjectGenerator.
 * 
 * @note       Object definition file: examplesettings.xml. 
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

#ifndef EXAMPLESETTINGS_H
#define EXAMPLESETTINGS_H

// Object constants
#define EXAMPLESETTINGS_OBJID 1640607828U
#define EXAMPLESETTINGS_NAME "ExampleSettings"
#define EXAMPLESETTINGS_METANAME "ExampleSettingsMeta"
#define EXAMPLESETTINGS_ISSINGLEINST 1
#define EXAMPLESETTINGS_ISSETTINGS 1
#define EXAMPLESETTINGS_NUMBYTES sizeof(ExampleSettingsData)

// Object access macros
#define ExampleSettingsGet(dataOut) UAVObjGetData(ExampleSettingsHandle(), dataOut)
#define ExampleSettingsSet(dataIn) UAVObjSetData(ExampleSettingsHandle(), dataIn)
#define ExampleSettingsInstGet(instId, dataOut) UAVObjGetInstanceData(ExampleSettingsHandle(), instId, dataOut)
#define ExampleSettingsInstSet(instId, dataIn) UAVObjSetInstanceData(ExampleSettingsHandle(), instId, dataIn)
#define ExampleSettingsConnectQueue(queue) UAVObjConnectQueue(ExampleSettingsHandle(), queue, EV_MASK_ALL_UPDATES)
#define ExampleSettingsConnectCallback(cb) UAVObjConnectCallback(ExampleSettingsHandle(), cb, EV_MASK_ALL_UPDATES)
#define ExampleSettingsCreateInstance() UAVObjCreateInstance(ExampleSettingsHandle())
#define ExampleSettingsRequestUpdate() UAVObjRequestUpdate(ExampleSettingsHandle())
#define ExampleSettingsRequestInstUpdate(instId) UAVObjRequestInstanceUpdate(ExampleSettingsHandle(), instId)
#define ExampleSettingsUpdated() UAVObjUpdated(ExampleSettingsHandle())
#define ExampleSettingsInstUpdated(instId) UAVObjUpdated(ExampleSettingsHandle(), instId)
#define ExampleSettingsGetMetadata(dataOut) UAVObjGetMetadata(ExampleSettingsHandle(), dataOut)
#define ExampleSettingsSetMetadata(dataIn) UAVObjSetMetadata(ExampleSettingsHandle(), dataIn)

// Object data
typedef struct {
    int32_t UpdatePeriod;
    int32_t StepSize;
    uint8_t StepDirection;

} __attribute__((packed)) ExampleSettingsData;

// Field information
// Field UpdatePeriod information
// Field StepSize information
// Field StepDirection information
/* Enumeration options for field StepDirection */
typedef enum { EXAMPLESETTINGS_STEPDIRECTION_UP=0, EXAMPLESETTINGS_STEPDIRECTION_DOWN=1,  } ExampleSettingsStepDirectionOptions;


// Generic interface functions
int32_t ExampleSettingsInitialize();
UAVObjHandle ExampleSettingsHandle();

#endif // EXAMPLESETTINGS_H
