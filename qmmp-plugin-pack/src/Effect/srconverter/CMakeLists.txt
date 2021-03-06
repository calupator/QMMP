project(libsrconverter)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# libqmmp
include_directories(${QMMP_INCLUDE_DIRS})
link_directories( ${QMMP_LIBRARY_DIRS})

pkg_check_modules(SAMPLERATE samplerate)

include_directories(${SAMPLERATE_INCLUDE_DIRS})
link_directories(${SAMPLERATE_LIBRARY_DIRS})
ADD_DEFINITIONS(${SAMPLERATE_CFLAGS})

SET(libsrconverter_SRCS
    srconverter.cpp
    settingsdialog.cpp
    effectsrconverterfactory.cpp
)

SET(libsrconverter_HDRS
    srconverter.h
)

SET(libsrconverter_RCCS translations/translations.qrc)

QT5_ADD_RESOURCES(libsrconverter_RCC_SRCS ${libsrconverter_RCCS})

# user interface


SET(libsrconverter_UIS
    settingsdialog.ui
)

QT5_WRAP_UI(libsrconverter_UIS_H ${libsrconverter_UIS})
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_BINARY_DIR})

IF(SAMPLERATE_FOUND)
ADD_LIBRARY(srconverter MODULE ${libsrconverter_SRCS} ${libsrconverter_UIS_H}
  ${libsrconverter_RCC_SRCS} ${libsrconverter_HDRS})
target_link_libraries(srconverter Qt5::Widgets ${QMMP_LDFLAGS} ${SAMPLERATE_LDFLAGS})
install(TARGETS srconverter DESTINATION ${PLUGIN_DIR}/Effect)
ENDIF(SAMPLERATE_FOUND)
