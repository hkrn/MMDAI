#include "vpvl/vpvl.h"

namespace vpvl
{

class VMDParser : public IMotionParser
{
public:
    VMDParser();
    ~VMDParser();

    virtual bool preparse();
    virtual IMotion *parse();

private:
    void parseHeader();
    void parseBoneFrames();
    void parseFaceFrames();
    void parseCameraFrames();
    void parseLightFrames();
    void parseSelfShadowFrames();
};

VMDParser::VMDParser()
{
}

VMDParser::~VMDParser()
{
}

bool VMDParser::preparse()
{
    return false;
}

IMotion *VMDParser::parse()
{
    return 0;
}

void VMDParser::parseHeader()
{
}

void VMDParser::parseBoneFrames()
{
}

void VMDParser::parseFaceFrames()
{
}

void VMDParser::parseCameraFrames()
{
}

void VMDParser::parseLightFrames()
{
}

void VMDParser::parseSelfShadowFrames()
{
}

}
