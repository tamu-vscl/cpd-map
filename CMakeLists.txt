project(cpd-map)

add_subdirectory(config)

set(SOURCES
   mapcontroller.cpp
   mapsettings.cpp
   mapwidget.cpp
   mapview.cpp
   mapsettings.cpp
   mapoverlay.cpp
)

set(HEADERS
   mapconsts.h
   mapcontroller.h
   mapsettings.cpp
   mapwidget.h
   mapview.h
   mapoverlay.h
)

set(MAP_RCC maprcc.qrc)
set(RESOURCES ../resources/resources.qrc)


add_library(${PROJECT_NAME} SHARED
   ${SOURCES}
   ${MOC_SRC}
   ${MAP_RCC}
   ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
   Qt5::Core
   Qt5::Widgets
   Qt5::Gui
   Qt5::WebEngine
   Qt5::WebEngineWidgets
   Qt5::Svg
   cpd-core
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
# install(FILES ${MAP_RCC} DESTINATION map/html)
