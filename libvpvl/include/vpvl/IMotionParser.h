#include "vpvl/IParser.h"

namespace vpvl
{

class IMotionParser : IParser
{
public:
    virtual IMotion *parse() = 0;
};

} /* namespace vpvl */
