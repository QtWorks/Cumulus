/***********************************************************************
**
**   taskdialog.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   Description: This class handles creation and modification of
**   flight tasks.
**
***********************************************************************/

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "airport.h"
#include "taskdialog.h"
#include "mapcontents.h"
#include "flighttask.h"
#include "distance.h"
#include "generalconfig.h"
#include "cumulusapp.h"

extern MapContents *_globalMapContents;
extern CumulusApp  *_globalCumulusApp;

TaskDialog::TaskDialog( QWidget* parent, const char* name, QStringList &taskNamesInUse,
                        FlightTask* task ) :
    QDialog( parent, name, false, Qt::WStyle_StaysOnTop ),
    taskNamesInUse( taskNamesInUse )
{
  wpList = _globalMapContents->getWaypointList();
  taskWPList.setAutoDelete(true);
  lastSelectedItem = -1;

  if( task )
    {
      planTask = task;
      editState = TaskDialog::edit;
      this->setWindowTitle(planTask->getTaskTypeString());
      editedTaskName = task->getTaskName();
    }
  else
    {
      planTask = new FlightTask( 0, false, "" );
      editState = TaskDialog::create;
      this->setWindowTitle(tr("New Task"));
    }

  taskName = new QLineEdit( this );
  taskName->setBackgroundMode( Qt::PaletteLight );

  taskList = new Q3ListView( this, "taskList" );
  taskList->addColumn( tr("Id"), 16 );
  taskList->addColumn( tr("Type"), 45 );
  taskList->addColumn( tr("Waypoint"), 90 );
  taskList->addColumn( tr("Distance"), 60 );

  taskList->setColumnAlignment( 3, Qt::AlignRight );

  taskList->setMinimumHeight( 100 );
  taskList->setMaximumHeight( 120 );
  taskList->setAllColumnsShowFocus( true );

  QPushButton* upButton = new QPushButton( this );
  upButton->setPixmap( GeneralConfig::instance()->loadPixmap( "moveup.png") );
  upButton->setFlat(true);

  QPushButton* downButton = new QPushButton( this );
  downButton->setPixmap( GeneralConfig::instance()->loadPixmap( "movedown.png") );
  downButton->setFlat(true);

  QPushButton* invertButton = new QPushButton( this );
  invertButton->setPixmap( GeneralConfig::instance()->loadPixmap( "invert.png") );
  invertButton->setFlat(true);

  QPushButton* addButton = new QPushButton( this );
  addButton->setPixmap( GeneralConfig::instance()->loadPixmap( "moveup.png") );
  addButton->setFlat(true);

  QPushButton* delButton = new QPushButton( this );
  delButton->setPixmap( GeneralConfig::instance()->loadPixmap( "movedown.png") );
  delButton->setFlat(true);

  QGridLayout* taskLayout = new QGridLayout( this, 12, 8, 2, 1);

  taskLayout->addMultiCellWidget( new QLabel( tr("Task Name:"), this ), 0, 0, 0, 0 );
  taskLayout->addMultiCellWidget( taskName, 0, 0, 1, 6 );
  taskLayout->addMultiCellWidget( taskList, 2, 6, 0, 6 );
  taskLayout->addWidget( invertButton, 3, 7, Qt::AlignCenter );
  taskLayout->addWidget( upButton, 4, 7, Qt::AlignCenter );
  taskLayout->addWidget( downButton, 5, 7, Qt::AlignCenter );

  listSelectCB = new QComboBox(false, this);
  listSelectText[0] = tr("Waypoints");
  listSelectText[1] = tr("Airfields");

  for(int i=0; i<NUM_LISTS; i++) {
    listSelectCB->insertItem(listSelectText[i], i);
    waypointList[i] = new Q3ListView( this, "waypointList" );
    waypointList[i]->addColumn( tr("Name") );
    waypointList[i]->addColumn( tr("Description") );
    waypointList[i]->addColumn( tr("ICAO") );
    waypointList[i]->setAllColumnsShowFocus( true );
    waypointList[i]->setFocus();
    filter[i] = new ListViewFilter(waypointList[i], this, "listfilter");
    taskLayout->addMultiCellWidget( filter[i], 10, 10, 0, 7 );
    taskLayout->addMultiCellWidget( waypointList[i], 11, 11, 0, 7 );
  }
  _globalCumulusApp->viewWP->fillWpList(wpList, waypointList[0], filter[0]);
  _globalCumulusApp->viewAF->fillWpList(waypointList[1], filter[1]);

  taskLayout->addWidget( listSelectCB, 8, 0 );
  taskLayout->addWidget( addButton, 8, 2, Qt::AlignCenter  );
  taskLayout->addWidget( delButton, 8, 3, Qt::AlignCenter  );

  // first selection is WPList if wp's are defined
  slotToggleList(wpList->count() ? 0 : 1);

  taskLayout->addRowSpacing( 7, 2 );
  taskLayout->addRowSpacing( 9, 2 );

  if( editState == TaskDialog::edit )
    {
      taskName->setText( planTask->getTaskName() );

      Q3PtrList<wayPoint> tmpList = planTask->getWPList();

      // @AP: Make a deep copy from all elements of the list
      for( uint i=0; i < tmpList.count(); i++ )
        {
          taskWPList.append( new wayPoint(*tmpList.at(i)) );
        }

      __showTask();
    }

  connect( addButton,    SIGNAL( clicked() ),
           this, SLOT( slotAddWaypoint() ) );
  connect( delButton,    SIGNAL( clicked() ),
           this, SLOT( slotRemoveWaypoint() ) );
  connect( upButton,     SIGNAL( clicked() ),
           this, SLOT( slotMoveWaypointUp() ) );
  connect( downButton,   SIGNAL( clicked() ),
           this, SLOT( slotMoveWaypointDown() ) );
  connect( invertButton, SIGNAL( clicked() ),
           this, SLOT( slotInvertWaypoints() ) );
  connect( listSelectCB, SIGNAL(activated(int)),
           this, SLOT(slotToggleList(int)));

  showMaximized();
}


