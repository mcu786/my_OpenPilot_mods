/**
 ******************************************************************************
 *
 * @file       airspeedgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#ifndef AIRSPEEDGADGETCONFIGURATION_H
#define AIRSPEEDGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* Despite its name, this is actually a generic analog dial
   supporting up to two rotating "needle" indicators.
  */
class AirspeedGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit AirspeedGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString dialFile){m_defaultDial=dialFile;}
    void setDialBackgroundID(QString elementID) { dialBackgroundID = elementID;}
    void setDialForegroundID(QString elementID) { dialForegroundID = elementID;}
    void setDialNeedleID1(QString elementID) { dialNeedleID1 = elementID;}
    void setDialNeedleID2(QString elementID) { dialNeedleID2 = elementID;}
    void setN1Min(double val) { needle1MinValue = val;}
    void setN2Min(double val) { needle2MinValue = val;}
    void setN1Max(double val) { needle1MaxValue = val;}
    void setN2Max(double val) { needle2MaxValue = val;}

    //get dial configuration functions
    QString dialFile() {return m_defaultDial;}
    QString dialBackground() {return dialBackgroundID;}
    QString dialForeground() {return dialForegroundID;}
    QString dialNeedle1() {return dialNeedleID1;}
    QString dialNeedle2() {return dialNeedleID2;}
    double getN1Min() { return needle1MinValue;}
    double getN2Min() { return needle2MinValue;}
    double getN1Max() { return needle1MaxValue;}
    double getN2Max() { return needle2MaxValue;}

    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultDial; // The name of the dial's SVG source file
    QString dialBackgroundID; // SVG elementID of the background
    QString dialForegroundID; // ... of the foreground
    QString dialNeedleID1;     // ... and the first needle
    QString dialNeedleID2;     // ... and the second
    // TODO: define additional elements such as secondary needle

    double needle1MinValue; // Value corresponding to a 0 degree angle;
    double needle1MaxValue; // Value corresponding to a 360 degree angle;
    double needle2MinValue;
    double needle2MaxValue;
};

#endif // AIRSPEEDGADGETCONFIGURATION_H
