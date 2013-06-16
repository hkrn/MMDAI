QT += core
QT -= gui

LIBS += -framework Cg
!mac:LIBS += -lCg -lCgGL

DEFINES += GL_GLEXT_PROTOTYPES

# COLLADA_DOM_ROOT_PATH = /Users/hkrn/src/collada-dom-2.3.1/product/install-root
# COLLADA_DOM_LIBRARY_PATH = $${COLLADA_DOM_ROOT_PATH}/lib
# COLLADA_DOM_INCLUDE_PATH = $${COLLADA_DOM_ROOT_PATH}/include/collada-dom
# LIBS += -L$${COLLADA_DOM_LIBRARY_PATH} -lcollada15dom -L/opt/local/lib -lboost_filesystem-mt -lboost_system-mt
# INCLUDEPATH += $${COLLADA_DOM_INCLUDE_PATH} $${COLLADA_DOM_INCLUDE_PATH}/1.5 /opt/local/include
# DEFINES += COLLADA_DOM_SUPPORT141 COLLADA_DOM_SUPPORT150 COLLADA_DOM_DAEFLOAT_IS64 COLLADA_DOM_USING_150

OPEN_COLLADA_ROOT_PATH = /Users/hkrn/src/OpenCOLLADA/product/install-root
OPEN_COLLADA_INCLUDE_PATH = $${OPEN_COLLADA_ROOT_PATH}/include/opencollada
LIBS += -L$${OPEN_COLLADA_ROOT_PATH}/lib/opencollada -lOpenCOLLADAStreamWriter -lOpenCOLLADAFramework -lOpenCOLLADABaseUtils -lpcre -lUTF -lbuffer -lftoa
INCLUDEPATH += $${OPEN_COLLADA_INCLUDE_PATH}/COLLADABaseUtils $${OPEN_COLLADA_INCLUDE_PATH}/COLLADAFramework $${OPEN_COLLADA_INCLUDE_PATH}/COLLADAStreamWriter $${OPEN_COLLADA_INCLUDE_PATH}/COLLADASaxFrameworkLoader

OSMESA_ROOT_PATH = /Users/hkrn/src/MMDAI/mesa-src/
OSMESA_LIBRARY_PATH = $${OSMESA_ROOT_PATH}/build/darwin-x86_64/mesa
LIBS += -L$${OSMESA_LIBRARY_PATH} -lmesa -L$${OSMESA_LIBRARY_PATH}/drivers/osmesa -losmesa -lGLEW
INCLUDEPATH += $${OSMESA_ROOT_PATH}/include ../../tinyxml2-src

TARGET = cgfx2dae
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cc ../../tinyxml2-src/tinyxml2.cpp
