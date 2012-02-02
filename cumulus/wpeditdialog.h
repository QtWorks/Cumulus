/***********************************************************************
**
**   wpeditdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WpEditDialog
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A waypoint editor widget.
 *
 * The WpEditDialog widget allows the creation of a new waypoint or the
 * modification of an existing waypoint. It is a modal window.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef WP_EDIT_DIALOG_H
#define WP_EDIT_DIALOG_H

#include <QWidget>
#include <QString>
#include <QTextEdit>

#include "waypoint.h"

class WpEditDialog : public QWidget
{
  Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( WpEditDialog )

public:

    /**
     * @param parent The parent widget.
     * @param wp A waypoint to be edited.
     */
    WpEditDialog(QWidget *parent=0, Waypoint *wp=0);

    virtual ~WpEditDialog();

private:

    /**
     * This method is called just before showing the dialog,
     * and loads the current waypoint data.
     */
    void loadWaypointData();

    /**
     * This method checks, if all mandatory waypoint data have been defined.
     * Returns true on ok otherwise false.
     */
    bool checkWaypointData( Waypoint& wp );

    /**
     * This method checks, if the passed waypoint name is already to find
     * in the global waypoint list. If yes the user is informed with a
     * message box about this fact.
     * Returns true if yes otherwise false.
     */
    bool isWaypointNameInList( QString& wpName );

    /**
     * This method checks, if the passed waypoint name is multiple to find
     * in the global waypoint list. If yes the user is informed with a
     * message box about this fact.
     * Returns true if yes otherwise false.
     */
    bool countWaypointNameInList( QString& wpName );
  
private slots:

    /**
     * Called if OK button is pressed
     */
    void accept();

signals:

    /**
     * Signal emitted to indicate the settings should be saved
     * to the configuration file
     */
    void save(Waypoint *);

    /**
     * Emitted to indicate that the settings should be
     * re-) loaded from the configuration file.
     */
    void load(Waypoint *);

    /**
     * This signal is emitted after a edited waypoint has been saved.
     * It is used to re-fill the waypoint list.
     */
    void wpListChanged(Waypoint &);

private:

    Waypoint * _wp;
    QString oldName; // old name of waypoint before rename
    QTextEdit * comment;
};

#endif
