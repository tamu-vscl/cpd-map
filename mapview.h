/* 
 * File:   MapView.
 * Author: Madison Treat <madison.treat@tamu.edu>
 *
 * Created on June 15, 2015, 11:21 AM
 * 
 * The MapView class contains the raw map from Google.
 */

#ifndef MAPVIEW_
#define MAPVIEW_

#include <QWidget>

#include "core/aircraft.h"

#include "mapconsts.h"


class QPaintEvent;
class QWebEngineView;

class MapController;
class MapSettings;
class GeocodeDataManager;


class MapView : public QWidget {
  Q_OBJECT;

public:
  MapView(MapSettings* _mapSettings, MapController* _mapC, ACMap* _acMap, QWidget* _parent = 0);
  MapView(const MapView& orig) = delete;
  virtual ~MapView();

  QSize sizeHint() const;
  void resize(int w, int h);
  void resize(const QSize& size);

  bool northUp() const;

  bool isFinishedLoading() const {return m_isFinishedLoading;}

public slots:
  void loadStarted();
  void loadProgress(int percent);
  void loadFinished(bool success);

  void showCoordinates(double lat, double lon, bool saveMarker=true);
  void errorOccurred(const QString& error);

  void setZoom(int level);
  void panToLocation(float _lat, float _lon);
  void setHeading(float _heading);

  void setOrientation(MapOrientation mo);

  void showSatMap(bool show);

  void updateAC(int id);
  void displayTraffic(bool show);

signals:
  void mapLoaded(bool success);

private:
  MapController* mapC;
  MapSettings*   mapSettings;

  bool    enabled;  // are the maps enabled?
  ACMap*  acMap;
  bool    m_isFinishedLoading;

  QWebEngineView*      webView;
  GeocodeDataManager*  geocode;

  double heading;      // current heading value in degrees
  double lat;          // current lat
  double lon;          // current lon
  bool   showTraffic;  // currently displaying traffic?

  //QVariant evaluateJS(QString js);
  void evaluateJS(QString js);
  void calculateDistanceScale();
};

#endif  // MAPVIEW_

