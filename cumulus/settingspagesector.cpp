/***********************************************************************
**
**   settingspagesector.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**

**   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGroupBox>

#include "settingspagesector.h"
#include "generalconfig.h"
#include "flighttask.h"
#include "mapcontents.h"

extern MapContents  *_globalMapContents;

/**
 * @short Configuration settings for start, turn and end points of a
 * task
 *
 * @author Axel Pauli
 */

// Constructor of class
SettingsPageSector::SettingsPageSector( QWidget *parent) :
  QWidget( parent ),
  loadedCylinderRadius(0),
  loadedInnerSectorRadius(0),
  loadedOuterSectorRadius(0)
{
  setObjectName("SettingsPageSector");

  GeneralConfig *conf = GeneralConfig::instance();

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin(3);
  topLayout->setSpacing(3);
  
  QGroupBox *tsBox = new QGroupBox( tr("TP Scheme"), this );
  topLayout->addWidget( tsBox, 0, 0 );

  QRadioButton* cylinder = new QRadioButton( tr("Cylinder"), this );
  QRadioButton* sector   = new QRadioButton( tr("Sector"), this );
  
  csScheme = new QButtonGroup;
  csScheme->addButton( cylinder, 0 );
  csScheme->addButton( sector, 1 );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget( cylinder );
  vbox->addWidget( sector );
  vbox->addStretch(1);
  tsBox->setLayout(vbox);

  cylinder->setEnabled(true);
  sector->setEnabled(true);
  sector->setChecked(true);

  // set active button as selected
  selectedCSScheme = (int) conf->getActiveCSTaskScheme();

  if( csScheme->button(selectedCSScheme) )
    {
      csScheme->button(selectedCSScheme)->setEnabled(true);
    }

  //---------------------------------------------------------------

  QGroupBox *ssBox = new QGroupBox( tr("Switch Scheme"), this );
  topLayout->addWidget( ssBox, 0, 1 );

  ntScheme = new QButtonGroup;

  QRadioButton* nearst   = new QRadioButton( tr("Nearst"), this );
  QRadioButton* touched  = new QRadioButton( tr("Touched"), this );
 
  ntScheme->addButton( nearst, 0 );
  ntScheme->addButton( touched, 1 );

  vbox = new QVBoxLayout;
  vbox->addWidget( nearst );
  vbox->addWidget( touched );
  vbox->addStretch(1);
  ssBox->setLayout(vbox);

  nearst->setEnabled(true);
  touched->setEnabled(true);
  touched->setChecked(true);

  // set active button as selected
  selectedNTScheme = (int) conf->getActiveNTTaskScheme();

  if( ntScheme->button(selectedNTScheme) )
    {
      ntScheme->button(selectedNTScheme)->setEnabled(true);
    }

  //--------------------------------------------------------------
    // as next cylinder group is added
  cylinderGroup = new QGroupBox( tr("Cylinder"), this );
  topLayout->addWidget( cylinderGroup, 1, 0, 1, 2 ); 
  
  QHBoxLayout *hbox = new QHBoxLayout;

  QLabel *lbl = new QLabel( tr("Radius:"), this );
  hbox->addWidget( lbl );
  
  cylinderRadius = new QDoubleSpinBox( this );
  cylinderRadius->setRange(0.1, 10.0);
  cylinderRadius->setSingleStep(0.1);
  cylinderRadius->setButtonSymbols(QSpinBox::PlusMinus);
  hbox->addWidget( cylinderRadius );
  
  // get current distance unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  distUnit = Distance::getUnit();

  const char *unit = "";

  // Input accepts different units 
  if( distUnit == Distance::kilometers )
    {
      unit = "km";
    }
  else if( distUnit == Distance::miles )
    {
      unit = "ml";
    }
  else // if( distUnit == Distance::nautmiles )
    {
      unit = "nm";
    }

  hbox->addWidget(new QLabel( unit, this ));

  cylinderGroup->setLayout(hbox);

  //--------------------------------------------------------------
  // as next sector group is added
  sectorGroup = new QGroupBox( tr("Sector"), this );
  topLayout->addWidget( sectorGroup, 2, 0, 1, 2 ); 
  
  QGridLayout *sectorLayout = new QGridLayout( sectorGroup );
  sectorLayout->setMargin(10);
  sectorLayout->setSpacing(3);

  int row = 0;

  lbl = new QLabel( tr("Inner Radius:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );

  innerSectorRadius = new QDoubleSpinBox( sectorGroup );
  innerSectorRadius->setRange(0.0, 10.0);
  innerSectorRadius->setSingleStep(0.1);
  innerSectorRadius->setButtonSymbols(QSpinBox::PlusMinus);

  sectorLayout->addWidget( innerSectorRadius, row, 1 );
  sectorLayout->addWidget( new QLabel( unit, sectorGroup), row, 2 );

  row++;
  lbl = new QLabel( tr("Outer Radius:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );
  outerSectorRadius = new QDoubleSpinBox( sectorGroup );
  outerSectorRadius->setRange(0.1, 10.0);
  outerSectorRadius->setSingleStep(0.1);
  outerSectorRadius->setButtonSymbols(QSpinBox::PlusMinus);
  sectorLayout->addWidget( outerSectorRadius, row, 1 );
  sectorLayout->addWidget( new QLabel( unit, sectorGroup), row, 2 );

  row++;
  lbl = new QLabel( tr("Angle:"), sectorGroup );
  sectorLayout->addWidget( lbl, row, 0 );
  sectorAngle = new QSpinBox( sectorGroup );
  sectorAngle->setRange( 90, 180 );
  sectorAngle->setSingleStep( 5 );
  sectorAngle->setButtonSymbols(QSpinBox::PlusMinus);
  sectorAngle->setWrapping( true );
  sectorLayout->addWidget( sectorAngle, row, 1 );
  sectorLayout->addWidget( new QLabel( tr("degree"), sectorGroup), row, 2 );
  sectorAngle->setValue( conf->getTaskSectorAngle() );

  //--------------------------------------------------------------
  // as next shape group is added
  shapeGroup = new QGroupBox( tr("Shape"), this );
  topLayout->addWidget( shapeGroup, 3, 0, 1, 2 );

  hbox = new QHBoxLayout;
  drawShape = new QCheckBox( tr("Draw Shape"), this );
  fillShape = new QCheckBox( tr("Fill Shape"), this );

  hbox->addWidget( drawShape );
  hbox->addWidget( fillShape );
  hbox->addStretch(1);

  shapeGroup->setLayout(hbox);

  drawShape->setChecked( conf->getTaskDrawShape() );
  fillShape->setChecked( conf->getTaskFillShape() );

  connect( csScheme, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonPressedCS(int)) );
  connect( ntScheme, SIGNAL(buttonClicked(int)), this, SLOT(slot_buttonPressedNT(int)) );
  connect( outerSectorRadius, SIGNAL(valueChanged(double)), this, SLOT(slot_outerSBChanged(double)) );
}

// Destructor of class
SettingsPageSector::~SettingsPageSector()
{
}

// value of outer spin box changed
void SettingsPageSector::slot_outerSBChanged( double /* value */ )
{
  // set max range of inner radius spin box to current value of this box
  innerSectorRadius->setMaximum( outerSectorRadius->value() );

  if( innerSectorRadius->value() > outerSectorRadius->value() )
    {
      // inner spin box value must be less equal outer spin box value
      innerSectorRadius->setValue( outerSectorRadius->value() );
    }
}

// Set the passed scheme widget as active and the other one to
// inactive
void SettingsPageSector::slot_buttonPressedCS( int newScheme )
{
  selectedCSScheme = newScheme;

  if( newScheme == GeneralConfig::Sector )
    {
      sectorGroup->setEnabled(true);
      cylinderGroup->setEnabled(false);
    }
  else
    {
      sectorGroup->setEnabled(false);
      cylinderGroup->setEnabled(true);
    }
}

void SettingsPageSector::slot_buttonPressedNT( int newScheme )
{
  selectedNTScheme = newScheme;
}

void SettingsPageSector::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  slot_buttonPressedCS( (int) conf->getActiveCSTaskScheme() );

  // @AP: radius is always fetched in meters.
  Distance cRadius = conf->getTaskCylinderRadius();
  Distance iRadius = conf->getTaskSectorInnerRadius();
  Distance oRadius = conf->getTaskSectorOuterRadius();

  if( distUnit == Distance::kilometers ) // user gets meters
    {
      cylinderRadius->setValue( cRadius.getKilometers() );
      innerSectorRadius->setValue( iRadius.getKilometers() );
      outerSectorRadius->setValue( oRadius.getKilometers() );
    }
  else if( distUnit == Distance::miles ) // user gets miles
    {
      cylinderRadius->setValue( cRadius.getMiles() );
      innerSectorRadius->setValue( iRadius.getMiles() );
      outerSectorRadius->setValue( oRadius.getMiles() );
    }
  else // ( distUnit == Distance::nautmiles )
    {
      cylinderRadius->setValue( cRadius.getNautMiles() );
      innerSectorRadius->setValue( iRadius.getNautMiles() );
      outerSectorRadius->setValue( oRadius.getNautMiles() );
    }

  // Save loaded integer values of spin boxes
  loadedCylinderRadius = cylinderRadius->value();
  loadedInnerSectorRadius = innerSectorRadius->value();
  loadedOuterSectorRadius = outerSectorRadius->value();

  drawShape->setChecked( conf->getTaskDrawShape() );
  fillShape->setChecked( conf->getTaskFillShape() );
}

void SettingsPageSector::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setActiveCSTaskScheme( (GeneralConfig::ActiveCSTaskScheme) selectedCSScheme );
  conf->setActiveNTTaskScheme( (GeneralConfig::ActiveNTTaskScheme) selectedNTScheme );

  // @AP: radius is always saved in meters. Save is done only after a
  // real change to avoid rounding errors.
  Distance cRadius;
  Distance iRadius;
  Distance oRadius;

  if( loadedCylinderRadius != cylinderRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          cRadius.setKilometers( cylinderRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          cRadius.setMiles( cylinderRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          cRadius.setNautMiles( cylinderRadius->value() );
        }

      conf->setTaskCylinderRadius( cRadius );
    }

  if( loadedInnerSectorRadius != innerSectorRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          iRadius.setKilometers( innerSectorRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          iRadius.setMiles( innerSectorRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          iRadius.setNautMiles( innerSectorRadius->value() );
        }
      
      conf->setTaskSectorInnerRadius( iRadius );
    }

  if( loadedOuterSectorRadius != outerSectorRadius->value() )
    {
      if( distUnit == Distance::kilometers ) // user gets kilometers
        {
          oRadius.setKilometers( outerSectorRadius->value() );
        }
      else if( distUnit == Distance::miles ) // user gets miles
        {
          oRadius.setMiles( outerSectorRadius->value() );
        }
      else // ( distUnit == Distance::nautmiles )
        {
          oRadius.setNautMiles( outerSectorRadius->value() ); 
        }

      conf->setTaskSectorOuterRadius( oRadius );
    }

  conf->setTaskSectorAngle( sectorAngle->value() );

  conf->setTaskDrawShape( drawShape->isChecked() );
  conf->setTaskFillShape( fillShape->isChecked() );

  // If a flight task is set, we must update the sector angles in it
   FlightTask *task = _globalMapContents->getCurrentTask();

   if( task != 0 )
     {
       task->updateTask();
     }
}
