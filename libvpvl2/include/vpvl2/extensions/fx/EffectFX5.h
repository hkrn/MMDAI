/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_EXTENSIONS_FX_EFFECTFX5_H_
#define VPVL2_EXTENSIONS_FX_EFFECTFX5_H_

#include <vpvl2/IEffect.h>

namespace vpvl2
{

class IEncoding;
class IString;

namespace extensions
{
namespace fx
{

class EffectFX5 {
public:
    struct ParseData;

    EffectFX5(IEncoding *encoding);
    ~EffectFX5();

    bool parse(const uint8_t *data, size_t size);
    bool compile();
    void setShaderVersion(int value);

private:
    struct Type;
    struct Assignable;
    struct Annotation;
    struct Parameter;
    struct Technique;
    struct Pass;

    static bool lookup(const ParseData &data, size_t offset, uint32_t &value);
    bool parseString(const ParseData &data, size_t offset, IString *&string);
    bool parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string);
    bool parseType(const ParseData &data, uint32_t offset, Type &type);
    bool parseAnnotation(ParseData &data);
    bool parseAssignments(ParseData &data, const uint32_t numAssignments, Assignable *assignable);
    void resolveAssignableVariables(Assignable *value);

    typedef Hash<HashString, Parameter *> String2ParameterRefHash;
    typedef Hash<HashString, Technique *> String2TechniqueRefHash;
    typedef Hash<HashString, Pass *> String2PassRefHash;
    typedef Hash<HashString, Annotation *> String2AnnotationRefHash;
    IEncoding *m_encodingRef;
    PointerArray<Parameter> m_parameters;
    String2ParameterRefHash m_name2ParameterRef;
    String2ParameterRefHash m_semantic2ParameterRef;
    PointerArray<Technique> m_techniques;
    String2TechniqueRefHash m_name2TechniqueRef;
    PointerArray<Pass> m_passes;
    String2PassRefHash m_name2PassRef;
    PointerArray<Annotation> m_annotations;
    String2AnnotationRefHash m_name2AnnotationRef;
    int m_shaderVersion;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectFX5)
};

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
