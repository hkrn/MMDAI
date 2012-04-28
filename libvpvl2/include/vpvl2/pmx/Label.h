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

#ifndef VPVL2_PMX_LABEL_H_
#define VPVL2_PMX_LABEL_H_

#include "vpvl2/ILabel.h"
#include "vpvl2/IString.h"
#include "vpvl2/pmx/Model.h"

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Label class represents a label of a Polygon Model Extended object.
 */

class VPVL2_API Label : public ILabel
{
public:
    struct Pair;

    /**
     * Constructor
     */
    Label();
    ~Label();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadLabels(const Array<Label *> &labels,
                           const Array<Bone *> &bones,
                           const Array<Morph *> &morphs);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;

    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    bool isSpecial() const { return m_special; }
    IBone *bone(int index) const;
    IMorph *morph(int index) const;
    int count() const;

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setSpecial(bool value);
    void addBone(IBone *value);
    void addMorph(IMorph *value);
    void removeBone(IBone *value);
    void removeMorph(IMorph *value);

private:
    IString *m_name;
    IString *m_englishName;
    Array<Pair *> m_pairs;
    bool m_special;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Label)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

