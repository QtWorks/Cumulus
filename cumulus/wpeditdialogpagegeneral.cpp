/***********************************************************************
**
**   wpeditdialogpagegeneral.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "altitude.h"
#include "basemapelement.h"
#include "generalconfig.h"
#include "wgspoint.h"
#include "wpeditdialogpagegeneral.h"

#ifdef FLICK_CHARM
#include "flickcharm.h"
#endif

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#endif

WpEditDialogPageGeneral::WpEditDialogPageGeneral(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("WpEditDialogPageGeneral");
  m_loadedLat = 0;
  m_loadedLon = 0;

  QGridLayout * topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  // The description maximum length is 25 characters. We calculate
  // the length of a M string of 27 characters. That is the maximum
  // width of the QLineEdit widgets.
  QFontMetrics fm( font() );
  int charWidth = fm.width(QChar('M'));

  int row=0;

  GeneralConfig *conf = GeneralConfig::instance();

  QLabel * lblName = new QLabel(tr("Name(8):"), this);
  topLayout->addWidget(lblName, row, 0);
  m_edtName = new QLineEdit(this);
  m_edtName->setMaxLength(8); // limit name to 8 characters
#ifndef ANDROID
  m_edtName->setMinimumWidth( 27*charWidth );
  m_edtName->setMaximumWidth( 27*charWidth );
#else
  m_edtName->setMinimumWidth( 22*charWidth );
  m_edtName->setMaximumWidth( 22*charWidth );
#endif
  topLayout->addWidget(m_edtName, row++, 1, 1, 3);

#ifndef ANDROID
  connect( m_edtName, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedName( const QString& )) );
#else
  // Android makes trouble, if word detection is enabled. Therefore the
  // entered string is modified, when the finish signal is emitted.
  connect( m_edtName, SIGNAL(editingFinished()),
           this, SLOT(slot_textEditedNameFinished()) );
#endif

  QLabel * lblDescription = new QLabel(tr("Description:"), this);
  topLayout->addWidget(lblDescription, row, 0);
  m_edtDescription = new QLineEdit(this);
  m_edtDescription->setMaxLength(25); // limit name to 25 characters
#ifndef ANDROID
  m_edtDescription->setMinimumWidth( 27*charWidth );
  m_edtDescription->setMaximumWidth( 27*charWidth );
#else
  m_edtDescription->setMinimumWidth( 22*charWidth );
  m_edtDescription->setMaximumWidth( 22*charWidth );
#endif
  topLayout->addWidget(m_edtDescription, row++, 1, 1, 3);

  QLabel * lblCountry = new QLabel(tr("Country(2):"), this);
  topLayout->addWidget(lblCountry, row, 0);
  m_edtCountry = new QLineEdit(this);
  m_edtCountry->setMaxLength(2); // limit name to 2 characters
  m_edtCountry->setMinimumWidth( 3*charWidth );
  m_edtCountry->setMaximumWidth( 3*charWidth );
  m_edtCountry->setText( conf->getHomeCountryCode() );
  QRegExp rx("[A-Za-z]{2}");
  m_edtCountry->setValidator( new QRegExpValidator(rx, this) );

#ifndef ANDROID
  connect( m_edtCountry, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedCountry( const QString& )) );
#else
  // Android makes trouble, if word detection is enabled. Therefore the
  // entered string is modified, when the finish signal is emitted.
  connect( m_edtCountry, SIGNAL(editingFinished()),
           this, SLOT(slot_textEditedCountryFinished()) );
#endif

  topLayout->addWidget(m_edtCountry, row++, 1, 1, 2);

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblLat = new QLabel(tr("Latitude:"), this);
  topLayout->addWidget(lblLat, row, 0);

#ifdef USE_NUM_PAD
  m_edtLat = new LatEditNumPad(this, conf->getHomeLat());
#else
  m_edtLat = new LatEdit(this, conf->getHomeLat());
#endif

  topLayout->addWidget(m_edtLat, row++, 1, 1, 2);

  QLabel * lblLon = new QLabel(tr("Longitude:"), this);
  topLayout->addWidget(lblLon, row, 0);

#ifdef USE_NUM_PAD
  m_edtLong = new LongEditNumPad(this, conf->getHomeLon());
#else
  m_edtLong = new LongEdit(this, conf->getHomeLon());
#endif

  topLayout->addWidget(m_edtLong, row++, 1, 1, 2);

  QLabel * lblElev = new QLabel(tr("Elevation:"), this);
  topLayout->addWidget(lblElev, row, 0);
  QBoxLayout * elevLayout = new QHBoxLayout();
  topLayout->addLayout(elevLayout, row++, 1);

#ifdef USE_NUM_PAD
  m_edtElev = new NumberEditor( this );
  m_edtElev->setDecimalVisible( false );
  m_edtElev->setSuffix( " " + Altitude::getUnitText() );
  m_edtElev->setMaxLength(6);
  m_edtElev->setAlignment( Qt::AlignLeft );
  m_edtElev->setText("0");
  QRegExpValidator *eValidator = new QRegExpValidator( QRegExp( "(0|-?[1-9][0-9]{0,4})" ), this );
  m_edtElev->setValidator( eValidator );
#else
  m_edtElev = new QLineEdit(this);
  m_edtElev->setValidator( new QRegExpValidator(QRegExp("[1-9][0-9]*"), this) );
#endif

  elevLayout->addWidget(m_edtElev);

#ifndef USE_NUM_PAD
  QLabel * lblElevUnit = new QLabel(Altitude::getUnitText(), this);
  elevLayout->addWidget(lblElevUnit);
#endif

  topLayout->setRowMinimumHeight(row++, 10);

  QLabel * lblGReg = new QLabel(tr("Type:"), this);
  topLayout->addWidget(lblGReg, row, 0);
  m_cmbType = new QComboBox(this);
  m_cmbType->setObjectName("Type");
  m_cmbType->setEditable(false);

#ifdef QSCROLLER
  QScroller::grabGesture(m_cmbType->view(), QScroller::LeftMouseButtonGesture);
#endif

#ifdef FLICK_CHARM
  FlickCharm *flickCharm = new FlickCharm(this);
  flickCharm->activateOn(m_cmbType->view());
#endif

  topLayout->addWidget(m_cmbType, row++, 1);

  // init combo boxes
  QStringList &tlist = BaseMapElement::getSortedTranslationList();

  for( int i=0; i < tlist.size(); i++ )
    {
      m_cmbType->addItem( tlist.at(i) );
    }

  // The default waypoint type is landmark.
  setWaypointType(BaseMapElement::Landmark );

  QLabel * lblGCall = new QLabel(tr("Importance:"), this);
  topLayout->addWidget(lblGCall, row, 0);
  m_cmbImportance = new QComboBox(this);
  m_cmbImportance->setObjectName("Importance");
  m_cmbImportance->setEditable(false);
  topLayout->addWidget(m_cmbImportance, row++, 1);
  m_cmbImportance->addItem(tr("low"));
  m_cmbImportance->addItem(tr("normal"));
  m_cmbImportance->addItem(tr("high"));
  m_cmbImportance->addItem(tr("top"));

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2, 10);
  topLayout->setColumnStretch(4, 10);
}

WpEditDialogPageGeneral::~WpEditDialogPageGeneral()
{
}

/** called if data needs to be loaded */
void WpEditDialogPageGeneral::slot_load(Waypoint *wp)
{
  if( wp )
    { //we don't need to load if the waypoint is not there
      m_edtName->setText( wp->name.left( 8 ).toUpper() );
      m_edtDescription->setText( wp->description );
      m_edtCountry->setText( wp->country );
      m_edtLat->setKFLogDegree( wp->origP.lat() );
      m_edtLong->setKFLogDegree( wp->origP.lon() );
      m_edtElev->setText( Altitude::getText( (wp->elevation), false, -1 ) );
      setWaypointType( wp->type );
      m_cmbImportance->setCurrentIndex( wp->priority );

      // save loaded values
      m_loadedLat = wp->origP.lat();
      m_loadedLon = wp->origP.lon();
    }
}

