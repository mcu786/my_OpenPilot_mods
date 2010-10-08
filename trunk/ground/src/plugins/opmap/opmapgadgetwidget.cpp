/**
 ******************************************************************************
 *
 * @file       opmapgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin OpenPilot Map Plugin
 * @{
 * @brief The OpenPilot Map plugin 
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

#include "opmapgadgetwidget.h"
#include "ui_opmap_widget.h"
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QClipboard>
#include <QtGui/QMenu>
#include <QStringList>
#include <QDir>
#include <QFile>

// *************************************************************************************

#define	max_digital_zoom   3	// maximum allowed digital zoom level

// *************************************************************************************

// constructor
OPMapGadgetWidget::OPMapGadgetWidget(QWidget *parent) : QWidget(parent)
{
    // **************

    m_widget = NULL;
    m_map = NULL;
    findPlaceCompleter = NULL;
    m_overlay_widget = NULL;

    m_map_graphics_scene = NULL;
    m_map_scene_proxy = NULL;
    m_zoom_slider_widget = NULL;
    m_statusbar_widget = NULL;

    m_plugin_manager = NULL;
    m_objManager = NULL;

    m_mouse_waypoint = NULL;

    prev_tile_number = 0;

    min_zoom = max_zoom = 0;

    m_map_mode = Normal_MapMode;

    // **************
    // fetch required UAVObjects

    m_plugin_manager = ExtensionSystem::PluginManager::instance();
    m_objManager = m_plugin_manager->getObject<UAVObjectManager>();

    // current position
    QPointF pos = getLatLon();
    internals::PointLatLng pos_lat_lon = internals::PointLatLng(pos.x(), pos.y());

    // **************
    // default home position

    home_position.coord = pos_lat_lon;
    home_position.altitude = 0.0;
    home_position.locked = false;

    // **************
    // default magic waypoint params

    magic_waypoint.map_wp_item = NULL;
    magic_waypoint.coord = home_position.coord;
    magic_waypoint.altitude = 0.0;
    magic_waypoint.description = "Magic waypoint";
    magic_waypoint.locked = false;
    magic_waypoint.time_seconds = 0;
    magic_waypoint.hold_time_seconds = 0;

    // **************
    // create the widget that holds the user controls and the map

    m_widget = new Ui::OPMap_Widget();
    m_widget->setupUi(this);

    // **************
    // create the central map widget

    m_map = new mapcontrol::OPMapWidget();	// create the map object

    m_map->setFrameStyle(QFrame::NoFrame);							    // no border frame
    m_map->setBackgroundBrush(Qt::black);							    // black background

    m_map->configuration->DragButton = Qt::LeftButton;		    // use the left mouse button for map dragging

//  m_map->SetMinZoom(minimum_zoom);
//  m_map->SetMaxZoom(maximum_zoom);				    // set the maximum zoom level

    m_widget->horizontalSliderZoom->setMinimum(m_map->MinZoom());			//
    m_widget->horizontalSliderZoom->setMaximum(m_map->MaxZoom() + max_digital_zoom);	//

    min_zoom = m_widget->horizontalSliderZoom->minimum();	// minimum zoom we can accept
    max_zoom = m_widget->horizontalSliderZoom->maximum();	// maximum zoom we can accept

    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::MousePositionWithoutCenter);	    // set how the mouse wheel zoom functions
    m_map->SetFollowMouse(true);				    // we want a contiuous mouse position reading

    m_map->SetShowHome(true);					    // display the HOME position on the map
    m_map->SetShowUAV(true);					    // display the UAV position on the map

    m_map->UAV->SetTrailTime(0.5);				    // seconds
    m_map->UAV->SetTrailDistance(0.01);				    // kilometers

//  m_map->UAV->SetTrailType(UAVTrailType::ByTimeElapsed);
    m_map->UAV->SetTrailType(UAVTrailType::ByDistance);

    // **************

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_map);
    m_widget->mapWidget->setLayout(layout);

    // **************
    // create the user controls overlayed onto the map

    // doing this makes the map VERY slow :(

/*
    m_zoom_slider_widget = new opmap_zoom_slider_widget();
    m_statusbar_widget = new opmap_statusbar_widget();

    m_map_graphics_scene = m_map->scene();

    m_map_scene_proxy = m_map_graphics_scene->addWidget(m_zoom_slider_widget);
    m_map_scene_proxy = m_map_graphics_scene->addWidget(m_statusbar_widget);

    m_zoom_slider_widget->move(m_map->width() - 20 - m_zoom_slider_widget->width(), 20);
    m_statusbar_widget->move(0, m_map->height() - m_statusbar_widget->height());
*/
/*
    m_overlay_widget = new opmap_overlay_widget(m_map);
    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->setSpacing(0);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(m_overlay_widget);
    m_map->setLayout(layout2);
*/
    // **************
    // set the user control options

    switch (m_map_mode)
    {
        case Normal_MapMode:
            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);
            m_widget->toolButtonCenterWaypoint->setEnabled(false);
            break;

        case MagicWaypoint_MapMode:
            m_widget->toolButtonNormalMapMode->setChecked(false);
            m_widget->toolButtonMagicWaypointMapMode->setChecked(true);
            m_widget->toolButtonCenterWaypoint->setEnabled(true);
            break;

        default:
            m_map_mode = Normal_MapMode;
            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);
            m_widget->toolButtonCenterWaypoint->setEnabled(false);
            break;
    }

    m_widget->labelUAVPos->setText("---");
    m_widget->labelMapPos->setText("---");
    m_widget->labelMousePos->setText("---");
    m_widget->labelMapZoom->setText("---");

    m_widget->splitter->setCollapsible(1, false);

    // set the size of the collapsable widgets
    QList<int> m_SizeList;
//    m_SizeList << m_widget->splitter->sizes();
    m_SizeList << 0 << 0 << 0;
    m_widget->splitter->setSizes(m_SizeList);

    m_widget->progressBarMap->setMaximum(1);

    m_widget->toolButtonShowUAVtrail->setChecked(true);

/*
    #if defined(Q_OS_MAC)
    #elif defined(Q_OS_WIN)
	m_widget->comboBoxFindPlace->clear();
	loadComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
	m_widget->comboBoxFindPlace->setCurrentIndex(-1);
    #else
    #endif
*/
    // **************
    // add an auto-completer to the find-place line edit box
/*
    findPlaceWordList << "england" << "london" << "birmingham" << "shropshire";
    QCompleter *findPlaceCompleter = new QCompleter(findPlaceWordList, this);
    findPlaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    findPlaceCompleter->setCompletionMode(QCompleter::PopupCompletion);
    findPlaceCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_widget->comboBoxFindPlace->setCompleter(findPlaceCompleter);
*/
    m_widget->comboBoxFindPlace->setAutoCompletion(true);

    connect( m_widget->comboBoxFindPlace->lineEdit(), SIGNAL(returnPressed()), this, SLOT(comboBoxFindPlace_returnPressed()));

    // **************
    // init the waypoint tree (shown on the left on the map plugin GUI)

    m_widget->treeViewWaypoints->setModel(&wayPoint_treeView_model);

