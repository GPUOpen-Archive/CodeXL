
add_library(Qt5::QDeclarativeViewPlugin MODULE IMPORTED)

_populate_Designer_plugin_properties(QDeclarativeViewPlugin RELEASE "designer/libqdeclarativeview.so")

list(APPEND Qt5Designer_PLUGINS Qt5::QDeclarativeViewPlugin)
