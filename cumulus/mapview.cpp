/***************************************************************************
  mapview.cpp

  This file is part of Cumulus.

  begin                : Sun Jul 21 2002

  copyright            : (C) 2002      by Andre Somers
                             2008      by Josua Dietze
                             2008-2014 by Axel Pauli <kflog.cumulus@gmail.com>

  $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "altimeterdialog.h"
#include "filetools.h"
#include "generalconfig.h"
#include "gliderflightdialog.h"
#include "gpsnmea.h"
#include "gpsstatusdialog.h"
#include "igclogger.h"
#include "interfaceelements.h"
#include "layout.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "map.h"
#include "mapinfobox.h"
#include "mapmatrix.h"
#include "mapview.h"
#include "preflightwindpage.h"
#include "speed.h"
#include "variomodedialog.h"
#include "waypointcatalog.h"
#include "waypoint.h"

MapView::MapView(QWidget *parent) : QWidget(parent)
{
  setObjectName("MapView");
  setContentsMargins(-9, -9, -9, -9);

  if( parent )
    {
      resize( parent->size() );
    }

  // Later on the Pretext can change depending on Mode
  GeneralConfig *conf = GeneralConfig::instance();
  _altimeterMode = conf->getAltimeterMode();

  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap( "arrows60pix-15.png" );
  //make the main box layout
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing(0);

  //@JD: the new "sidebar" layout
  QBoxLayout *centerLayout = new QHBoxLayout;
  topLayout->addLayout(centerLayout);
  topLayout->setStretchFactor( centerLayout, 1 );
  centerLayout->setSpacing(0);

  QBoxLayout *sideLayout = new QVBoxLayout;
  sideLayout->setSpacing(0);

  _sidebarWidget = new QWidget(this);
  _sidebarWidget->setContentsMargins(-9,-9,-6,-9);
  _sidebarWidget->setLayout(sideLayout);
  centerLayout->addWidget(_sidebarWidget);

  // three 'grouping' widgets with slightly differing
  // background color and fixed width (to avoid frequent resizing)

  // This fixed size is based on a screen resolution of 800x480. Android devices
  // like the Galaxy III have a screen resolution of 1280x720. To handle that
  // in a better way, a second fixed size limit is introduced.
  int leftFixedWidth = 220;

  if( parent->size().width() > 1000 )
    {
      // For higher resolution screens
      leftFixedWidth = 280;
    }

  // If we have screens with a larger resolution, the heught of the info boxes
  // are increased. The default height is 60 pixels. For higher resolution we
  // should use 70 pixels.
  int textLabelBoxHeight = 60;

  if( parent->size().height() > 600 )
    {
      // For higher resolution screens
      textLabelBoxHeight = 70;
    }

  // widget to group waypoint functions
  QWidget *wayBar = new QWidget( this );
  wayBar->setFixedWidth(leftFixedWidth);
  wayBar->setContentsMargins(-9,-9,-9,-6);
  wayBar->setAutoFillBackground(true);
  wayBar->setBackgroundRole(QPalette::Window);
  wayBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  // vertical layout for waypoint widgets
  QVBoxLayout *wayLayout = new QVBoxLayout( wayBar );
  wayLayout->setSpacing(2);

  //add Waypoint widget (whole line)
  _waypoint = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _waypoint->setPreText("To");
  _waypoint->setValue("-");
  _waypoint->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  wayLayout->addWidget( _waypoint, 1 );
  connect( _waypoint, SIGNAL(mouseShortPress()),
           MainWindow::mainWindow(), SLOT(slotSwitchToWPListViewExt()));

  //layout for Glide Path and Relative Bearing
  QHBoxLayout *GRLayout = new QHBoxLayout;
  wayLayout->addLayout(GRLayout);
  GRLayout->setSpacing(2);

  //add Glide Path widget
  _glidepath = new MapInfoBox( this, conf->getMapFrameColor().name(), true, false, 40 );
  _glidepathBGColor = wayBar->palette().color(QPalette::Window);
  _glidepath->setValue("-");
  _glidepath->setPreText("Arr");
  _glidepath->setPreUnit( Altitude::getUnitText() );
  _glidepath->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  GRLayout->addWidget( _glidepath );

  connect( _glidepath, SIGNAL(mouseShortPress()),
           MainWindow::mainWindow(), SLOT(slotSwitchToReachListView()));

  // add Relative Bearing widget
  QPixmap arrow = _arrows.copy( 24*60+3, 3, 54, 54 );
  _rel_bearing = new MapInfoBox( this, conf->getMapFrameColor().name(), arrow );
  _rel_bearing->setFixedSize(textLabelBoxHeight, textLabelBoxHeight);
  _rel_bearing->setToolTip( tr("Click here to save current position as waypoint") );
  GRLayout->addWidget(_rel_bearing);

  connect(_rel_bearing, SIGNAL(mouseShortPress()),
          MainWindow::mainWindow(), SLOT(slotRememberWaypoint()) );

  //layout for Distance/ETA and Bearing
  QBoxLayout *DEBLayout = new QHBoxLayout;
  wayLayout->addLayout(DEBLayout, 1);
  DEBLayout->setSpacing(2);

  //add Distance widget
  _distance = new MapInfoBox( this, conf->getMapFrameColor().name(), true );
  _distance->setPreText("Dis");
  _distance->setValue("-");
  _distance->setPreUnit( Distance::getUnitText() );
  _distance->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  _distance->setUpdateInterval( 750 );
  DEBLayout->addWidget( _distance );
  connect(_distance, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleDistanceEta()));

  //add ETA widget
  _eta = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _eta->setVisible(false);
  _eta->setPreText( "Eta" );
  _eta->setValue("-");
  _eta->setUpdateInterval( 750 );
  _eta->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  DEBLayout->addWidget( _eta );
  connect(_eta, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleDistanceEta()));

  //add Bearing widget
  _bearingMode = 1; // normal bearing display is default
  _lastBearing = -1;
  _bearingTimer = new QTimer(this);
  connect (_bearingTimer, SIGNAL(timeout()),
           this, SLOT(slot_resetInversBearing()));
  _bearing = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _bearingBGColor = wayBar->palette().color(QPalette::Window);
  _bearing->setValue("-");
  _bearing->setPreText("Brg");
  _bearing->setUpdateInterval( 750 );
  _bearing->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  DEBLayout->addWidget( _bearing);
  connect(_bearing, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleBearing()));

  sideLayout->addWidget( wayBar, 3 );

  // widget to group common displays
  QWidget *commonBar = new QWidget( this );
  commonBar->setFixedWidth(leftFixedWidth);
  commonBar->setContentsMargins( -9, -6, -9, -6);
  commonBar->setAutoFillBackground(true);
  commonBar->setBackgroundRole(QPalette::Window);
  commonBar->setPalette( QPalette(QColor(230, 230, 230)) );

  // vertical layout for common display widgets
  QBoxLayout *commonLayout = new QVBoxLayout( commonBar );
  commonLayout->setSpacing(2);

  //layout for Speed and Heading
  QBoxLayout *SHLayout = new QHBoxLayout;
  commonLayout->addLayout(SHLayout);
  SHLayout->setSpacing(2);

  //add Speed widget
  _speed = new MapInfoBox( this, "#c0c0c0" );
  //_speed = new MapInfoBox( this, QColor(Qt::darkGray).name() );

  _speed->setPreText("Gs");
  _speed->setValue("-");
  _speed->setUpdateInterval( 750 );
  _speed->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  SHLayout->addWidget( _speed);

  //add Heading widget
  _heading = new MapInfoBox( this, "#c0c0c0" );
  //_heading = new MapInfoBox( this, QColor(Qt::darkGray).name() );
  _heading->setPreText("Trk");
  _heading->setValue("-");
  _heading->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  SHLayout->addWidget( _heading);

#ifdef FLARM
  connect(_heading, SIGNAL(mouseShortPress()), this, SLOT(slot_OpenFlarmWidget()));
#endif

  //layout for Wind/LD
  QBoxLayout *WLLayout = new QHBoxLayout;
  commonLayout->addLayout(WLLayout, 1);

  //add Wind widget; this is head/tailwind, no direction given !
  _wind = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _wind->setPreText("Wd");
  _wind->setValue("-");
  _wind->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  WLLayout->addWidget(_wind );
  connect(_wind, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleWindAndLD()));
  connect(_wind, SIGNAL(mouseLongPress()), this, SLOT(slot_openManualWind()));

  //add LD widget
  _ld = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _ld->setVisible(false);
  _ld->setPreText( "LD" );
  _ld->setValue("-/-");
  _ld->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  WLLayout->addWidget( _ld );
  connect(_ld, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleWindAndLD()));

  //layout for Vario and Altitude
  QBoxLayout *VALayout = new QHBoxLayout;
  commonLayout->addLayout(VALayout);
  VALayout->setSpacing(2);

  // add altitude widget
  _altitude = new MapInfoBox( this, conf->getMapFrameColor().name(), true, true );
  _altitude->setPreText(AltimeterDialog::mode2String());
  _altitude->setPreUnit( Altitude::getUnitText() );
  _altitude->setValue("-");
  _altitude->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  VALayout->addWidget( _altitude, 3 );
  connect(_altitude, SIGNAL(mouseShortPress()), this, SLOT(slot_AltimeterDialog()));

  // add Vario widget
  _vario = new MapInfoBox( this, conf->getMapFrameColor().name(), false, true );
  _vario->setPreText("Var");
  _vario->setValue("-");
  _vario->setUpdateInterval( 750 );
  _vario->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  VALayout->addWidget(_vario, 2 );
  connect(_vario, SIGNAL(mouseShortPress()), this, SLOT(slot_VarioDialog()));

  sideLayout->addWidget( commonBar, 3 );

  // widget to group McCready functions
  QWidget *mcBar = new QWidget( this );
  mcBar->setFixedWidth(leftFixedWidth);
  mcBar->setContentsMargins(-9,-6,-9,-9);

  mcBar->setAutoFillBackground(true);
  mcBar->setBackgroundRole(QPalette::Window);
  mcBar->setPalette( QPalette(QColor(Qt::lightGray)) );

  //layout for McCready and Best Speed
  QBoxLayout *MSLayout = new QHBoxLayout(mcBar);
  MSLayout->setSpacing(2);

  //add McCready widget
  _mc = new MapInfoBox( this, conf->getMapFrameColor().name() );
  _mc->setPreText("Mc");
  _mc->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  calculator->glider() ? _mc->setValue("0.0") : _mc->setValue("-");
  MSLayout->addWidget( _mc );
  connect(_mc, SIGNAL(mouseShortPress()), this, SLOT(slot_gliderFlightDialog()));

  //add Best Speed widget
  _speed2fly = new MapInfoBox( this, "#a6a6a6", true );
  _speed2fly->setPreText("S2f");
  _speed2fly->setValue(tr("Menu"));
  _speed2fly->setPreUnit( "|=|" );
  _speed2fly->setVisible( false );
  _speed2fly->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  MSLayout->addWidget( _speed2fly );
  connect(_speed2fly, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleMenu()));

   //add menu toggle widget
  _menuToggle = new MapInfoBox( this, QColor(Qt::gray).name(), false, false, 30 );
  _menuToggle->setPreTextWidgetVisible( false );
  _menuToggle->setTextLabelBGColor( "lightGray" );
  _menuToggle->setValue(tr("Menu"));
  _menuToggle->setMapInfoBoxMaxHeight( textLabelBoxHeight );
  MSLayout->addWidget( _menuToggle );
  connect(_menuToggle, SIGNAL(mouseShortPress()), this, SLOT(slot_toggleMenu()));

  sideLayout->addWidget( mcBar, 1 );

  //--------------------------------------------------------------------
#ifdef QSCROLLER1
  // scroll area for the map, used by Android
  mapArea = new QScrollArea(this);
  mapArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mapArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mapArea->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
  mapArea->setFrameStyle( QFrame::NoFrame );

  centerLayout->addWidget(mapArea, 10);

  _theMap = new Map( mapArea );
  mapArea->setWidget( _theMap );
  mapScroller = QScroller::scroller(mapArea);

  QScrollerProperties sp = mapScroller->scrollerProperties();
  sp.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.5);
  sp.setScrollMetric(QScrollerProperties::MaximumVelocity, QPointF(0.0, 0.0));
  sp.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOn);
  sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOn);
  mapScroller->setScrollerProperties(sp);

  connect( mapScroller, SIGNAL(stateChanged(QScroller::State)),
           this, SLOT( slot_scrollerStateChanged(QScroller::State) ) );

  QScroller::grabGesture(mapArea, QScroller::LeftMouseButtonGesture);

#else

  // normal layout for the map
  QBoxLayout *MapLayout = new QHBoxLayout;
  centerLayout->addLayout(MapLayout);
  centerLayout->setStretchFactor( MapLayout, 1 );
  _theMap = new Map(this);
  MapLayout->addWidget(_theMap, 10);

#endif

  _theMap->setMode(Map::headUp);

#ifdef FLARM

  // Flarm widget with radar view
  _flarmWidget = new FlarmWidget(this);
  MapLayout->addWidget(_flarmWidget, 10);
  _flarmWidget->setVisible( false );
  connect( _flarmWidget, SIGNAL(closed()), this, SLOT(slot_OpenMapView()) );
#endif

  //--------------------------------------------------------------------
  // Status bar
  _statusbar = new QStatusBar(this);
  _statusbar->setObjectName("status");
  _statusbar->setSizeGripEnabled(false);

  QFont font = _statusbar->font();
  font.setBold(true);
  Layout::fitStatusbarFont( font );
  _statusbar->setFont(font);

  // Fixing the height caused some times problems on larger screens
  // QFontMetrics fm(font);
  // _statusbar->setFixedHeight(fm.boundingRect("WE°'?\"").height() + 6 );

  _statusGps = new CuLabel(tr("Man"), _statusbar);
  _statusGps->setFont(font);
  _statusGps->setLineWidth(0);
  _statusGps->setAlignment(Qt::AlignCenter);
  _statusGps->setMargin(0);
  _statusbar->addWidget(_statusGps);
  connect(_statusGps, SIGNAL(mousePress()), this, SLOT(slot_gpsStatusDialog()));

  _statusFlightstatus = new QLabel(tr("?","Unknown"), _statusbar);
  _statusFlightstatus->setFont(font);
  _statusFlightstatus->setLineWidth(0);
  _statusFlightstatus->setAlignment(Qt::AlignCenter);
  _statusFlightstatus->setMargin(0);
  _statusFlightstatus->setMinimumSize(_statusFlightstatus->fontMetrics().boundingRect(" L ? ").width(), 5);
  _statusbar->addWidget(_statusFlightstatus);

#ifdef FLARM

  _statusFlarm = new CuLabel( tr( "F" ), _statusbar );
  _statusFlarm->setFont(font);
  _statusFlarm->setLineWidth( 0 );
  _statusFlarm->setAlignment( Qt::AlignCenter );
  _statusFlarm->setMargin( 0 );
  _statusbar->addWidget( _statusFlarm );
  _statusFlarm->setVisible( false );

#endif

  _statusPosition = new QLabel(_statusbar);
  _statusPosition->setFont(font);
  _statusPosition->setLineWidth(0);
  _statusPosition->setAlignment(Qt::AlignCenter);
  _statusPosition->setMargin(0);
  _statusbar->addWidget(_statusPosition);

  _statusGlider = new QLabel(_statusbar);
  _statusGlider->setFont(font);
  _statusGlider->setLineWidth(0);
  _statusGlider->setAlignment(Qt::AlignCenter);
  _statusGlider->setMargin(0);
  _statusbar->addWidget(_statusGlider);

  _statusInfo = new QLabel(_statusbar);
  _statusInfo->setFont(font);
  _statusInfo->setAlignment(Qt::AlignCenter);
  _statusInfo->setMargin(0);
  _statusbar->addWidget(_statusInfo, 1);

  QFrame* filler = new QFrame(_statusbar);
  filler->setFont(font);
  filler->setLineWidth(0);
  _statusbar->addWidget(filler);
  topLayout->addWidget(_statusbar);

  lastPositionChangeSource = Calculator::MAN;
}

MapView::~MapView()
{
  // qDebug("MapView::~MapView() destructor is called");
}

void MapView::showEvent( QShowEvent* event )
{
  Q_UNUSED( event )

  // Used map info box widgets
  MapInfoBox *boxWidgets[14] = { _heading,
                                 _bearing,
                                 _rel_bearing,
                                 _distance,
                                 _speed,
                                 _speed2fly,
                                 _mc,
                                 _vario,
                                 _wind,
                                 _ld,
                                 _waypoint,
                                 _eta,
                                 _altitude,
                                 _glidepath };

  int gtWidth = 0;

  // Adapt the pretext display width to the text size.
  for( int i = 0; i < 14; i++ )
    {
      MapInfoBox *ptr = boxWidgets[i];

      if( ptr->isVisible() )
        {
          QFontMetrics fm( ptr->font() );

          int w = fm.width( ptr->getPreText() );

          // qDebug() << "Text=" << ptr->getPreText() << "Width=" << w;

          if( ! ptr->getPreText().isEmpty() )
            {
              ptr->getPreTextLabelWidget()->setFixedWidth( w );
            }

          if( w > gtWidth )
            {
              gtWidth = w;
            }
        }
    }

#if defined ANDROID || defined MAEMO

  // Limit statusbar width under Android and Maemo otherwise the map can be
  // resized to large.
  _statusbar->setMaximumWidth( width() );

#endif
}

/** called if heading has changed */
void MapView::slot_Heading(int head)
{
  static QTime lastDisplay = QTime::currentTime();

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  _heading->setValue(QString("%1").arg( head, 3, 10, QChar('0') ));
  _theMap->setHeading(head);
}


