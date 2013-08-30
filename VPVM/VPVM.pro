cache()

MMDAI_ROOT_PATH = $$clean_path($$PWD/..)
VPVM_ROOT_PATH = $$PWD
include("VPVM.pri")

# Add more folders to ship with the application, here
QMLSOURCES.source = qml/VPVM
QMLSOURCES.target = qml
TRANSLATIONS.source = translations
TRANSLATIONS.target = .

CONFIG(debug, debug|release)   { DEPLOYMENTFOLDERS = QMLSOURCES TRANSLATIONS }
CONFIG(release, debug|release) { DEPLOYMENTFOLDERS = TRANSLATIONS }

QT += qml quick widgets concurrent

# Additional import path used to resolve QML modules in Creator's code model

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# Installation path
# target.path =

# Please do not modify the following two lines. Required for deployment.
include(qtquick2applicationviewer/qtquick2applicationviewer.pri)
qtcAddDeployment()

CONFIG += -std=c++11

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    ProjectProxy.cc \
    RenderTarget.cc \
    ModelProxy.cc \
    MotionProxy.cc \
    Grid.cc \
    Util.cc \
    BoneRefObject.cc \
    MorphRefObject.cc \
    CameraRefObject.cc \
    LabelRefObject.cc \
    BoneKeyframeRefObject.cc \
    MorphKeyframeRefObject.cc \
    BaseKeyframeRefObject.cc \
    BoneMotionTrack.cc \
    MorphMotionTrack.cc \
    BaseMotionTrack.cc \
    LightRefObject.cc \
    Preference.cc \
    CameraKeyframeRefObject.cc \
    LightKeyframeRefObject.cc \
    CameraMotionTrack.cc \
    LightMotionTrack.cc \
    UIAuxHelper.cc

HEADERS += \
    ProjectProxy.h \
    RenderTarget.h \
    ModelProxy.h \
    MotionProxy.h \
    Grid.h \
    Util.h \
    BoneRefObject.h \
    MorphRefObject.h \
    CameraRefObject.h \
    LabelRefObject.h \
    BoneKeyframeRefObject.h \
    MorphKeyframeRefObject.h \
    BaseKeyframeRefObject.h \
    BoneMotionTrack.h \
    MorphMotionTrack.h \
    BaseMotionTrack.h \
    LightRefObject.h \
    Preference.h \
    CameraKeyframeRefObject.h \
    LightKeyframeRefObject.h \
    CameraMotionTrack.h \
    LightMotionTrack.h \
    UIAuxHelper.h
