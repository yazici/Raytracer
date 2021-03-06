#include "PCH.h"
#include "MeshLoader.h"

#include "../Core/Utils/Logger.h"
#include "../Core/Utils/Bitmap.h"

namespace helpers {

using namespace rt;
using namespace math;

struct TriangleIndices
{
    int vertex;
    int normal;
    int texCoord;

    explicit TriangleIndices(const tinyobj::index_t& idx)
        : vertex(idx.vertex_index)
        , normal(idx.normal_index)
        , texCoord(idx.texcoord_index)
    {}

    bool operator == (const TriangleIndices& rhs) const
    {
        return (vertex < rhs.vertex) && (normal < rhs.normal) && (texCoord < rhs.texCoord);
    }

    bool operator < (const TriangleIndices& rhs) const
    {
        if (vertex < rhs.vertex) return true;
        if (normal < rhs.normal) return true;
        if (texCoord < rhs.texCoord) return true;
        return false;
    }
};

TexturePtr LoadTexture(const std::string& baseDir, const std::string& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    std::string fullPath = baseDir + path;
    if ((fullPath.rfind(".png") == fullPath.length() - 4) || (fullPath.rfind(".jpg") == fullPath.length() - 4))
    {
        fullPath.replace(fullPath.length() - 4, 4, ".bmp");
    }

    // cache bitmaps so they are loaded only once
    static std::map<std::string, BitmapPtr> bitmapsList;
    BitmapPtr& bitmapPtr = bitmapsList[fullPath];

    if (!bitmapPtr)
    {
        bitmapPtr = BitmapPtr(new Bitmap(path.c_str()));
        bitmapPtr->Load(fullPath.c_str());
    }

    if (bitmapPtr->GetWidth() > 0 && bitmapPtr->GetHeight() > 0)
    {
        return bitmapPtr;
    }

    return nullptr;
}

MaterialPtr LoadMaterial(const std::string& baseDir, const tinyobj::material_t& sourceMaterial)
{
    auto material = MaterialPtr(new Material);

    material->SetBsdf(Material::DefaultBsdfName); // TODO
    material->debugName = sourceMaterial.name;
    material->baseColor = Vector4(sourceMaterial.diffuse[0], sourceMaterial.diffuse[1], sourceMaterial.diffuse[2], 0.0f);
    material->emission.baseValue = Vector4(sourceMaterial.emission[0], sourceMaterial.emission[1], sourceMaterial.emission[2], 0.0f);
    material->baseColor.texture = LoadTexture(baseDir, sourceMaterial.diffuse_texname);
    material->normalMap = LoadTexture(baseDir, sourceMaterial.normal_texname);
    material->maskMap = LoadTexture(baseDir, sourceMaterial.alpha_texname);
    material->roughness = 0.8f;

    material->Compile();

    return material;
}

MaterialPtr CreateDefaultMaterial(MaterialsMap& outMaterials)
{
    auto material = MaterialPtr(new Material);
    material->debugName = "default";
    material->baseColor = Vector4(0.8f, 0.8f, 0.8f, 0.0f);
    material->emission.baseValue = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    material->roughness = 0.75f;
    material->SetBsdf(Material::DefaultBsdfName);
    material->Compile();

    outMaterials[material->debugName] = material;
    return material;
}

MeshPtr LoadMesh(const std::string& filePath, MaterialsMap& outMaterials, const float scale)
{
    RT_LOG_DEBUG("Loading mesh file: '%s'...", filePath.c_str());

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    const std::string meshBaseDir = filePath.substr(0, filePath.find_last_of("\\/")) + "/";

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str(), meshBaseDir.c_str(), true);
    if (!err.empty())
    {
        RT_LOG_INFO("Mesh '%s' loading message:\n%s", filePath.c_str(), err.c_str());
    }
    if (!ret)
    {
        RT_LOG_ERROR("Failed to load mesh '%s'", filePath.c_str());
        return nullptr;
    }


    std::vector<Uint32> vertexIndices;
    std::vector<Uint32> materialIndices;

    std::vector<float> vertexPositions;
    std::vector<float> vertexNormals;
    std::vector<float> vertexTangents;
    std::vector<float> vertexTexCoords;
    std::vector<MaterialPtr> materialPointers;

    Uint32 totalIndices = 0;
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces
        Uint32 indexOffset = 0;
        const auto& mesh = shapes[s].mesh;

