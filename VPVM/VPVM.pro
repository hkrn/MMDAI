cache()

MMDAI_ROOT_PATH = $$clean_path($$PWD/..)
VPVM_ROOT_PATH = $$PWD
VPVM_QML_PATH = $$clean_path($$VPVM_ROOT_PATH/qml/VPVM)
include("VPVM.pri")

CONFIG(debug, debug|release)   { DEPLOYMENTFOLDERS = QMLSOURCES TRANSLATIONS }
CONFIG(release, debug|release) { DEPLOYMENTFOLDERS = TRANSLATIONS }

QT += qml quick multimedia
!android: !ios: !blackberry: qtHaveModule(widgets): QT += widgets

CONFIG += -std=c++11

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += $$VPVM_ROOT_PATH/main.cpp \
    $$VPVM_ROOT_PATH/ProjectProxy.cc \
    $$VPVM_ROOT_PATH/RenderTarget.cc \
    $$VPVM_ROOT_PATH/ModelProxy.cc \
    $$VPVM_ROOT_PATH/MotionProxy.cc \
    $$VPVM_ROOT_PATH/Grid.cc \
    $$VPVM_ROOT_PATH/Util.cc \
    $$VPVM_ROOT_PATH/BoneRefObject.cc \
    $$VPVM_ROOT_PATH/MorphRefObject.cc \
    $$VPVM_ROOT_PATH/CameraRefObject.cc \
    $$VPVM_ROOT_PATH/LabelRefObject.cc \
    $$VPVM_ROOT_PATH/BoneKeyframeRefObject.cc \
    $$VPVM_ROOT_PATH/MorphKeyframeRefObject.cc \
    $$VPVM_ROOT_PATH/BaseKeyframeRefObject.cc \
    $$VPVM_ROOT_PATH/BoneMotionTrack.cc \
    $$VPVM_ROOT_PATH/MorphMotionTrack.cc \
    $$VPVM_ROOT_PATH/BaseMotionTrack.cc \
    $$VPVM_ROOT_PATH/LightRefObject.cc \
    $$VPVM_ROOT_PATH/Preference.cc \
    $$VPVM_ROOT_PATH/CameraKeyframeRefObject.cc \
    $$VPVM_ROOT_PATH/LightKeyframeRefObject.cc \
    $$VPVM_ROOT_PATH/CameraMotionTrack.cc \
    $$VPVM_ROOT_PATH/LightMotionTrack.cc \
    $$VPVM_ROOT_PATH/UIAuxHelper.cc \
    $$VPVM_ROOT_PATH/WorldProxy.cc \
    $$VPVM_ROOT_PATH/GraphicsDevice.cc \
    $$VPVM_ROOT_PATH/ALAudioContext.cc \
    $$VPVM_ROOT_PATH/ALAudioEngine.cc

HEADERS += \
    $$VPVM_ROOT_PATH/ProjectProxy.h \
    $$VPVM_ROOT_PATH/RenderTarget.h \
    $$VPVM_ROOT_PATH/ModelProxy.h \
    $$VPVM_ROOT_PATH/MotionProxy.h \
    $$VPVM_ROOT_PATH/Grid.h \
    $$VPVM_ROOT_PATH/Util.h \
    $$VPVM_ROOT_PATH/BoneRefObject.h \
    $$VPVM_ROOT_PATH/MorphRefObject.h \
    $$VPVM_ROOT_PATH/CameraRefObject.h \
    $$VPVM_ROOT_PATH/LabelRefObject.h \
    $$VPVM_ROOT_PATH/BoneKeyframeRefObject.h \
    $$VPVM_ROOT_PATH/MorphKeyframeRefObject.h \
    $$VPVM_ROOT_PATH/BaseKeyframeRefObject.h \
    $$VPVM_ROOT_PATH/BoneMotionTrack.h \
    $$VPVM_ROOT_PATH/MorphMotionTrack.h \
    $$VPVM_ROOT_PATH/BaseMotionTrack.h \
    $$VPVM_ROOT_PATH/LightRefObject.h \
    $$VPVM_ROOT_PATH/Preference.h \
    $$VPVM_ROOT_PATH/CameraKeyframeRefObject.h \
    $$VPVM_ROOT_PATH/LightKeyframeRefObject.h \
    $$VPVM_ROOT_PATH/CameraMotionTrack.h \
    $$VPVM_ROOT_PATH/LightMotionTrack.h \
    $$VPVM_ROOT_PATH/UIAuxHelper.h \
    $$VPVM_ROOT_PATH/WorldProxy.h \
    $$VPVM_ROOT_PATH/GraphicsDevice.h \
    $$VPVM_ROOT_PATH/ALAudioContext.h \
    $$VPVM_ROOT_PATH/ALAudioEngine.h \
    $$VPVM_ROOT_PATH/Common.h

OTHER_FILES += $$VPVM_QML_PATH/main.qml \
    $$VPVM_QML_PATH/AboutWindow.qml \
    $$VPVM_QML_PATH/AxesSpinBox.qml \
    $$VPVM_QML_PATH/CameraHandleSet.qml \
    $$VPVM_QML_PATH/CameraTab.qml \
    $$VPVM_QML_PATH/ConfirmWindow.qml \
    $$VPVM_QML_PATH/ExportTab.qml \
    $$VPVM_QML_PATH/FPSCountPanel.qml \
    $$VPVM_QML_PATH/FontAwesome.js \
    $$VPVM_QML_PATH/FontAwesome.otf \
    $$VPVM_QML_PATH/FontAwesome.ttf \
    $$VPVM_QML_PATH/GlobalPreference.qml \
    $$VPVM_QML_PATH/InfoPanel.qml \
    $$VPVM_QML_PATH/InterpolationPanel.qml \
    $$VPVM_QML_PATH/LabelSectionDelegate.qml \
    $$VPVM_QML_PATH/LightTab.qml \
    $$VPVM_QML_PATH/ModelTab.qml \
    $$VPVM_QML_PATH/NotificationArea.qml \
    $$VPVM_QML_PATH/ProjectPreference.qml \
    $$VPVM_QML_PATH/RotationButton.qml \
    $$VPVM_QML_PATH/SaveDialog.qml \
    $$VPVM_QML_PATH/Scene.qml \
    $$VPVM_QML_PATH/SelectableObjectDelegate.qml \
    $$VPVM_QML_PATH/Timeline.qml \
    $$VPVM_QML_PATH/TimelineTab.qml \
    $$VPVM_QML_PATH/TransformHandleSet.qml \
    $$VPVM_QML_PATH/TransformHandle.qml \
    $$VPVM_QML_PATH/TransformMode.qml \
    $$VPVM_QML_PATH/TranslationButton.qml \
    $$VPVM_QML_PATH/Video.qml \
    $$VPVM_QML_PATH/WindowLoader.qml