/** Called if speed has changed */
void MapView::slot_Speed(const Speed& speed)
{
  if (speed.getMph() < 0 )
    {
      _speed->setValue("-");
    }
  else
    {
      _speed->setValue(speed.getHorizontalText(false,0));
    }
}


/** called if the waypoint is changed in the calculator */
void MapView::slot_Waypoint(const Waypoint *wp)
{
  // qDebug("MapView::slot_Waypoint");
  QString dest = "";

  if (wp)
    {
      if (wp->taskPointIndex != -1)
        {
          QString idx = QString("%1 ").arg( wp->taskPointIndex );
          dest += tr("TP") + idx;
        }

      // @JD: suggestion: removal of spaces in wp display -> bigger font
      for (int i = 0; i < wp->name.size(); i++)
        {
          if (wp->name[i] != QChar(' '))
            {
              dest += wp->name[i];
            }

          if (dest.size() == 8)
            {
              break;
            }
        }

      dest += QString("(%1)").arg(Altitude::getText(wp->elevation, false, 0));

      _waypoint->setValue(dest);
    }
  else
    {
      // @AP: No waypoint is selected, reset waypoint and glidepath display
      _waypoint->setValue("");
      _glidepath->setValue("-");
      _glidepath->setPreWidgetsBGColor(_glidepathBGColor);
      // @JD: reset distance too
      _distance->setValue("-");
      QPixmap arrow = _arrows.copy(24 * 60 + 3, 3, 54, 54);
      _rel_bearing->setPixmap(arrow);
    }

  _theMap->scheduleRedraw(Map::informationLayer); // this is not really helpful -> it is: the bearingline won't change otherwise!
}


