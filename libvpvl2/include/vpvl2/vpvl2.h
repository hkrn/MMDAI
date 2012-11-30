/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#pragma once
#ifndef vpvl2_vpvl2_H_
#define vpvl2_vpvl2_H_

#include "vpvl2/Common.h"
#include "vpvl2/Factory.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IBoneKeyframe.h"
#include "vpvl2/ICamera.h"
#include "vpvl2/ICameraKeyframe.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/IEffectKeyframe.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IKeyframe.h"
#include "vpvl2/ILabel.h"
#include "vpvl2/ILight.h"
#include "vpvl2/ILightKeyframe.h"
#include "vpvl2/IMaterial.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IModelKeyframe.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IMorphKeyframe.h"
#include "vpvl2/IMotion.h"
#include "vpvl2/IProjectKeyframe.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/IString.h"
#include "vpvl2/IVertex.h"
#include "vpvl2/Scene.h"

#ifdef vpvl2_ENABLE_PROJECT
#include "vpvl2/Project.h"
#endif

#endif /* vpvl2_vpvl2_H_ */
