/***********************************************************************
**
**   generalconfig.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Andr� Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GENERALCONFIG_H
#define GENERALCONFIG_H

#include <QtGlobal>
#include <QSettings>
#include <QPixmap>
#include <QString>
#include <QSize>

#include "airspace.h"
#include "basemapelement.h"
#include "altitude.h"
#include "waypoint.h"

/**
  * @short Configuration encapsulation class
  * @author Andr� Somers
  *
  * This class is used to store and retrieve all kinds of
  * configuration options. This class is a singleton class. Use the
  * static @ref instance method to get a reference to the instance.
  */

// default window display times
#define MIN_POPUP_DISPLAY_TIME 2
#define AIRFIELD_DISPLAY_TIME_DEFAULT 3
#define AIRSPACE_DISPLAY_TIME_DEFAULT 3
#define INFO_DISPLAY_TIME_DEFAULT 7
#define WAYPOINT_DISPLAY_TIME_DEFAULT 3
#define WARNING_DISPLAY_TIME_DEFAULT  3
#define WARNING_SUPPRESS_TIME_DEFAULT 0 // time in minutes

// default for audible alarm switch
#define ALARM_SOUND_DEFAULT true
// default for calculator of nearest sites (true = ON)
#define NEAREST_SITE_CALCULATOR_DEFAULT true


// We do derive from the QT settings class as base class
class GeneralConfig : protected QSettings
{

public:

  enum UseInMode {
    never,
    standstill,
    circling,
    cruising,
    wave,
    always
  };

  // CS Task scheme data
  enum ActiveCSTaskScheme {
    Cylinder=0,
    Sector=1
  };
    
  // CS Task scheme data
  enum ActiveNTTaskScheme {
    Nearst=0,
    Touched=1
  };
  /**
   * Constructor. The relevant settings are loaded on construction.
   * @see load
   */
  
    GeneralConfig();
    /**
     * Destructor. The contained settings are stored before destruction.
     * @see save
     */
    ~GeneralConfig();

    /**
     * @returns The instance of GeneralConfig. Creates an instance if nessecairy.
     */
    static GeneralConfig *instance();

    /**
     * Saves the configuration settings.
     */
    void save();

    /**
     * Loads the configuration settings.
     */
    void load();

    //---------------------------
    // properties get/set methods
    //---------------------------

    /** gets the installation root of cumulus */
    QString &getInstallRoot()
    {
      return _installRoot;
    };

    void setInstallRoot( QString &newRoot )
    {
      _installRoot = newRoot;
    };

    /** get main window size */
    QSize getWindowSize()
    {
      return _windowSize;
    };

    /** set main window size */
    void setWindowSize( QSize size )
    {
      _windowSize = size;
    };

    /**
     * @returns requested pixmap
     */

    QPixmap loadPixmap( const char* pixmapName );

    /**
     * @returns Struct with warning distances for airspace warnings
     */
    AirspaceWarningDistance getAirspaceWarningDistances();
    /**
     * Sets the warningdistances for airspaces
     */
    void setAirspaceWarningDistances(const AirspaceWarningDistance& awd);
    /**
     * @return True if warnings are enabled for the given type.
     * @param objectType The type of object (defined in @ref BaseMapElement) to query
     */
    bool getAirspaceWarningEnabled (BaseMapElement::objectType type) const;
    /**
     * Enables or disables the airspacewarnings for this type of airspace
     */
    void setAirspaceWarningEnabled (BaseMapElement::objectType type, bool enable=true)
    {
        _airspaceWarning [type]=enable;
    };
    /**
     * @return True if warnings are enabled in general
     */
    bool getAirspaceWarningEnabled () const
    {
        return _airspaceWarningGeneral;
    };
    /**
     * Enables or disables the airspacewarnings in general
     */
    void setAirspaceWarningEnabled (bool enable=true)
    {
        _airspaceWarningGeneral=enable;
    };

    /**
     * @return True if forcing of airspace drawing for closeby
     * structures is enabled
     */
    bool getForceAirspaceDrawingEnabled () const
    {
        return _forceDrawing;
    };
    /**
     * Enables or disables the forcing of airspace drawing for closeby
     * structures
     */
    void setForceAirspaceDrawingEnabled (bool enable=true)
    {
        _forceDrawing=enable;
    };

