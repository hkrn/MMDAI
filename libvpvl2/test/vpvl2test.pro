QT += core gui opengl

TARGET = vpvl2test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
macx:DEFINES += USE_FILE32API

QMAKE_CXXFLAGS = -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings

ASSIMP_PATH = ../../assimp-src
BULLET_PATH = ../../bullet-src
ICU_PATH = ../../icu-src
LIBXML2_PATH = ../../libxml2-src
VPVL1_PATH = ../../libvpvl
VPVL2_PATH = ..
INSTALL_ROOT_DIR = install-root
PRODUCT_DIRECTORY = build-debug/$${INSTALL_ROOT_DIR}

LIBS += -L$${VPVL1_PATH}/$${PRODUCT_DIRECTORY}/lib \
        -L$${VPVL2_PATH}/build-debug/lib \ # uses internal library
        -L$${LIBXML2_PATH}/$${PRODUCT_DIRECTORY}/lib \
        -L$${BULLET_PATH}/$${PRODUCT_DIRECTORY}/lib \
        -L$${ASSIMP_PATH}/$${PRODUCT_DIRECTORY}/lib \
        -L$${ICU_PATH}/$${PRODUCT_DIRECTORY}/lib \
        -lvpvl2qtcommon -lvpvl2 -lvpvl -licuuc -licui18n -licudata -lGLEW \
        -lxml2 -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lassimp -lz
macx:LIBS += -framework Cg
linux-*:LIBS += -lCg -lCgGL

INCLUDEPATH += gtest-1.6.0 gmock-1.6.0 \
               gtest-1.6.0/include gmock-1.6.0/include \
               $${VPVL2_PATH}/include \ # uses internal headers
               $${VPVL2_PATH}/$${PRODUCT_DIRECTORY}/include \
               $${VPVL1_PATH}/$${PRODUCT_DIRECTORY}/include \
               $${LIBXML2_PATH}/$${PRODUCT_DIRECTORY}/include/libxml2 \
               $${BULLET_PATH}/$${PRODUCT_DIRECTORY}/include/bullet \
               $${ASSIMP_PATH}/$${PRODUCT_DIRECTORY}/include/assimp \
               $${ICU_PATH}/$${PRODUCT_DIRECTORY}/include

linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  VPVL2_TEST_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${VPVL2_TEST_RPATH}\'
}

HEADERS += mock/Bone.h \
    mock/BoneKeyframe.h \
    mock/Camera.h \
    mock/CameraKeyframe.h \
    mock/Effect.h \
    mock/Label.h \
    mock/Light.h \
    mock/LightKeyframe.h \
    mock/Model.h \
    mock/Morph.h \
    mock/MorphKeyframe.h \
    mock/Motion.h \
    mock/RenderContext.h \
    Common.h

SOURCES += main.cc \
    InternalTest.cc \
    AssetModelTest.cc \
    PMDModelTest.cc \
    PMXModelTest.cc \
    VMDMotionTest.cc \
    MVDMotionTest.cc \
    ProjectTest.cc \
    StringTest.cc \
    EncodingTest.cc \
    ArchiveTest.cc \
    EffectTest.cc \
    FactoryTest.cc \
    SceneTest.cc \
    Common.cc \
    gmock-1.6.0/src/gmock-all.cc \
    gtest-1.6.0/src/gtest-all.cc

RESOURCES += \
    fixtures.qrc
