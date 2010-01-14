/***********************************************************************
**
**   tasklistview.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
**   Displays all points of a task as list with different buttons for
**   actions. Can be used in two modes, as display only, buttons for
**   actions are not visible or with command buttons.
**
***********************************************************************/

#include <QDesktopWidget>
#include <QShortcut>

#include "mainwindow.h"
#include "tasklistview.h"
#include "flighttask.h"
#include "distance.h"
#include "calculator.h"
#include "sonne.h"

extern MapConfig * _globalMapConfig;
extern Calculator* calculator;

TaskListView::TaskListView( QWidget *parent, bool showButtons ) :
  QWidget(parent),
  rowDelegate(0)
{
  setObjectName("TaskListView");

  _showButtons = showButtons;
  _task = 0;
  _selectedWp = 0;
  _currSelectedTp = 0;
  _newSelectedTp = 0;
  _selectText = tr("Select");
  _unselectText = tr("Unselect");

  QVBoxLayout *topLayout = new QVBoxLayout( this );

  if ( ! showButtons )
    {
      topLayout->setMargin(0);
    }

  QHBoxLayout *total = new QHBoxLayout;
  topLayout->addLayout( total );

  speedTotal = new QLabel("", this );
  wind       = new QLabel("", this );
  distTotal  = new QLabel("", this );
  timeTotal  = new QLabel("", this );

  total->addWidget( speedTotal );
  total->addWidget( wind );
  total->addWidget( distTotal );
  total->addWidget( timeTotal );

  list = new QTreeWidget( this );
  list->setObjectName("TaskListView");

  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::NoSelection);
  list->setColumnCount(7);
  list->setFocusPolicy(Qt::NoFocus);
//  list->setEnabled(false);

  QStringList sl;
  sl << tr("Type")
  << tr("Name")
  << tr("Dist.")
  << tr("TH")
  << tr("Time")
  << tr("SS")
  << tr("Description");
  list->setHeaderLabels(sl);

  topLayout->addWidget(list, 10);

  if ( showButtons )
    {
      total->setSpacing(5);
      list->setAllColumnsShowFocus(true);
      list->setSelectionMode(QAbstractItemView::SingleSelection);
      list->setSelectionBehavior(QAbstractItemView::SelectRows);
      list->setFocusPolicy( Qt::StrongFocus );

      // Don't show any buttons, if required
      QBoxLayout *buttonrow=new QHBoxLayout;
      topLayout->addLayout( buttonrow );

      QPushButton *cmdClose = new QPushButton(tr("Close"), this);
      buttonrow->addWidget(cmdClose);

      QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
      buttonrow->addWidget(cmdInfo);

      cmdSelect = new QPushButton(_selectText, this);
      buttonrow->addWidget(cmdSelect);

      connect( cmdSelect, SIGNAL(clicked()),
               this, SLOT(slot_Select()) );
      connect( cmdInfo, SIGNAL(clicked() ),
               this, SLOT(slot_Info()));
      connect( cmdClose, SIGNAL(clicked() ),
               this, SLOT(slot_Close()) );
      connect( list, SIGNAL(itemSelectionChanged()),
               this, SLOT(slot_Selected()) );

      // activate keyboard shortcut Return as select
      QShortcut* scSelect = new QShortcut( this );
      scSelect->setKey( Qt::Key_Return );
      connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
    }
}

TaskListView::~TaskListView()
{
  // qDebug("TaskListView::~TaskListView()");
  if ( _task )
    {
      delete _task;
    }
}

void TaskListView::slot_Selected()
{
  _newSelectedTp = list->currentItem();

  if ( _newSelectedTp == 0 )
    {
      return;
    }

  _selectedWp = ((_TaskPoint*)_newSelectedTp)->tp;

  if ( _selectedWp->taskPointIndex == 0 )
    {
      // Take-off point should not be selectable in taskview
      cmdSelect->setEnabled(false);
      return;
    }

  cmdSelect->setEnabled(true);

  if (_newSelectedTp == _currSelectedTp)
    {
      cmdSelect->setText(_unselectText);
    }
  else
    {
      cmdSelect->setText(_selectText);
    }
  //qDebug("New Selected Waypoint name: %s, Index=%d",
  //       _selectedWp->name.latin1(), _selectedWp->taskPointIndex );
}

