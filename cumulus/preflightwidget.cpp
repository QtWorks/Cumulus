/***********************************************************************
 **
 **   preflightwidget.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2003      by André Somers
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "flickcharm.h"
#include "map.h"
#include "mapcontents.h"
#include "preflightgliderpage.h"
#include "preflightwidget.h"
#include "preflighttasklist.h"
#include "preflightmiscpage.h"
#include "preflightwaypointpage.h"

#include "calculator.h"
#include "layout.h"

extern MapContents* _globalMapContents;

PreFlightWidget::PreFlightWidget(QWidget* parent, const char* name) :
  QWidget(parent)
{
  // qDebug("PreFlightWidget::PreFlightWidget()");
  setObjectName("PreFlightWidget");
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Preflight settings"));

  tabWidget = new QTabWidget(this);
  tabWidget->setTabPosition(QTabWidget::West);

  gliderpage = new PreFlightGliderPage(this);
  tabWidget->addTab(gliderpage, "");

  taskpage = new PreFlightTaskList(this);
  tabWidget->addTab(taskpage, "");

  wppage = new PreFlightWaypointPage(this);

  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( wppage );
#ifdef QSCROLLER
  QScroller::grabGesture(sa, QScroller::LeftMouseButtonGesture);
#else
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(sa);
#endif

  tabWidget->addTab(sa, "");

  connect( wppage, SIGNAL(waypointsAdded()),
            Map::getInstance(), SLOT(slotRedraw()) );

  miscpage = new PreFlightMiscPage(this);

  sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( miscpage );
#ifdef QSCROLLER
  QScroller::grabGesture(sa, QScroller::LeftMouseButtonGesture);
#else
  flickCharm = new FlickCharm(this);
  flickCharm->activateOn(sa);
#endif

  tabWidget->addTab(sa, "");

  QShortcut* scLeft = new QShortcut(Qt::Key_Left, this);
  QShortcut* scRight = new QShortcut(Qt::Key_Right, this);

  connect(scLeft, SIGNAL(activated()), this, SLOT(slot_keyLeft()));
  connect(scRight, SIGNAL(activated()), this, SLOT(slot_keyRight()));

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("preflight.png"));

  connect(ok, SIGNAL(released()), this, SLOT(slot_accept()));
  connect(cancel, SIGNAL(released()), this, SLOT(slot_reject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);

  QHBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addWidget(tabWidget);
  contentLayout->addLayout(buttonBox);

  setLayout(contentLayout);

  miscpage->load();
  wppage->load();

  // check to see which tabulator to bring forward
  if (QString(name) == "taskselection")
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(taskpage));
      lastPage = 1;
    }
  else
    {
      tabWidget->setCurrentIndex(tabWidget->indexOf(gliderpage));
      lastPage = 0;
    }

  connect( tabWidget, SIGNAL(currentChanged( int)),
            SLOT(slot_tabChanged(int)) );

  setLabels();
  setVisible( true );
}

PreFlightWidget::~PreFlightWidget()
{
  // qDebug("PreFlightWidget::~PreFlightWidget()");
}

/** Sets all widget labels, which need a translation. */
void PreFlightWidget::setLabels()
{
  tabWidget->setTabText( 0, tr("Glider") );
  tabWidget->setTabText( 1, tr("Task") );
  tabWidget->setTabText( 2, tr("Waypoints") );
  tabWidget->setTabText( 3, tr("Common") );
}

/** Used to handle language change events */
void PreFlightWidget::changeEvent( QEvent* event )
{
  if( event->type() == QEvent::LanguageChange )
    {
      setLabels();
    }
  else
    {
      QWidget::changeEvent( event );
    }
}

void PreFlightWidget::keyReleaseEvent( QKeyEvent* event )
{
  // close the dialog on key press
  switch(event->key())
    {
      case Qt::Key_Close:
      case Qt::Key_Escape:
        slot_reject();
        break;

      default:
        QWidget::keyReleaseEvent( event );
        break;
    }
}

void PreFlightWidget::slot_tabChanged( int index )
{
  // Save done changes on glider page when it is left to have them available
  // on task page. E.g. Flarm IGC setup page. The method save is only called,
  // if the task page is selected as new page after the glider page.
  if( index == 1 && lastPage == 0 )
    {
      gliderpage->save();
    }

  lastPage = index;
}

void PreFlightWidget::slot_accept()
{
  FlightTask *curTask = _globalMapContents->getCurrentTask();

  // Note we have overtaken the ownership about this object!
  FlightTask *newTask = taskpage->takeSelectedTask();

  bool newTaskPassed = true;

  // Check, if a new task has been passed for accept.
  if (curTask && newTask && curTask->getTaskName() == newTask->getTaskName())
    {
      newTaskPassed = false; // task names identical
    }

  if (curTask && newTask && newTaskPassed)
    {
      QMessageBox mb( QMessageBox::Question,
                      tr( "Replace current task?" ),
                      tr( "<html>"
                          "Do you want to replace the current task?<br>"
                          "A selected target is reset to task start."
                          "</html>" ),
                      QMessageBox::Yes | QMessageBox::No,
                      this );

      mb.setDefaultButton( QMessageBox::No );

    #ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

    #endif


      if( mb.exec() != QMessageBox::Yes )
        {
          // do nothing change
          delete newTask;
          slot_reject();
          return;
        }
    }

  // Forward of new task in every case, user can have modified
  // content. MapContent will overtake the ownership of the task
  // object.
  _globalMapContents->setCurrentTask(newTask);

  // @AP: Open problem with waypoint selection, if user has modified
  // task content. We ignore that atm.

  if ( newTask == static_cast<FlightTask *> (0) )
    {
      // No new task has been passed. Check, if a selected waypoint
      // exists and this waypoint belongs to a task. In this case we
      // will reset the selection.
      extern Calculator* calculator;
      const Waypoint *calcWp = calculator->getselectedWp();

      if (calcWp && calcWp->taskPointIndex != -1)
        {
          // reset taskpoint selection
          emit newWaypoint(static_cast<Waypoint *> (0), true);
        }
    }
  else
    {
      extern Calculator* calculator;

      // If a waypoint selection exists, we do overwrite it with the begin
      // point of the new flight task.
      if( calculator->getselectedWp() )
        {
          // Reset taskpoint selection in calculator to prevent user query.
          emit newWaypoint(static_cast<Waypoint *> (0), true);

          // Select the start point of the new task.
          calculator->slot_startTask();
        }

      // Inform others about the new task
      emit newTaskSelected();
    }

  gliderpage->save();
  miscpage->save();
  wppage->save();
  GeneralConfig::instance()->save();

  setVisible( false );
  emit settingsChanged();
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slot_reject()
{
  // qDebug("PreFlightWidget::slot_reject()");
  setVisible( false );
  emit closeConfig();
  QWidget::close();
}

void PreFlightWidget::slot_keyRight()
{
  int tabCount = tabWidget->count();
  int curIndex = tabWidget->currentIndex();

  if( tabCount == 0 || curIndex == -1 )
    {
      return;
    }

  int nextIndex = curIndex + 1;

  if( nextIndex >= tabCount )
    {
      nextIndex = 0;
    }

  tabWidget->setCurrentIndex( nextIndex );
}

void PreFlightWidget::slot_keyLeft()
{
  int tabCount = tabWidget->count();
  int curIndex = tabWidget->currentIndex();

  if( tabCount == 0 || curIndex == -1 )
    {
      return;
    }

  int nextIndex = curIndex - 1;

  if( nextIndex < 0 )
    {
      nextIndex = tabCount - 1;
    }

  tabWidget->setCurrentIndex( nextIndex );
}