/*
    // test
//    wayPoint_treeView_model = new QStandardItemModel(5, 2);
    for (int r = 0; r < 5; r++)
    {
	for (int c = 0; c < 2; c++)
	{
	    QStandardItem *item = new QStandardItem(QString("Row:%0, Column:%1").arg(r).arg(c));

	    if (c == 0)
    	    {
		for (int i = 0; i < 3; i++)
		{
		    QStandardItem *child = new QStandardItem(QString("Item %0").arg(i));
		    child->setEditable(false);
		    item->appendRow(child);
		}
	    }

	    wayPoint_treeView_model->setItem(r, c, item);
	}
    }
    wayPoint_treeView_model->setHorizontalHeaderItem(0, new QStandardItem("Foo"));
    wayPoint_treeView_model->setHorizontalHeaderItem(1, new QStandardItem("Bar-Baz"));

//    m_widget->treeViewWaypoints->setModel(wayPoint_treeView_model);
*/





    // test only
/*
    // create a waypoint group
    QStandardItem *item = new QStandardItem(tr("Camera shoot at the town hall"));
    // add some waypoints
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "North side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "East side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "South side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    {
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), "West side window view");
	child->setEditable(true);
	item->appendRow(child);
    }
    wayPoint_treeView_model.appendRow(item);
*/
    // create another waypoint group
    QStandardItem *item = new QStandardItem(tr("Flight path 62"));
    for (int i = 1; i < 8; i++)
    {   // add some waypoints
	QStandardItem *child = new QStandardItem(QIcon(QString::fromUtf8(":/opmap/images/waypoint.png")), QString("Waypoint %0").arg(i));
	child->setEditable(true);
	item->appendRow(child);
    }
    wayPoint_treeView_model.appendRow(item);










    // **************
    // map stuff

    connect(m_map, SIGNAL(zoomChanged(double, double, double)), this, SLOT(zoomChanged(double, double, double)));					// map zoom change signals
    connect(m_map, SIGNAL(OnCurrentPositionChanged(internals::PointLatLng)), this, SLOT(OnCurrentPositionChanged(internals::PointLatLng)));    // map poisition change signals
    connect(m_map, SIGNAL(OnTileLoadComplete()), this, SLOT(OnTileLoadComplete()));					// tile loading stop signals
    connect(m_map, SIGNAL(OnTileLoadStart()), this, SLOT(OnTileLoadStart()));					// tile loading start signals
    connect(m_map, SIGNAL(OnMapDrag()), this, SLOT(OnMapDrag()));							// map drag signals
    connect(m_map, SIGNAL(OnMapZoomChanged()), this, SLOT(OnMapZoomChanged()));					// map zoom changed
    connect(m_map, SIGNAL(OnMapTypeChanged(MapType::Types)), this, SLOT(OnMapTypeChanged(MapType::Types)));		// map type changed
    connect(m_map, SIGNAL(OnEmptyTileError(int, core::Point)), this, SLOT(OnEmptyTileError(int, core::Point)));	// tile error
    connect(m_map, SIGNAL(OnTilesStillToLoad(int)), this, SLOT(OnTilesStillToLoad(int)));				// tile loading signals
    connect(m_map, SIGNAL(WPNumberChanged(int const&,int const&,WayPointItem*)), this, SLOT(WPNumberChanged(int const&,int const&,WayPointItem*)));
    connect(m_map, SIGNAL(WPValuesChanged(WayPointItem*)), this, SLOT(WPValuesChanged(WayPointItem*)));
    connect(m_map, SIGNAL(WPInserted(int const&, WayPointItem*)), this, SLOT(WPInserted(int const&, WayPointItem*)));
    connect(m_map, SIGNAL(WPDeleted(int const&)), this, SLOT(WPDeleted(int const&)));

    m_map->SetCurrentPosition(home_position.coord);         // set the map position
    m_map->Home->SetCoord(home_position.coord);             // set the HOME position
    m_map->UAV->SetUAVPos(home_position.coord, 0.0);        // set the UAV position

    // **************
    // create various context menu (mouse right click menu) actions

    createActions();

    // **************
    // create the desired timers

    m_updateTimer = new QTimer();
    m_updateTimer->setInterval(200);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));
    m_updateTimer->start();

    m_statusUpdateTimer = new QTimer();
    m_statusUpdateTimer->setInterval(200);
    connect(m_statusUpdateTimer, SIGNAL(timeout()), this, SLOT(updateMousePos()));
    m_statusUpdateTimer->start();

    // **************
}

// destructor
OPMapGadgetWidget::~OPMapGadgetWidget()
{


    // this destructor doesn't appear to be called at shutdown???




//    #if defined(Q_OS_MAC)
//    #elif defined(Q_OS_WIN)
//	saveComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
//    #else
//    #endif

    m_waypoint_list_mutex.lock();
    foreach (t_waypoint *wp, m_waypoint_list)
    {
        if (!wp) continue;


        // todo:


        delete wp->map_wp_item;
    }
    m_waypoint_list_mutex.unlock();
    m_waypoint_list.clear();

    if (m_zoom_slider_widget) delete m_zoom_slider_widget;
    if (m_statusbar_widget) delete m_statusbar_widget;
    if (m_map) delete m_map;
    if (m_widget) delete m_widget;
}

// *************************************************************************************
// widget signals

void OPMapGadgetWidget::resizeEvent(QResizeEvent *event)
{
//    update();
    QWidget::resizeEvent(event);
}

void OPMapGadgetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_map)
    {
//	mouse_lat_lon = m_map->currentMousePosition();			    // fetch the current mouse lat/longitude position
//	if (mouse_lat_lon != lat_lon)
//	{	// the mouse has moved
//	    mouse_lat_lon = lat_lon;
//
//	    QString coord_str = " " + QString::number(mouse_lat_lon.Lat(), 'f', 7) + "   " + QString::number(mouse_lat_lon.Lng(), 'f', 7) + " ";
//
//	    statusLabel.setText(coord_str);
//	    widget->labelStatus->setText(coord_str);
//	}
    }

    if (event->buttons() & Qt::LeftButton)
    {
	 QPoint pos = event->pos();
    }

    QWidget::mouseMoveEvent(event);
}

void OPMapGadgetWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QString s;

    if (!m_widget || !m_map)
        return;

    if (event->reason() != QContextMenuEvent::Mouse)
        return;	// not a mouse click event

    // current mouse position
    QPoint p = m_map->mapFromGlobal(QCursor::pos());

    // save the current lat/lon mouse position
    mouse_lat_lon = m_map->currentMousePosition();

    if (!m_map->contentsRect().contains(p))
        return;					    // the mouse click was not on the map

    // find out if we have a waypoint under the mouse cursor
    QGraphicsItem *item = m_map->itemAt(p);
    m_mouse_waypoint = qgraphicsitem_cast<mapcontrol::WayPointItem *>(item);

    // find out if the waypoint is locked (or not)
    bool waypoint_locked = false;
    if (m_mouse_waypoint)
        waypoint_locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;

    // ****************
    // Dynamically create the popup menu

    QMenu menu(this);

    menu.addAction(closeAct);

    menu.addSeparator();

    menu.addAction(reloadAct);

    menu.addSeparator();

    switch (m_map_mode)
    {
        case Normal_MapMode: s = tr(" (Normal)"); break;
        case MagicWaypoint_MapMode: s = tr(" (Magic Waypoint)"); break;
        default: s = tr(" (Unknown)"); break;
    }
    for (int i = 0; i < mapModeAct.count(); i++)
    {   // set the menu to checked (or not)
        QAction *act = mapModeAct.at(i);
        if (!act) continue;
        if (act->data().toInt() == (int)m_map_mode)
            act->setChecked(true);
    }
    QMenu mapModeSubMenu(tr("Map mode") + s, this);
    for (int i = 0; i < mapModeAct.count(); i++)
        mapModeSubMenu.addAction(mapModeAct.at(i));
    menu.addMenu(&mapModeSubMenu);

    menu.addSeparator();

    QMenu copySubMenu(tr("Copy"), this);
    copySubMenu.addAction(copyMouseLatLonToClipAct);
    copySubMenu.addAction(copyMouseLatToClipAct);
    copySubMenu.addAction(copyMouseLonToClipAct);
    menu.addMenu(&copySubMenu);

    menu.addSeparator();

    menu.addAction(findPlaceAct);

    menu.addSeparator();

    menu.addAction(showCompassAct);

    menu.addSeparator()->setText(tr("Zoom"));

    menu.addAction(zoomInAct);
    menu.addAction(zoomOutAct);

    QMenu zoomSubMenu(tr("&Zoom ") + "(" + QString::number(m_map->ZoomReal()) + ")", this);
    for (int i = 0; i < zoomAct.count(); i++)
        zoomSubMenu.addAction(zoomAct.at(i));
    menu.addMenu(&zoomSubMenu);

    menu.addSeparator();

    menu.addAction(goMouseClickAct);

    menu.addSeparator()->setText(tr("HOME"));

    menu.addAction(setHomeAct);
    menu.addAction(showHomeAct);
    menu.addAction(goHomeAct);

    menu.addSeparator()->setText(tr("UAV"));

    menu.addAction(showUAVAct);
    menu.addAction(showUAVtrailAct);
    menu.addAction(followUAVpositionAct);
    menu.addAction(followUAVheadingAct);
    menu.addAction(goUAVAct);

    menu.addAction(clearUAVtrailAct);

    // *********

    switch (m_map_mode)
    {
        case Normal_MapMode:    // only show the waypoint stuff if not in 'magic waypoint' mode
            menu.addSeparator()->setText(tr("Waypoints"));

            menu.addAction(wayPointEditorAct);
            menu.addAction(addWayPointAct);

            if (m_mouse_waypoint)
            {	// we have a waypoint under the mouse
                menu.addAction(editWayPointAct);

                lockWayPointAct->setChecked(waypoint_locked);
                menu.addAction(lockWayPointAct);

                if (!waypoint_locked)
                    menu.addAction(deleteWayPointAct);
            }

            m_waypoint_list_mutex.lock();
            if (m_waypoint_list.count() > 0)
                menu.addAction(clearWayPointsAct);	// we have waypoints
            m_waypoint_list_mutex.unlock();

            break;

        case MagicWaypoint_MapMode:
            menu.addSeparator()->setText(tr("Waypoints"));
            menu.addAction(homeMagicWaypointAct);
            menu.addAction(centerMagicWaypointAct);
            break;
    }

    // *********

//    menu.addSeparator();

    menu.exec(event->globalPos());  // popup the menu

    // ****************
}

void OPMapGadgetWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) // ESC
    {
    }
    else
    if (event->key() == Qt::Key_F1) // F1
    {
    }
    else
    if (event->key() == Qt::Key_F2) // F2
    {
    }
    else
    if (event->key() == Qt::Key_Up)
    {
    }
    else
    if (event->key() == Qt::Key_Down)
    {
    }
    else
    if (event->key() == Qt::Key_Left)
    {
    }
    else
    if (event->key() == Qt::Key_Right)
    {
    }
    else
    if (event->key() == Qt::Key_PageUp)
    {
    }
    else
    if (event->key() == Qt::Key_PageDown)
    {
    }
    else
    {
	qDebug() << event->key() << endl;
    }
}

// *************************************************************************************
// timer signals

void OPMapGadgetWidget::updatePosition()
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);

    QPointF pos = getLatLon();

    internals::PointLatLng uav_pos = internals::PointLatLng(pos.x(), pos.y());	// current UAV position
    float uav_heading = 0; //data.Heading;                              		// current UAV heading
    float uav_altitude_meters = 0; //data.Altitude;                 			// current UAV height
    float uav_ground_speed_meters_per_second = 0; //data.Groundspeed;			// current UAV ground speed

    // display the UAV lat/lon position
    QString str =   "lat: " + QString::number(uav_pos.Lat(), 'f', 7) +
		    "  lon: " + QString::number(uav_pos.Lng(), 'f', 7) +
		    "  " + QString::number(uav_heading, 'f', 1) + "deg" +
		    "  " + QString::number(uav_altitude_meters, 'f', 1) + "m" +
		    "  " + QString::number(uav_ground_speed_meters_per_second, 'f', 1) + "m/s";
    m_widget->labelUAVPos->setText(str);

    m_map->UAV->SetUAVPos(uav_pos, uav_altitude_meters);			// set the maps UAV position
    m_map->UAV->SetUAVHeading(uav_heading);							// set the maps UAV heading
}

void OPMapGadgetWidget::updateMousePos()
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);

    internals::PointLatLng lat_lon = m_map->currentMousePosition();				// fetch the current lat/lon mouse position

    if (mouse_lat_lon == lat_lon)
        return; // the mouse has not moved

    // yes it has!

     mouse_lat_lon = lat_lon;

     QString str = QString::number(mouse_lat_lon.Lat(), 'f', 7) + "  " + QString::number(mouse_lat_lon.Lng(), 'f', 7);
     m_widget->labelMousePos->setText(str);
}

// *************************************************************************************
// map signals

void OPMapGadgetWidget::zoomChanged(double zoomt, double zoom, double zoomd)
{
    if (!m_widget || !m_map)
        return;

    QString s = "tot:" + QString::number(zoomt, 'f', 1) + " rea:" + QString::number(zoom, 'f', 1) + " dig:" + QString::number(zoomd, 'f', 1);
    m_widget->labelMapZoom->setText(s);

    int i_zoom = (int)(zoomt + 0.5);

    if (i_zoom < min_zoom) i_zoom = min_zoom;
    else
    if (i_zoom > max_zoom) i_zoom = max_zoom;

    if (m_widget->horizontalSliderZoom->value() != i_zoom)
	m_widget->horizontalSliderZoom->setValue(i_zoom);	// set the GUI zoom slider position

    int index0_zoom = i_zoom - min_zoom;			// zoom level starting at index level '0'
    if (index0_zoom < zoomAct.count())
	zoomAct.at(index0_zoom)->setChecked(true);		// set the right-click context menu zoom level
}

void OPMapGadgetWidget::OnMapDrag()
{
}

void OPMapGadgetWidget::OnCurrentPositionChanged(internals::PointLatLng point)
{
    if (!m_widget || !m_map)
        return;

    QString coord_str = QString::number(point.Lat(), 'f', 7) + "   " + QString::number(point.Lng(), 'f', 7) + " ";
    m_widget->labelMapPos->setText(coord_str);
}