void TaskListView::showEvent(QShowEvent *)
{
  if ( _showButtons == false )
    {
      // do nothing as display, there are no buttons visible
      return;
    }

  const wayPoint *calcWp = calculator->getselectedWp();
  bool foundWp = false;

  for ( int i = 0; i < list->topLevelItemCount(); i++)
    {

      _TaskPoint* _tp = (_TaskPoint *) list->topLevelItem(i);
      wayPoint*   wp = _tp->tp;

      // Waypoints can be selected from different windows. We will
      // consider only waypoints for a selection, which are member of a
      // flighttask. In this case the taskPointIndex should be unequal to -1.
      if ( calcWp && calcWp->origP == wp->origP &&
           calcWp->taskPointIndex == wp->taskPointIndex )
        {
          list->setCurrentItem( list->topLevelItem(i), 0 );
          _currSelectedTp = _tp;
          _newSelectedTp = _tp;
          _selectedWp = wp;
          cmdSelect->setText(_unselectText);
          foundWp = true;
          break;
        }
    }

  if ( foundWp == false )
    {
      // if no calculator waypoint selected, clear selection on listview
      list->clearSelection();
      _selectedWp = 0;
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }

  list->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void TaskListView::slot_Select()
{
  // qDebug("TaskListView::slot_Select(): Selected WP= %s, Index=%d",
  //       _selectedWp->name.latin1(), _selectedWp->taskPointIndex );

  if (_newSelectedTp == _currSelectedTp)
    {
      // this was an unselect
      //calculator->slot_WaypointChange(0, true);
      emit newWaypoint(0, true);
      list->clearSelection();
      cmdSelect->setText(_selectText);
      _selectedWp = 0;
      _currSelectedTp = 0;
      _newSelectedTp = 0;
    }
  else
    {
      _currSelectedTp = _newSelectedTp; // save last selection
      emit newWaypoint(getSelectedWaypoint(), true);
      emit done();
    }
}

/** This slot is called if the info button has been clicked */
void TaskListView::slot_Info()
{
  if (getSelectedWaypoint())
    {
      emit info(getSelectedWaypoint());
    }
}

/** @ee This slot is called if the listview is closed without selecting */
void TaskListView::slot_Close ()
{
  if ( _newSelectedTp != _currSelectedTp )
    {
      _newSelectedTp = _currSelectedTp; // selection change was not committed
    }

  emit done();
}

/**
 * Retrieves the waypoints from the task, and fills the list.
 */
void TaskListView::slot_setTask(const FlightTask *tsk)
{
  // qDebug("TaskListView::slot_setTask()");

  // delete old task
  if ( _task )
    {
      delete _task;
      _task = 0;
    }

  list->clear();

  if (tsk == 0 )
    {
      // No new task passed
      _task = 0;
      return;
    }

  if ( _showButtons == true )
    {
      // set row height at each list fill - has probably changed.
      // Note: rpMargin is a manifold of 2 to ensure symmetry
      int rpMargin = GeneralConfig::instance()->getListDisplayRPMargin();

      // qDebug( "rpMargin=%d", rpMargin );

      if ( rowDelegate )
        {
          rowDelegate->setVerticalMargin( rpMargin );
        }
      else
        {
          rowDelegate = new RowDelegate( list, rpMargin );
          list->setItemDelegate( rowDelegate );
        }
    }

  // create a deep task copy
  _task = new FlightTask(*tsk);

  // Check if a waypoint is selected in calculator. In this case set
  // it in tasklist as selected too.
  const wayPoint *calcWp = calculator->getselectedWp();

  QList<TaskPoint *> tmpList = _task->getTpList();

  for ( int loop = 0; loop < tmpList.count(); loop++ )
    {
      TaskPoint* tp = tmpList.at( loop );
      _TaskPoint* _tp = new _TaskPoint( list, tp );

      if ( calcWp && calcWp->origP == tp->origP )
        {
          list->setCurrentItem( _tp, 0 );
          _currSelectedTp = _tp;
          _selectedWp = tp;
        }
    }

  // set the total values in the header of this view
  speedTotal->setText( "TAS=" + _task->getSpeedString() );

  // wind direction and speed
  wind->setText( "W=" + _task->getWindString() );

  distTotal->setText( "S=" + _task->getTotalDistanceString() );
  timeTotal->setText( "T=" + _task->getTotalDistanceTimeString() );

  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);
  list->resizeColumnToContents(3);
  list->resizeColumnToContents(4);
  list->resizeColumnToContents(5);
  list->resizeColumnToContents(6);
}

/**
 * Updates the internal task data. Will be called after
 * configuration changes of task sector items
 */
void TaskListView::slot_updateTask()
{
  if ( _task )
    {
      _task->updateTask();
      _task->updateProjection();
    }
}

/** Returns a pointer to the currently highlighted taskpoint. */
wayPoint * TaskListView::getSelectedWaypoint()
{
  return _selectedWp;
}

void TaskListView::clear()
{
  list->clear();
  wind->setText("");
  distTotal->setText("");
  speedTotal->setText("");
  timeTotal->setText("");
}

TaskListView::_TaskPoint::_TaskPoint (QTreeWidget *tpList, TaskPoint *point ) : QTreeWidgetItem(tpList)
{
  tp = point;
  QString typeName = tp->getTaskPointTypeString();

  // calculate sunset for the task points
  QString sr, ss, tz;
  QDate date = QDate::currentDate();
  Sonne::sonneAufUnter( sr, ss, date, tp->origP, tz );

  setText(0, typeName);
  setText(1, tp->name);
  setText(2, " " + Distance::getText(tp->distance*1000,false,1));
  setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );

  if ( tp->bearing == -1.0 )
    {
      // bearing is undefined
      setText(3, " ");
    }
  else
    {
      int bearing = (int) rint( tp->bearing*180./M_PI );
      setText(3, " " + QString::number( bearing ) + QString(Qt::Key_degree));
    }

  setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );

  setText(4, " " + FlightTask::getDistanceTimeString(tp->distTime));
  setTextAlignment( 4, Qt::AlignRight|Qt::AlignVCenter );
  setText(5, " " + ss + " " + tz);
  setText(6, " " + tp->description);

  setIcon(1, QIcon(_globalMapConfig->getPixmap(tp->type, false, true)) );
}