    /**
     * Gets the distance for the forcing of airspace drawing for closeby
     * structures
     */
    Distance getForceAirspaceDrawingDistance() const
    {
        return _forceDrawingDistance;
    }
    /**
     * Sets the distance for the forcing of airspace drawing for closeby
     * structures
     */
    void setForceAirspaceDrawingDistance(const Distance dist)
    {
        _forceDrawingDistance = dist;
    }

    /** gets disclaimer version */
    int getDisclaimerVersion() const
    {
        return _disclaimerVersion;
    };
    /** sets disclaimer version */
    void setDisclaimerVersion( const int newValue )
    {
        _disclaimerVersion = newValue;
    };
    /**
     * return the set safety altitude
     */
    Altitude &getSafetyAltitude ()
    {
        return _safetyAltitude;
    };
    /**
     * Sets the safety altitude
     */
    void setSafetyAltitude( const Altitude& alt )
    {
        _safetyAltitude=alt;
    };

    /** Gets the surname */
    QString &getSurname()
    {
        return _surname;
    };
    /** Sets the surname */
    void setSurname( const QString newValue )
    {
        _surname = newValue;
    };

    /** Gets the birthday */
    QString &getBirthday()
    {
        return _birthday;
    };
    /** Sets the birthday */
    void setBirthday( const QString newValue )
    {
        _birthday = newValue;
    };

    /** gets qnh */
    int getQNH() const
    {
        return _qnh;
    };
    /** sets qnhe */
    void setQNH( const int newValue )
    {
        _qnh = newValue;
    };

    /** gets logger interval */
    int getLoggerInterval() const
    {
        return _loggerInterval;
    };
    /** sets logger interval */
    void setLoggerInterval( const int newValue )
    {
        _loggerInterval = newValue;
    };

    /** gets cruising speed */
    int getCruisingSpeed() const
    {
        return _cruisingSpeed;
    };
    /** sets cruising speed */
    void setCruisingSpeed( const int newValue )
    {
        _cruisingSpeed = newValue;
    };

    /** gets Homesite Latitude */
    int getHomeLat() const
    {
        return _homeWp->origP.lat();
    };
    /** sets Homesite Latitude */
    void setHomeLat( const int newValue )
    {
        _homeWp->origP.setLat( newValue );
        resetHomeWp();
    };

    /** gets Homesite  Longitude */
    int getHomeLon()  const
    {
        return _homeWp->origP.lon();
    };
    /** sets Homesite  Longitude */
    void setHomeLon( const int newValue )
    {
        _homeWp->origP.setLon( newValue );
        resetHomeWp();
    };

    /** gets Homesite waypoint */
    wayPoint *getHomeWp()  const
    {
        return _homeWp;
    };

    /** sets Homesite  Longitude */
    void setHomeWp( wayPoint *newValue )
    {
      if( ! newValue ) return;

      if(_homeWp)
        {
          delete _homeWp;
        }

      _homeWp = new wayPoint( *newValue );

    };

    void resetHomeWp();

    /** gets Center Latitude */
    int getCenterLat()  const
    {
        return _centerLat;
    };
    /** sets Center Latitude */
    void setCenterLat( const int newValue )
    {
        _centerLat = newValue;
    };

    /** gets Center  Longitude */
    int getCenterLon()  const
    {
        return _centerLon;
    };
    /** sets Center  Longitude */
    void setCenterLon( const int newValue )
    {
        _centerLon = newValue;
    };

    /** gets map scale */
    double getMapScale() const
    {
        return _mapScale;
    };
    /** sets map scale */
    void setMapScale( const double newValue )
    {
        _mapScale = newValue;
    };

    /** gets map Lower Limit */
    int getMapLowerLimit() const
    {
        return _mapLowerLimit;
    };
    /** sets map  */
    void setMapLowerLimit( const int newValue )
    {
        _mapLowerLimit = newValue;
    };

    /** gets map Upper Limit */
    int getMapUpperLimit() const
    {
        return _mapUpperLimit;
    };
    /** sets map Upper Limit */
    void setMapUpperLimit( const int newValue )
    {
        _mapUpperLimit = newValue;
    };