/** This slot is called by calculator if a new bearing has been calculated */
void MapView::slot_Bearing(int bearing)
{
  _lastBearing = bearing; // save received value

  if( bearing < 0 )
    {
      _bearing->setValue("-");
    }
  else
    {
      int ival = bearing;

      if( _bearingMode == 0 )
        {
          // display the reverse value
          ival > 180 ? ival -=180 : ival +=180;
        }

      QString val=QString("%1").arg( ival, 3, 10, QChar('0') );
      _bearing->setValue(val);
    }
}


/** This slot is called if the user presses the bearing display */
void MapView::slot_toggleBearing()
{
  // display inverse bearing
  _bearing->setPreWidgetsBGColor( QColor(Qt::red) );

  _bearingMode = 0;
  slot_Bearing( _lastBearing );

  _bearingTimer->setSingleShot(true);
  _bearingTimer->start(5000);
}

/** Called to toggle the menu of the main window. */
void MapView::slot_toggleMenu()
{
  if( Map::getInstance()->isVisible() == true )
    {
      // Toggle menu only, if map widget is visible
      emit toggleMenu();
    }
}

/**
 * Reset reverse bearing after the timeout
 */
void MapView::slot_resetInversBearing()
{
  // display bearing in normal mode
  _bearing->setPreWidgetsBGColor( _bearingBGColor );
  _bearingMode = 1;
  slot_Bearing( _lastBearing );
}

