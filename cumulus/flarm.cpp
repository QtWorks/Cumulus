/***********************************************************************
**
**   flarm.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2015 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include <QtCore>

#include "altitude.h"
#include "flarm.h"
#include "flarmdisplay.h"
#include "flarmaliaslist.h"
#include "generalconfig.h"
#include "layout.h"

Flarm::Flarm(QObject* parent) : QObject(parent), FlarmBase()
{
  // Load Flarm alias data
  FlarmAliasList::loadAliasData();

  // Setup timer for data clearing
  m_timer = new QTimer( this );
  m_timer->setSingleShot( true );
  connect( m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()) );
}

Flarm::~Flarm()
{
}

/**
 * Extracts all items from $PFLAU sentence from Flarm device.
 */
bool Flarm::extractPflau( const QStringList& stringList )
{
  m_flarmStatus.valid = false;

  if ( stringList[0] != "$PFLAU" || stringList.size() < 11 )
    {
      qWarning("$PFLAU contains too less parameters!");
      return false;
    }

  /**
    $PFLAU,<RX>,<TX>,<GPS>,<Power>,<AlarmLevel>,<RelativeBearing>,<AlarmType>,
    <RelativeVertical>,<RelativeDistance>,<ID>
  */

  bool ok;
  short value;

  // RX number of received devices
  m_flarmStatus.RX = 0;
  value = stringList[1].toShort( &ok );

  if( ok )
    {
      m_flarmStatus.RX = value;
    }

  // TX Transmission status
  m_flarmStatus.TX = 0;
  value = stringList[2].toShort( &ok );

  if( ok )
    {
      m_flarmStatus.TX = value;
    }

  // GPS status
  m_flarmStatus.Gps = NoFix;
  value = stringList[3].toShort( &ok );

  if( ok )
    {
      m_flarmStatus.Gps = static_cast<enum GpsStatus> (value);
    }

  // Power status
  m_flarmStatus.Power = 0;
  value = stringList[4].toShort( &ok );

  if( ok )
    {
      m_flarmStatus.Power = value;
    }

  // AlarmLevel
  value = stringList[5].toShort( &ok );
  m_flarmStatus.Alarm = No;

  if( ok )
    {
      m_flarmStatus.Alarm = static_cast<enum AlarmLevel> (value);
    }

  // RelativeBearing
  m_flarmStatus.RelativeBearing = stringList[6];

  // AlarmType
  value = stringList[7].toShort( &ok );
  m_flarmStatus.AlarmType = 0;

  if( ok )
    {
      m_flarmStatus.AlarmType = value;
    }

  // RelativeVertical
  m_flarmStatus.RelativeVertical = stringList[8];

  // RelativeDistance
  m_flarmStatus.RelativeDistance = stringList[9];

  // ID 6-digit hex value
  m_flarmStatus.ID = stringList[10];

  m_flarmStatus.valid = true;

  if( m_flarmStatus.Alarm != No && m_flarmStatus.AlarmType != 0 &&
      m_flarmStatus.RelativeBearing.isEmpty() == false &&
      GeneralConfig::instance()->getPopupFlarmAlarms() == true )
    {
      createTrafficMessage();
    }

  return true;
}

/**
 * Extracts all items from the $PFLAA sentence sent by the Flarm device.
 */
