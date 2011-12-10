/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef VPVL_PROJECT_H_
#define VPVL_PROJECT_H_

#include <libxml2/libxml/SAX2.h>
#include <string>

#include "vpvl/Common.h"

namespace vpvl
{

class Asset;
class PMDModel;
class VMDMotion;

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Project class represents a project file (*.vpvx)
 */

class VPVL_API Project
{
public:
    typedef struct Parser Parser;

    Project();
    ~Project();

    bool load(const char *path);
    bool load(const uint8_t *data, size_t size);
    void save(const char *path);

    const Array<Asset *> &assets() const;
    const Array<PMDModel *> &models() const;
    const Array<VMDMotion *> &motions() const;
    const std::string globalSetting(const char *key) const;
    const std::string localAssetSetting(Asset *asset, const char *key) const;
    const std::string localModelSetting(PMDModel *model, const char *key) const;

    Array<Asset *> *mutableAssets();
    Array<PMDModel *> *mutableModels();
    Array<VMDMotion *> *mutableMotions();
    void setGlobalSetting(const char *key, std::string &value);
    void setLocalAssetSetting(Asset *asset, const char *key, const std::string &value) const;
    void setLocalModelSetting(PMDModel *model, const char *key, const std::string &value) const;

private:
    Parser *m_parser;
    xmlSAXHandler m_handler;

    VPVL_DISABLE_COPY_AND_ASSIGN(Project)
};

} /* namespace vpvl */

#endif
