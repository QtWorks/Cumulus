/***********************************************************************
 **
 **   filetools.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by Andr� Somers, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QRect>

#include "filetools.h"
#include "resource.h"

/** Save a QByteArray to a stream with only one leading byte instead of 4. */
void ShortSave (QDataStream &s, const QByteArray &str )
{
  if( str.size() > 255 )
    {
      qWarning( "ShortSave will truncate QByteArray to 255 characters!" );
    }

  qint8 len;
  len=MIN(str.size(), 255);
  s << len;
  s.writeRawData( str.data(), len );
}


/** Save a QString to a stream with only one leading byte instead of 4. */
void ShortSave (QDataStream &s, const QString  &str )
{
  //this is basicly a wrapper around the above function, provided for conveneance.
  ShortSave(s, str.toUtf8());   //just use the QCString version
  return;
}


/** Load a QByteArray from a stream with only one leading byte instead of 4. */
void ShortLoad (QDataStream &s, QByteArray &str)
{
  str.detach();
  qint8 len;
  s >> len;			    // read size of string

  if ( len == 0 || s.atEnd() ) {  // end of file reached
    str.resize( 0 );
    return;
  }

  str.resize( len );
  s.readRawData( str.data(), len );
  return;
}


/** Load a QString from a stream with only one leading byte instead of 4. */
void ShortLoad (QDataStream &s, QString &str)
{
  //this is basicly a wrapper around the above function, provided for conveneance.
  QByteArray tmp;
  ShortLoad(s, tmp);
  str=QString::fromUtf8(tmp);
  return;
}


/** Save a QPolygon to a stream in a shorter form. */
void ShortSave (QDataStream & s, const QPolygon& a)
{
  //first, check if we _can_ do this the short way...
  QRect bBox=a.boundingRect();
  if (bBox.width()<=254 && bBox.height()<=254) {
    //qDebug("using  8 bits format");
    //ok, we can do our own saving in 8 bits format to save space
    s << (qint8)1; //set flag to say we used this format :-)
    QPoint topLeft=bBox.topLeft();  //get the coordinates of the top left of the box
    s << topLeft;
    QPolygon ca(a);
    ca.translate(-topLeft.x(),-topLeft.y()); // translate the box so it's top left on (0,0)
    //now, all points in the array fit into 16 bits!
    s << (quint32)ca.count();
    for (int i=0; i<ca.count(); i++) {
      s << (quint8)ca.point(i).x();
      s << (quint8)ca.point(i).y();
    }
  } else if (bBox.width()<65500 && bBox.height()<65500) {
    //qDebug("using 16 bits format");
    //ok, we can do our own saving in 16 bits format to save space
    s << (qint8)2; //set flag to say we used this format :-)
    QPoint topLeft=bBox.topLeft();  //get the coordinates of the top left of the box
    s << topLeft;
    QPolygon ca(a);
    ca.translate(-topLeft.x(),-topLeft.y()); // translate the box so it's top left on (0,0)
    //now, all points in the array fit into 32 bits!
    s << (quint32)ca.count();
    for (int i=0; i<ca.count(); i++) {
      s << (quint16)ca.point(i).x();
      s << (quint16)ca.point(i).y();
    }
  } else {
    //qDebug("using long 32 bits format.");
    //too big. We need to use the normal 2x32 bits format :-(
    s << (qint8)4; //we need to set a flag that we use the normal format
    s << a;
  }
}


/** Load a QPolygon in a short form from a stream. */
void ShortLoad (QDataStream & s, QPolygon& a)
{
  qint8 ar_type;
  QPoint topLeft;
  quint32 pointcount;
  quint16 px2, py2;
  quint8 px1, py1;

  s >> ar_type;      //the type is basicly the number of bytes used per coordinate-part
  if (ar_type==4) {
    //the array was stored as a standard QPointArray
    s >> a;
  } else if (ar_type==2) {
    //the array was stored using a top left point + 16 bit points
    s >> topLeft;
    s >> pointcount;
    a.resize(pointcount);
    for (uint i=0; i<pointcount; i++) {
      s >> px2;
      s >> py2;
      a.setPoint(i, px2,py2);
    }
    a.translate(topLeft.x(),topLeft.y()); //translate back to original coordinates
  } else if (ar_type==1) {
    //the array was stored using a top left point + 8 bit points
    s >> topLeft;
    s >> pointcount;
    a.resize(pointcount);

    for (uint i=0; i<pointcount; i++) {
      s >> px1;
      s >> py1;
      a.setPoint(i, px1,py1);
    }
    a.translate(topLeft.x(),topLeft.y()); //translate back to original coordinates
  } else  {
    // WOW!!! we don't know what this is!
    qWarning( "ShortLoad: Unknown polygon format!" );
  }
}
