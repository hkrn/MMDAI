namespace vpvl
{

class Vertex
{
public:
    Vertex();
    ~Vertex();

    static size_t size();

    void parse(const void *data);

private:
    btAlignedObjectArray<VertexChunk> m_vertices;
};

} /* namespace vpvl */

