/**
 ******************************************************************************
 *
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectsPlugin UAVObjects Plugin
 * @{
 * @brief      The UAVUObjects GCS plugin 
 *   
 * @note       Object definition file: headingactual.xml. 
 *             This is an automatically generated file.
 *             DO NOT modify manually.
 *
 * @file       headingactual.h
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
#ifndef HEADINGACTUAL_H
#define HEADINGACTUAL_H

#include "uavdataobject.h"
#include "uavobjectmanager.h"

class UAVOBJECTS_EXPORT HeadingActual: public UAVDataObject
{
    Q_OBJECT

public:
    // Field structure
    typedef struct {
        qint16 raw[3];
        float heading;

    } __attribute__((packed)) DataFields;

    // Field information
    // Field raw information
    /* Array element names for field raw */
    typedef enum { RAW_X=0, RAW_Y=1, RAW_Z=2,  } rawElem;
    /* Number of elements for field raw */
    static const quint32 RAW_NUMELEM = 3;
    // Field heading information

  
    // Constants
    static const quint32 OBJID = 2921442332U;
    static const QString NAME;
    static const bool ISSINGLEINST = 1;
    static const bool ISSETTINGS = 0;
    static const quint32 NUMBYTES = sizeof(DataFields);

    // Functions
    HeadingActual();

    DataFields getData();
    void setData(const DataFields& data);
    Metadata getDefaultMetadata();
    UAVDataObject* clone(quint32 instID);

    static HeadingActual* GetInstance(UAVObjectManager* objMngr, quint32 instID = 0);
	
private:
    DataFields data;

    void setDefaultFieldValues();

};

#endif // HEADINGACTUAL_H