/** This slot is called by calculator if a new relative bearing has been calculated */
void MapView::slot_RelBearing(int relbearing)
{
  // qDebug("RelBearing: %d", relbearing );
  if (relbearing < -360)
    {
      // we need an icon when no relative bearing is available ?!
      // @JD: here it is
      QPixmap arrow = _arrows.copy( 24*60+3, 3, 54, 54 );
      _rel_bearing->setPixmap (arrow);
      return;
    }

  static QTime lastDisplay = QTime::currentTime();

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  while (relbearing < 0)
    {
      relbearing += 360;
    }

  //we only want to rotate in steps of 15 degrees. Finer is not useful.
  int rot=((relbearing+7)/15) % 24;
  QPixmap arrow = _arrows.copy( rot*60+3, 3, 54, 54 );
  _rel_bearing->setPixmap (arrow);
}


/** This slot is called by calculator if a new distance has been calculated. */
void MapView::slot_Distance(const Distance& distance)
{
  if (distance.getMeters() < 0 )
    {
      _distance->setValue("-");
    }
  else
    {
      _distance->setValue(distance.getText(false, 1, (uint) 2 ) );
    }
}


/**
 * This slot is called by the calculator if a new ETA (Estimated Time to Arrival
 * or the time that is approximately needed to arrive at the waypoint) has been
 * calculated.
 */
