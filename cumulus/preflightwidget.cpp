/***********************************************************************
 **
 **   preflightwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "igclogger.h"
#include "layout.h"
#include "mainwindow.h"
#include "map.h"
#include "preflightgliderpage.h"
#include "preflightwidget.h"
#include "preflightmiscpage.h"

#ifdef ANDROID
#include "preflightreturnerpage.h"
#endif

#include "preflighttaskpage.h"
#include "preflightwaypointpage.h"
#include "rowdelegate.h"

// Menu labels
#define PREFLIGHT "Preflight Menu"
#define COMMON    "Common"
#define GLIDER    "Glider"
#define RETURNER  "Returner"
#define TASK      "Task"
#define WAYPOINTS "Waypoints"

PreFlightWidget::PreFlightWidget( QWidget* parent ) :
  QWidget(parent)
{
  setObjectName("PreFlightWidget");
  setWindowTitle(tr("Preflight Settings"));
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  m_setupTree = new QTreeWidget( this );
  m_setupTree->setRootIsDecorated( false );
  m_setupTree->setItemsExpandable( false );
  m_setupTree->setSortingEnabled( true );
  m_setupTree->setSelectionMode( QAbstractItemView::SingleSelection );
  m_setupTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_setupTree->setAlternatingRowColors(true);
  m_setupTree->setColumnCount( 1 );
  m_setupTree->setFocusPolicy( Qt::StrongFocus );
  m_setupTree->setUniformRowHeights(true);
  m_setupTree->setHeaderLabel( tr( "Preflight Menu" ) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( m_setupTree, 10 );
  m_setupTree->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = m_setupTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  m_setupTree->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  m_setupTree->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture(m_setupTree->viewport(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture(m_setupTree->viewport(), QtScroller::LeftMouseButtonGesture);
#endif

  connect( m_setupTree, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT( slotPageClicked( QTreeWidgetItem*, int )) );

  contentLayout->addWidget( m_setupTree, 5 );

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr(GLIDER) );
  item->setData( 0, Qt::UserRole, GLIDER );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(TASK) );
  item->setData( 0, Qt::UserRole, TASK );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(WAYPOINTS) );
  item->setData( 0, Qt::UserRole, WAYPOINTS );
  m_setupTree->addTopLevelItem( item );

  item = new QTreeWidgetItem;
  item->setText( 0, tr(COMMON) );
  item->setData( 0, Qt::UserRole, COMMON );
  m_setupTree->addTopLevelItem( item );

#ifdef ANDROID
  item = new QTreeWidgetItem;
  item->setText( 0, tr(RETURNER) );
  item->setData( 0, Qt::UserRole, RETURNER );
  m_setupTree->addTopLevelItem( item );
#endif

  m_setupTree->sortByColumn ( 0, Qt::AscendingOrder );

  contentLayout->addSpacing( 25 );
  m_menuCb = new QCheckBox( tr("close menu") );
  m_menuCb->setChecked( GeneralConfig::instance()->getClosePreFlightMenu() == true ? Qt::Checked : Qt::Unchecked );
  contentLayout->addWidget( m_menuCb, 0, Qt::AlignVCenter|Qt::AlignBottom );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);

  contentLayout->addStretch( 10 );
  contentLayout->addLayout(buttonBox);

  m_headerLabels << ( tr ("Preflight Menu") )
                 << ( tr ("Glider") )
                 << ( tr ("Task") )
#ifdef ANDROID
                 << ( tr ("Returner") )
#endif
                 << ( tr ("Waypoints") )
                 << ( tr ("Common") );

  m_setupTree->setMinimumWidth( Layout::maxTextWidth( m_headerLabels, font() ) + 100 );
  setVisible( true );
}

PreFlightWidget::~PreFlightWidget()
{
}

void PreFlightWidget::keyReleaseEvent( QKeyEvent* event )
{
  // close the dialog on key press
  switch(event->key())
    {
      case Qt::Key_Close:
      case Qt::Key_Escape:
        slotReject();
        break;

      default:
        QWidget::keyReleaseEvent( event );
        break;
    }
}

/**
 * Called, if an item is pressed in the tree view.
 */
void PreFlightWidget::slotPageClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  if( itemText == COMMON )
    {
      PreFlightMiscPage* pfmp = new PreFlightMiscPage( this );

      connect( pfmp, SIGNAL( settingsChanged() ),
               IgcLogger::instance(), SLOT( slotReadConfig() ) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfmp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfmp->show();
    }
  else if( itemText == GLIDER )
    {
      PreFlightGliderPage* pfgp = new PreFlightGliderPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfgp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfgp->show();
    }
  else if( itemText == TASK )
    {
      PreFlightTaskPage* pftp = new PreFlightTaskPage( this );

      connect( pftp, SIGNAL( newTaskSelected() ),
               IgcLogger::instance(), SLOT( slotNewTaskSelected() ) );

      connect( pftp, SIGNAL( newTaskSelected() ),
               MainWindow::mainWindow(), SLOT( slotPreFlightDataChanged() ) );

      connect( pftp, SIGNAL( newWaypoint( Waypoint*, bool ) ),
               calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pftp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pftp->show();
    }
  else if( itemText == WAYPOINTS )
    {
      PreFlightWaypointPage* pfwp = new PreFlightWaypointPage( this );

      connect( pfwp, SIGNAL(waypointsAdded()),
               Map::getInstance(), SLOT(slotRedraw()) );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfwp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfwp->show();
    }

#ifdef ANDROID

  else if( itemText == RETURNER )
    {
      PreFlightReturnerPage* pfrp = new PreFlightReturnerPage( this );

      if( m_menuCb->checkState() == Qt::Checked )
        {
          connect( pfrp, SIGNAL( closingWidget() ), this, SLOT( slotAccept() ) );
        }

      pfrp->show();
    }

#endif

}

void PreFlightWidget::slotAccept()
{
  setVisible( false );
  GeneralConfig::instance()->setClosePreFlightMenu( m_menuCb->checkState() == Qt::Checked ? true : false );
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slotReject()
{
  setVisible( false );
  GeneralConfig::instance()->setClosePreFlightMenu( m_menuCb->checkState() == Qt::Checked ? true : false );
  emit closeConfig();
  QWidget::close();
}
