/**
 ******************************************************************************
 * @addtogroup UAVObjects OpenPilot UAVObjects
 * @{ 
 * @addtogroup AhrsStatus AhrsStatus 
 * @brief Status for the @ref AHRSCommsModule, including communication errors
 *
 * Autogenerated files and functions for AhrsStatus Object
 
 * @{ 
 *
 * @file       ahrsstatus.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of the AhrsStatus object. This file has been 
 *             automatically generated by the UAVObjectGenerator.
 * 
 * @note       Object definition file: ahrsstatus.xml. 
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

#ifndef AHRSSTATUS_H
#define AHRSSTATUS_H

// Object constants
#define AHRSSTATUS_OBJID 1854089084U
#define AHRSSTATUS_NAME "AhrsStatus"
#define AHRSSTATUS_METANAME "AhrsStatusMeta"
#define AHRSSTATUS_ISSINGLEINST 1
#define AHRSSTATUS_ISSETTINGS 0
#define AHRSSTATUS_NUMBYTES sizeof(AhrsStatusData)

// Object access macros
/**
 * @function AhrsStatusGet(dataOut)
 * @brief Populate a AhrsStatusData object
 * @param[out] dataOut 
 */
#define AhrsStatusGet(dataOut) UAVObjGetData(AhrsStatusHandle(), dataOut)
#define AhrsStatusSet(dataIn) UAVObjSetData(AhrsStatusHandle(), dataIn)
#define AhrsStatusInstGet(instId, dataOut) UAVObjGetInstanceData(AhrsStatusHandle(), instId, dataOut)
#define AhrsStatusInstSet(instId, dataIn) UAVObjSetInstanceData(AhrsStatusHandle(), instId, dataIn)
#define AhrsStatusConnectQueue(queue) UAVObjConnectQueue(AhrsStatusHandle(), queue, EV_MASK_ALL_UPDATES)
#define AhrsStatusConnectCallback(cb) UAVObjConnectCallback(AhrsStatusHandle(), cb, EV_MASK_ALL_UPDATES)
#define AhrsStatusCreateInstance() UAVObjCreateInstance(AhrsStatusHandle())
#define AhrsStatusRequestUpdate() UAVObjRequestUpdate(AhrsStatusHandle())
#define AhrsStatusRequestInstUpdate(instId) UAVObjRequestInstanceUpdate(AhrsStatusHandle(), instId)
#define AhrsStatusUpdated() UAVObjUpdated(AhrsStatusHandle())
#define AhrsStatusInstUpdated(instId) UAVObjUpdated(AhrsStatusHandle(), instId)
#define AhrsStatusGetMetadata(dataOut) UAVObjGetMetadata(AhrsStatusHandle(), dataOut)
#define AhrsStatusSetMetadata(dataIn) UAVObjSetMetadata(AhrsStatusHandle(), dataIn)
#define AhrsStatusReadOnly(dataIn) UAVObjReadOnly(AhrsStatusHandle())

// Object data
typedef struct {
    uint8_t SerialNumber[25];
    uint8_t CPULoad;
    uint8_t IdleTimePerCyle;
    uint8_t RunningTimePerCyle;
    uint8_t CommErrors[5];
    uint8_t AlgorithmSet;
    uint8_t CalibrationSet;
    uint8_t HomeSet;

} __attribute__((packed)) AhrsStatusData;

// Field information
// Field SerialNumber information
/* Number of elements for field SerialNumber */
#define AHRSSTATUS_SERIALNUMBER_NUMELEM 25
// Field CPULoad information
// Field IdleTimePerCyle information
// Field RunningTimePerCyle information
// Field CommErrors information
/* Array element names for field CommErrors */
typedef enum { AHRSSTATUS_COMMERRORS_ALGORITHM=0, AHRSSTATUS_COMMERRORS_UPDATE=1, AHRSSTATUS_COMMERRORS_ATTITUDERAW=2, AHRSSTATUS_COMMERRORS_HOMELOCATION=3, AHRSSTATUS_COMMERRORS_CALIBRATION=4 } AhrsStatusCommErrorsElem;
/* Number of elements for field CommErrors */
#define AHRSSTATUS_COMMERRORS_NUMELEM 5
// Field AlgorithmSet information
/* Enumeration options for field AlgorithmSet */
typedef enum { AHRSSTATUS_ALGORITHMSET_FALSE=0, AHRSSTATUS_ALGORITHMSET_TRUE=1 } AhrsStatusAlgorithmSetOptions;
// Field CalibrationSet information
/* Enumeration options for field CalibrationSet */
typedef enum { AHRSSTATUS_CALIBRATIONSET_FALSE=0, AHRSSTATUS_CALIBRATIONSET_TRUE=1 } AhrsStatusCalibrationSetOptions;
// Field HomeSet information
/* Enumeration options for field HomeSet */
typedef enum { AHRSSTATUS_HOMESET_FALSE=0, AHRSSTATUS_HOMESET_TRUE=1 } AhrsStatusHomeSetOptions;


// Generic interface functions
int32_t AhrsStatusInitialize();
UAVObjHandle AhrsStatusHandle();

#endif // AHRSSTATUS_H

/**
 * @}
 * @}
 */
