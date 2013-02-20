/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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

#ifndef VPVL2_INTERNAL_KEYFRAME_H_
#define VPVL2_INTERNAL_KEYFRAME_H_

#define VPVL2_KEYFRAME_INITIALIZE_FIELDS() \
  m_namePtr(0), \
  m_timeIndex(0), \
  m_layerIndex(0)

#define VPVL2_KEYFRAME_DESTROY_FIELDS() \
  delete m_namePtr; \
  m_namePtr = 0; \
  m_timeIndex = 0; \
  m_layerIndex = 0; \

#define VPVL2_KEYFRAME_DEFINE_METHODS() \
  const IString *name() const { return m_namePtr; } \
  IKeyframe::TimeIndex timeIndex() const { return m_timeIndex; } \
  IKeyframe::LayerIndex layerIndex() const { return m_layerIndex; } \
  void setTimeIndex(const IKeyframe::TimeIndex &value) { m_timeIndex = value; } \
  void setLayerIndex(const IKeyframe::LayerIndex &value) { m_layerIndex = value; }

#define VPVL2_KEYFRAME_DEFINE_FIELDS() \
  IString *m_namePtr; \
  IKeyframe::TimeIndex m_timeIndex; \
  IKeyframe::LayerIndex m_layerIndex;

#endif
