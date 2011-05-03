/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef MMDAI_PREFERENCE_H_
#define MMDAI_PREFERENCE_H_

#include <MMDME/MMDME.h>

namespace MMDAI {

enum PreferenceKeys {
    kPreferenceUseCartoonRendering,
    kPreferenceUseMMDLikeCartoon,
    kPreferenceCartoonEdgeWidth,
    kPreferenceCartoonEdgeStep,
    kPreferenceStageSize,
    kPreferenceShowFPS,
    kPreferenceFPSPosition,
    kPreferenceWindowSize,
    kPreferenceTopMost,
    kPreferenceFullScreen,
    kPreferenceLogSize,
    kPreferenceLogPosition,
    kPreferenceLogScale,
    kPreferenceLightDirection,
    kPreferenceLightIntensity,
    kPreferenceLightColor,
    kPreferenceCampusColor,
    kPreferenceMaxMultiSampling,
    kPreferenceMaxMultiSamplingColor,
    kPreferenceMotionAdjustFrame,
    kPreferenceBulletFPS,
    kPreferenceRotateStep,
    kPreferenceTranslateStep,
    kPreferenceDistanceStep,
    kPreferenceFovyStep,
    kPreferenceUseShadowMapping,
    kPreferenceShadowMappingTextureSize,
    kPreferenceShadowMappingSelfDensity,
    kPreferenceShadowMappingFloorDensity,
    kPreferenceShadowMappingLightFirst,
    kPreferenceCartoonEdgeSelectedColor,
    kPreferenceCameraRotation,
    kPreferenceCameraTransition,
    kPreferenceCameraDistance,
    kPreferenceCameraFovy,
    kPreferenceMaxModelSize,
    kPreferenceMaxLengthOfKeys
};

class IPreference
{
public:
    virtual ~IPreference() {};

    virtual bool getBool(const PreferenceKeys key) = 0;
    virtual int getInt(const PreferenceKeys key) = 0;
    virtual float getFloat(const PreferenceKeys key) = 0;
    virtual void  getFloat3(const PreferenceKeys key, float *values) = 0;
    virtual void  getFloat4(const PreferenceKeys key, float *values) = 0;
    virtual void  setBool(const PreferenceKeys key, bool value) = 0;
    virtual void  setInt(const PreferenceKeys key, int value) = 0;
    virtual void  setFloat(const PreferenceKeys key, float value) = 0;
    virtual void  setFloat3(const PreferenceKeys key, const float *values) = 0;
    virtual void  setFloat4(const PreferenceKeys key, const float *values) = 0;
};

} /* namespace */

#endif
