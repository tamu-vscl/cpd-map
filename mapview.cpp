/* 
 * File:   mapview.cpp
 * Author: Madison Treat <madison.treat@tamu.edu>
 * 
 * Created on June 15, 2015, 11:19 AM
 */

#include "mapview.h"
#include <QWebView>
#include <QWebFrame>
#include <QWebElement>
#include <QMessageBox>

#include "qt-google-maps/geocode_data_manager.h"
#include "qt-google-maps/mapsettings.h"

#include "core/aircraft.h"
#include "core/hddsettings.h"
#include "core/mapconsts.h"


MapView::MapView(HDDSettings* _hddSettings, MapSettings* _settings, ACMap* _acMap, QWidget* _parent)
: QWidget(_parent),
  hddSettings(_hddSettings),
  settings(_settings),
  acMap(_acMap),
  heading(0.0),
  lat(0.0),
  lon(0.0)
{
   geocode = new GeocodeDataManager(settings->apiKey(), this);
   connect(geocode, SIGNAL(coordinatesReady(double,double)),  this, SLOT(showCoordinates(double,double)));
   connect(geocode, SIGNAL(errorOccurred(QString)),           this, SLOT(errorOccurred(QString)));
   
   QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
//   QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
   webView = new QWebView(this);
   connect(webView, SIGNAL(loadStarted()), this, SLOT(startedLoading()));
   connect(webView, SIGNAL(loadProgress(int)), this, SLOT(loadingProgress(int)));
   connect(webView, SIGNAL(loadFinished(bool)), this, SLOT(finishedLoading(bool)));
   
   qDebug() << "WebView loading HTML file from" << settings->mapHtmlPath();
   webView->setUrl(QUrl::fromLocalFile(settings->mapHtmlPath()));
   
   if (settings->canEnableMaps()) {
      enabled = true;
   }
   setMinimumSize(QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT));
//   resize(QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT));
}

//MapView::MapView(const MapView& orig)
//{
//}

MapView::~MapView()
{
}

QSize MapView::sizeHint() const
{
   return QSize(DEFAULT_MAP_WIDTH, DEFAULT_MAP_HEIGHT);
}

void MapView::resize(int w, int h)
{
   resize(QSize(w, h));
}

void MapView::resize(const QSize& size)
{
   QWidget::resize(size);
//   qDebug() << "Resizing MapView to" << size;
   // resize the map, keeping the current center
//   QString str = "var center = map.getCenter();";
//   str += "google.maps.event.trigger(map,\"'resize\");";
//   str += "map.setCenter(center);";
   
//   QString str = "map.updateSize();";
   QString str = "resize();";
   QVariant response = evaluateJS(str);
//   qDebug() << "   JS response:" << response.toString();
   
   // set the web page's size
//   qDebug() << "   Current viewport size:" << webView->page()->viewportSize();
//   qDebug() << "   Current webframe size:" << webView->page()->currentFrame()->contentsSize();
   
   // The page's viewport size contains the whole HTML document, so this can be
   // used to resize at least the visible map to transition correctly on move
   // and zoom events and generally, for the visible map to look right.
   webView->resize(size);
   webView->page()->setViewportSize(size);
//   qDebug() << "       New viewport size:" << webView->page()->viewportSize();
}


QVariant MapView::evaluateJS(QString js)
{
//   return webView->page()->currentFrame()->documentElement().evaluateJavaScript(js);
   return webView->page()->mainFrame()->evaluateJavaScript(js);
}

void MapView::calculateDistanceScale()
{
//   qDebug() << "Calculating distance scale...";
   QString latlon1 = QString("map.getBounds().getNorthEast()");
   QString latlon2 = QString("map.getBounds().getSouthWest()");

   QString str = QString("google.maps.geometry.spherical.computeDistanceBetween (%1, %2);").arg(latlon1).arg(latlon2);
   QVariant diagDist = evaluateJS(str);
//   qDebug() << "diagDist =" << diagDist;
}

void MapView::startedLoading()
{
   qDebug() << "Started loading URL";
}

void MapView::loadingProgress(int percent)
{
//   qDebug() << "Loading progress:" << percent << "%";
}

void MapView::finishedLoading(bool success)
{
   qDebug() << "Finished loading URL, successful:" << success;
   resize(size());
}


void MapView::showCoordinates(double lat, double lon, bool saveMarker)
{
//   qDebug() << "Form, showCoordinates" << lat << "," << lon;
//   
//   QString str = QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(lat).arg(lon);
//   str += QString("map.setCenter(newLoc);");
//   str += QString("map.setZoom(%1);").arg(settings->zoom());
//   
//   qDebug() << str;
//   
//   evaluateJS(str);
}

void MapView::errorOccurred(const QString& error)
{
   QMessageBox::warning(this, tr("Geocode Error"), error);
}

void MapView::setZoom(int level)
{
   QString str = QString("zoomTo(%1);").arg(level);
   evaluateJS(str);
   calculateDistanceScale();
}

void MapView::panToLocation(float _lat, float _lon)
{
   lat = _lat;
   lon = _lon;
   QString str = QString("panTo(%1, %2);").arg(lat).arg(lon);
   
   QVariant ret = evaluateJS(str);
}

void MapView::setHeading(float _heading)
{
   heading = _heading;
   QString js = QString("rotate(%1, %2, %3);").arg((orientation == TRACK_UP) ? (int) -heading : 0).arg(lat).arg(lon);
   evaluateJS(js);
}

void MapView::setOrientation(MapOrientation mo)
{
   orientation = mo;
   setHeading(heading);
}

void MapView::showSatMap(bool show)
{
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
void MapView::updateAC(int id)
{
   if (id == 0) return; // dont draw this ac
   Aircraft* a = acMap->value(id);
   QString str;
   if (a->hasBeenDisplayed()) {
//      str = QString("updateAircraft");
      str = QString("addNewAircraft");
   }
   else {
      str = QString("addNewAircraft");
      a->setHasBeenDisplayed();
   }
   str += QString("(%1, %2, %3, %4, %5, %6, %7);").arg(a->getID()).arg(a->getLat()).arg(a->getLon()).arg(a->getRng()).arg(a->getBer()).arg(a->getAlt()).arg(a->getHdg());
//   qDebug() << "Updating AC Map Icon:" << str;
   evaluateJS(str);
}