    /** gets map Border 1 */
    int getMapBorder1() const
    {
        return _mapBorder1;
    };
    /** sets map Border 1 */
    void setMapBorder1( const int newValue )
    {
        _mapBorder1 = newValue;
    };

    /** gets map Border 2 */
    int getMapBorder2() const
    {
        return _mapBorder2;
    };
    /** sets map Border 2 */
    void setMapBorder2( const int newValue )
    {
        _mapBorder2 = newValue;
    };

    /** gets map Border 3 */
    int getMapBorder3() const
    {
        return _mapBorder3;
    };
    /** sets map Border 3 */
    void setMapBorder3( const int newValue )
    {
        _mapBorder3 = newValue;
    };

    /** gets map Switch Scale */
    int getMapSwitchScale() const
    {
        return _mapSwitchScale;
    };
    /** sets map Switch Scale */
    void setMapSwitchScale( const int newValue )
    {
        _mapSwitchScale = newValue;
    };

    /** gets Map BearLine */
    bool getMapBearLine() const
    {
        return _mapBearLine;
    };
    /** sets Map BearLine */
    void setMapBearLine(const bool newValue)
    {
        _mapBearLine = newValue;
    };

    /** gets Map LoadIsoLines */
    bool getMapLoadIsoLines() const
    {
        return _mapLoadIsoLines;
    };
    /** sets Map LoadIsoLines */
    void setMapLoadIsoLines(const bool newValue)
    {
        _mapLoadIsoLines = newValue;
    };

    /** gets Map ShowIsoLineBorders */
    bool getMapShowIsoLineBorders() const
    {
        return _mapShowIsoLineBorders;
    };
    /** sets Map ShowIsoLineBorders */
    void setMapShowIsoLineBorders(const bool newValue)
    {
        _mapShowIsoLineBorders = newValue;
    };

    /** gets Map ShowWaypointLabels */
    bool getMapShowWaypointLabels() const
    {
        return _mapShowWaypointLabels;
    };
    /** sets Map ShowWaypointLabels */
    void setMapShowWaypointLabels(const bool newValue)
    {
        _mapShowWaypointLabels = newValue;
    };

    /** gets Map ShowWaypointLabelsExtraInfo */
    bool getMapShowWaypointLabelsExtraInfo() const
    {
        return _mapShowWaypointLabelsExtraInfo;
    };
    /** sets Map ShowWaypointLabelsExtraInfo */
    void setMapShowWaypointLabelsExtraInfo(const bool newValue)
    {
        _mapShowWaypointLabelsExtraInfo = newValue;
    };

    /** gets Map LoadRoads */
    bool getMapLoadRoads() const
    {
        return _mapLoadRoads;
    };
    /** sets Map LoadRoads */
    void setMapLoadRoads(const bool newValue)
    {
        _mapLoadRoads = newValue;
    };

    /** gets Map LoadHighways */
    bool getMapLoadHighways() const
    {
        return _mapLoadHighways;
    };
    /** sets Map LoadHighways */
    void setMapLoadHighways(const bool newValue)
    {
        _mapLoadHighways = newValue;
    };

    /** gets Map LoadRailroads */
    bool getMapLoadRailroads() const
    {
        return _mapLoadRailroads;
    };
    /** sets Map LoadRailroads */
    void setMapLoadRailroads(const bool newValue)
    {
        _mapLoadRailroads = newValue;
    };

    /** gets Map LoadCities */
    bool getMapLoadCities() const
    {
        return _mapLoadCities;
    };
    /** sets Map LoadCities */
    void setMapLoadCities(const bool newValue)
    {
        _mapLoadCities = newValue;
    };

    /** gets Map LoadWaterways */
    bool getMapLoadWaterways() const
    {
        return _mapLoadWaterways;
    };
    /** sets Map LoadWaterways */
    void setMapLoadWaterways(const bool newValue)
    {
        _mapLoadWaterways = newValue;
    };

    /** gets Map LoadForests */
    bool getMapLoadForests() const
    {
        return _mapLoadForests;
    };
    /** sets Map LoadForests */
    void setMapLoadForests(const bool newValue)
    {
        _mapLoadForests = newValue;
    };