void OPMapGadgetWidget::OnTilesStillToLoad(int number)
{
    if (!m_widget || !m_map)
        return;

//	if (prev_tile_number < number || m_widget->progressBarMap->maximum() < number)
//	    m_widget->progressBarMap->setMaximum(number);

	if (m_widget->progressBarMap->maximum() < number)
	    m_widget->progressBarMap->setMaximum(number);

	m_widget->progressBarMap->setValue(m_widget->progressBarMap->maximum() - number);	// update the progress bar

//	m_widget->labelNumTilesToLoad->setText(QString::number(number));

	prev_tile_number = number;
}

void OPMapGadgetWidget::OnTileLoadStart()
{
    if (!m_widget || !m_map)
        return;

    m_widget->progressBarMap->setVisible(true);
}

void OPMapGadgetWidget::OnTileLoadComplete()
{
    if (!m_widget || !m_map)
        return;

    m_widget->progressBarMap->setVisible(false);
}

void OPMapGadgetWidget::OnMapZoomChanged()
{
    // to do
}

void OPMapGadgetWidget::OnMapTypeChanged(MapType::Types type)
{
    Q_UNUSED(type);
    // to do
}

void OPMapGadgetWidget::OnEmptyTileError(int zoom, core::Point pos)
{
    Q_UNUSED(zoom);
    Q_UNUSED(pos);
    // to do
}

void OPMapGadgetWidget::WPNumberChanged(int const &oldnumber, int const &newnumber, WayPointItem *waypoint)
{
    Q_UNUSED(oldnumber);
    Q_UNUSED(newnumber);
    Q_UNUSED(waypoint);
    // to do
}

void OPMapGadgetWidget::WPValuesChanged(WayPointItem *waypoint)
{
    qDebug("WPValuesChanged");

    switch (m_map_mode)
    {
        case Normal_MapMode:
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {   // search for the waypoint in our own waypoint list and update it
                if (!wp) continue;
                if (!wp->map_wp_item) continue;
                if (wp->map_wp_item != waypoint) continue;
                // found the waypoint in our list
                wp->coord = waypoint->Coord();
                wp->altitude = waypoint->Altitude();
                wp->description = waypoint->Description();
                break;
            }
            m_waypoint_list_mutex.unlock();
            break;

        case MagicWaypoint_MapMode:
            // update our copy of the magic waypoint
            if (magic_waypoint.map_wp_item && magic_waypoint.map_wp_item == waypoint)
            {
                magic_waypoint.coord = waypoint->Coord();
                magic_waypoint.altitude = waypoint->Altitude();
                magic_waypoint.description = waypoint->Description();
            }
            break;
    }

}

void OPMapGadgetWidget::WPInserted(int const &number, WayPointItem *waypoint)
{
    Q_UNUSED(number);
    Q_UNUSED(waypoint);
    // to do
}

void OPMapGadgetWidget::WPDeleted(int const &number)
{
    Q_UNUSED(number);
    // to do
}

// *************************************************************************************
// user control signals

void OPMapGadgetWidget::comboBoxFindPlace_returnPressed()
{
    if (!m_widget || !m_map)
        return;

    QString place = m_widget->comboBoxFindPlace->currentText().simplified();
    if (place.isNull() || place.isEmpty()) return;

    if (!findPlaceWordList.contains(place, Qt::CaseInsensitive))
    {
	findPlaceWordList << place;	// add the new word into the history list
/*
	m_widget->comboBoxFindPlace->setCompleter(NULL);
	delete findPlaceCompleter;
	findPlaceCompleter = new QCompleter(findPlaceWordList, this);
	findPlaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	findPlaceCompleter->setCompletionMode(QCompleter::PopupCompletion);
	findPlaceCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	m_widget->comboBoxFindPlace->setCompleter(findPlaceCompleter);
*/
/*
	#if defined(Q_OS_MAC)
	#elif defined(Q_OS_WIN)
	    saveComboBoxLines(m_widget->comboBoxFindPlace, QCoreApplication::applicationDirPath() + "/opmap_find_place_history.txt");
	#else
	#endif
*/
    }

    core::GeoCoderStatusCode::Types x = m_map->SetCurrentPositionByKeywords(place);
    QString returned_text = mapcontrol::Helper::StrFromGeoCoderStatusCode(x);

    QMessageBox::information(this, tr("OpenPilot GCS"), returned_text, QMessageBox::Ok);
}

void OPMapGadgetWidget::on_toolButtonFindPlace_clicked()
{
    if (!m_widget || !m_map)
        return;

    m_widget->comboBoxFindPlace->setFocus();
    comboBoxFindPlace_returnPressed();
}

void OPMapGadgetWidget::on_toolButtonZoomP_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    zoomIn();
}

void OPMapGadgetWidget::on_toolButtonZoomM_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    zoomOut();
}

void OPMapGadgetWidget::on_toolButtonMapHome_clicked()
{
    QMutexLocker locker(&m_map_mutex);
    goHome();
}

void OPMapGadgetWidget::on_toolButtonMapUAV_clicked()
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);

    followUAVpositionAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonMapUAVheading_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    followUAVheadingAct->toggle();
}

void OPMapGadgetWidget::on_toolButtonShowUAVtrail_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    showUAVtrailAct->toggle();
}

void OPMapGadgetWidget::on_horizontalSliderZoom_sliderMoved(int position)
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);

    setZoom(position);
}

void OPMapGadgetWidget::on_toolButtonHome_clicked()
{
    if (!m_widget || !m_map)
        return;

    QMutexLocker locker(&m_map_mutex);

    // to do
}


void OPMapGadgetWidget::on_toolButtonPrevWaypoint_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    // to do
}

void OPMapGadgetWidget::on_toolButtonNextWaypoint_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    // to do
}

void OPMapGadgetWidget::on_toolButtonHoldPosition_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    // to do
}

void OPMapGadgetWidget::on_toolButtonGo_clicked()
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    // to do
}

void OPMapGadgetWidget::on_toolButtonAddWaypoint_clicked()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    QMutexLocker locker(&m_waypoint_list_mutex);
//    m_waypoint_list_mutex.lock();

	// create a waypoint at the center of the map
    t_waypoint *wp = new t_waypoint;
    wp->map_wp_item = NULL;
    wp->coord = m_map->CurrentPosition();
    wp->altitude = 0;
    wp->description = "";
    wp->locked = false;
    wp->time_seconds = 0;
    wp->hold_time_seconds = 0;
    wp->map_wp_item = m_map->WPCreate(wp->coord, wp->altitude, wp->description);

    wp->map_wp_item->setFlag(QGraphicsItem::ItemIsMovable, !wp->locked);

    if (wp->map_wp_item)
    {
        if (!wp->locked)
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
        else
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
        wp->map_wp_item->update();
    }

	// and remember it in our own local waypoint list
    m_waypoint_list.append(wp);

//    m_waypoint_list_mutex.unlock();
}