TaskDialog::~TaskDialog()
{
  // qDebug("TaskDialog::~TaskDialog()");
  for(int i=0; i<NUM_LISTS; i++) {
    waypointList[i]->clear();
  }
}

void TaskDialog::__showTask()
{
  if( taskWPList.count() == 0 )
    {
      this->setWindowTitle(tr("New Task"));
      return;
    }

  planTask->setWaypointList( FlightTask::copyWpList( &taskWPList ) );

  QString txt = planTask->getTaskTypeString() +
                " / " + planTask->getTaskDistanceString();

  this->setWindowTitle(txt);

  Q3PtrList<wayPoint> tmpList = planTask->getWPList();

  taskList->clear();

  QString typeName, distance, idString;

  for(unsigned int loop = 0; loop < tmpList.count(); loop++)
    {
      wayPoint* wp = tmpList.at( loop );
      typeName = wp->getTaskPointTypeString();

      distance = Distance::getText(wp->distance*1000, true, 1);
      idString.sprintf( "%02d", loop );

      Q3ListViewItem *newLvi =
        new Q3ListViewItem( taskList,
                            idString, typeName, wp->name, distance );

      taskList->insertItem( newLvi );

      // reselect last selected item
      if( lastSelectedItem == (int) loop )
        {
          taskList->setSelected( newLvi, true );
          lastSelectedItem = -1;
        }
    }
}


void TaskDialog::slotAddWaypoint()
{
  if( waypointList[listSelectCB->currentItem()]->selectedItem() == 0 )
    {
      return;
    }

  if(listSelectCB->currentItem() == 0)
    {
      wayPoint *wp = _globalCumulusApp->viewWP->getSelectedWaypoint(waypointList[listSelectCB->currentItem()]);
      // if this is not a copy, the global wpList gets corrupted
      // since taskWPList has setAutoDelete to true
      taskWPList.append( new wayPoint(*wp) );
    }
  else if(listSelectCB->currentItem() == 1)
    {
      wayPoint *wp = _globalCumulusApp->viewAF->getSelectedAirfield(waypointList[listSelectCB->currentItem()]);
      // if this is not a copy, all taskpoints are the same since the
      // returned wp is (always the same) local variable of AirfieldListView
      taskWPList.append( new wayPoint(*wp) );
    }

  __showTask();
}


