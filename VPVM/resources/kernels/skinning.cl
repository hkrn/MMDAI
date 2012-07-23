static float4 matrixMultVector4(const float16 *m, const float4 *v)
{
    return (float4)(
       dot(m->s048c, *v) + m->s3,
       dot(m->s159d, *v) + m->s7,
       dot(m->s26ae, *v) + m->sb,
       1.0
    );
}

static float4 matrixMultVector3(const float16 *m, const float4 *v)
{
    return (float4)(
       dot((float4)(m->s048, 0.0), *v),
       dot((float4)(m->s159, 0.0), *v),
       dot((float4)(m->s26a, 0.0), *v),
       0.0
    );
}

__kernel
void performSkinning(const __global float16 *localMatrices,
                     const __global float *boneWeights,
                     const __global int2 *boneIndices,
                     const __global float *edgeOffset,
                     const float4 lightDirection,
                     const float edgeScaleFactor,
                     const int nvertices,
                     const int strideSize,
                     const int offsetPosition,
                     const int offsetNormal,
                     const int offsetTexCoord,
                     const int offsetEdge,
                     __global float4 *vertices)
{
    int id = get_global_id(0);
    if (id < nvertices) {
        int strideOffset = strideSize * id;
        float4 position4 = vertices[strideOffset + offsetPosition];
        float4 normal4 = vertices[strideOffset + offsetNormal];
        float4 position = (float4)(position4.xyz, 1.0);
        float4 normal = (float4)(normal4.xyz, 0.0);
        int2 boneIndex = boneIndices[id];
        float weight = boneWeights[id];
        float16 transform1 = localMatrices[boneIndex.x];
        float16 transform2 = localMatrices[boneIndex.y];
        float4 v1 = matrixMultVector4(&transform1, &position);
        float4 v2 = matrixMultVector4(&transform2, &position);
        float4 n1 = matrixMultVector3(&transform1, &normal);
        float4 n2 = matrixMultVector3(&transform2, &normal);
        float s = 1.0f - weight;
        float4 position2 = s * v2 + weight * v1;
        float4 normal2 = s * n2 + weight * n1;
        vertices[strideOffset + offsetPosition] = position2;
        vertices[strideOffset + offsetNormal] = normal2;
        vertices[strideOffset + offsetTexCoord].zw = (float2)(0.0, 0.5 + dot(lightDirection, normal2) * 0.5);
        vertices[strideOffset + offsetEdge] = position2 + normal2 * edgeOffset[id] * edgeScaleFactor;
    }
}

__kernel
void performSkinning2(const __global float16 *localMatrices,
                      const __global float4 *boneWeights,
                      const __global int4 *boneIndices,
                      const __global float *materialEdgeSize,
                      const float4 lightDirection,
                      const float edgeScaleFactor,
                      const int nvertices,
                      const int strideSize,
                      const int offsetPosition,
                      const int offsetNormal,
                      const int offsetTexCoord,
                      const int offsetEdgeVertex,
                      const int offsetEdgeSize,
                      __global float4 *vertices)
{
    int id = get_global_id(0);
    if (id < nvertices) {
        int strideOffset = strideSize * id;
        float4 position4 = vertices[strideOffset + offsetPosition];
        float4 normal4 = vertices[strideOffset + offsetNormal];
        float vertexId = position4.w;
        float edgeSize = normal4.w * materialEdgeSize[id] * edgeScaleFactor;
        float4 position = (float4)(position4.xyz, 1.0);
        float4 normal = (float4)(normal4.xyz, 0.0);
        int4 boneIndex = boneIndices[id];
        float4 weight = boneWeights[id];
        float4 position2, normal2;
        if (boneIndex.w >= 0) { // bdef4
            float16 transform1 = localMatrices[boneIndex.x];
            float16 transform2 = localMatrices[boneIndex.y];
            float16 transform3 = localMatrices[boneIndex.z];
            float16 transform4 = localMatrices[boneIndex.w];
            float4 v1 = matrixMultVector4(&transform1, &position);
            float4 v2 = matrixMultVector4(&transform2, &position);
            float4 v3 = matrixMultVector4(&transform3, &position);
            float4 v4 = matrixMultVector4(&transform4, &position);
            float4 n1 = matrixMultVector3(&transform1, &normal);
            float4 n2 = matrixMultVector3(&transform2, &normal);
            float4 n3 = matrixMultVector3(&transform3, &normal);
            float4 n4 = matrixMultVector3(&transform4, &normal);
            position2 = weight.x * v1 + weight.y * v2 + weight.z * v3 + weight.w * v4;
            normal2 = weight.x * n1 + weight.y * n2 + weight.z * n3 + weight.w * n4;;
            vertices[strideOffset + offsetPosition] = position2;
            vertices[strideOffset + offsetNormal] = normal2;
            vertices[strideOffset + offsetTexCoord].zw = (float2)(0.0, 0.5 + dot(lightDirection, normal2) * 0.5);
        }
        else if (boneIndex.y >= 0) { // bdef2 or sdef2
            float16 transform1 = localMatrices[boneIndex.x];
            float16 transform2 = localMatrices[boneIndex.y];
            float4 v1 = matrixMultVector4(&transform1, &position);
            float4 v2 = matrixMultVector4(&transform2, &position);
            float4 n1 = matrixMultVector3(&transform1, &normal);
            float4 n2 = matrixMultVector3(&transform2, &normal);
            float w = weight.x;
            float s = 1.0f - w;
            position2 = s * v2 + w * v1;
            normal2 = s * n2 + w * n1;
            vertices[strideOffset + offsetPosition] = position2;
            vertices[strideOffset + offsetNormal] = normal2;
            vertices[strideOffset + offsetTexCoord].zw = (float2)(0.0, 0.5 + dot(lightDirection, normal2) * 0.5);
        } else { // bdef1
            float16 transform = localMatrices[boneIndex.x];
            position2 = matrixMultVector4(&transform, &position);
            normal2 = matrixMultVector3(&transform, &normal);
            vertices[strideOffset + offsetPosition] = position2;
            vertices[strideOffset + offsetNormal] = normal2;
            vertices[strideOffset + offsetTexCoord].zw = (float2)(0.0, 0.5 + dot(lightDirection, normal2) * 0.5);
        }
        vertices[strideOffset + offsetPosition].w = vertexId;
        vertices[strideOffset + offsetEdgeVertex] = position2 + normal2 * edgeSize;
    }
}