bool Flarm::extractPflaa( const QStringList& stringList, FlarmAcft& aircraft )
{
  if ( stringList[0] != "$PFLAA" || stringList.size() < 12 )
    {
      qWarning("$PFLAA contains too less parameters!");
      return false;
    }

  /**
    00: $PFLAA,
    01: <AlarmLevel>,
    02: <RelativeNorth>,
    03: <RelativeEast>,
    04: <RelativeVertical>,
    05: <IDType>,
    06: <ID>,
    07: <Track>,
    08: <TurnRate>,
    09: <GroundSpeed>,
    10: <ClimbRate>,
    11: <AcftType>
  */

  bool ok;

  aircraft.TimeStamp = QTime::currentTime();

  // AlarmLevel
  aircraft.Alarm = static_cast<enum AlarmLevel> (stringList[1].toInt( &ok ));

  if( ! ok )
    {
      aircraft.Alarm = No;
    }

  aircraft.RelativeNorth = stringList[2].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeNorth = INT_MIN;
    }

  aircraft.RelativeEast = stringList[3].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeEast = INT_MIN;
    }

  aircraft.RelativeVertical = stringList[4].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeVertical = INT_MIN;
    }

  aircraft.IdType = stringList[5].toShort( &ok );

  if( ! ok )
    {
      aircraft.IdType = 0;
    }

  aircraft.ID = stringList[6];

  // 0-359 or INT_MIN in stealth mode
  aircraft.Track = stringList[7].toInt( &ok );

  if( ! ok )
    {
      aircraft.Track = INT_MIN;
    }

  // degrees per second or INT_MIN in stealth mode
  aircraft.TurnRate = stringList[8].toDouble( &ok );

  if( ! ok )
    {
      aircraft.TurnRate = INT_MIN;
    }

  // meters per second or INT_MIN in stealth mode
  aircraft.GroundSpeed = stringList[9].toDouble( &ok );

  if( ! ok )
    {
      aircraft.GroundSpeed = INT_MIN;
    }

  // meters per second or INT_MIN in stealth mode
  aircraft.ClimbRate = stringList[10].toDouble( &ok );

  if( ! ok )
    {
      aircraft.ClimbRate = INT_MIN;
    }

  aircraft.AcftType = stringList[11].toShort( &ok );

  if( ! ok )
    {
      aircraft.AcftType = 0; // unknown
    }

  // Check, if parsed data should be collected. In this case the data record
  // is put or updated in the pflaaHash hash dictionary.
  QString key = createHashKey( aircraft.IdType, aircraft.ID );

  if( m_collectPflaa == true || key == FlarmDisplay::getSelectedObject() )
    {
      // first check, if record is already contained in the hash.
      if( m_pflaaHash.contains( key ) == true )
        {
          // update entry
          FlarmAcft& aircraftEntry = m_pflaaHash[key];
          aircraftEntry = aircraft;
        }
      else
        {
          // insert new entry
          m_pflaaHash.insert( key, aircraft );
        }
    }

  return true;
}

bool Flarm::extractPflav(const QStringList& stringList)
{
  if ( stringList[0] != "$PFLAV" || stringList.size() < 5 )
     {
       qWarning("$PFLAV contains too less parameters!");
       return false;
     }

  /**
   PFLAV,<QueryType>,<HwVersion>,<SwVersion>,<ObstVersion>
   $PFLAV,A,2.00,5.00,alps20110221_*
  */
  m_flarmVersion.hwVersion   = stringList[2];
  m_flarmVersion.swVersion   = stringList[3];
  m_flarmVersion.obstVersion = stringList[4];

  emit flarmVersionInfo( m_flarmVersion );
  return true;
}

bool Flarm::extractPflae(const QStringList& stringList)
{
  /**
   * PFLAE,<QueryType>,<Severity>,<ErrorCode>(,<Message>)
   *
   * <QueryType>
      R = request FLARM to send status, disregard other fields then
      A = FLARM sends status (requested and spontaneous)

     <Severity>
      0 = no error, i.e. normal operation, disregard other fields then
      1 = information only, i.e. normal operation
      2 = functionality may be reduced
      3 = fatal problem, device will not work

     <ErrorCode>
      Two digit hex value
      11 = Firmware timeout (requires valid GPS information, i.e. will not be
           available in the first minute after power-on)
      21 = Power (e.g. voltage < 8V, might occur during operations)
      31 = GPS communication
      32 = Configuration of GPS module
      41 = RF communication
      51 = Communication
      61 = Flash memory
      71 = Pressure sensor
      81 = Obstacle database
      91 = Flight recorder A1 = Transponder receiver
      F1 = Other

    <Message>
      Textual description of error on Flarm version 7 onwards. Maybe omitted
      or empty.

     $PFLAE,A,2,81*
   */

  if( stringList[0] != "$PFLAE" || stringList.size() < 4 )
    {
      qWarning("$PFLAE contains too less parameters!");
      return false;
    }

  m_flarmError.severity  = stringList[2];
  m_flarmError.errorCode = stringList[3];

  if( stringList.size() >= 5 )
    {
      m_flarmError.errorText = stringList[4];
    }
  else
    {
      m_flarmError.errorText.clear();
    }

  emit flarmErrorInfo( m_flarmError );
  return true;
}

bool Flarm::extractPflac(QStringList& stringList)
{
  /**
   * PFLAC,<QueryType>,<Key>,<Value>
   *
   * Attention, response can be "$PFLAC,A,ERROR*", That means the stringList
   * has only 3 elements!
   */
  if ( stringList[0] != "$PFLAC" || stringList.size() < 3 )
      {
        qWarning("$PFLAC contains too less parameters!");
        return false;
      }

  emit flarmConfigurationInfo( stringList );
  return true;
}

