/***********************************************************************
**
**   settingspagesector.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef __SettingsPageSector__h
#define __SettingsPageSector__h

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QButtonGroup>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "altitude.h"

/**
 * @short Configuration settings for flight tasks
 *
 * @author Axel Pauli
 */

class SettingsPageSector : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageSector )

public:

  SettingsPageSector( QWidget *parent=0);
  
  virtual ~SettingsPageSector();

public slots:

  void slot_load();
  void slot_save();

private slots:

  // radio button of cs scheme was pressed
  void slot_buttonPressedCS( int newScheme );

  // radio button of nt scheme was pressed
  void slot_buttonPressedNT( int newScheme );

  // value inner spin box changed
  void slot_outerSBChanged( double value );

  /** Opens the color chooser dialog for the course line */
  void slot_editClColor();

private:

  // active scheme
  QButtonGroup* csScheme; // cylinder-sector scheme
  QButtonGroup* ntScheme; // nearst-touched scheme

  // Cylinder widgets
  QGroupBox*      cylinderGroup;
  QDoubleSpinBox* cylinderRadius; // Radius of cylinder task point in meter or feet

  // Sector widgets
  QGroupBox*      sectorGroup;
  QDoubleSpinBox* innerSectorRadius; // inner sector radius of task point in meter or feet
  QDoubleSpinBox* outerSectorRadius; // outer sector radius of task point in meter or feet
  QSpinBox*       sectorAngle;       // 0-180 degrees
    
  // Drawing options
  QGroupBox* shapeGroup;
  QCheckBox* drawShape;
  QCheckBox* fillShape;

  // course line options
  QGroupBox*   clGroup;
  QSpinBox*    clWidth;
  QPushButton* clColorButton;

  // temporary storage of course line color
  QColor clColor;
  QColor selectedClColor;

  // store selected task course line width
  int seletedClWidth;

  /** saves distance unit set during construction of object */
  Distance::distanceUnit distUnit;

  // store selected cs scheme button
  int selectedCSScheme;
  // store selected nt scheme button
  int selectedNTScheme;
  // store cylinder radius after load
  double loadedCylinderRadius;
  // store inner sector radius after load
  double loadedInnerSectorRadius;
  // store outer sector radius after load
  double loadedOuterSectorRadius;
};

#endif
