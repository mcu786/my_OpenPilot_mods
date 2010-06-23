/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief      GPS gadget to display GPS Status
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   gpsdisplayplugin
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

#include "gpsdisplaygadget.h"
#include "gpsdisplaywidget.h"
#include "gpsdisplaygadgetconfiguration.h"

GpsDisplayGadget::GpsDisplayGadget(QString classId, GpsDisplayWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

GpsDisplayGadget::~GpsDisplayGadget()
{
}

/*
  This is called when a configuration is loaded, and updates the plugin's settings.
  Careful: the plugin is already drawn before the loadConfiguration method is called the
  first time, so you have to be careful not to assume all the plugin values are initialized
  the first time you use them
 */
void GpsDisplayGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    GpsDisplayGadgetConfiguration *m = qobject_cast<GpsDisplayGadgetConfiguration*>(config);
    //m_widget->setSystemFile(m->getSystemFile()); // Triggers widget repaint
}