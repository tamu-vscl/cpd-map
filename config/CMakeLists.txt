project(cpd-map-config)

set(CONFIG
   google-maps.js
   ol-map.js
   google-maps.example.ini
)

install(FILES ${CONFIG} DESTINATION map/config)