void OPMapGadgetWidget::on_treeViewWaypoints_clicked(QModelIndex index)
{
    if (!m_widget || !m_map)
        return;

//    QMutexLocker locker(&m_map_mutex);

    QStandardItem *item = wayPoint_treeView_model.itemFromIndex(index);
    if (!item) return;

    // to do
}

void OPMapGadgetWidget::on_toolButtonNormalMapMode_clicked()
{
    setMapMode(Normal_MapMode);
}

void OPMapGadgetWidget::on_toolButtonMagicWaypointMapMode_clicked()
{
    setMapMode(MagicWaypoint_MapMode);
}

void OPMapGadgetWidget::on_toolButtonHomeWaypoint_clicked()
{
    homeMagicWaypoint();
}

void OPMapGadgetWidget::on_toolButtonCenterWaypoint_clicked()
{
    centerMagicWaypoint();
}

// *************************************************************************************
// public functions

void OPMapGadgetWidget::setHome(QPointF pos)
{
    if (!m_widget || !m_map)
        return;

    setHome(internals::PointLatLng(pos.x(), pos.y()));
}

void OPMapGadgetWidget::setHome(internals::PointLatLng pos_lat_lon)
{
    if (!m_widget || !m_map)
        return;

    home_position.coord = pos_lat_lon;

    m_map->Home->SetCoord(home_position.coord);
    m_map->Home->RefreshPos();
}

void OPMapGadgetWidget::goHome()
{
    if (!m_widget || !m_map)
        return;

    followUAVpositionAct->setChecked(false);

    internals::PointLatLng home_pos = home_position.coord;	// get the home location
    m_map->SetCurrentPosition(home_pos);                    // center the map onto the home location
}


