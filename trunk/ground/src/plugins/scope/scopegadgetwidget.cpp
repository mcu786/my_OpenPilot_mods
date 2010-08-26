/**
 ******************************************************************************
 *
 * @file       scopegadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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


#include "uavobjects/uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"
#include "scopegadgetwidget.h"
#include "utils/stylehelper.h"

#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_legend.h"
#include "qwt/src/qwt_legend_item.h"
#include "qwt/src/qwt_plot_grid.h"

#include <iostream>
#include <math.h>
#include <QDebug>
#include <QColor>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>


TestDataGen* ScopeGadgetWidget::testDataGen;

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent)
{
//    if(testDataGen == 0)
//        testDataGen = new TestDataGen();

    //Setup the timer that replots data
    replotTimer = new QTimer(this);
    connect(replotTimer, SIGNAL(timeout()), this, SLOT(replotNewData()));
}

void ScopeGadgetWidget::preparePlot(PlotType plotType)
{
    m_plotType = plotType;

    clearCurvePlots();

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Show a title
    setTitle("Scope");    

    setCanvasBackground(Utils::StyleHelper::baseColor());

    //Add grid lines
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajPen(QPen(Qt::gray, 0, Qt::DashLine));
    grid->setMinPen(QPen(Qt::lightGray, 0 , Qt::DotLine));
    grid->attach(this);

    // Show a legend at the bottom
    if (legend() == 0) {
        QwtLegend *legend = new QwtLegend();
        legend->setItemMode(QwtLegend::CheckableItem);
        legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
        insertLegend(legend, QwtPlot::BottomLegend);
    }

    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),this, SLOT(showCurve(QwtPlotItem *, bool)));

    if(!replotTimer->isActive())
        replotTimer->start(m_refreshInterval);
    else
    {
        replotTimer->setInterval(m_refreshInterval);
    }

}

void ScopeGadgetWidget::showCurve(QwtPlotItem *item, bool on)
{
    item->setVisible(!on);
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

    replot();
}

void ScopeGadgetWidget::setupSequencialPlot()
{
    preparePlot(SequencialPlot);

    setAxisTitle(QwtPlot::xBottom, "Index");
    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    setAxisScale(QwtPlot::xBottom, 0, m_xWindowSize);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
}

void ScopeGadgetWidget::setupChronoPlot()
{
    preparePlot(ChronoPlot);

    setAxisTitle(QwtPlot::xBottom, "Time [h:m:s]");
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    uint NOW = QDateTime::currentDateTime().toTime_t();
    setAxisScale(QwtPlot::xBottom, NOW - m_xWindowSize / 1000, NOW);
    setAxisLabelRotation(QwtPlot::xBottom, -15.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    /*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
    const int fmh = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, fmh / 2);
}

void ScopeGadgetWidget::addCurvePlot(QString uavObject, QString uavField, int scaleOrderFactor, QPen pen)
{
    PlotData* plotData;

    if (m_plotType == SequencialPlot)
        plotData = new SequencialPlotData(uavObject, uavField);
    else if (m_plotType == ChronoPlot)
        plotData = new ChronoPlotData(uavObject, uavField);
    //else if (m_plotType == UAVObjectPlot)
    //    plotData = new UAVObjectPlotData(uavObject, uavField);

    plotData->m_xWindowSize = m_xWindowSize;
    plotData->scalePower = scaleOrderFactor;

    //If the y-bounds are supplied, set them
    if (plotData->yMinimum != plotData->yMaximum)
        setAxisScale(QwtPlot::yLeft, plotData->yMinimum, plotData->yMaximum);

    //Create the curve
    QString curveName = (plotData->uavObject) + "." + (plotData->uavField);
    QwtPlotCurve* plotCurve = new QwtPlotCurve(curveName);
    plotCurve->setPen(pen);
    plotCurve->setData(*plotData->xData, *plotData->yData);
    plotCurve->attach(this);
    plotData->curve = plotCurve;


    //Keep the curve details for later
    m_curvesData.insert(curveName, plotData);

    //Get the object to monitor
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject((plotData->uavObject)));

    //Link to the signal of new data only if this UAVObject has not been to connected yet
    if (!m_connectedUAVObjects.contains(obj->getName())) {
        m_connectedUAVObjects.append(obj->getName());
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

    replot();
}

void ScopeGadgetWidget::removeCurvePlot(QString uavObject, QString uavField)
{
    QString curveName = uavObject + "." + uavField;

    PlotData* plotData = m_curvesData.take(curveName);
    m_curvesData.remove(curveName);
    plotData->curve->detach();

    delete plotData->curve;
    delete plotData;

    replot();
}

void ScopeGadgetWidget::uavObjectReceived(UAVObject* obj)
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->append(obj);
    }
}

void ScopeGadgetWidget::replotNewData()
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->removeStaleData();
        plotData->curve->setData(*plotData->xData, *plotData->yData);
    }

    QDateTime NOW = QDateTime::currentDateTime();
    double toTime = NOW.toTime_t();
    toTime += NOW.time().msec() / 1000.0;
    if (m_plotType == ChronoPlot) {
        setAxisScale(QwtPlot::xBottom, toTime - m_xWindowSize, toTime);
    }
     //qDebug() << "replotNewData from " << NOW.addSecs(- m_xWindowSize) << " to " << NOW;

    replot();
}


void ScopeGadgetWidget::setupExamplePlot()
{
    preparePlot(SequencialPlot);

    // Show the axes

    setAxisTitle(xBottom, "x");
    setAxisTitle(yLeft, "y");

    // Calculate the data, 500 points each
    const int points = 500;
    double x[ points ];
    double sn[ points ];
    double cs[ points ];
    double sg[ points ];

    for (int i = 0; i < points; i++) {
        x[i] = (3.0 * 3.14 / double(points)) * double(i);
        sn[i] = 2.0 * sin(x[i]);
        cs[i] = 3.0 * cos(x[i]);
        sg[i] = (sn[i] > 0) ? 1 : ((sn[i] < 0) ? -1 : 0);
    }

    // add curves
    QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
    curve1->setPen(QPen(Qt::blue));
    QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");
    curve2->setPen(QPen(Qt::red));
    QwtPlotCurve *curve3 = new QwtPlotCurve("Curve 3");
    curve3->setPen(QPen(Qt::green));

    // copy the data into the curves
    curve1->setData(x, sn, points);
    curve2->setData(x, cs, points);
    curve3->setData(x, sg, points);



    curve1->attach(this);
    curve2->attach(this);
    curve3->attach(this);



    // finally, refresh the plot
    replot();
}


ScopeGadgetWidget::~ScopeGadgetWidget()
{
    if (replotTimer)
        replotTimer->stop();
    delete replotTimer;
    replotTimer = 0;

    //Get the object to de-monitor
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    foreach(QString uavObjName, m_connectedUAVObjects) {
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(uavObjName));
        disconnect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

    clearCurvePlots();
}

void ScopeGadgetWidget::clearCurvePlots()
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->curve->detach();

        delete plotData->curve;
        delete plotData;
    }

    m_curvesData.clear();
}


TestDataGen::TestDataGen()
{
    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();

    baroAltitude = BaroAltitude::GetInstance(objManager);
    gps = PositionActual::GetInstance(objManager);

    //Setup timer
    periodMs = 4;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(genTestData()));
    timer->start(periodMs);

    debugCounter = 0;
}

void TestDataGen::genTestData()
{
    // Update BaroAltitude object
    BaroAltitude::DataFields baroAltitudeData;
    baroAltitudeData.Altitude = 500 * sin(1 * testTime) + 200 * cos(4 * testTime) + 800;
    baroAltitudeData.Temperature = 30 * sin(0.5 * testTime);
    baroAltitudeData.Pressure = 100;
    baroAltitude->setData(baroAltitudeData);


    // Update gps objects
    PositionActual::DataFields gpsData;
    gpsData.Altitude = 0;
    gpsData.Heading = 0;
    gpsData.Groundspeed = 0;
    gpsData.Latitude = 0;
    gpsData.Longitude = 0;
    gpsData.Satellites = 10;
    gps->setData(gpsData);

    testTime += (periodMs / 1000.0);

//    debugCounter++;
//    if (debugCounter % (100/periodMs) == 0 )
//        qDebug() << "Test Time = " << testTime;
}

TestDataGen::~TestDataGen()
{
    if (timer)
        timer->stop();

    delete timer;
}