bool Flarm::extractPflar(QStringList& stringList)
{
  /**
   * PFLAR,<QueryType>
   *
   * $PFLAR,A*24
   *
   * Answer send by Flarm after a reset command.
   */
  if ( stringList[0] != "$PFLAR" || stringList.size() < 2 )
      {
        qWarning("$PFLAR contains too less parameters!");
        return false;
      }

  emit flarmResetResponse( stringList );
  return true;
}

bool Flarm::extractPflai(QStringList& stringList)
{
  /**
   * PFLAI,<IGC Command>
   *
   * $PFLAI,IGCREADOUT
   *   $PFLAI,IGCREADOUT,ERROR,<ERROR>
   *   $PFLAI,IGCREADOUT,OK
   *
   * $PFLAI,PILOTEVENT
   *   $PFLAI,PILOTEVENT,OK
   *
   * Answer send by Flarm after an IGC command.
   */
  if ( stringList[0] != "$PFLAI" || stringList.size() < 2 )
      {
        qWarning("$PFLAI contains too less parameters!");
        return false;
      }

  emit flarmIgcResponse( stringList );
  return true;
}

bool Flarm::extractPflao(QStringList& stringList)
{
  /**
   * 00: PFLAO,
   * 01: <AlarmLevel>,
   * 02: <Inside>,
   * 03: <Latitude>,
   * 04: <Longitude>,
   * 05: <Radius>,
   * 06: <Bottom>,
   * 07: <Top>,
   * 08: <ActivityLimit>,
   * 09: <ID>,
   * 10: <ID-Type>,
   * 11: <ZoneType>
   *
   * Alert Zone warnings.
   */
  if( stringList[0] != "$PFLAO" || stringList.size() < 12 )
    {
      qWarning("$PFLAO contains too less parameters!");
      return false;
    }

  bool ok = false;

  FlarmAlertZone faz;

  // Set structure as valid
  faz.valid = true;

  // 1: Alarm Level
  faz.Alarmlevel = static_cast<enum AlarmLevel> (stringList[1].toInt( &ok ));

  if( ! ok )
    {
      return false;
    }

  // 2. Inside
  faz.Inside = (stringList[2] == "0" ) ? false : true;

  // 3. Latitude
  double dval = stringList[3].toDouble( &ok );

  if( ! ok )
    {
      return false;
    }

  dval = rint((dval / 10000000) * 600000);
  faz.Latitude = static_cast<int>(dval);

  // 4. Longitude
  dval = stringList[4].toDouble( &ok );

  if( ! ok )
    {
      return false;
    }

  dval = rint((dval / 10000000) * 600000);
  faz.Longitude = static_cast<int>(dval);

  // 5. Radius
  faz.Radius = stringList[5].toInt( &ok );

  if( ! ok )
    {
      return false;
    }

  // 6. Bottom
  faz.Bottom = stringList[6].toInt( &ok );

  if( ! ok )
    {
      return false;
    }

  // 7. Top
  faz.Top = stringList[7].toInt( &ok );

  if( ! ok )
    {
      return false;
    }

  // 8. ActivityLimit
  faz.ActivityLimit = stringList[8].toULong( &ok );

  if( ! ok )
    {
      return false;
    }

  if( faz.isActive() == false )
    {
      qWarning() << "Flarm Alert Zone" << stringList[9]
                 << "activity limit has expired! Ignoring $PFLAO.";
      return false;
    }

  // 9. ID
  faz.ID = stringList[9];

  // 10. ID-Type
  faz.IdType = stringList[10].toShort( &ok );

  if( ! ok )
    {
      return false;
    }

  // 11. ZoneType
  faz.ZoneType = stringList[11].toShort( &ok, 16 );

  if( ! ok )
    {
      return false;
    }

  faz.Key = FlarmBase::createHashKey( faz.IdType, faz.ID );
  faz.TimeStamp = QTime::currentTime();

  // Post new/updated alert zone.
  emit flarmAlertZoneInfo( faz );
  return true;
}

bool Flarm::extractError(QStringList& stringList)
{
  /**
   * $ERROR,CKSUM*37
   */
  if ( stringList[0] != "$ERROR" || stringList.size() < 2 )
      {
        qWarning("$ERROR contains too less parameters!");
        return false;
      }

  emit flarmError( stringList );
  return true;
}