void OPMapGadgetWidget::zoomIn()
{
    if (!m_widget || !m_map)
        return;

    int zoom = m_map->ZoomTotal() + 1;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::zoomOut()
{
    if (!m_widget || !m_map)
        return;

    int zoom = m_map->ZoomTotal() - 1;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    m_map->SetZoom(zoom);
}

void OPMapGadgetWidget::setZoom(int zoom)
{
    if (!m_widget || !m_map)
        return;

    if (zoom < min_zoom) zoom = min_zoom;
    else
    if (zoom > max_zoom) zoom = max_zoom;

    internals::MouseWheelZoomType::Types zoom_type = m_map->GetMouseWheelZoomType();
    m_map->SetMouseWheelZoomType(internals::MouseWheelZoomType::ViewCenter);

    m_map->SetZoom(zoom);

    m_map->SetMouseWheelZoomType(zoom_type);
}

void OPMapGadgetWidget::setPosition(QPointF pos)
{
    if (!m_widget || !m_map)
        return;

    m_map->SetCurrentPosition(internals::PointLatLng(pos.y(), pos.x()));
}

void OPMapGadgetWidget::setMapProvider(QString provider)
{
    if (!m_widget || !m_map)
        return;

    m_map->SetMapType(mapcontrol::Helper::MapTypeFromString(provider));
}

void OPMapGadgetWidget::setAccessMode(QString accessMode)
{
    if (!m_widget || !m_map)
        return;

    m_map->configuration->SetAccessMode(mapcontrol::Helper::AccessModeFromString(accessMode));
}

void OPMapGadgetWidget::setUseOpenGL(bool useOpenGL)
{
    if (!m_widget || !m_map)
        return;

    m_map->SetUseOpenGL(useOpenGL);
}

void OPMapGadgetWidget::setShowTileGridLines(bool showTileGridLines)
{
    if (!m_widget || !m_map)
        return;

    m_map->SetShowTileGridLines(showTileGridLines);
}

void OPMapGadgetWidget::setUseMemoryCache(bool useMemoryCache)
{
    if (!m_widget || !m_map)
        return;

    m_map->configuration->SetUseMemoryCache(useMemoryCache);
}

void OPMapGadgetWidget::setCacheLocation(QString cacheLocation)
{
    if (!m_widget || !m_map)
        return;

    cacheLocation = cacheLocation.simplified();	// remove any surrounding spaces

    if (cacheLocation.isEmpty()) return;

//    #if defined(Q_WS_WIN)
//	if (!cacheLocation.endsWith('\\')) cacheLocation += '\\';
//    #elif defined(Q_WS_X11)
	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
//    #elif defined(Q_WS_MAC)
//	if (!cacheLocation.endsWith(QDir::separator())) cacheLocation += QDir::separator();
//    #endif

    QDir dir;
    if (!dir.exists(cacheLocation))
        if (!dir.mkpath(cacheLocation))
            return;

//    qDebug() << "map cache dir: " << cacheLocation;

    m_map->configuration->SetCacheLocation(cacheLocation);
}

void OPMapGadgetWidget::setMapMode(opMapModeType mode)
{
    if (!m_widget || !m_map)
        return;

    if (mode != Normal_MapMode && mode != MagicWaypoint_MapMode)
        mode = Normal_MapMode;  // fix error

    if (m_map_mode == mode)
        return;     // no change in map mode

    switch (mode)
    {
        case Normal_MapMode:
            m_map_mode = Normal_MapMode;

            m_widget->toolButtonMagicWaypointMapMode->setChecked(false);
            m_widget->toolButtonNormalMapMode->setChecked(true);

            m_widget->toolButtonCenterWaypoint->setEnabled(false);

            // delete the magic waypoint from the map
            if (magic_waypoint.map_wp_item)
            {
                magic_waypoint.coord = magic_waypoint.map_wp_item->Coord();
                magic_waypoint.altitude = magic_waypoint.map_wp_item->Altitude();
                magic_waypoint.description = magic_waypoint.map_wp_item->Description();
                magic_waypoint.map_wp_item = NULL;
            }
            m_map->WPDeleteAll();

            // restore the normal waypoints on the map
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {
                if (!wp) continue;
                wp->map_wp_item = m_map->WPCreate(wp->coord, wp->altitude, wp->description);
                if (!wp->map_wp_item) continue;
                wp->map_wp_item->setFlag(QGraphicsItem::ItemIsMovable, !wp->locked);
                if (!wp->locked)
                    wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
                else
                    wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
                wp->map_wp_item->update();
            }
            m_waypoint_list_mutex.unlock();

            break;

        case MagicWaypoint_MapMode:
            m_map_mode = MagicWaypoint_MapMode;

            m_widget->toolButtonNormalMapMode->setChecked(false);
            m_widget->toolButtonMagicWaypointMapMode->setChecked(true);

            m_widget->toolButtonCenterWaypoint->setEnabled(true);

            // delete the normal waypoints from the map
            m_waypoint_list_mutex.lock();
            foreach (t_waypoint *wp, m_waypoint_list)
            {
                if (!wp) continue;
                if (!wp->map_wp_item) continue;
                wp->coord = wp->map_wp_item->Coord();
                wp->altitude = wp->map_wp_item->Altitude();
                wp->description = wp->map_wp_item->Description();
                wp->locked = (wp->map_wp_item->flags() & QGraphicsItem::ItemIsMovable) == 0;
                wp->map_wp_item = NULL;
            }
            m_map->WPDeleteAll();
            m_waypoint_list_mutex.unlock();

            // restore the magic waypoint on the map
            magic_waypoint.map_wp_item = m_map->WPCreate(magic_waypoint.coord, magic_waypoint.altitude, magic_waypoint.description);
            magic_waypoint.map_wp_item->SetShowNumber(false);
            magic_waypoint.map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker3.png"));

            break;
    }
}

// *************************************************************************************
// Context menu stuff

void OPMapGadgetWidget::createActions()
{
    if (!m_widget)
	return;

    // ***********************
    // create menu actions

    closeAct = new QAction(tr("&Close menu"), this);
//    closeAct->setShortcuts(QKeySequence::New);
    closeAct->setStatusTip(tr("Close the context menu"));

    reloadAct = new QAction(tr("&Reload map"), this);
    reloadAct->setShortcut(tr("F5"));
    reloadAct->setStatusTip(tr("Reload the map tiles"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(onReloadAct_triggered()));

    copyMouseLatLonToClipAct = new QAction(tr("Mouse latitude and longitude"), this);
    copyMouseLatLonToClipAct->setStatusTip(tr("Copy the mouse latitude and longitude to the clipboard"));
    connect(copyMouseLatLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatLonToClipAct_triggered()));

    copyMouseLatToClipAct = new QAction(tr("Mouse latitude"), this);
    copyMouseLatToClipAct->setStatusTip(tr("Copy the mouse latitude to the clipboard"));
    connect(copyMouseLatToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLatToClipAct_triggered()));

    copyMouseLonToClipAct = new QAction(tr("Mouse longitude"), this);
    copyMouseLonToClipAct->setStatusTip(tr("Copy the mouse longitude to the clipboard"));
    connect(copyMouseLonToClipAct, SIGNAL(triggered()), this, SLOT(onCopyMouseLonToClipAct_triggered()));

    findPlaceAct = new QAction(tr("&Find place"), this);
    findPlaceAct->setShortcut(tr("Ctrl+F"));
    findPlaceAct->setStatusTip(tr("Find a location"));
    connect(findPlaceAct, SIGNAL(triggered()), this, SLOT(onFindPlaceAct_triggered()));

    showCompassAct = new QAction(tr("Show compass"), this);
    showCompassAct->setStatusTip(tr("Show/Hide the compass"));
    showCompassAct->setCheckable(true);
    showCompassAct->setChecked(true);
    connect(showCompassAct, SIGNAL(toggled(bool)), this, SLOT(onShowCompassAct_toggled(bool)));

    showHomeAct = new QAction(tr("Show Home"), this);
    showHomeAct->setStatusTip(tr("Show/Hide the Home location"));
    showHomeAct->setCheckable(true);
    showHomeAct->setChecked(true);
    connect(showHomeAct, SIGNAL(toggled(bool)), this, SLOT(onShowHomeAct_toggled(bool)));

    showUAVAct = new QAction(tr("Show UAV"), this);
    showUAVAct->setStatusTip(tr("Show/Hide the UAV"));
    showUAVAct->setCheckable(true);
    showUAVAct->setChecked(true);
    connect(showUAVAct, SIGNAL(toggled(bool)), this, SLOT(onShowUAVAct_toggled(bool)));

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(Qt::Key_PageUp);
    zoomInAct->setStatusTip(tr("Zoom the map in"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(onGoZoomInAct_triggered()));

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(Qt::Key_PageDown);
    zoomOutAct->setStatusTip(tr("Zoom the map out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(onGoZoomOutAct_triggered()));

    goMouseClickAct = new QAction(tr("Go to where you right clicked the mouse"), this);
    goMouseClickAct->setStatusTip(tr("Center the map onto where you right clicked the mouse"));
    connect(goMouseClickAct, SIGNAL(triggered()), this, SLOT(onGoMouseClickAct_triggered()));

    setHomeAct = new QAction(tr("Set the home location"), this);
    setHomeAct->setStatusTip(tr("Set the home location to where you clicked"));
    connect(setHomeAct, SIGNAL(triggered()), this, SLOT(onSetHomeAct_triggered()));

    goHomeAct = new QAction(tr("Go to &Home location"), this);
    goHomeAct->setShortcut(tr("Ctrl+H"));
    goHomeAct->setStatusTip(tr("Center the map onto the home location"));
    connect(goHomeAct, SIGNAL(triggered()), this, SLOT(onGoHomeAct_triggered()));

    goUAVAct = new QAction(tr("Go to &UAV location"), this);
    goUAVAct->setShortcut(tr("Ctrl+U"));
    goUAVAct->setStatusTip(tr("Center the map onto the UAV location"));
    connect(goUAVAct, SIGNAL(triggered()), this, SLOT(onGoUAVAct_triggered()));

    followUAVpositionAct = new QAction(tr("Follow UAV position"), this);
    followUAVpositionAct->setShortcut(tr("Ctrl+F"));
    followUAVpositionAct->setStatusTip(tr("Keep the map centered onto the UAV"));
    followUAVpositionAct->setCheckable(true);
    followUAVpositionAct->setChecked(false);
    connect(followUAVpositionAct, SIGNAL(toggled(bool)), this, SLOT(onFollowUAVpositionAct_toggled(bool)));

    followUAVheadingAct = new QAction(tr("Follow UAV heading"), this);
    followUAVheadingAct->setShortcut(tr("Ctrl+F"));
    followUAVheadingAct->setStatusTip(tr("Keep the map rotation to the UAV heading"));
    followUAVheadingAct->setCheckable(true);
    followUAVheadingAct->setChecked(false);
    connect(followUAVheadingAct, SIGNAL(toggled(bool)), this, SLOT(onFollowUAVheadingAct_toggled(bool)));

    showUAVtrailAct = new QAction(tr("Show UAV trail"), this);
    showUAVtrailAct->setStatusTip(tr("Show/Hide the UAV trail"));
    showUAVtrailAct->setCheckable(true);
    showUAVtrailAct->setChecked(true);
    connect(showUAVtrailAct, SIGNAL(toggled(bool)), this, SLOT(onShowUAVtrailAct_toggled(bool)));

    clearUAVtrailAct = new QAction(tr("Clear UAV trail"), this);
    clearUAVtrailAct->setStatusTip(tr("Clear the UAV trail"));
    connect(clearUAVtrailAct, SIGNAL(triggered()), this, SLOT(onClearUAVtrailAct_triggered()));

    wayPointEditorAct = new QAction(tr("&Waypoint editor"), this);
    wayPointEditorAct->setShortcut(tr("Ctrl+W"));
    wayPointEditorAct->setStatusTip(tr("Open the waypoint editor"));
    wayPointEditorAct->setEnabled(false);   // temporary
    connect(wayPointEditorAct, SIGNAL(triggered()), this, SLOT(onOpenWayPointEditorAct_triggered()));

    addWayPointAct = new QAction(tr("&Add waypoint"), this);
    addWayPointAct->setShortcut(tr("Ctrl+A"));
    addWayPointAct->setStatusTip(tr("Add waypoint"));
    connect(addWayPointAct, SIGNAL(triggered()), this, SLOT(onAddWayPointAct_triggered()));

    editWayPointAct = new QAction(tr("&Edit waypoint"), this);
    editWayPointAct->setShortcut(tr("Ctrl+E"));
    editWayPointAct->setStatusTip(tr("Edit waypoint"));
    connect(editWayPointAct, SIGNAL(triggered()), this, SLOT(onEditWayPointAct_triggered()));

    lockWayPointAct = new QAction(tr("&Lock waypoint"), this);
    lockWayPointAct->setStatusTip(tr("Lock/Unlock a waypoint"));
    lockWayPointAct->setCheckable(true);
    lockWayPointAct->setChecked(false);
    connect(lockWayPointAct, SIGNAL(triggered()), this, SLOT(onLockWayPointAct_triggered()));

    deleteWayPointAct = new QAction(tr("&Delete waypoint"), this);
    deleteWayPointAct->setShortcut(tr("Ctrl+D"));
    deleteWayPointAct->setStatusTip(tr("Delete waypoint"));
    connect(deleteWayPointAct, SIGNAL(triggered()), this, SLOT(onDeleteWayPointAct_triggered()));

    clearWayPointsAct = new QAction(tr("&Clear waypoints"), this);
    clearWayPointsAct->setShortcut(tr("Ctrl+C"));
    clearWayPointsAct->setStatusTip(tr("Clear waypoints"));
    connect(clearWayPointsAct, SIGNAL(triggered()), this, SLOT(onClearWayPointsAct_triggered()));

    homeMagicWaypointAct = new QAction(tr("Home magic waypoint"), this);
    homeMagicWaypointAct->setStatusTip(tr("Move the magic waypoint to the home position"));
    connect(homeMagicWaypointAct, SIGNAL(triggered()), this, SLOT(onHomeMagicWaypointAct_triggered()));

    centerMagicWaypointAct = new QAction(tr("Center magic waypoint"), this);
    centerMagicWaypointAct->setStatusTip(tr("Move the magic waypoint to the center of the map"));
    connect(centerMagicWaypointAct, SIGNAL(triggered()), this, SLOT(onCenterMagicWaypointAct_triggered()));

    mapModeActGroup = new QActionGroup(this);
    connect(mapModeActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onMapModeActGroup_triggered(QAction *)));
    mapModeAct.clear();
    {
        QAction *map_mode_act;

        map_mode_act = new QAction(tr("Normal"), mapModeActGroup);
        map_mode_act->setCheckable(true);
        map_mode_act->setChecked(m_map_mode == Normal_MapMode);
        map_mode_act->setData((int)Normal_MapMode);
        mapModeAct.append(map_mode_act);

        map_mode_act = new QAction(tr("Magic Waypoint"), mapModeActGroup);
        map_mode_act->setCheckable(true);
        map_mode_act->setChecked(m_map_mode == MagicWaypoint_MapMode);
        map_mode_act->setData((int)MagicWaypoint_MapMode);
        mapModeAct.append(map_mode_act);
    }

    zoomActGroup = new QActionGroup(this);
    connect(zoomActGroup, SIGNAL(triggered(QAction *)), this, SLOT(onZoomActGroup_triggered(QAction *)));
    zoomAct.clear();
    for (int i = min_zoom; i <= max_zoom; i++)
    {
        QAction *zoom_act = new QAction(QString::number(i), zoomActGroup);
        zoom_act->setCheckable(true);
        zoom_act->setData(i);
        zoomAct.append(zoom_act);
    }

    // ***********************
}

void OPMapGadgetWidget::onReloadAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    m_map->ReloadMap();
}

void OPMapGadgetWidget::onCopyMouseLatLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lat(), 'f', 7) + ", " + QString::number(mouse_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLatToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lat(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onCopyMouseLonToClipAct_triggered()
{
//    QClipboard *clipboard = qApp->clipboard();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(mouse_lat_lon.Lng(), 'f', 7), QClipboard::Clipboard);
}

void OPMapGadgetWidget::onFindPlaceAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    m_widget->comboBoxFindPlace->setFocus();	// move focus to the 'find place' text box

/*
    bool ok;
    QString place = QInputDialog::getText(this, tr("OpenPilot GCS"), tr("Find place"), QLineEdit::Normal, QString::null, &ok);
    place = place.simplified();
    if (!ok || place.isNull() || place.isEmpty()) return;

    if (!findPlaceWordList.contains(place, Qt::CaseInsensitive))
    {
	findPlaceWordList += place;	// add the new word into the history list
    }

    if (!m_map) return;

    core::GeoCoderStatusCode::Types x = m_map->SetCurrentPositionByKeywords(place);
    QString returned_text = mapcontrol::Helper::StrFromGeoCoderStatusCode(x);

    QMessageBox::information(this, tr("OpenPilot GCS"), returned_text, QMessageBox::Ok);
*/
}

void OPMapGadgetWidget::onShowCompassAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->SetShowCompass(show);
}

