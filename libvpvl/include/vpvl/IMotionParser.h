#ifndef VPVL_IMOTIONPARSER_H_
#define VPVL_IMOTIONPARSER_H_

#include "vpvl/IParser.h"
#include "vpvl/IMotion.h"

namespace vpvl
{

class IMotionParser : IParser
{
public:
    virtual IMotion *parse() = 0;
};

} /* namespace vpvl */

#endif
