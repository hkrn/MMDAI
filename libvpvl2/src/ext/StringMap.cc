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

#include <vpvl2/extensions/icu4c/StringMap.h>

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

StringMap::StringMap()
    : std::map<const UnicodeString, UnicodeString, String::Less>()
{
}

StringMap::~StringMap()
{
}

bool StringMap::bval(const UnicodeString &key, bool defval) const
{
    const_iterator it = find(key);
    return it != end() ? String::toBoolean(it->second) : defval;
}

int StringMap::ival(const UnicodeString &key, int defval) const
{
    const_iterator it = find(key);
    return it != end() ? String::toInt(it->second) : defval;
}
double StringMap::dval(const UnicodeString &key, double defval) const
{
    const_iterator it = find(key);
    return it != end() ? String::toDouble(it->second) : defval;
}

float StringMap::fval(const UnicodeString &key, float defval) const
{
    return float(dval(key, defval));
}

UnicodeString StringMap::sval(const UnicodeString &key, const UnicodeString &defval) const
{
    const_iterator it = find(key);
    return it != end() ? it->second : defval;
}

} /* namespace icu4c */
} /* namespace extensions */
} /* namespace vpvl2 */
