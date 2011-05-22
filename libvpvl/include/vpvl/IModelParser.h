#include "vpvl/IParser.h"

namespace vpvl
{

class IModelParser :IParser
{
public:
    virtual IModel *parse() = 0;
};

} /* namespace vpvl */