void OPMapGadgetWidget::onShowHomeAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->Home->setVisible(show);
}

void OPMapGadgetWidget::onShowUAVAct_toggled(bool show)
{
    if (!m_widget || !m_map)
        return;

    m_map->UAV->setVisible(show);
}

void OPMapGadgetWidget::onMapModeActGroup_triggered(QAction *action)
{
    if (!m_widget || !m_map || !action)
        return;

    opMapModeType mode = (opMapModeType)action->data().toInt();

    setMapMode(mode);
}

void OPMapGadgetWidget::onGoZoomInAct_triggered()
{
    zoomIn();
}

void OPMapGadgetWidget::onGoZoomOutAct_triggered()
{
    zoomOut();
}

void OPMapGadgetWidget::onZoomActGroup_triggered(QAction *action)
{
    if (!m_widget || !action)
        return;

    setZoom(action->data().toInt());
}

void OPMapGadgetWidget::onGoMouseClickAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    m_map->SetCurrentPosition(m_map->currentMousePosition());   // center the map onto the mouse position
}

void OPMapGadgetWidget::onSetHomeAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    setHome(mouse_lat_lon);
}

void OPMapGadgetWidget::onGoHomeAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    goHome();
}

void OPMapGadgetWidget::onGoUAVAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    QPointF pos = getLatLon();

    internals::PointLatLng uav_pos = internals::PointLatLng(pos.x(), pos.y());	// current UAV position
    internals::PointLatLng map_pos = m_map->CurrentPosition();				// current MAP position
    if (map_pos != uav_pos) m_map->SetCurrentPosition(uav_pos);				// center the map onto the UAV
}

void OPMapGadgetWidget::onFollowUAVpositionAct_toggled(bool checked)
{
    if (!m_widget || !m_map)
        return;

    if (m_widget->toolButtonMapUAV->isChecked() != checked)
        m_widget->toolButtonMapUAV->setChecked(checked);

    setMapFollowingMode();
}

void OPMapGadgetWidget::onFollowUAVheadingAct_toggled(bool checked)
{
    if (!m_widget || !m_map)
        return;

    if (m_widget->toolButtonMapUAVheading->isChecked() != checked)
        m_widget->toolButtonMapUAVheading->setChecked(checked);

    setMapFollowingMode();
}

void OPMapGadgetWidget::onShowUAVtrailAct_toggled(bool checked)
{
    if (!m_widget || !m_map)
        return;

    if (m_widget->toolButtonShowUAVtrail->isChecked() != checked)
        m_widget->toolButtonShowUAVtrail->setChecked(checked);

    m_map->UAV->SetShowTrail(checked);
}

