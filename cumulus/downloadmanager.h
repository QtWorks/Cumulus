/***********************************************************************
**
**   downloadmanager.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Manager for HTTP download handling
 *
 * This class handles the HTTP download requests in Cumulus. Downloads
 * of different map files can be requested.
 */

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <QtCore>

#include "httpclient.h"

class DownloadManager : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( DownloadManager )

 public:

  /**
   * Creates a download manager instance.
   */
  DownloadManager( QObject *parent = 0 );

  /**
   * Requests to download the passed url and to store the result under the
   * passed file destination. Destination must consist of a full path.
   */
  bool downloadRequest( QString &url, QString &destination );

  signals:

   /** Sends out a status message. */
   void status( const QString& msg );

   /** Sends a finish signal if all requested downloads are done. */
   void finished( int requests, int errors );

   /**
    *  Sends a finish signal if the Welt2000 file has been downloaded successfully.
    */
   void welt2000Downloaded();

   /**
    *  Sends a signal if an airspace file has been downloaded successfully.
    */
   void airspaceDownloaded();

 private:

  /**
   * Returns the free size of the file system in bytes for non root users.
   */
  ulong getFreeUserSpace( QString& path );

 private slots:

  /** Catch a finish signal with the downloaded url and the related result. */
  void slotFinished( QString &url, QNetworkReply::NetworkError code );

 private:

  /** HTTP download client */
  HttpClient *client;

  /** Download working flag */
  bool downloadRunning;

  /** Set of urls to be downloaded, used for fast checks */
  QSet<QString> urlSet;

  /**
   * The download queue containing url and destination as string pair.
   */
  QQueue< QPair<QString, QString> > queue;

  /** Mutex to protect data accesses. */
  QMutex mutex;

  /** Counter for download request. */
  int requests;

  /** Counter for download errors. */
  int errors;

  /**
   * Required minimum space in bytes on file system destination to
   * execute the download request.
   */
  static const ulong MinFsSpace;
};

#endif /* DOWNLOAD_MANAGER_H */