void MapView::slot_ETA(const QTime& eta)
{
  if( eta.isNull() )
    {
      _eta->setValue("-");
    }
  else
    {
      QString txt = QString("%1:%2").arg( eta.hour() )
                                    .arg( eta.minute(), 2, 10, QChar('0') );
      _eta->setValue(txt);
    }
}


/** This slot is called if a new position fix has been established. */
void MapView::slot_Position(const QPoint& position, const int source)
{
  static QTime lastDisplay = QTime::currentTime();

  // remember for slot_settingsChange
  lastPositionChangeSource = source;

  // this slot is called:
  // a) from GPS data coming in, not in manual mode
  // b) for Manual data if no GPS data coming in
  // c) for Manual data if in manual mode during GPS data coming in too

  // check airspace for newPosition from GPS and not for Manual
  // this covers a) and b)
  if( ! calculator->isManualInFlight() )
    {
      _theMap->checkAirspace (position);
    }

  if( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
      source != Calculator::MAN )
    {
      // If we have a valid fix the display is updated every 3 seconds only.
      // That will reduce the X-Server load.
      if( lastDisplay.elapsed() < 3000 )
        {
          return;
        }

      lastDisplay = QTime::currentTime();
    }

  // if in manual mode: show position in status bar for the cross, not for the glider
  // this covers a) and b) or c) with source manual
  if( !calculator->isManualInFlight() ||
      (calculator->isManualInFlight() && (source == Calculator::MAN)))
    {
      _statusPosition->setText(" " + WGSPoint::printPos(position.x(),true) +
                             " / " + WGSPoint::printPos(position.y(),false) + " ");
    }
}


/** This slot is called if the status of the GPS changes. */
void MapView::slot_GPSStatus(GpsNmea::GpsStatus status)
{
  QColor color;

  switch ( status )
    {
    case GpsNmea::validFix:
      slot_info( tr( "GPS OK" ) );
      color = Qt::green;
      break;
    case GpsNmea::noFix:
      slot_info(tr( "GPS no fix" ) );
      color = Qt::yellow;
      break;
    default:
      slot_info( tr( "GPS ?" ) );
      color = Qt::red;
      break;
    }

  if( status != GpsNmea::notConnected )
    {
      _statusGps->setText( tr( "GPS" ) );

#ifdef QSCROLLER1
      QScroller::ungrabGesture(mapArea);
      mapArea->ensureVisible( 0, 0 );
#endif
    }
  else
    {
      _statusGps->setText( tr( "Man" ) );
      _heading->setValue( "-" );
      _speed->setValue( "-" );
      _altitude->setValue( "-" );

#ifdef QSCROLLER1
      QScroller::grabGesture( mapArea, QScroller::LeftMouseButtonGesture );
#endif
    }

  // Set the color background according to the current GPS status.
  _statusGps->setAutoFillBackground( true );
  _statusGps->setBackgroundRole( QPalette::Window );
  _statusGps->setPalette( QPalette( color ) );

  // Only show glider symbol if the GPS is connected.
  _theMap->setShowGlider( status == GpsNmea::validFix );
}


/** This slot is called if a log entry has been made by the logger. */
void MapView::slot_LogEntry()
{
  slot_setLoggerStatus();
}


/** This slot is being called if the altitude has been changed. */
void MapView::slot_Altitude(const Altitude& altitude )
{
  _altitude->setValue( altitude.getText( false, 0 ) );
}


/** This slot is called when the "above glide path" value has changed */
void MapView::slot_GlidePath (const Altitude& above)
{
  static QColor lastColor = _glidepathBGColor;

  _glidepath->setValue(above.getText(false,0));

  if( above.getMeters() < 0.0 && lastColor != QColor(Qt::red) )
    {
      // display red background if arrival is under zero
      _glidepath->setPreWidgetsBGColor( QColor(Qt::red) );
      lastColor = QColor(Qt::red);
    }
  else if( above.getMeters() >= 0.0 && lastColor != _glidepathBGColor )
    {
      _glidepath->setPreWidgetsBGColor( _glidepathBGColor );
     lastColor = _glidepathBGColor;
    }
}

