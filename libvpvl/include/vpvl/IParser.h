namespace vpvl
{

class IParser
{
    virtual ~IParser();
    virtual bool preparse() = 0;
    virtual void parse() = 0;
};

} /* namespace vpvl */