    /** gets Map delete after compile */
    bool getMapDeleteAfterCompile() const
    {
        return _mapDelete;
    };
    /** sets Map delete after compile*/
    void setMapDeleteAfterCompile(const bool newValue)
    {
        _mapDelete = newValue;
    };

    /** gets Map unload unneeded */
    bool getMapUnload() const
    {
        return _mapUnload;
    };
    /** sets Map unload unneeded */
    void setMapUnload(const bool newValue)
    {
        _mapUnload = newValue;
    };

    /** Gets the welt 2000 country filter */
    QString &getWelt2000CountryFilter()
    {
        return _welt2000CountryFilter;
    };
    /** Sets the welt 2000 country filter */
    void setWelt2000CountryFilter( const QString newValue )
    {
        _welt2000CountryFilter = newValue;
    };

    /** gets the welt 2000 home radius */
    int getWelt2000HomeRadius() const
    {
        return _welt2000HomeRadius;
    };
    /** sets the welt 2000 home radius */
    void setWelt2000HomeRadius( const int newValue )
    {
        _welt2000HomeRadius = newValue;
    };

    /** gets map Projection Type */
    int getMapProjectionType()  const
    {
        return _mapProjectionType;
    };
    /** sets map Projection Type */
    void setMapProjectionType( const int newValue )
    {
        _mapProjectionType = newValue;
    };

    /** gets Lambert Parallel1 */
    int getLambertParallel1()  const
    {
        return _lambertParallel1;
    };
    /** sets map Projection Type */
    void setLambertParallel1( const int newValue )
    {
        _lambertParallel1 = newValue;
    };

    /** gets Lambert Parallel2 */
    int getLambertParallel2()  const
    {
        return _lambertParallel2;
    };
    /** sets map Projection Type */
    void setLambertParallel2( const int newValue )
    {
        _lambertParallel2 = newValue;
    };

    /** gets Lambert Orign */
    int getLambertOrign()  const
    {
        return _lambertOrign;
    };
    /** sets Lambert Orign */
    void setLambertOrign( const int newValue )
    {
        _lambertOrign = newValue;
    };

    /** gets Cylinder Parallel */
    int getCylinderParallel()  const
    {
        return _cylinderParallel;
    };
    /** sets Cylinder Parallel */
    void setCylinderParallel( const int newValue )
    {
        _cylinderParallel = newValue;
    };

    // NOTICE! @AP: got some trouble with inline function
    // definition. They didn't work up to here. Not clear why.

    /** gets vario integration time */
    int getVarioIntegrationTime() const;
    /** sets vario integration time */
    void setVarioIntegrationTime(const int newValue);

    /** gets vario vario step width */
    int getVarioStepWidth() const;
    /** sets vario vario step width */
    void setVarioStepWidth(const int newValue);

    /** gets vario tek adjust */
    int getVarioTekAdjust() const;
    /** sets vario tek adjust */
    void setVarioTekAdjust(const int newValue);

    /** gets vario tek compensation */
    bool getVarioTekCompensation() const;
    /** sets vario tek compensation */
    void setVarioTekCompensation(const bool newValue);

    /** gets altimeter mode */
    int getAltimeterMode() const;
    /** sets altimeter mode */
    void setAltimeterMode(const int newValue);

    /** gets altimeter toggle mode */
    bool getAltimeterToggleMode() const;
    /** sets altimeter toggle mode */
    void setAltimeterToggleMode(const bool newValue);

    /** gets log to file mode */
    bool getLog2FileMode() const;
    /** sets log to file mode */
    void setLog2FileMode(const bool newValue);

    /** gets sys log mode flag */
    bool getSystemLogMode() const;
    /** sets sys log mode flag */
    void setSystemLogMode(const bool newValue);

    /** gets nearest site calculator switch */
    bool getNearestSiteCalculatorSwitch() const;
    /** sets nearest site calculator switch */
    void setNearestSiteCalculatorSwitch(const bool newValue);

    /** gets max nearest site calculator sites */
    int getMaxNearestSiteCalculatorSites() const;
    /** sets max nearest site calculator sites */
    void setMaxNearestSiteCalculatorSites(const int newValue);

