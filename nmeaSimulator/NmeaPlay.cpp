/***************************************************************************
                          NmeaPlay.cpp - description
                             -------------------
    begin                : 11.01.2012

    copyright            : (C) 2012-2014 by Axel Pauli

    email                : kflog.cumulus@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <iostream>

#include <QtCore>

#include "NmeaPlay.h"

int NmeaPlay::startPlaying( const int skip, const int pause )
{
  int m_skip = skip;
  m_pause = pause;

  if( m_fileName.isEmpty() )
    {
      qWarning() << "Play::startPlaying: No file name is defined!";
      return -1;
    }

  QFile file(m_fileName);

  if( ! file.open(QIODevice::ReadOnly) )
    {
      qWarning() << "Play::startPlaying: Cannot open file" << m_fileName;
      return -1;
    }

  QTextStream inStream(&file);

  while( ! inStream.atEnd() )
    {
      QString line = inStream.readLine().trimmed();

      if( m_skip > 0 )
        {
          m_skip--;
          continue;
        }

      if( line.isEmpty() )
        {
          continue;
        }

      line += "\r\n";

      ssize_t written = write( m_fifo, line.toLatin1().data(), line.length() );

      std::cout << line.toLatin1().data();

      if( line.startsWith("$GPRMC") )
        {
          // make a break after this sentence
          usleep( m_pause * 1000 );
        }
    }

  file.close();

  return 0;
}