/** This slot is called when the best speed value has changed. */
void MapView::slot_bestSpeed (const Speed& speed)
{
  if( speed.isValid() )
    {
      _speed2fly->setValue(speed.getHorizontalText(false, 0));
      _speed2fly->setVisible( true );
      _menuToggle->setVisible( false );
    }
  else
    {
      _speed2fly->setValue("-");
      _menuToggle->setValue( tr("Menu") );
      _speed2fly->setVisible( false );
      _menuToggle->setVisible( true );
    }
}

/** This slot is called if a new McCready value has been set */
void MapView::slot_Mc (const Speed& mc)
{
  if( mc.isValid() )
    {
      _mc->setValue(mc.getVerticalText(false, 1));
    }
  else
    {
      _mc->setValue("-");
    }
}

/** This slot is called if a new variometer value has been set */
void MapView::slot_Vario (const Speed& vario)
{
  if( ! vario.isValid() )
    {
      _vario->setValue("-");
      return;
    }

  QString varValue;

  // if altitude has more than 4 digits, vario is rounded to one
  // digit. Normal vario display is e.g. 1.1 (2 digits plus decimal
  // point)
  if( _altitude->getValue().size() > 4 )
    {
      varValue = vario.getVerticalText(false, 0);
    }
  else
    {
      varValue = vario.getVerticalText(false, 1);
    }

  _vario->setValue( varValue );
}


/** This slot is called if a new wind value has been set */
void MapView::slot_Wind( Vector& wind )
{
  QString ws = "-";

  if( wind.isValid() && wind.getSpeed().getMps() > 0.0 )
    {
      ws = QString("%1/" + wind.getSpeed().getWindText(false, 0) ).arg( wind.getAngleDeg() );
    }

  _wind->setValue( ws );

  _theMap->slotNewWind( wind );
}

/** This slot is called if a new current LD value has been set */
void MapView::slot_LD( const double& rLD, const double& cLD )
{
  static QTime lastDisplay = QTime::currentTime();

  // The display is updated every 1 seconds only.
  // That will reduce the X-Server load.
  if( lastDisplay.elapsed() < 750 )
    {
      return;
    }
  else
    {
      lastDisplay = QTime::currentTime();
    }

  // qDebug( "MapView::slot_LD: %f, %f", rLD, cLD );

  QString cld, rld;

  // format required LD
  if( rLD < 0.0 )
    {
      rld = " - ";
    }
  else if( rLD < 100.0 )
    {
      rld = QString("%1").arg( rLD, 2, 'f', 1 );
    }
  else if( rLD < 1000.0 )
    {
      rld = QString("%1").arg( rLD, 3, 'f', 0 );
    }
  else
    {
      rld = ">999";
    }

  // format current LD
  if( cLD < 0.0 )
    {
      cld = " - ";
    }
  else if( cLD < 100.0 )
    {
      cld = QString("%1").arg( cLD, 2, 'f', 1 );
    }
  else
    {
      cld = ">99";
    }

  _ld->setValue( rld + "/" + cld );
}


/**
 * This slot is called if the glider selection has been modified
 */
void MapView::slot_glider( const QString& glider )
{
  _statusGlider->setText( glider );

  if( glider.isEmpty() )
    {
      // reset arrival display because glider has been removed
      _glidepath->setValue("-");
    }
}

/**
 * This slot is called if a info message shall be displayed
 */
void MapView::slot_info( const QString& info )
{
  _statusInfo->setText( info );
}


/** This slot is called if the settings have been changed.
 * It refreshes all displayed data because units might have been changed.
 */
void MapView::slot_settingsChange()
{
  // qDebug("MapView::slot_settingsChange");
  slot_newAltimeterMode();
  slot_Position(calculator->getlastPosition(), lastPositionChangeSource);
  slot_Speed(calculator->getLastSpeed());
  slot_Distance(calculator->getlastDistance());
  slot_GlidePath(calculator->getlastGlidePath());
  slot_bestSpeed(calculator->getlastBestSpeed());
  slot_Mc(calculator->getlastMc());
  slot_Waypoint(calculator->getselectedWp());

  _glidepath->setPreUnit( Altitude::getUnitText() );
  _distance->setPreUnit( Distance::getUnitText() );
}


/** This slot is called if the number of satellites changes. */
void MapView::slot_SatCount( SatInfo& satInfo )
{
  // Display the number of satellites in use.
  QString msg = QString ("G-%1").arg(satInfo.satsInUse);
  _statusGps->setText (msg);
}

#ifdef FLARM

/** This slot is called if the number of received Flarms has been changed. */
void MapView::slot_FlarmCount( int flarmCount )
{
  // Display the number of Flarms received.
  if( flarmCount == -1 )
    {
      // hide Flarm display
      _statusFlarm->setVisible( false );
      return;
    }

  QString msg = QString ("F-%1").arg( flarmCount );
  _statusFlarm->setText (msg);

  if( _statusFlarm->isVisible() == false )
    {
      _statusFlarm->setVisible( true );
    }
}