        for (size_t f = 0; f < mesh.num_face_vertices.size(); f++)
        {
            int fv = mesh.num_face_vertices[f];
            if (fv != 3)
            {
                RT_LOG_ERROR("Expected only triangles (shape index = %zu, face index = %zu)", s, f);
                return nullptr;
            }

            const tinyobj::index_t idx[3] =
            {
                mesh.indices[indexOffset + 0],
                mesh.indices[indexOffset + 1],
                mesh.indices[indexOffset + 2],
            };

            bool hasNormals = idx[0].normal_index != -1 && idx[1].normal_index != -1 && idx[2].normal_index != -1;
            bool hasTexCoords = idx[0].texcoord_index != -1 && idx[1].texcoord_index != -1 && idx[2].texcoord_index != -1;

            // Loop over vertices in the face.
            Vector4 verts[3];
            for (size_t i = 0; i < 3; i++)
            {
                verts[i] = Vector4(
                    attrib.vertices[3 * idx[i].vertex_index + 0],
                    attrib.vertices[3 * idx[i].vertex_index + 1],
                    attrib.vertices[3 * idx[i].vertex_index + 2],
                    0.0f);

                vertexPositions.push_back(scale * verts[i][0]);
                vertexPositions.push_back(scale * verts[i][1]);
                vertexPositions.push_back(scale * verts[i][2]);
            }

            Vector4 normals[3];
            Vector4 tangents[3];

            if (hasNormals)
            {
                // OBJ has normals
                for (size_t i = 0; i < 3; i++)
                {
                    normals[i] = Vector4(
                        attrib.normals[3 * idx[i].normal_index + 0],
                        attrib.normals[3 * idx[i].normal_index + 1],
                        attrib.normals[3 * idx[i].normal_index + 2],
                        0.0f);
                    normals[i].Normalize3();
                }
            }
            else
            {
                // calculate per-face normal
                Vector4 normal = Vector4::Cross3(verts[1] - verts[0], verts[2] - verts[0]).Normalized3();

                normals[0] = normal;
                normals[1] = normal;
                normals[2] = normal;
            }

            for (size_t i = 0; i < 3; i++)
            {
                vertexNormals.push_back(normals[i].x);
                vertexNormals.push_back(normals[i].y);
                vertexNormals.push_back(normals[i].z);
            }

            // calculate tangent vector
            // TODO use tex coord !!!
            const Vector4 edge0 = (verts[2] - verts[0]).Normalized3();
            for (size_t i = 0; i < 3; i++)
            {
                tangents[i] = Vector4::Orthogonalize(edge0, normals[i]).Normalized3();
            }

            for (size_t i = 0; i < 3; i++)
            {
                vertexTangents.push_back(tangents[i].x);
                vertexTangents.push_back(tangents[i].y);
                vertexTangents.push_back(tangents[i].z);
            }

            if (hasTexCoords)
            {
                for (size_t i = 0; i < 3; i++)
                {
                    vertexTexCoords.push_back(attrib.texcoords[2 * idx[i].texcoord_index + 0]);
                    vertexTexCoords.push_back(attrib.texcoords[2 * idx[i].texcoord_index + 1]);
                }
            }

            // TODO compress vertex data (right now index buffer is useless)
            vertexIndices.push_back(totalIndices + indexOffset + 0);
            vertexIndices.push_back(totalIndices + indexOffset + 1);
            vertexIndices.push_back(totalIndices + indexOffset + 2);

            materialIndices.push_back(mesh.material_ids[f]);

            indexOffset += fv;
        }

        totalIndices += indexOffset;
    }

    // load materials
    materialPointers.reserve(materials.size());
    for (size_t i = 0; i < materials.size(); i++)
    {
        auto material = LoadMaterial(meshBaseDir, materials[i]);
        materialPointers.push_back(material);
        outMaterials[material->debugName] = material;
    }

    // fallback to default material
    if (materials.empty())
    {
        RT_LOG_WARNING("No materials found in mesh '%s'. Falling back to the default material.", filePath.c_str());

        materialPointers.push_back(CreateDefaultMaterial(outMaterials));

        for (Uint32& index : materialIndices)
        {
            index = 0;
        }
    }

    RT_LOG_DEBUG("Mesh file '%s' loaded, shapes = %zu, materials = %zu, indices = %u",
                 filePath.c_str(), shapes.size(), materialPointers.size(), totalIndices);

    MeshDesc meshDesc;
    meshDesc.path = filePath;
    meshDesc.vertexBufferDesc.numTriangles = static_cast<Uint32>(vertexIndices.size() / 3);
    meshDesc.vertexBufferDesc.numVertices = static_cast<Uint32>(vertexPositions.size() / 3);
    meshDesc.vertexBufferDesc.numMaterials = static_cast<Uint32>(materialPointers.size());
    meshDesc.vertexBufferDesc.materials = materialPointers.data();
    meshDesc.vertexBufferDesc.materialIndexBuffer = materialIndices.data();
    meshDesc.vertexBufferDesc.vertexIndexBuffer = vertexIndices.data();
    meshDesc.vertexBufferDesc.positions = vertexPositions.data();
    meshDesc.vertexBufferDesc.normals = vertexNormals.data();
    meshDesc.vertexBufferDesc.tangents = vertexTangents.data();
    meshDesc.vertexBufferDesc.texCoords = vertexTexCoords.data();

    MeshPtr mesh = MeshPtr(new Mesh);
    bool result = mesh->Initialize(meshDesc);
    if (!result)
    {
        return nullptr;
    }

    return mesh;
}

} // namespace helpers