    /** gets AirfieldDisplayTime */
    int getAirfieldDisplayTime() const;
    /** sets AirfieldDisplayTime */
    void setAirfieldDisplayTime(const int newValue);

    /** gets AirspaceDisplayTime */
    int getAirspaceDisplayTime() const;
    /** sets AirspaceDisplayTime */
    void setAirspaceDisplayTime(const int newValue);

    /** gets InfoDisplayTime */
    int getInfoDisplayTime() const;
    /** sets InfoDisplayTime */
    void setInfoDisplayTime(const int newValue);

    /** gets WaypointDisplayTime */
    int getWaypointDisplayTime() const;
    /** sets WaypointDisplayTime */
    void setWaypointDisplayTime(const int newValue);

    /** gets WarningDisplayTime */
    int getWarningDisplayTime() const;
    /** sets WarningDisplayTime */
    void setWarningDisplayTime(const int newValue);

    /** gets WarningSuppressTime */
    int getWarningSuppressTime() const;
    /** sets WarningSuppressTime */
    void setWarningSuppressTime(const int newValue);

    /** gets AlarmSound */
    bool getAlarmSoundOn() const;
    /** sets AlarmSound */
    void setAlarmSoundOn(const bool newValue);

    /** gets Popup Airspace Warnings */
    bool getPopupAirspaceWarnings() const;
    /** sets Popup Airspace Warnings */
    void setPopupAirspaceWarnings(const bool newValue);

    /** Gets the Gps Device */
    QString &getGpsDevice();
    /** Sets the Gps Device */
    void setGpsDevice( const QString newValue );

    /** Gets the Gps Speed */
    int getGpsSpeed() const;
    /** Sets the Gps Speed */
    void setGpsSpeed( const int newValue );

    /** gets Gps altitude */
    int getGpsAltitude() const;
    /** sets Gps altitude */
    void setGpsAltitude(const int newValue);
    /** gets Gps altitude correction */
    Altitude getGpsUserAltitudeCorrection() const;
    /** sets Gps altitude correction */
    void setGpsUserAltitudeCorrection(const Altitude newValue);

    /** gets Gps soft start */
    bool getGpsSoftStart() const;
    /** sets Gps soft start */
    void setGpsSoftStart(const bool newValue);

    /** gets Gps hard start */
    bool getGpsHardStart() const;
    /** sets Gps hard start */
    void setGpsHardStart(const bool newValue);

    /** gets Gps sync system clock */
    bool getGpsSyncSystemClock() const;
    /** sets Gps sync system clock */
    void setGpsSyncSystemClock(const bool newValue);

    /** gets Gps Ipc port */
    ushort getGpsIpcPort() const;
    /** sets Gps Ipc port */
    void setGpsIpcPort(const ushort newValue);

    /** gets Gps client start option */
    bool getGpsStartClientOption() const;
    /** sets Gps client start option */
    void setGpsStartClientOption(const bool newValue);

    /** Gets the Gps Client path */
    QString getGpsClientPath();

    /** gets Gps last fix latitude */
    int getGpsLastFixLat() const;
    /** sets Gps last fix latitude */
    void setGpsLastFixLat(const int newValue);

    /** gets Gps last fix logitude */
    int getGpsLastFixLon() const;
    /** sets Gps last fix logitude */
    void setGpsLastFixLon(const int newValue);

    /** gets Gps last fix altitude */
    int getGpsLastFixAlt() const;
    /** sets Gps last fix altitude */
    void setGpsLastFixAlt(const int newValue);

    /** gets Gps last fix clock */
    int getGpsLastFixClk() const;
    /** sets Gps last fix clock */
    void setGpsLastFixClk(const int newValue);

    /** gets minimum sat cout for wind calculation */
    int getWindMinSatCount() const;
    /** sets minimum sat cout for wind calculation */
    void setMinSatCount(const int newValue);

    /** gets wind altitude range */
    int getWindAltitudeRange() const;
    /** sets wind altitude range */
    void setWindAltitudeRange(const int newValue);

    /** gets wind time range */
    int getWindTimeRange() const;
    /** sets wind time range */
    void setWindTimeRange(const int newValue);