bool Flarm::getFlarmRelativeBearing( int &relativeBearing )
{
  if( ! m_flarmStatus.valid && m_flarmStatus.RelativeBearing.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeBearing = m_flarmStatus.RelativeBearing.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool Flarm::getFlarmRelativeVertical( int &relativeVertical )
{
  if( ! m_flarmStatus.valid && m_flarmStatus.RelativeVertical.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeVertical = m_flarmStatus.RelativeVertical.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool Flarm::getFlarmRelativeDistance( int &relativeDistance )
{
  if( ! m_flarmStatus.valid && m_flarmStatus.RelativeDistance.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeDistance = m_flarmStatus.RelativeDistance.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * PFLAA data collection is finished.
 */
void Flarm::collectPflaaFinished()
{
  // Check the hash dictionary for expired data. This old data items have to
  // be removed. Seems to be the best place, to do it after the end trigger as
  // to trust that following methods will do that.
  QMutableHashIterator<QString, Flarm::FlarmAcft> it(m_pflaaHash);

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      Flarm::FlarmAcft& acft = it.value();

      // Make time expire check, check time unit is in milli seconds.
      if( acft.TimeStamp.elapsed() > 3000 )
        {
          // Object was longer time not updated, so we do remove it from the
          // hash. No other way available as the time expire check.
          it.remove();
        }
    }

  // Start Flarm PFLAA data clearing supervision. There is no other way
  // of solution because the PFLAA sentences are only sent if other
  // aircrafts are in view of the FLARM receiver.
  m_timer->start( 3000 );

  // Emit signal, if further processing in radar view is required.
  if( Flarm::getCollectPflaa() )
    {
      emit newFlarmPflaaData();
    }
}

/** Called if timer has expired. Used for Flarm PFLAA data clearing. */
void Flarm::slotTimeout()
{
  m_pflaaHash.clear();

  // Emit signal, if further processing in radar view is required.
  if( Flarm::getCollectPflaa() )
    {
      emit flarmPflaaDataTimeout();
    }
}

void Flarm::createTrafficMessage()
{
  bool ok;

  if( m_flarmStatus.AlarmType >= 0x10 && m_flarmStatus.AlarmType <= 0xff )
    {
      // Alert Zone Alarm. Handled as airspace.
      return;
    }

  int dir = m_flarmStatus.RelativeBearing.toInt(&ok);
  const int SD = Layout::getIntScaledDensity();

  if( ! ok )
    {
      return;
    }

  if( dir < 0 )
    {
      dir += 360;
    }

  dir = (dir + 15) / 30;

  if( dir == 0 )
    {
      // zero must be translated as 12 o'clock
      dir = 12;
    }

  // Traffic angle for arrow picture
  int ta = (dir == 12) ? 0 : dir * 30;

  int rvert = m_flarmStatus.RelativeVertical.toInt(&ok);

  QString rverts = (rvert > 0) ? "+" : "";

  if( ! ok )
    {
      return;
    }

  int rdist = m_flarmStatus.RelativeDistance.toInt(&ok);

  if( ! ok )
    {
      return;
    }

  QString almType = FlarmBase::translateAlarmType( m_flarmStatus.AlarmType );

  QString almlevel;

  switch( m_flarmStatus.Alarm )
  {
    case 0:
      almlevel = tr("Info");
      break;
    case 1:
      almlevel = tr("Caution");
      break;
    case 2:
      almlevel = tr("Warning");
      break;
    case 3:
      almlevel = tr("Alarm");
      break;
    default:
      break;
  }

  // Load an arrow pixmap to show the traffic direction more in detail.
  QString arrow;
  arrow.sprintf( "%s/icons/windarrows/wind-arrow-80px-%03d.png\" width=\"%d\" height=\"%d\"",
                 GeneralConfig::instance()->getAppRoot().toLatin1().data(),
                 ta,
                 80 * SD,
                 80 * SD );

  QString text = "<html><table border=1 cellpadding=\"5\"><tr><th align=center colspan=\"2\">" +
                  almlevel + "&nbsp;" + almType +
                 "</th></tr>";

  text += "<tr><td align=left valign=middle>" + tr("Direction") + "</td>" +
          "<td align=right valign=middle>" + "<img src=\"" + arrow + ">" +
          QString::number(dir) + " " + tr("o'clock") + "</td></tr>" +
          "<tr><td align=left>" + tr("Vertical") + "</td>" +
          "<td align=right>" + rverts + Altitude::getText( rvert, true, 0 ) + "</td></tr>" +
          "<tr><td align=left>" + tr("Distance") + "</td>" +
          "<td align=right>" + Altitude::getText( rdist, true, 0 ) + "</td></tr>";

  // If an alias is known, it is added to the table
  QString alias = FlarmAliasList::getAliasHash().value( m_flarmStatus.ID );

  if( alias.isEmpty() == false )
    {
      text += "<tr><td align=left>" + tr("Alias") + "</td>" +
              "<td align=right>" + alias + "</td></tr>";
    }

  text += "</table></html>";

  emit flarmTrafficInfo( text );
}
