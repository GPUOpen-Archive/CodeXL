
add_library(Qt5::QGeoServiceProviderFactoryOsm MODULE IMPORTED)

_populate_Location_plugin_properties(QGeoServiceProviderFactoryOsm RELEASE "geoservices/libqtgeoservices_osm.so")

list(APPEND Qt5Location_PLUGINS Qt5::QGeoServiceProviderFactoryOsm)