    /** gets manual navigation mode altitude */
    int getManualNavModeAltitude() const;
    /** sets manual navigation mode altitude */
    void setManualNavModeAltitude(const int newValue);

    /** Gets the waypoint file name with path */
    QString &getWaypointFile();
    /** Sets the waypoint file name with path */
    void setWaypointFile( const QString newValue );

    /** Gets the unit for altitude*/
    int getUnitAlt() const;
    /** Sets the unit for altitude */
    void setUnitAlt(const int newValue);

    /** Gets the unit for distance*/
    int getUnitDist() const;
    /** Sets the unit for distance */
    void setUnitDist(const int newValue);

    /** Gets the unit for speed */
    int getUnitSpeed() const;
    /** Sets the unit for speed */
    void setUnitSpeed(const int newValue);

    /** Gets the unit for vario */
    int getUnitVario() const;
    /** Sets the unit for vario */
    void setUnitVario(const int newValue);

    /** Gets the unit for wind */
    int getUnitWind() const;
    /** Sets the unit for wind */
    void setUnitWind(const int newValue);

    /** Gets the unit for position */
    int getUnitPos() const;
    /** Sets the unit for position */
    void setUnitPos(const int newValue);

    /**
     * Get whether airspace filling is enabled at all
     */
    bool getAirspaceFillingEnabled() {return m_airspaceFillingEnabled;};
    /**
     * Set whether airspace filling is enabled at all
     */
    void setAirspaceFillingEnabled(bool enabled) {m_airspaceFillingEnabled = enabled;};

    /**
     * Get the filling percentages for the indicated "nearness"
     * in the vertical direction only.
     * @arg nearness Which filling are you looking for?
     * @returns a percentage filling
     */
    int getAirspaceFillingVertical(Airspace::ConflictType nearness);
    /**
     * Set the filling percentages for the indicated "nearness"
     * in the vertical direction only.
     * @arg nearness Which filling are you looking for?
     * @arg filling a percentage filling (0-100 inclusive)
     */
    void setAirspaceFillingVertical(Airspace::ConflictType nearness, int filling);

    /**
     * Get the filling percentages for the indicated "nearness"
     * in both directions.
     * @arg nearness Which filling are you looking for?
     * @returns a percentage filling
     */
    int getAirspaceFillingTotal(Airspace::ConflictType nearness);
    /**
     * Set the filling percentages for the indicated "nearness"
     * in both directions.
     * @arg nearness Which filling are you looking for?
     * @arg filling a percentage filling (0-100 inclusive)
     */
    void setAirspaceFillingTotal(Airspace::ConflictType nearness, int filling);

    /**
     * @returns the filling for the given nearnesses.
     * This is a conveneance funtion that combines the results of
     * getAirspaceFillingVertical and getAirspaceFillingTotal. It returns
     * 0 if airspace fillings are not enabled.
     * 
     * @arg vertical nearness in vertical direction only
     * @arg total total nearness in both directions
     */
    int airspaceFilling(Airspace::ConflictType vertical,
                        Airspace::ConflictType total)
    {
      if (!m_airspaceFillingEnabled)
        return 0;
      return qMax(getAirspaceFillingVertical(vertical),
                  getAirspaceFillingTotal(total)
                  );
    }

    /**
     * Draw a trail?
     */
    UseInMode getDrawTrail() {return _drawTrail;};

    /**
     * Sets whether to draw a trail or not.
     */
    void setDrawTrail(UseInMode draw) {_drawTrail = draw;};

    /** Gets the active cs task scheme */
    enum ActiveCSTaskScheme getActiveCSTaskScheme() const
    {
      return _taskActiveCSScheme;
    };

    /** Sets the active cs task scheme */
    void setActiveCSTaskScheme( const enum ActiveCSTaskScheme newValue )
    {
      _taskActiveCSScheme = newValue;
    };
    
    /** Gets the active nt task scheme */
    enum ActiveNTTaskScheme getActiveNTTaskScheme() const
    {
      return _taskActiveNTScheme;
    };

    /** Sets the active nt task scheme */
    void setActiveNTTaskScheme( const enum ActiveNTTaskScheme newValue )
    {
      _taskActiveNTScheme = newValue;
    };

