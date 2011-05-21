namespace vpvl
{

class IK
{
public:
    IK();
    ~IK();

    void parse(const void *data, const btAlignedObjectArray<Bone> &bones);
    void solve();

    static size_t size();

    bool isSimulated() {
        return m_bones[0]->isSimulated();
    }

private:
    Bone *m_destination;
    Bone *m_target;
    btAlignedObjectArray<Bone *> m_bones;
    uint16_t m_iteration;
    float m_angleConstraint;
}

} /* namespace vpvl */

