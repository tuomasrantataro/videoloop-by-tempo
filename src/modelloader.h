#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <string>
#include <QMatrix4x4>
#include <vector>
#include <QFile>
#include <QSharedPointer>
#include <QDir>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

struct MaterialInfo
{
    QString Name;
    QVector3D Ambient;
    QVector3D Diffuse;
    QVector3D Specular;
    float Shininess;
};

struct LightInfo
{
    QVector4D Position;
    QVector3D Intensity;
};

struct Mesh
{
    QString name;
    unsigned int indexCount;
    unsigned int indexOffset;
    QSharedPointer<MaterialInfo> material;
};

struct Node
{
    QString name;

    QMatrix4x4 transformation;
    QVector<QSharedPointer<Mesh> > meshes;
    QVector<Node> nodes;
};

struct ModelData {
    QVector<float> vertices;
    QVector<float> normals;
    QVector<unsigned int> indices;

    QVector<QVector<float> > textureUV; // multiple channels
    QVector<unsigned int > textureUVComponents; // multiple channels

    QSharedPointer<Node> rootNode;
};
class ModelLoader
{
public:

    ModelLoader(bool transformToUnitCoordinates = true);

    ModelData getModelData(QString filePath);

private:

    bool load3DModel(QString filePath);
    void getBufferData( QVector<float> *vertices, QVector<float> *normals,
                        QVector<unsigned int> *indices);

    void getTextureData( QVector<QVector<float>> *textureUV);

    QSharedPointer<Node> getNodeData() { return m_rootNode; }

    // Texture information
    int numUVChannels() { return m_textureUV.size(); }
    int numUVComponents(int channel) { return m_textureUVComponents.at(channel); }

    QSharedPointer<MaterialInfo> processMaterial(aiMaterial *mater);
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode);

    void transformToUnitCoordinates();
    void findObjectDimensions(Node *node, QMatrix4x4 transformation, QVector3D &minDimension, QVector3D &maxDimension);

    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<unsigned int> m_indices;

    QVector<QVector<float> > m_textureUV; // multiple channels
    QVector<unsigned int > m_textureUVComponents; // multiple channels

    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;
    bool m_transformToUnitCoordinates;
};

#endif