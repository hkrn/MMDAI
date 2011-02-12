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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef PMDMODELLOADERFACTORY_H
#define PMDMODELLOADERFACTORY_H

#include <MMDME/PMDModelLoader.h>
#include <MMDME/VMDLoader.h>
#include "MMDAI/LipSyncLoader.h"

class PMDModelLoaderFactory
{
public:
  virtual ~PMDModelLoaderFactory() {}

  virtual PMDModelLoader *createModelLoader(const char *filename) = 0;

  virtual VMDLoader *createMotionLoader(const char *filename) = 0;

  virtual LipSyncLoader *createLipSyncLoader(const char *filename) = 0;

  virtual void releaseModelLoader(PMDModelLoader *loader) = 0;

  virtual void releaseMotionLoader(VMDLoader *loader) = 0;

  virtual void releaseLipSyncLoader(LipSyncLoader *loader) = 0;
};

#endif // SCENEEVENTHANDLER_H

