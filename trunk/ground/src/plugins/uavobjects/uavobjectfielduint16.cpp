/**
 ******************************************************************************
 *
 * @file       uavobjectfielduint16.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjects_plugin
 * @{
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
#include "uavobjectfielduint16.h"

/**
 * Constructor
 */
UAVObjectFieldUInt16::UAVObjectFieldUInt16(const QString& name, const QString& units, quint32 numElements):
        UAVObjectField(name, units, numElements)
{
    numBytesPerElement = sizeof(quint16);
}

UAVObjectFieldUInt16::UAVObjectFieldUInt16(const QString& name, const QString& units, const QStringList& elementNames):
        UAVObjectField(name, units, elementNames)
{
    numBytesPerElement = sizeof(quint16);
}

/**
 * Initialize all values
 */
void UAVObjectFieldUInt16::initializeValues()
{
    for (quint32 n = 0; n < numElements; ++n)
    {
        setValue(0, n);
    }
}

/**
 * Pack the field in to an array of bytes.
 */
qint32 UAVObjectFieldUInt16::pack(quint8* dataOut)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        quint16 value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        qToLittleEndian<quint16>(value, &dataOut[numBytesPerElement*index]);
    }
    // Done
    return getNumBytes();
}

/**
 * Unpack the field from an array of bytes.
 */
qint32 UAVObjectFieldUInt16::unpack(const quint8* dataIn)
{
    QMutexLocker locker(obj->getMutex());
    // Pack each element in output buffer
    for (quint32 index = 0; index < numElements; ++index)
    {
        quint16 value;
        value = qFromLittleEndian<quint16>(&dataIn[numBytesPerElement*index]);
        memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
    }
    // Done
    return getNumBytes();
}

/**
 * Get the field value as a double.
 */
double UAVObjectFieldUInt16::getDouble(quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    double ret = 0.0;
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        quint16 value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        ret = (double)value;
    }
    // Done
    return ret;
}

/**
 * Set the field value from a double.
 */
void UAVObjectFieldUInt16::setDouble(double value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        quint16 tmpValue;
        tmpValue = (quint16)value;
        memcpy(&data[offset + numBytesPerElement*index], &tmpValue, numBytesPerElement);
    }

    // Emit updated event
    emit fieldUpdated(this);
}

/**
 * Get the number of bytes per field element.
 */
quint32 UAVObjectFieldUInt16::getNumBytesElement()
{
    return numBytesPerElement;
}

/**
 * Get the field value.
 */
quint16 UAVObjectFieldUInt16::getValue(quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        quint16 value;
        memcpy(&value, &data[offset + numBytesPerElement*index], numBytesPerElement);
        return value;
    }
    else
    {
        return 0;
    }
}

/**
 * Set the field value.
 */
void UAVObjectFieldUInt16::setValue(quint16 value, quint32 index)
{
    QMutexLocker locker(obj->getMutex());
    // Check if index is out of bounds or no data available
    if (index < numElements && data != NULL)
    {
        memcpy(&data[offset + numBytesPerElement*index], &value, numBytesPerElement);
    }

    // Emit updated event
    emit fieldUpdated(this);
}