    /** Gets task shape alpha transparency. 0 represents a fully
	transparent color, while 255 represents a fully opaque
	color. See also
	http://doc.trolltech.com/4.2/qcolor.html#alpha-blended-drawing
	for more info.
    */
    int getTaskShapeAlpha()
    {
      return _taskShapeAlpha;
    };

    /** Sets task shape alpha transparency. */
    void setTaskShapeAlpha( const int newValue )
    {
      _taskShapeAlpha = newValue;
    };
    
    /** Gets task cylinder radius in meters */
    Distance &getTaskCylinderRadius()
    {
      return _taskCylinderRadius;
    };

    /** Sets task cylinder radius. Unit must be meters. */
    void setTaskCylinderRadius( const Distance &newValue )
    {
      _taskCylinderRadius = newValue;
    };

    /** gets cylinder/sector draw option */
    bool getTaskDrawShape() const
    {
      return _taskDrawShape;
    };

    /** sets cylinder/sector draw option */
    void setTaskDrawShape( const bool newValue )
    {
      _taskDrawShape = newValue;
    };

    /** gets cylinder/sector fill option */
    bool getTaskFillShape() const
    {
      return _taskFillShape;
    };
    /** sets cylinder/secotr fill option */
    void setTaskFillShape( const bool newValue )
    {
      _taskFillShape = newValue;
    };

    /** Gets task sector inner radius in meters */
    Distance &getTaskSectorInnerRadius()
    {
      return _taskSectorInnerRadius;
    };

    /** Sets task sector inner radius. Unit must be meters. */
    void setTaskSectorInnerRadius( const Distance &newValue )
    {
      _taskSectorInnerRadius = newValue;
    };

    /** Gets task sector outer radius in meters */
    Distance &getTaskSectorOuterRadius()
    {
      return _taskSectorOuterRadius;
    };

    /** Sets task sector outer radius. Unit must be meters. */
    void setTaskSectorOuterRadius( const Distance &newValue )
    {
      _taskSectorOuterRadius = newValue;
    };

    /** Gets task sector angle 90-180 degrees.  */
    int getTaskSectorAngle() const
    {
      return _taskSectorAngle;
    };

    /** Sets task sector angle 90-180 degrees.  */
    void setTaskSectorAngle( const int newValue )
    {
      _taskSectorAngle = newValue;
    };

private:

    static GeneralConfig *_theInstance;

    // Root path of cumulus installation
    QString _installRoot;

    // Main window size
    QSize _windowSize;

    //properties
    //used to store the distances for airspace warnings
    AirspaceWarningDistance _awd;
    //display a warning for this type?
    bool _airspaceWarning [BaseMapElement::objectTypeSize];
    //display airspace warnings at all?
    bool _airspaceWarningGeneral;
    //fillings for airspaces
    int _verticalAirspaceFillings[4];
    //fillings for airspaces
    int _totalAirspaceFillings[4];
    //enable airspace filling?
    bool m_airspaceFillingEnabled;

    //force drawing for airspaces close by?
    bool _forceDrawing;
    //vertical distance below airspace
    Distance _forceDrawingDistance;

    // disclaimer version
    int _disclaimerVersion;
    Altitude _safetyAltitude;
    // surname
    QString _surname;
    // birthday
    QString _birthday;
    // qnh
    int _qnh;
    // logger interval
    int _loggerInterval;
    // cruising speed
    int _cruisingSpeed;

    // Homesite
    wayPoint * _homeWp;
    // Center Latitude
    int _centerLat;
    // Center Longitude
    int _centerLon;

    // Map Scale
    double _mapScale;
    // Map Data Projection Type
    int _mapProjectionType;

    // Map delete after comlile
    bool _mapDelete;
    // Map unload unneeded
    bool _mapUnload;

    // Welt 2000 country filter
    QString _welt2000CountryFilter;
    // Welt 2000 home radius
    int _welt2000HomeRadius;

