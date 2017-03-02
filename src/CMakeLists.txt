PROJECT(Csmania CXX)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${Csmania_SOURCE_DIR}
	${Csmania_BINARY_DIR})

# add Qt modules here, the include will setup QT_LIBRARIES
SET(QT_USE_QT TRUE)
SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTWEBKIT TRUE)

INCLUDE(${QT_USE_FILE})

# .cxx sources
SET(Csmania_SRCS_CXX
	settings.cpp
	error.cpp
	utils.cpp
	networkmanager.cpp
	mainwindow.cpp
	main.cpp
)

# .h headers
SET(Csmania_SRCS_HEADERS
	error.h
	utils.h
)

# files which need to be moc'd by Qt
SET(Csmania_MOC_SRCS
	settings.h
	networkmanager.h
	mainwindow.h
)


SET(Csmania_RCCS csmania.qrc)

SET(Csmania_UI
	settings.ui
)

# build ui_XXX files from the XML-style .ui files
QT4_WRAP_UI(Csmania_SRCS_CXX ${Csmania_UI})

# this moc's the above variable and appends to the cxx sources
QT4_WRAP_CPP(Csmania_SRCS_CXX ${Csmania_MOC_SRCS})

# generate rules for building source files from the resources
QT4_ADD_RESOURCES(Csmania_RCC_SRCS ${Csmania_RCCS})

ADD_EXECUTABLE(csmania ${Csmania_SRCS_CXX} ${Csmania_RCC_SRCS} ${Csmania_SRCS_HEADERS})
TARGET_LINK_LIBRARIES(csmania ${QT_LIBRARIES})
