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

#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDModel.h"
#include "vpvl/internal/VMDMotion.h"

namespace vpvl
{

Scene::Scene()
    : m_cameraMotion(0)
{
}

Scene::~Scene()
{
    m_cameraMotion = 0;
}

void Scene::addModel(PMDModel *model)
{
    addModel(reinterpret_cast<const char *>(model->name()), model);
}

void Scene::addModel(const char *name, PMDModel *model)
{
    m_models.insert(btHashString(name), model);
}

PMDModel *Scene::findModel(const char *name) const
{
    PMDModel **ptr = const_cast<PMDModel **>(m_models.find(btHashString(name)));
    return ptr ? 0 : *ptr;
}

void Scene::removeModel(PMDModel *model)
{
    removeModel(reinterpret_cast<const char *>(model->name()));
}

void Scene::removeModel(const char *name)
{
    m_models.remove(btHashString(name));
}

void Scene::setCameraMotion(VMDMotion *motion)
{
    m_cameraMotion = motion;
}

void Scene::update(float deltaFrame)
{
    uint32_t nModels = m_models.size();
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = *m_models.getAtIndex(i);
        model->updateRootBone();
        model->updateMotion(deltaFrame);
        model->updateSkins();
    }
}

}
