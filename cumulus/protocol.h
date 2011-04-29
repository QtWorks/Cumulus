/***********************************************************************
**
**   protocol.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2011 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

/**
 * This file contains the message keys used during interprocess communication
 * between the Cumulus and the GPS client process.
 */

#ifndef _Protocol_h_
#define _Protocol_h_

// Message key word definitions

//------- Used by Command/Response channel -------//

#define MSG_PROTOCOL   "Cumulus-GPS_Client_IPC_V1.4_Axel@kflog.org"

#define MSG_MAGIC      "\\Magic\\"

#define MSG_POS		     "\\Positive\\"

#define MSG_NEG		     "\\Negative\\"

// open connection to the GPS device "Open" <device> <speed>
#define MSG_OPEN	     "\\Open\\"

// close connection to the GPS device
#define MSG_CLOSE	     "\\Close\\"

// send message to GPS device
#define MSG_SM		     "\\Send_Msg\\"

// Switch on forwarding of GPS data
#define MSG_FGPS_ON		 "\\Gps_Data_On\\"

// Switch off forwarding of GPS data
#define MSG_FGPS_OFF   "\\Gps_Data_Off\\"

// shutdown request
#define MSG_SHD	       "\\Shutdown\\"

// GPS message keys to be processed.
#define MSG_GPS_KEYS   "\\Gps_Msg_Keys\\"

//------- Used by Forward data channel -------//

// GPS data message
#define MSG_GPS_DATA  "\\Gps_Data\\"

#define MSG_CON_OFF     "#GPS_Connection_off#"

#define MSG_CON_ON      "#GPS_Connection_on#"

#endif  // #ifndef _Protocol_h_