/** Opens the Flarm widget. */
void MapView::slot_OpenFlarmWidget()
{
  if( _theMap->isVisible() == true )
    {
      MainWindow::mainWindow()->setView( MainWindow::flarmView );
      _theMap->setVisible( false );
      _flarmWidget->setVisible( true );
    }
}

/** Opens the Map view. */
void MapView::slot_OpenMapView()
{
  if( _theMap->isVisible() == false )
    {
      MainWindow::mainWindow()->setView( MainWindow::mapView );
      _theMap->setVisible( true );
      _flarmWidget->setVisible( false );
    }
}

#endif

/** This slot updates the FlightStatus status bar-widget with the
    current logging and flight mode status */
void MapView::slot_setFlightStatus( Calculator::FlightMode fm )
{
  QString status="";

  // flight mode status
  switch (fm)
    {
    case Calculator::unknown:
      status += (tr("?","Unknown"));
      break;
    case Calculator::standstill:
      status += (tr("S","Standstill"));
      break;
    case Calculator::cruising:
      status += (tr("C","Cruising"));
      break;
    case Calculator::circlingL:
      status += (tr("L","Circling Left"));
      break;
    case Calculator::circlingR:
      status += (tr("R","Circling Right"));
      break;
    }

  _statusFlightstatus->setText(status);
}

/*
 * Sets the logger status in the status bar.
 */
void MapView::slot_setLoggerStatus()
{
  QString status = "";

  IgcLogger* logger = IgcLogger::instance();

  if( ! logger )
    {
      return;
    }

  if( logger->getIsLogging() )
    {
      // are we logging right now?
      _statusFlightstatus->setAutoFillBackground( true );
      _statusFlightstatus->setBackgroundRole( QPalette::Window );
      _statusFlightstatus->setPalette( QPalette( QColor( Qt::green ) ) );

      status += tr( "L", "Logging" ) + " "; // so just insert an L
    }
  else
    {
      if( logger->getIsStandby() )
        {
          _statusFlightstatus->setAutoFillBackground( true );
          _statusFlightstatus->setBackgroundRole( QPalette::Window );
          _statusFlightstatus->setPalette( QPalette( QColor( Qt::yellow ) ) );

          status += tr( "Ls", "LoggingStandby" ) + " ";
        }
      else
        {
          _statusFlightstatus->setAutoFillBackground( true );
          _statusFlightstatus->setBackgroundRole( QPalette::Window );
          _statusFlightstatus->setPalette( QPalette( _bearingBGColor ) );

          status += ""; //  we are not logging
        }
    }
}

/** toggle between distance and eta widget on mouse signal */
void MapView::slot_toggleDistanceEta()
{
  if( _distance->isVisible() )
    {
      _distance->setVisible(false);
      _eta->setVisible(true);
      _eta->setValue( _eta->getValue() );
      emit toggleETACalculation( true );
    }
  else
    {
      _eta->setVisible(false);
      _distance->setVisible(true);
      _distance->setValue( _distance->getValue() );
      emit toggleETACalculation( false );
    }
}

/** toggle between wind, vario and LD widget on mouse signal */
void MapView::slot_toggleWindAndLD()
{
  if( _wind->isVisible() )
    {
      _wind->setVisible(false);
      _ld->setVisible(true);
      _ld->setValue( _ld->getValue() );
      // switch on LD calculation in calculator
      emit toggleLDCalculation( true );
    }
  else
    {
      _ld->setVisible(false);
      _wind->setVisible(true);
      _wind->setValue( _wind->getValue() );
      // switch off LD calculation in calculator
      emit toggleLDCalculation( false );
    }
}

/** Opens the Altimeter settings dialog. */
void MapView::slot_AltimeterDialog()
{
  if( AltimeterDialog::getNrOfInstances() > 0 )
    {
      // Sometimes the mouse event is delayed under Maemo, which triggers this
      // method. In such a case multiple dialogs are opened. This check shall
      // prevent that.
      return;
    }

  AltimeterDialog *amDlg = new AltimeterDialog( this );

  amDlg->slotAltitudeChanged( calculator->getAltimeterAltitude() );

  connect( amDlg, SIGNAL( newAltimeterMode() ),
           this, SLOT( slot_newAltimeterMode() ) );
  connect( amDlg, SIGNAL( newAltimeterSettings() ),
           GpsNmea::gps, SLOT( slot_reset() ) );
  connect( calculator, SIGNAL( newUserAltitude( const Altitude& ) ),
           amDlg, SLOT( slotAltitudeChanged( const Altitude& ) ) );

  connect( amDlg, SIGNAL( closingWidget() ),
           MainWindow::mainWindow(), SLOT( slotSubWidgetClosed() ) );

  amDlg->setVisible(true);

#ifdef ANDROID

  QSize ms = amDlg->minimumSizeHint();

  ms += QSize(10, 10);

  // A dialog is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  amDlg->setGeometry( (width() - ms.width()) / 2, (height() - ms.height()) / 2,
                       ms.width(), ms.height() );

#endif

  MainWindow::setRootWindow( false );
}