void OPMapGadgetWidget::onClearUAVtrailAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    m_map->UAV->DeleteTrail();
}

void OPMapGadgetWidget::onOpenWayPointEditorAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    waypoint_editor_dialog.show();
}

void OPMapGadgetWidget::onAddWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    m_waypoint_list_mutex.lock();

	// create a waypoint on the map at the last known mouse position
    t_waypoint *wp = new t_waypoint;
    wp->map_wp_item = NULL;
    wp->coord = mouse_lat_lon;
    wp->altitude = 0;
    wp->description = "";
    wp->locked = false;
    wp->time_seconds = 0;
    wp->hold_time_seconds = 0;
    wp->map_wp_item = m_map->WPCreate(wp->coord, wp->altitude, wp->description);

    wp->map_wp_item->setFlag(QGraphicsItem::ItemIsMovable, !wp->locked);

    if (wp->map_wp_item)
    {
        if (!wp->locked)
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
        else
            wp->map_wp_item->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
        wp->map_wp_item->update();
    }

    // and remember it in our own local waypoint list
    m_waypoint_list.append(wp);

    m_waypoint_list_mutex.unlock();
}

void OPMapGadgetWidget::onEditWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    waypoint_edit_dialog.editWaypoint(m_mouse_waypoint);

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onLockWayPointAct_triggered()
{
    if (!m_widget || !m_map || !m_mouse_waypoint)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;
    m_mouse_waypoint->setFlag(QGraphicsItem::ItemIsMovable, locked);

    if (!locked)
        m_mouse_waypoint->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker2.png"));
    else
        m_mouse_waypoint->picture.load(QString::fromUtf8(":/opmap/images/waypoint_marker1.png"));
    m_mouse_waypoint->update();

    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onDeleteWayPointAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    if (!m_mouse_waypoint)
        return;

    bool locked = (m_mouse_waypoint->flags() & QGraphicsItem::ItemIsMovable) == 0;

    if (locked) return;	// waypoint is locked

    QMutexLocker locker(&m_waypoint_list_mutex);

    for (int i = 0; i < m_waypoint_list.count(); i++)
    {
        t_waypoint *wp = m_waypoint_list.at(i);
        if (!wp) continue;
        if (!wp->map_wp_item || wp->map_wp_item != m_mouse_waypoint) continue;

        // delete the waypoint from the map
        m_map->WPDelete(wp->map_wp_item);

        // delete the waypoint from our local waypoint list
        m_waypoint_list.removeAt(i);

        delete wp;

        break;
    }
/*
    foreach (t_waypoint *wp, m_waypoint_list)
    {
        if (!wp) continue;
        if (!wp->map_wp_item || wp->map_wp_item != m_mouse_waypoint) continue;

	    // delete the waypoint from the map
        m_map->WPDelete(wp->map_wp_item);

	    // delete the waypoint from our local waypoint list
        m_waypoint_list.removeOne(wp);

        delete wp;

	    break;
	}
*/
    m_mouse_waypoint = NULL;
}

void OPMapGadgetWidget::onClearWayPointsAct_triggered()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != Normal_MapMode)
        return;

    QMutexLocker locker(&m_waypoint_list_mutex);

	m_map->WPDeleteAll();

    foreach (t_waypoint *wp, m_waypoint_list)
    {
        if (wp)
        {
            delete wp;
            wp = NULL;
        }
    }

    m_waypoint_list.clear();
}

void OPMapGadgetWidget::onHomeMagicWaypointAct_triggered()
{
    // center the magic waypoint on the home position
    homeMagicWaypoint();
}

void OPMapGadgetWidget::onCenterMagicWaypointAct_triggered()
{
    // center the magic waypoint on the map
    centerMagicWaypoint();
}

// *************************************************************************************
// move the magic waypoint to the home position

void OPMapGadgetWidget::homeMagicWaypoint()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != MagicWaypoint_MapMode)
        return;

    magic_waypoint.coord = home_position.coord;

    if (magic_waypoint.map_wp_item)
        magic_waypoint.map_wp_item->SetCoord(magic_waypoint.coord);
}

// *************************************************************************************
// move the magic waypoint to the center of the map

void OPMapGadgetWidget::centerMagicWaypoint()
{
    if (!m_widget || !m_map)
        return;

    if (m_map_mode != MagicWaypoint_MapMode)
        return;

    magic_waypoint.coord = m_map->CurrentPosition();

    if (magic_waypoint.map_wp_item)
        magic_waypoint.map_wp_item->SetCoord(magic_waypoint.coord);
}

// *************************************************************************************
// temporary until an object is created for managing the save/restore

// load the contents of a simple text file into a combobox
void OPMapGadgetWidget::loadComboBoxLines(QComboBox *comboBox, QString filename)
{
    if (!comboBox) return;
    if (filename.isNull() || filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();
        if (line.isNull() || line.isEmpty()) continue;
        comboBox->addItem(line);
    }

    file.close();
}

// save a combobox text contents to a simple text file
void OPMapGadgetWidget::saveComboBoxLines(QComboBox *comboBox, QString filename)
{
    if (!comboBox) return;
    if (filename.isNull() || filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);

    for (int i = 0; i < comboBox->count(); i++)
    {
        QString line = comboBox->itemText(i).simplified();
        if (line.isNull() || line.isEmpty()) continue;
        out << line << "\n";
    }

    file.close();
}

// *************************************************************************************

QPointF OPMapGadgetWidget::getLatLon()
{
    double BaseECEF[3];
    double NED[3];
    double LLA[3];
    UAVObject *obj;

    obj = dynamic_cast<UAVDataObject*>(m_objManager->getObject(QString("HomeLocation")));
    BaseECEF[0] = obj->getField(QString("ECEF"))->getDouble(0) / 100;
    BaseECEF[1] = obj->getField(QString("ECEF"))->getDouble(1) / 100;
    BaseECEF[2] = obj->getField(QString("ECEF"))->getDouble(2) / 100;

    obj = dynamic_cast<UAVDataObject*>(m_objManager->getObject(QString("PositionActual")));
    NED[0] = obj->getField(QString("North"))->getDouble() / 100;
    NED[1] = obj->getField(QString("East"))->getDouble() / 100;
    NED[2] = obj->getField(QString("Down"))->getDouble() / 100;

    Utils::CoordinateConversions().GetLLA(BaseECEF, NED, LLA);
    return QPointF(LLA[0],LLA[1]);
}

// *************************************************************************************

void OPMapGadgetWidget::setMapFollowingMode()
{
    if (!m_widget || !m_map)
        return;

    if (!followUAVpositionAct->isChecked())
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::None);
        m_map->SetRotate(0);								// reset map rotation to 0deg
    }
    else
    if (!followUAVheadingAct->isChecked())
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterMap);
        m_map->SetRotate(0);								// reset map rotation to 0deg
    }
    else
    {
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterMap);      // the map library won't let you reset the uav rotation if it's already in rotate mode

        m_map->UAV->SetUAVHeading(0);                                   // reset the UAV heading to 0deg
        m_map->UAV->SetMapFollowType(UAVMapFollowType::CenterAndRotateMap);
    }
}

// *************************************************************************************