/** called if data needs to be saved */
void WpEditDialogPageGeneral::slot_save(Waypoint *wp)
{
  if ( wp )
    {
      wp->name = m_edtName->text().trimmed().toUpper();

      wp->description = m_edtDescription->text().trimmed();

      wp->country = m_edtCountry->text().toUpper();

      if( m_edtLat->isInputChanged() )
        {
          wp->origP.setLat( m_edtLat->KFLogDegree() );
        }
      else
        {
          wp->origP.setLat( m_loadedLat );
        }

      if( m_edtLong->isInputChanged() )
        {
          wp->origP.setLon( m_edtLong->KFLogDegree() );
        }
      else
        {
          wp->origP.setLon( m_loadedLon );
        }

      wp->elevation = static_cast<float> (Altitude::convertToMeters(m_edtElev->text().toDouble()));
      wp->type = getWaypointType();
      wp->priority = ( enum Waypoint::Priority ) m_cmbImportance->currentIndex();
    }
}

void WpEditDialogPageGeneral::slot_textEditedName( const QString& text )
{
  // Change edited text to upper cases
  m_edtName->setText( text.toUpper() );
}

void WpEditDialogPageGeneral::slot_textEditedCountry( const QString& text )
{
  // Change edited text to upper cases
  m_edtCountry->setText( text.toUpper() );
}

/**
 * Called to make all text to upper cases.
 */
void WpEditDialogPageGeneral::slot_textEditedNameFinished()
{
  m_edtName->setText( m_edtName->text().trimmed().toUpper() );
}

/**
 * Called to make all text to upper cases.
 */
void WpEditDialogPageGeneral::slot_textEditedCountryFinished()
{
  m_edtCountry->setText( m_edtCountry->text().trimmed().toUpper() );
}

/** return internal type of waypoint */
int WpEditDialogPageGeneral::getWaypointType()
{
  int type = m_cmbType->currentIndex();

  if (type != -1)
    {
      const QString &text = BaseMapElement::getSortedTranslationList().at(type);
      type = BaseMapElement::text2Item( text );
    }

  return type;
}


/** set waypoint type in combo box translate internal id to index */
void WpEditDialogPageGeneral::setWaypointType(int type)
{
  int index = -1;

  if (type != -1)
    {
      index = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(type));
    }
  else
    {
      index = BaseMapElement::getSortedTranslationList().indexOf(BaseMapElement::item2Text(0));
    }

  m_cmbType->setCurrentIndex(index);
}

