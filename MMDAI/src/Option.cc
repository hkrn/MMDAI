/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* headers */

#include "MMDAI/MMDAI.h"

namespace MMDAI {

/* str2bool: convert string to boolean */
static bool str2bool(char *str)
{
    if(MMDAIStringEquals(str, "TRUE") || MMDAIStringEquals(str, "True") || MMDAIStringEquals(str, "true"))
        return true;
    else
        return false;
}

/* str2int: convert string to integer */
static int str2int(char *str)
{
    return atoi(str);
}

/* str2float: convert string to float */
static float str2float(char *str)
{
    return (float) atof(str);
}

/* str2ivec2: convert string to 2 integer */
static bool str2ivec2(char *str, int *ivec2)
{
    int i = 0;
    char *p;

    for(p = strtok(str, ","); p && i < 2; p = strtok(NULL, ","))
        ivec2[i++] = str2int(p);
    if(i == 2)
        return true;
    else
        return false;
}

/* str2fvec3: convert string to 3 float */
static bool str2fvec3(char *str, float *fvec3)
{
    int i = 0;
    char *p;

    for(p = strtok(str, ","); p && i < 3; p = strtok(NULL, ","))
        fvec3[i++] = str2float(p);
    if(i == 3)
        return true;
    else
        return false;
}

/* str2fvec4: convert string to 4 float */
static bool str2fvec4(char *str, float *fvec4)
{
    int i = 0;
    char *p;

    for(p = strtok(str, ","); p && i < 4; p = strtok(NULL, ","))
        fvec4[i++] = str2float(p);
    if(i == 4)
        return true;
    else
        return false;
}

/* Option::initialize: initialize options */
void Option::initialize()
{
    m_useCartoonRendering = OPTION_USECARTOONRENDERING_DEF;
    m_useMMDLikeCartoon = OPTION_USEMMDLIKECARTOON_DEF;
    m_cartoonEdgeWidth = OPTION_CARTOONEDGEWIDTH_DEF;
    m_cartoonEdgeStep = OPTION_CARTOONEDGESTEP_DEF;

    m_stageSize[0] = OPTION_STAGESIZEW_DEF;
    m_stageSize[1] = OPTION_STAGESIZED_DEF;
    m_stageSize[2] = OPTION_STAGESIZEH_DEF;

    m_showFps = OPTION_SHOWFPS_DEF;
    m_fpsPosition[0] = OPTION_FPSPOSITIONX_DEF;
    m_fpsPosition[1] = OPTION_FPSPOSITIONY_DEF;
    m_fpsPosition[2] = OPTION_FPSPOSITIONZ_DEF;

    m_windowSize[0] = OPTION_WINDOWSIZEW_DEF;
    m_windowSize[1] = OPTION_WINDOWSIZEH_DEF;
    m_topMost = OPTION_TOPMOST_DEF;
    m_fullScreen = OPTION_FULLSCREEN_DEF;

    m_logSize[0] = OPTION_LOGSIZEW_DEF;
    m_logSize[1] = OPTION_LOGSIZEH_DEF;
    m_logPosition[0] = OPTION_LOGPOSITIONX_DEF;
    m_logPosition[1] = OPTION_LOGPOSITIONY_DEF;
    m_logPosition[2] = OPTION_LOGPOSITIONZ_DEF;
    m_logScale = OPTION_LOGSCALE_DEF;

    m_lightDirection[0] = OPTION_LIGHTDIRECTIONX_DEF;
    m_lightDirection[1] = OPTION_LIGHTDIRECTIONY_DEF;
    m_lightDirection[2] = OPTION_LIGHTDIRECTIONZ_DEF;
    m_lightDirection[3] = OPTION_LIGHTDIRECTIONI_DEF;
    m_lightIntensity = OPTION_LIGHTINTENSITY_DEF;
    m_lightColor[0] = OPTION_LIGHTCOLORR_DEF;
    m_lightColor[1] = OPTION_LIGHTCOLORG_DEF;
    m_lightColor[2] = OPTION_LIGHTCOLORB_DEF;

    m_campusColor[0] = OPTION_CAMPUSCOLORR_DEF;
    m_campusColor[1] = OPTION_CAMPUSCOLORG_DEF;
    m_campusColor[2] = OPTION_CAMPUSCOLORB_DEF;

    m_maxMultiSampling = OPTION_MAXMULTISAMPLING_DEF;
    m_maxMultiSamplingColor = OPTION_MAXMULTISAMPLINGCOLOR_DEF;

    m_motionAdjustFrame = OPTION_MOTIONADJUSTFRAME_DEF;

    m_bulletFps = OPTION_BULLETFPS_DEF;

    m_rotateStep = OPTION_ROTATESTEP_DEF;
    m_translateStep = OPTION_TRANSLATESTEP_DEF;
    m_scaleStep = OPTION_SCALESTEP_DEF;

    m_useShadowMapping = OPTION_USESHADOWMAPPING_DEF;
    m_shadowMapTextureSize = OPTION_SHADOWMAPPINGTEXTURESIZE_DEF;
    m_shadowMapSelfDensity = OPTION_SHADOWMAPPINGSELFDENSITY_DEF;
    m_shadowMapFloorDensity = OPTION_SHADOWMAPPINGFLOORDENSITY_DEF;
    m_shadowMapLightFirst = OPTION_SHADOWMAPPINGLIGHTFIRST_DEF;
}

/* Option::Option: constructor */
Option::Option()
{
    initialize();
}

/* Option::load: load options from config file */
bool Option::load(const char *file)
{
    FILE *fp;
    char buf[OPTION_MAXBUFLEN];
    char *p1;

    int ivec2[2];
    float fvec3[3];
    float fvec4[4];

    fp = fopen(file, "r");
    if (fp == NULL)
        return false;

    while (fgets(buf, OPTION_MAXBUFLEN, fp)) {
        p1 = &(buf[MMDAIStringLength(buf)-1]);
        while (p1 >= &(buf[0]) && (*p1 == '\n' || *p1 == '\r' || *p1 == '\t' || *p1 == ' ')) {
            *p1 = L'\0';
            p1--;
        }
        p1 = &(buf[0]);
        if (*p1 == '#') continue;
        while (*p1 != L'=' && *p1 != L'\0') p1++;
        if (*p1 == L'\0') continue;
        *p1 = L'\0';
        p1++;

        /* overwrite option values */
        if(MMDAIStringEquals(buf, OPTION_USECARTOONRENDERING_STR)) {
            setUseCartoonRendering(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_USEMMDLIKECARTOON_STR)) {
            setUseMMDLikeCartoon(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_CARTOONEDGEWIDTH_STR)) {
            setCartoonEdgeWidth(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_CARTOONEDGESTEP_STR)) {
            setCartoonEdgeStep(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_STAGESIZE_STR)) {
            if(str2fvec3(p1, fvec3))
                setStageSize(fvec3);
        } else if(MMDAIStringEquals(buf, OPTION_SHOWFPS_STR)) {
            setShowFps(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_FPSPOSITION_STR)) {
            if(str2fvec3(p1, fvec3))
                setFpsPosition(fvec3);
        } else if(MMDAIStringEquals(buf, OPTION_WINDOWSIZE_STR)) {
            if(str2ivec2(p1, ivec2))
                setWindowSize(ivec2);
        } else if(MMDAIStringEquals(buf, OPTION_TOPMOST_STR)) {
            setTopMost(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_FULLSCREEN_STR)) {
            setFullScreen(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_LOGSIZE_STR)) {
            if(str2ivec2(p1, ivec2))
                setLogSize(ivec2);
        } else if(MMDAIStringEquals(buf, OPTION_LOGPOSITION_STR)) {
            if(str2fvec3(p1, fvec3))
                setLogPosition(fvec3);
        } else if(MMDAIStringEquals(buf, OPTION_LOGSCALE_STR)) {
            setLogScale(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_LIGHTDIRECTION_STR)) {
            if(str2fvec4(p1, fvec4))
                setLightDirection(fvec4);
        } else if(MMDAIStringEquals(buf, OPTION_LIGHTINTENSITY_STR)) {
            setLightIntensity(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_LIGHTCOLOR_STR)) {
            if(str2fvec3(p1, fvec3))
                setLightColor(fvec3);
        } else if(MMDAIStringEquals(buf, OPTION_CAMPUSCOLOR_STR)) {
            if(str2fvec3(p1, fvec3))
                setCampusColor(fvec3);
        } else if(MMDAIStringEquals(buf, OPTION_MAXMULTISAMPLING_STR)) {
            setMaxMultiSampling(str2int(p1));
        } else if(MMDAIStringEquals(buf, OPTION_MAXMULTISAMPLINGCOLOR_STR)) {
            setMaxMultiSamplingColor(str2int(p1));
        } else if(MMDAIStringEquals(buf, OPTION_MOTIONADJUSTFRAME_STR)) {
            setMotionAdjustFrame(str2int(p1));
        } else if(MMDAIStringEquals(buf, OPTION_BULLETFPS_STR)) {
            setBulletFps(str2int(p1));
        } else if(MMDAIStringEquals(buf, OPTION_ROTATESTEP_STR)) {
            setRotateStep(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_TRANSLATESTEP_STR)) {
            setTranslateStep(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_SCALESTEP_STR)) {
            setScaleStep(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_USESHADOWMAPPING_STR)) {
            setUseShadowMapping(str2bool(p1));
        } else if(MMDAIStringEquals(buf, OPTION_SHADOWMAPPINGTEXTURESIZE_STR)) {
            setShadowMappingTextureSize(str2int(p1));
        } else if(MMDAIStringEquals(buf, OPTION_SHADOWMAPPINGSELFDENSITY_STR)) {
            setShadowMappingSelfDensity(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_SHADOWMAPPINGFLOORDENSITY_STR)) {
            setShadowMappingFloorDensity(str2float(p1));
        } else if(MMDAIStringEquals(buf, OPTION_SHADOWMAPPINGLIGHTFIRST_STR)) {
            setShadowMappingLightFirst(str2bool(p1));
        }
    }
    fclose(fp);

    return true;
}

/* Option::getUseCartoonRendering: get cartoon rendering flag */
bool Option::getUseCartoonRendering()
{
    return m_useCartoonRendering;
}

/* Option::setUseCartoonRendering: set cartoon rendering flag */
void Option::setUseCartoonRendering(bool b)
{
    m_useCartoonRendering = b;
}

/* Option::getUseMMDLikeCartoon: get MMD like cartoon flag */
bool Option::getUseMMDLikeCartoon()
{
    return m_useMMDLikeCartoon;
}

/* Option::setUseMMDLikeCartoon: set MMD like cartoon flag */
void Option::setUseMMDLikeCartoon(bool b)
{
    m_useMMDLikeCartoon = b;
}

/* Option::getCartoonEdgeWidth: get edge width for catoon */
float Option::getCartoonEdgeWidth()
{
    return m_cartoonEdgeWidth;
}

/* Option::setCartoonEdgeWidth: set edge width for catoon */
void Option::setCartoonEdgeWidth(float f)
{
    if(OPTION_CARTOONEDGEWIDTH_MAX < f)
        m_cartoonEdgeWidth = OPTION_CARTOONEDGEWIDTH_MAX;
    else if(OPTION_CARTOONEDGEWIDTH_MIN > f)
        m_cartoonEdgeWidth = OPTION_CARTOONEDGEWIDTH_MIN;
    else
        m_cartoonEdgeWidth = f;
}

/* Option::getCartoonEdgeStep: get cartoon edge step */
float Option::getCartoonEdgeStep()
{
    return m_cartoonEdgeStep;
}

/* Option::setCartoonEdgeStep: set cartoon edge step */
void Option::setCartoonEdgeStep(float f)
{
    if(OPTION_CARTOONEDGESTEP_MAX < f)
        m_cartoonEdgeStep = OPTION_CARTOONEDGESTEP_MAX;
    else if(OPTION_CARTOONEDGESTEP_MIN > f)
        m_cartoonEdgeStep = OPTION_CARTOONEDGESTEP_MIN;
    else
        m_cartoonEdgeStep = f;
}

/* Option::getStageSize: get stage size */
float *Option::getStageSize()
{
    return m_stageSize;
}

/* Option::setStageSize: set stage size */
void Option::setStageSize(float *f)
{
    if(OPTION_STAGESIZE_MAX < f[0])
        m_stageSize[0] = OPTION_STAGESIZE_MAX;
    else if(OPTION_STAGESIZE_MIN > f[0])
        m_stageSize[0] = OPTION_STAGESIZE_MIN;
    else
        m_stageSize[0] = f[0];

    if(OPTION_STAGESIZE_MAX < f[1])
        m_stageSize[1] = OPTION_STAGESIZE_MAX;
    else if(OPTION_STAGESIZE_MIN > f[1])
        m_stageSize[1] = OPTION_STAGESIZE_MIN;
    else
        m_stageSize[1] = f[1];

    if(OPTION_STAGESIZE_MAX < f[2])
        m_stageSize[2] = OPTION_STAGESIZE_MAX;
    else if(OPTION_STAGESIZE_MIN > f[2])
        m_stageSize[2] = OPTION_STAGESIZE_MIN;
    else
        m_stageSize[2] = f[2];
}

/* Option::getShowFps: get fps flag */
bool Option::getShowFps()
{
    return m_showFps;
}

/* Option::setShowFps: set fps flag */
void Option::setShowFps(bool b)
{
    m_showFps = b;
}

/* Option::getFpsPosition: get fps position */
float *Option::getFpsPosition()
{
    return m_fpsPosition;
}

/* Option::setFpsPosition: set fps position */
void Option::setFpsPosition(float *f)
{
    m_fpsPosition[0] = f[0];
    m_fpsPosition[1] = f[1];
    m_fpsPosition[2] = f[2];
}

/* Option::getWindowSize: get window size */
int *Option::getWindowSize()
{
    return m_windowSize;
}

/* Option::setWindowSize: set window size */
void Option::setWindowSize(int *i)
{
    if(OPTION_WINDOWSIZE_MAX < i[0])
        m_windowSize[0] = OPTION_WINDOWSIZE_MAX;
    else if(OPTION_WINDOWSIZE_MIN > i[0])
        m_windowSize[0] = OPTION_WINDOWSIZE_MIN;
    else
        m_windowSize[0] = i[0];

    if(OPTION_WINDOWSIZE_MAX < i[1])
        m_windowSize[1] = OPTION_WINDOWSIZE_MAX;
    else if(OPTION_WINDOWSIZE_MIN > i[1])
        m_windowSize[1] = OPTION_WINDOWSIZE_MIN;
    else
        m_windowSize[1] = i[1];
}

/* Option::getTopMost: get top most flag */
bool Option::getTopMost()
{
    return m_topMost;
}

/* Option::setTopMost: set top most flag */
void Option::setTopMost(bool b)
{
    m_topMost = b;
}

/* Option::getFullScreen: get full screen flag */
bool Option::getFullScreen()
{
    return m_fullScreen;
}

/* Option::setFullScreen: set full screen flag */
void Option::setFullScreen(bool b)
{
    m_fullScreen = b;
}

/* Option::getLogSize: get log window size */
int *Option::getLogSize()
{
    return m_logSize;
}

/* Option::setLogSize: set log window size */
void Option::setLogSize(int *i)
{
    if(OPTION_LOGSIZE_MAX < i[0])
        m_logSize[0] = OPTION_LOGSIZE_MAX;
    else if(OPTION_LOGSIZE_MIN > i[0])
        m_logSize[0] = OPTION_LOGSIZE_MIN;
    else
        m_logSize[0] = i[0];

    if(OPTION_LOGSIZE_MAX < i[1])
        m_logSize[1] = OPTION_LOGSIZE_MAX;
    else if(OPTION_LOGSIZE_MIN > i[1])
        m_logSize[1] = OPTION_LOGSIZE_MIN;
    else
        m_logSize[1] = i[1];
}

/* Option::getLogPosition: get log window position */
float *Option::getLogPosition()
{
    return m_logPosition;
}

/* Option::setLogPosition: set log window position */
void Option::setLogPosition(float *f)
{
    m_logPosition[0] = f[0];
    m_logPosition[1] = f[1];
    m_logPosition[2] = f[2];
}

/* Option::getLogScale: get log window scale */
float Option::getLogScale()
{
    return m_logScale;
}

/* Option::setLogScale: set log window scale */
void Option::setLogScale(float f)
{
    if(OPTION_LOGSCALE_MAX < f)
        m_logScale = OPTION_LOGSCALE_MAX;
    else if(OPTION_LOGSCALE_MIN > f)
        m_logScale = OPTION_LOGSCALE_MIN;
    else
        m_logScale = f;
}

/* Option::getLogDirection: get light direction */
float *Option::getLightDirection()
{
    return m_lightDirection;
}

/* Option::setLogDirection: set light direction */
void Option::setLightDirection(float *f)
{
    m_lightDirection[0] = f[0];
    m_lightDirection[1] = f[1];
    m_lightDirection[2] = f[2];
    m_lightDirection[3] = f[3];
}

/* Option::getLogIntensity: get light intensity */
float Option::getLightIntensity()
{
    return m_lightIntensity;
}

/* Option::setLogIntensity: set light intensity */
void Option::setLightIntensity(float f)
{
    if(OPTION_LIGHTINTENSITY_MAX < f)
        m_lightIntensity = OPTION_LIGHTINTENSITY_MAX;
    else if(OPTION_LIGHTINTENSITY_MIN > f)
        m_lightIntensity = OPTION_LIGHTINTENSITY_MIN;
    else
        m_lightIntensity = f;
}

/* Option::getLightColor: get light color */
float *Option::getLightColor()
{
    return m_lightColor;
}

/* Option::setLightColor: set light color */
void Option::setLightColor(float *f)
{
    if(OPTION_LIGHTCOLOR_MAX < f[0])
        m_lightColor[0] = OPTION_LIGHTCOLOR_MAX;
    else if(OPTION_LIGHTCOLOR_MIN > f[0])
        m_lightColor[0] = OPTION_LIGHTCOLOR_MIN;
    else
        m_lightColor[0] = f[0];

    if(OPTION_LIGHTCOLOR_MAX < f[1])
        m_lightColor[1] = OPTION_LIGHTCOLOR_MAX;
    else if(OPTION_LIGHTCOLOR_MIN > f[1])
        m_lightColor[1] = OPTION_LIGHTCOLOR_MIN;
    else
        m_lightColor[1] = f[1];

    if(OPTION_LIGHTCOLOR_MAX < f[2])
        m_lightColor[2] = OPTION_LIGHTCOLOR_MAX;
    else if(OPTION_LIGHTCOLOR_MIN > f[2])
        m_lightColor[2] = OPTION_LIGHTCOLOR_MIN;
    else
        m_lightColor[2] = f[2];
}

/* Option::getCampusColor: get campus color */
float *Option::getCampusColor()
{
    return m_campusColor;
}

/* Option::setCampusColor: set campus color */
void Option::setCampusColor(float *f)
{
    if(OPTION_CAMPUSCOLOR_MAX < f[0])
        m_campusColor[0] = OPTION_CAMPUSCOLOR_MAX;
    else if(OPTION_CAMPUSCOLOR_MIN > f[0])
        m_campusColor[0] = OPTION_CAMPUSCOLOR_MIN;
    else
        m_campusColor[0] = f[0];

    if(OPTION_CAMPUSCOLOR_MAX < f[1])
        m_campusColor[1] = OPTION_CAMPUSCOLOR_MAX;
    else if(OPTION_CAMPUSCOLOR_MIN > f[1])
        m_campusColor[1] = OPTION_CAMPUSCOLOR_MIN;
    else
        m_campusColor[1] = f[1];

    if(OPTION_CAMPUSCOLOR_MAX < f[2])
        m_campusColor[2] = OPTION_CAMPUSCOLOR_MAX;
    else if(OPTION_CAMPUSCOLOR_MIN > f[2])
        m_campusColor[2] = OPTION_CAMPUSCOLOR_MIN;
    else
        m_campusColor[2] = f[2];
}

/* Option::getMaxMultiSampling: get max number of multi sampling */
int Option::getMaxMultiSampling()
{
    return m_maxMultiSampling;
}

/* Option::setMaxMultiSampling: set max number of multi sampling */
void Option::setMaxMultiSampling(int i)
{
    if(OPTION_MAXMULTISAMPLING_MAX < i)
        m_maxMultiSampling = OPTION_MAXMULTISAMPLING_MAX;
    else if(OPTION_MAXMULTISAMPLING_MIN > i)
        m_maxMultiSampling = OPTION_MAXMULTISAMPLING_MIN;
    else
        m_maxMultiSampling = i;
}

/* Option::getMaxMultiSamplingColor: get max number of multi sampling color */
int Option::getMaxMultiSamplingColor()
{
    return m_maxMultiSamplingColor;
}

/* Option::setMaxMultiSamplingColor: set max number of multi sampling color */
void Option::setMaxMultiSamplingColor(int i)
{
    if(OPTION_MAXMULTISAMPLINGCOLOR_MAX < i)
        m_maxMultiSamplingColor = OPTION_MAXMULTISAMPLINGCOLOR_MAX;
    else if(OPTION_MAXMULTISAMPLINGCOLOR_MIN > i)
        m_maxMultiSamplingColor = OPTION_MAXMULTISAMPLINGCOLOR_MIN;
    else
        m_maxMultiSamplingColor = i;
}

/* Option::getMotionAdjustFrame: get motion adjust frame */
int Option::getMotionAdjustFrame()
{
    return m_motionAdjustFrame;
}

/* Option::setMotionAdjustFrame: set motion adjust frame */
void Option::setMotionAdjustFrame(int i)
{
    m_motionAdjustFrame = i;
}

/* Option::getBulletFps: get bullet fps */
int Option::getBulletFps()
{
    return m_bulletFps;
}

/* Option::setBulletFps: set bullet fps */
void Option::setBulletFps(int i)
{
    if(OPTION_BULLETFPS_MAX < i)
        m_bulletFps = OPTION_BULLETFPS_MAX;
    else if(OPTION_BULLETFPS_MIN > i)
        m_bulletFps = OPTION_BULLETFPS_MIN;
    else
        m_bulletFps = i;
}

/* Option::getRotateStep: get rotate step */
float Option::getRotateStep()
{
    return m_rotateStep;
}

/* Option::setRotateStep: set rotate step */
void Option::setRotateStep(float f)
{
    if(OPTION_ROTATESTEP_MAX < f)
        m_rotateStep = OPTION_ROTATESTEP_MAX;
    else if(OPTION_ROTATESTEP_MIN > f)
        m_rotateStep = OPTION_ROTATESTEP_MIN;
    else
        m_rotateStep = f;
}

/* Option::getTranslateStep: get translate step */
float Option::getTranslateStep()
{
    return m_translateStep;
}

/* Option::setTranslateStep: set translate step */
void Option::setTranslateStep(float f)
{
    if(OPTION_TRANSLATESTEP_MAX < f)
        m_translateStep = OPTION_TRANSLATESTEP_MAX;
    else if(OPTION_TRANSLATESTEP_MIN > f)
        m_translateStep = OPTION_TRANSLATESTEP_MIN;
    else
        m_translateStep = f;
}

/* Option::getScaleStep: get scale step */
float Option::getScaleStep()
{
    return m_scaleStep;
}

/* Option::setScaleStep: set scale step */
void Option::setScaleStep(float f)
{
    if(OPTION_SCALESTEP_MAX < f)
        m_scaleStep = OPTION_SCALESTEP_MAX;
    else if(OPTION_SCALESTEP_MIN > f)
        m_scaleStep = OPTION_SCALESTEP_MIN;
    else
        m_scaleStep = f;
}

/* Option::getUseShadowMapping: get shadow mapping flag */
bool Option::getUseShadowMapping()
{
    return m_useShadowMapping;
}

/* Option::setUseShadowMapping: set shadow mapping flag */
void Option::setUseShadowMapping(bool b)
{
    m_useShadowMapping = b;
}

/* Option::getShadowMappingTextureSize: get texture size of shadow mapping */
int Option::getShadowMappingTextureSize()
{
    return m_shadowMapTextureSize;
}

/* Option::setShadowMappingTextureSize: set texture size of shadow mapping */
void Option::setShadowMappingTextureSize(int i)
{
    if(OPTION_SHADOWMAPPINGTEXTURESIZE_MAX < i)
        m_shadowMapTextureSize = OPTION_SHADOWMAPPINGTEXTURESIZE_MAX;
    else if(OPTION_SHADOWMAPPINGTEXTURESIZE_MIN > i)
        m_shadowMapTextureSize = OPTION_SHADOWMAPPINGTEXTURESIZE_MIN;
    else
        m_shadowMapTextureSize = i;
}

/* Option::getShadowMappingSelfDensity: get self density of shadow mapping */
float Option::getShadowMappingSelfDensity()
{
    return m_shadowMapSelfDensity;
}

/* Option::setShadowMappingSelfDensity: set self density of shadow mapping */
void Option::setShadowMappingSelfDensity(float f)
{
    if(OPTION_SHADOWMAPPINGSELFDENSITY_MAX < f)
        m_shadowMapSelfDensity = OPTION_SHADOWMAPPINGSELFDENSITY_MAX;
    else if(OPTION_SHADOWMAPPINGSELFDENSITY_MIN > f)
        m_shadowMapSelfDensity = OPTION_SHADOWMAPPINGSELFDENSITY_MIN;
    else
        m_shadowMapSelfDensity = f;
}

/* Option::getShadowMappingFloorDensity: get floor density of shadow mapping */
float Option::getShadowMappingFloorDensity()
{
    return m_shadowMapFloorDensity;
}

/* Option::setShadowMappingFloorDensity: set floor density of shadow mapping */
void Option::setShadowMappingFloorDensity(float f)
{
    if(OPTION_SHADOWMAPPINGFLOORDENSITY_MAX < f)
        m_shadowMapFloorDensity = OPTION_SHADOWMAPPINGFLOORDENSITY_MAX;
    else if(OPTION_SHADOWMAPPINGFLOORDENSITY_MIN > f)
        m_shadowMapFloorDensity = OPTION_SHADOWMAPPINGFLOORDENSITY_MIN;
    else
        m_shadowMapFloorDensity = f;
}

/* Option::getShadowMappingLightFirst: get first light flag */
bool Option::getShadowMappingLightFirst()
{
    return m_shadowMapLightFirst;
}

/* Option::setShadowMappingLightFirst: set first light flag */
void Option::setShadowMappingLightFirst(bool b)
{
    m_shadowMapLightFirst = b;
}

} /* namespace */