void TaskDialog::slotRemoveWaypoint()
{
  if( taskList->selectedItem() == 0 )
    {
      return;
    }

  int id( taskList->selectedItem()->text(0).toInt() );

  taskWPList.remove( id );
  taskList->takeItem( taskList->selectedItem() );

  __showTask();
}


void TaskDialog::slotInvertWaypoints()
{
  if( taskWPList.count() < 2 )
    {
      // not possible to invert order, if elements are less 2
      return;
    }

  // invert list order
  for( int i= (int) taskWPList.count()-2; i >= 0; i-- )
    {
      taskWPList.append( taskWPList.take(i) );
    }

  __showTask();
}


void TaskDialog::accept()
{
  // qDebug("TaskDialog::accept()");

  // Check, if a sensefull task has been defined. Tasks with less as
  // four waypoints are incomplete and will be ignored.
  if( taskWPList.count() < 4 )
    {
      QMessageBox::warning(this,tr("Task incomplete!"),
                           tr("<qt>"
                              "Task is incomplete<br>"
                              "Please add some more waypoints!"
                              "</qt>"),
                           QMessageBox::Ok | QMessageBox::Default, 0 );
      return;
    }

  QString txt = taskName->text();

  // Check, if the user has entered a task name. Nameless task will
  // be ignored.
  if( txt.length() == 0 )
    {
      QMessageBox::warning(this,tr("Missing name?"),
                           tr("<qt>"
                              "Please enter a task name before exit!"
                              "</qt>"),
                           QMessageBox::Ok | QMessageBox::Default, 0 );
      return;
    }


  if( editState == TaskDialog::create )
    {
      // Check the name of the task, if it does not conflictct with
      // exististing onces. The name must be unique in the task name
      // space.

      if( taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::warning(this,tr("Name already in use!"),
                               tr("<qt>"
                                  "Task name already in use<br>"
                                  "Please enter another name before exit!"
                                  "</qt>"),
                               QMessageBox::Ok | QMessageBox::Default, 0 );
          return;
        }
    }
  else
    {
      // Check, if the name of the edited task has been changed. In
      // this case we have to look, if the new name is unique in the
      // task name space.

      if( txt != editedTaskName && taskNamesInUse.contains( txt ) > 0 )
        {
          QMessageBox::warning(this,tr("Name already in use!"),
                               tr("<qt>"
                                  "Task name already in use<br>"
                                  "Please enter another name before exit!"
                                  "</qt>"),
                               QMessageBox::Ok | QMessageBox::Default, 0 );
          return;
        }
    }

  // Overtake changed task data and publish it
  QDialog::accept();
  planTask->setTaskName(txt);

  if( editState == TaskDialog::create )
    {
      emit newTask( planTask );
    }
  else
    {
      emit editedTask( planTask );
    }

  planTask = 0;
  delete this;
}


void TaskDialog::reject()
{
  // qDebug("TaskDialog::reject()");
  QDialog::reject();

  delete planTask;
  delete this;
}


void TaskDialog::slotMoveWaypointUp()
{
  if( taskList->selectedItem() == 0 || taskList->childCount() <= 2 )
    return;

  unsigned int id( taskList->selectedItem()->text( 0 ).toInt() );
  // we can't  move the first item
  if( id == 0 )
    return;

  lastSelectedItem = id - 1;

  wayPoint* tmp = taskWPList.take( id );

  taskWPList.insert( id - 1, tmp );

  __showTask();
}


void TaskDialog::slotMoveWaypointDown()
{
  if( taskList->selectedItem() == 0 || taskList->childCount() <= 2 )
    return;

  int id( taskList->selectedItem()->text( 0 ).toInt() );
  // we can't move the last item
  if( id == taskList->childCount() - 1 )
    return;

  lastSelectedItem = id + 1;

  wayPoint* tmp = taskWPList.take( id );

  taskWPList.insert( id + 1, tmp );

  __showTask();
}

/** Toggle between WP/AF/... list on user request */
void TaskDialog::slotToggleList(int index)
{
  for(int i=0; i<NUM_LISTS; i++) {
    if(i != index) {
      waypointList[i]->hide();
      filter[i]->hide();
    } else {
      waypointList[i]->show();
      filter[i]->show();
    }
  }
}
