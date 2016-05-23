/* 
 * File:   mapview.cpp
 * Author: Madison Treat <madison.treat@tamu.edu>
 * 
 * Created on June 15, 2015, 11:19 AM
 */

#include "mapview.h"
#include <QWebEngineView>
#include <QWebEngineSettings>
//#include <QWebEngineFrame>
//#include <QWebEngineElement>
#include <QMessageBox>
#include <QNetworkProxy>

#include "core/aircraft.h"

#include "mapcontroller.h"
#include "mapsettings.h"


MapView::MapView(
  MapSettings* _mapSettings,
  MapController* _mapC,
  ACMap* _acMap,
  QWidget* _parent
)
: QWidget(_parent),
mapC(_mapC),
mapSettings(_mapSettings),
acMap(_acMap),
m_isFinishedLoading(false),
heading(0.0),
lat(0.0),
lon(0.0),
showTraffic(false) {
  connect(mapC, &MAPC::updateZoom, this, &MapView::setZoom);
  connect(mapSettings, &MapSettings::mapOrientationChanged, this, &MapView::setOrientation);

  if (mapSettings->useProxy()) {
    qDebug() << "Using a proxy for the map data";
    qDebug() << "   Host:" << mapSettings->proxyHost();
    qDebug() << "   Port:" << mapSettings->proxyPort();
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName(mapSettings->proxyHost());
    proxy.setPort(mapSettings->proxyPort());
    QNetworkProxy::setApplicationProxy(proxy);
  }

  QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
//  QWebEngineSettings::globalSettings()->setAttribute(QWebEngineSettings::DeveloperExtrasEnabled, true);
  webView = new QWebEngineView(this);
  connect(webView, SIGNAL(loadStarted()),      this, SLOT(loadStarted()));
  connect(webView, SIGNAL(loadProgress(int)),  this, SLOT(loadProgress(int)));
  connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
  connect(webView, SIGNAL(loadFinished(bool)), this, SIGNAL(mapLoaded(bool)));

  qDebug() << "WebView loading HTML file from" << mapSettings->mapHtmlPath();
  webView->setUrl(QUrl::fromLocalFile(mapSettings->mapHtmlPath()));

  if (mapSettings->canEnableMaps()) {
    enabled = true;
  }
  setMinimumSize(QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT));
//  resize(QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT));
}

//MapView::MapView(const MapView& orig) {
//}

MapView::~MapView() {
}

QSize MapView::sizeHint() const {
  return QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT);
}

void MapView::resize(int w, int h) {
  resize(QSize(w, h));
}

void MapView::resize(const QSize& size) {
  QWidget::resize(size);
  QString str = "resize();";
  evaluateJS(str);

  // The page's viewport size contains the whole HTML document, so this can be
  // used to resize at least the visible map to transition correctly on move
  // and zoom events and generally, for the visible map to look right.
  webView->resize(size);
}

bool MapView::northUp() const {
  return mapSettings->mapOrientation() == NORTH_UP;
}


//QVariant MapView::evaluateJS(QString js)
void MapView::evaluateJS(QString js) {
  if (isFinishedLoading()) {
    // qDebug() << "Evaluating JS string" << js;
    webView->page()->runJavaScript(js); // Qt 5.5.1+
  }
  else {
    // Do nothing.
    // MapWidget and finishedLoading() will set up initial values.
  }
}

void MapView::calculateDistanceScale() {
//  qDebug() << "Calculating distance scale...";
  QString latlon1 = QString("map.getBounds().getNorthEast()");
  QString latlon2 = QString("map.getBounds().getSouthWest()");

  QString str;// = QString("google.maps.geometry.spherical.computeDistanceBetween (%1, %2);").arg(latlon1).arg(latlon2);
  //QVariant diagDist = evaluateJS(str);
  evaluateJS(str);
//  qDebug() << "diagDist =" << diagDist;
}

void MapView::loadStarted() {
  qDebug() << "Started loading URL";
}

void MapView::loadProgress(int percent) {
//  qDebug() << "Loading progress:" << percent << "%";
}

void MapView::loadFinished(bool success) {
  m_isFinishedLoading = success;
  qDebug() << "Finished loading URL, successful:" << success;
  resize(size());
}


void MapView::showCoordinates(double lat, double lon, bool saveMarker) {
//  qDebug() << "Form, showCoordinates" << lat << "," << lon;
//
//  QString str = QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(lat).arg(lon);
//  str += QString("map.setCenter(newLoc);");
//  str += QString("map.setZoom(%1);").arg(mapSettings->zoom());
//
//  qDebug() << str;
//
//  evaluateJS(str);
}

void MapView::errorOccurred(const QString& error) {
  QMessageBox::warning(this, tr("Geocode Error"), error);
}

void MapView::setZoom(int level) {
  QString str = QString("zoomTo(%1);").arg(level);
  evaluateJS(str);
  calculateDistanceScale();
}

void MapView::panToLocation(float _lat, float _lon) {
  lat = _lat;
  lon = _lon;
  QString str = QString("panTo(%1, %2);").arg(lat).arg(lon);

  evaluateJS(str);
}

void MapView::setHeading(float _heading){
  heading = _heading;
  QString js = QString("rotate(%1, %2, %3);").arg(
    (!northUp()) ? (int) -heading : 0).arg(lat).arg(lon);
  evaluateJS(js);
}

void MapView::setOrientation(MapOrientation mo) {
  setHeading(heading);
}

void MapView::showSatMap(bool show) {
  QString str;
  if (show) {
    str = "addSatLayer();";
  }
  else {
    str = "removeSatLayer();";
  }
  evaluateJS(str);
}

/*
 * Update the icons displayed on the map itself
 */
void MapView::updateAC(int id) {
  if (id == 0 || !showTraffic) return; // dont draw this ac
  Aircraft* a = acMap->value(id);
  QString str;
  if (a->hasBeenDisplayed()) {
    // str = QString("updateAircraft"); // TODO: figure out why this somehow skips the initial draw
    str = QString("addNewAircraft");
  }
  else {
    str = QString("addNewAircraft");
    a->setHasBeenDisplayed();
  }
  str += QString("(%1, %2, %3, %4, %5, %6, %7);").arg(a->getID()).
                                                  arg(a->getLat()).
                                                  arg(a->getLon()).
                                                  arg(a->getRng()).
                                                  arg(a->getBer()).
                                                  arg(a->getAlt()).
                                                  arg(a->getHdg());
//  qDebug() << "Updating AC Map Icon:" << str;
  evaluateJS(str);
}

void MapView::displayTraffic(bool show) {
  showTraffic = show;
  QString str;
  if (showTraffic) {
     str = "showAircraft();";
  }
  else {
     str = "clearAircraft();";
  }
  evaluateJS(str);
}
