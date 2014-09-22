/***********************************************************************
**
**   openAipLoaderThread.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <csignal>

#include <QtCore>

#include "airfield.h"
#include "openAipPoiLoader.h"
#include "openAipLoaderThread.h"
#include "radiopoint.h"

OpenAipLoaderThread::OpenAipLoaderThread( QObject *parent,
                                          enum Poi poiSource,
                                          bool readSource ) :
  QThread( parent ),
  m_poiSource(poiSource),
  m_readSource(readSource)
{
  setObjectName( "OpenAipLoaderThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

OpenAipLoaderThread::~OpenAipLoaderThread()
{
}

void OpenAipLoaderThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  if( m_poiSource == Airfields )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedAfList( int, QList<Airfield>* )) ) == 0 )
	{
	  qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedAfList!";
	  return;
	}
    }
  else if( m_poiSource == NavAids )
    {
      // Check if signal is connected to a slot.
      if( receivers( SIGNAL( loadedNavAidsList( int, QList<RadioPoint>* )) ) == 0 )
	{
	  qWarning() << "OpenAipLoaderThread: No Slot connection to Signal loadedNavAidsList!";
	  return;
	}
    }

  int ok = false;

  if( m_poiSource == Airfields )
    {
      QList<Airfield>* poiList = new QList<Airfield>;
      OpenAipPoiLoader oaipl;

      ok = oaipl.load( *poiList, m_readSource );

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed lists. Otherwise a big
       * memory leak will occur.
       */
      emit loadedAfList( ok, poiList );
    }
  else if( m_poiSource == NavAids )
    {
      QList<RadioPoint>* poiList = new QList<RadioPoint>;

      // TODO call loading of NavAid list

      /* It is expected that a receiver slot is connected to this signal. The
       * receiver is responsible to delete the passed lists. Otherwise a big
       * memory leak will occur.
       */
      emit loadedNavAidsList( ok, poiList );
    }
}