/** Called, if altimeter mode has been changed. */
void MapView::slot_newAltimeterMode()
{
  _altitude->setPreText( AltimeterDialog::mode2String() );
  _altitude->setPreUnit( Altitude::getUnitText() );
  _glidepath->setPreUnit( Altitude::getUnitText() );

  // Altimeter unit change needs always an altitude display update.
  slot_Altitude( calculator->getAltimeterAltitude() );

  if( _glidepath->getValue() != "-" )
    {
      // Altimeter unit change needs always an arrival display update,
      // if display shows a real value.
      slot_GlidePath( calculator->getlastGlidePath() );
    }
}

/** Opens the Variometer settings dialog. */
void MapView::slot_VarioDialog()
{
  if( VarioModeDialog::getNrOfInstances() > 0 )
    {
      // Sometimes the mouse event is delayed under Maemo, which triggers this
      // method. In such a case multiple dialogs are opened. This check shall
      // prevent that.
      return;
    }

  VarioModeDialog *vmDlg = new VarioModeDialog( this );

  connect( vmDlg, SIGNAL( newVarioTime( int ) ),
           calculator->getVario(), SLOT( slotNewVarioTime( int ) ) );
  connect( vmDlg, SIGNAL( newTEKMode( bool ) ),
           calculator->getVario(), SLOT( slotNewTEKMode( bool ) ) );
  connect( vmDlg, SIGNAL( newTEKAdjust( int ) ),
           calculator->getVario(), SLOT( slotNewTEKAdjust( int ) ) );

  connect( vmDlg, SIGNAL( closingWidget() ),
           MainWindow::mainWindow(), SLOT( slotSubWidgetClosed() ) );

  vmDlg->setVisible(true);

#ifdef ANDROID

  QSize ms = vmDlg->minimumSizeHint();

  ms += QSize(10, 10);

  // A dialog is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  vmDlg->setGeometry( (width() - ms.width()) / 2, (height() - ms.height()) / 2,
                       ms.width(), ms.height() );

#endif

  MainWindow::setRootWindow( false );
}

/** Opens the GPS status dialog */
void MapView::slot_gpsStatusDialog()
{
  if( GpsStatusDialog::getNrOfInstances() > 0 )
    {
      // Only one instance of GPS status dialog is allowed.
      return;
    }

  GpsStatusDialog *gpsDlg = new GpsStatusDialog( this );

  connect( gpsDlg, SIGNAL( closingWidget() ),
           MainWindow::mainWindow(), SLOT( slotSubWidgetClosed() ) );

  gpsDlg->setVisible(true);

  MainWindow::setRootWindow( false );
}

/** Opens the inflight glider settings dialog. */
void MapView::slot_gliderFlightDialog()
{
  if( ! calculator->glider() || GliderFlightDialog::getNrOfInstances() > 0 )
    {
      // Open dialog only, if a glider is selected.
      // Sometimes the mouse event is delayed under Maemo, which triggers this
      // method. In such a case multiple dialogs are opened. This check shall
      // prevent that.
      return;
    }

  GliderFlightDialog *gfDlg = new GliderFlightDialog( this );

  connect( gfDlg, SIGNAL(newMc(const Speed&)),
           calculator, SLOT(slot_Mc(const Speed&)) );

  connect( gfDlg, SIGNAL(newWaterAndBugs(const int, const int)),
           calculator, SLOT(slot_WaterAndBugs(const int, const int)) );

  connect( gfDlg, SIGNAL( closingWidget() ),
           MainWindow::mainWindow(), SLOT( slotSubWidgetClosed() ) );

  gfDlg->setVisible(true);

#ifdef ANDROID

  QSize ms = gfDlg->minimumSizeHint();

  ms += QSize(10, 10);

  // A dialog is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  gfDlg->setGeometry( (width() - ms.width()) / 2, (height() - ms.height()) / 2,
                       ms.width(), ms.height() );

#endif

  MainWindow::setRootWindow( false );
}

void MapView::slot_showInfoBoxes( bool show )
{
  _sidebarWidget->setVisible( show );
}

void MapView::slot_openManualWind()
{
  PreFlightWindPage* pfwp = new PreFlightWindPage( this );

  connect( pfwp, SIGNAL(manualWindStateChange(bool)),
           calculator, SLOT(slot_ManualWindChanged(bool)) );

  pfwp->show();
}

/**
 * @writes a message into the status bar for the given time. Default
 * is 5s. If time is zero, the message will never disappear.
 */
void MapView::message( const QString& message, int ms )
{
  _statusbar->showMessage( message, ms );
}

#ifdef QSCROLLER1

void MapView::slot_scrollerStateChanged(QScroller::State new_s)
{
  // qDebug() << "MapView::slot_scrollerStateChanged: State=" << new_s;

  if( new_s != QScroller::Scrolling )
    {
      return;
    }

  extern MapMatrix* _globalMapMatrix;

  QPointF statePoint = mapScroller->overshootPosition();

  // qDebug() << "mapScroller: calling pause(), StatePoint=" << statePoint;
  mapScroller->pause();

  QPoint delta = QPoint( (int)statePoint.x(), (int)statePoint.y() );
  QPoint mapCenter = QPoint( _theMap->width()/2, _theMap->height()/2);
  mapCenter += delta;
  QPoint newWgs = _globalMapMatrix->mapToWgs(mapCenter);
  newWgs = QPoint(newWgs.y(),newWgs.x());
  calculator->slot_Position( newWgs );
}

/** Reset the QScroller after pausing and map redraw */
void MapView::resetScrolling()
{
  mapScroller->stop();
}

#endif