    // Map BearLine
    bool _mapBearLine;
    // Map LoadIsoLines
    bool _mapLoadIsoLines;
    // Map ShowIsoLineBorders
    bool _mapShowIsoLineBorders;
    // Map ShowWaypointLabels
    bool _mapShowWaypointLabels;
    // Map ShowWaypointLabelsExtraInfo
    bool _mapShowWaypointLabelsExtraInfo;
    // Map LoadRoads
    bool _mapLoadRoads;
    // Map LoadHighways
    bool _mapLoadHighways;
    // Map LoadRailroads
    bool _mapLoadRailroads;
    // Map LoadCities
    bool _mapLoadCities;
    // Map LoadWaterways
    bool _mapLoadWaterways;
    // Map LoadForests
    bool _mapLoadForests;
    // Map trail
    UseInMode _drawTrail;

    // Map Lower Limit
    int _mapLowerLimit;
    // Map Upper Limit
    int _mapUpperLimit;
    // Map Border 1
    int _mapBorder1;
    // Map Border 2
    int _mapBorder2;
    // Map Border 3
    int _mapBorder3;
    // Map Switch Scale
    int _mapSwitchScale;

    // Lambert Projection
    int _lambertParallel1;
    // Lambert Projection
    int _lambertParallel2;
    // Lambert Projection
    int _lambertOrign;
    // Cylindrical Projection
    int _cylinderParallel;

    // vario integration time
    int _varioIntegrationTime;
    // vario step width
    int _varioStepWidth;
   // vario tek compensation
    bool _varioTekCompensation;
    // vario tek adjust
    int _varioTekAdjust;

    // altimeter mode
    int _altimeterMode;
    // altimeter toggle mode
    bool _altimeterToggleMode;

    // logging into a fixed file for debugging purposes
    bool _log2File;
    // use syslog facility instead of stderr
    bool _useSysLog;

    // nearest site calculator switch
    bool _nearestSiteCalculatorSwitch;
    // maximum sites considered by nearest site calculator
    int _maxNearestSiteCalculatorSites;

    // AirfieldDisplayTime
    int _airfieldDisplayTime;
    // AirspaceDisplayTime
    int _airspaceDisplayTime;
    // InfoDisplayTime
    int _infoDisplayTime;
    // WaypointDisplayTime
    int _waypointDisplayTime;
    // WarningDisplayTime
    int _warningDisplayTime;
    // WarningSuppressTime
    int _warningSuppressTime;
    // AlarmSound
    bool _alarmSound;
    // Popup Airspace Warnings
    bool _popupAirspaceWarnings;

    // Gps device
    QString _gpsDevice;
    // Gps speed
    int _gpsSpeed;
    // Gps delivered altitude
    int _gpsAltitude;
    // Gps deliverd altitude user correction
    Altitude _gpsAltitudeUserCorrection;
    // Gps soft start
    bool _gpsSoftStart;
    // Gps hard start
    bool _gpsHardStart;
    // Gps synchronize system clock
    bool _gpsSyncSystemClock;
    // Gps IPC port
    ushort _gpsIpcPort;
    // Gps client start option
    bool _gpsStartClient;
    // Gps last fix latitude
    int _gpsLastFixLat;
    // Gps last fix logitude
    int _gpsLastFixLon;
    // Gps last fix altitude
    int _gpsLastFixAlt;
    // Gps last fix clock
    int _gpsLastFixClk;

    // minimum sat count for wind calculation
    int _windMinSatCount;
    // wind altitude range
    int _windAltitudeRange;
    // wind time range
    int _windTimeRange;
    // manual navigation mode altitude
    int _manualNavModeAltitude;

    // Waypoint data file name
    QString _waypointFile;

    // unit altitude
    int _unitAlt;
    // unit distance
    int _unitDist;
    // unit speed
    int _unitSpeed;
    // unit vario
    int _unitVario;
    // unit wind
    int _unitWind;
    // unit position
    int _unitPos;

    // aktive cs task scheme
    enum ActiveCSTaskScheme _taskActiveCSScheme;
    enum ActiveNTTaskScheme _taskActiveNTScheme;

    // Task shape alpha transparency
    int _taskShapeAlpha;

    // Cylinder scheme
    Distance _taskCylinderRadius;

    // Sector scheme
    Distance _taskSectorInnerRadius;
    Distance _taskSectorOuterRadius;
    int _taskSectorAngle;

    // Cylinder/Sector draw options
    bool _taskDrawShape;
    bool _taskFillShape;
};

#endif
