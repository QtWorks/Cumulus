/***********************************************************************
 **
 **   glidersite.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef GLIDERSITE_H
#define GLIDERSITE_H

#include "radiopoint.h"
#include "runway.h"

/**
 * This class provides handling the glider-sites.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class GliderSite : public RadioPoint
{
 public:
  /**
   * Creates a new GliderSite-object.
   *
   * @param  name  The name
   * @param  icao  The icao-name
   * @param  abbr  The abbreviation, used for the gps-logger
   * @param  pos  The projected position
   * @param  wgsPos  The original WGS-position
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  winsh  "true", if only winsh-launch is available
   */
  GliderSite(const QString& name, const QString& icao, const QString& abbr, const WGSPoint& wgsPos,
             const QPoint& pos, unsigned int elevation, const char* frequency, bool winch, runway *rw);

  /**
   * Destructor.
   */
  ~GliderSite();

  /**
   * @return the frequency of the glidersite.
   */
  QString getFrequency() const;

  /**
   * @return a runway-struct, containing the data of the given runway.
   */
  runway getRunway(int index = 0) const;

  /**
   * @return the number of runways.
   */
  unsigned int getRunwayNumber() const;

  /**
   * @return "true", if only winch launching is available.
   */
  bool isWinch() const;

  /**
   * Return a short html-info-string about the airport, containg the
   * name, the alias, the elevation and the frequency as well as a small
   * icon of the airporttype.
   *
   * Reimplemented from SinglePoint.
   *
   * @return the infostring
   */
  virtual QString getInfoString() const;

  /**
   * Prints the element. Reimplemented from BaseMapElement.
   *
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */

  /**
   * Draws the element into the given painter.
   */
  virtual void drawMapElement(QPainter* targetP, QPainter* maskP);

 private:
  /**
   * The launching-type. "true" if the site only has a winch, "false",
   * if aero tow is also available.
   */
  bool winch;

  /**
   * Contains the runway-data.
   */
  runway* rwData;

  /**
   * Contains the number of runways.
   */
  unsigned int rwNum;

  unsigned int shift;

};

#endif
