#include "openglwidget2.h"

OpenGLWidget2::OpenGLWidget2(int maxFrames) : m_maxFrames(maxFrames)
{
    m_error = false;
    m_initDone = false;
}

OpenGLWidget2::~OpenGLWidget2()
{
    m_texture->destroy();
    delete m_texture;
}

void OpenGLWidget2::initializeGL()
{
    initializeOpenGLFunctions();

    createShaderProgram("./assets/shaders/ads_fragment.vert", "./assets/shaders/ads_fragment.frag");
    createBuffers();
    createAttributes();
    setupLightingAndMatrices();

    initTextures();

    glEnable(GL_DEPTH_TEST);
    glClearColor(.0f, .0f, .0f ,1.0f);

    m_initDone = true;
    emit initDone();
}

void OpenGLWidget2::createShaderProgram(QString vShader, QString fShader)
{
    // Compile vertex shader
    if ( !m_shaderProgram.addShaderFromSourceFile( QOpenGLShader::Vertex, vShader.toUtf8() ) )
        qCritical() << "Unable to compile vertex shader. Log:" << m_shaderProgram.log();

    // Compile fragment shader
    if ( !m_shaderProgram.addShaderFromSourceFile( QOpenGLShader::Fragment, fShader.toUtf8() ) )
        qCritical() << "Unable to compile fragment shader. Log:" << m_shaderProgram.log();

    // Link the shaders together into a program
    if ( !m_shaderProgram.link() )
        qCritical() << "Unable to link shader program. Log:" << m_shaderProgram.log();
}

void OpenGLWidget2::setBufferData(  QVector<float>& vertices,
                                    QVector<float>& normals,
                                    QVector<QVector<float>>& textureUV,
                                    QVector<unsigned int>& indices)
{
    m_vertices = vertices;
    m_normals = normals;
    m_textureUV = textureUV;
    m_indices = indices;
}

void OpenGLWidget2::setNodeData(QSharedPointer<Node> rootNode)
{
    m_rootNode = rootNode;
}

void OpenGLWidget2::createBuffers()
{
    m_vao.create();
    m_vao.bind();

    // Create a buffer and copy the vertex data to it
    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate( &(m_vertices[0]), m_vertices.size() * sizeof( float ) );

    // Create a buffer and copy the vertex data to it
    m_normalBuffer.create();
    m_normalBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_normalBuffer.bind();
    m_normalBuffer.allocate( &(m_normals[0]), m_normals.size() * sizeof( float ) );

    int texSize = 0;
    if(m_textureUV.size() != 0)
    {
        // Create a buffer and copy the vertex data to it
        m_textureUVBuffer.create();
        m_textureUVBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
        m_textureUVBuffer.bind();
        
        for(int ii=0; ii<m_textureUV.size(); ++ii)
            texSize += m_textureUV.at(ii).size();
        m_textureUVBuffer.allocate( &(m_textureUV[0][0]), texSize * sizeof( float ) );
    }

    // Create a buffer and copy the index data to it
    m_indexBuffer.create();
    m_indexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_indexBuffer.bind();
    m_indexBuffer.allocate( &(m_indices[0]), m_indices.size() * sizeof( unsigned int ) );

}

void OpenGLWidget2::createAttributes()
{
    if(m_error)
        return;

    m_vao.bind();
    // Set up the vertex array state
    m_shaderProgram.bind();

    // Map vertex data to the vertex shader's layout location '0'
    m_vertexBuffer.bind();
    m_shaderProgram.enableAttributeArray( 0 );      // layout location
    m_shaderProgram.setAttributeBuffer( 0,          // layout location
                                        GL_FLOAT,   // data's type
                                        0,          // Offset to data in buffer
                                        3);         // number of components (3 for x,y,z)

    // Map normal data to the vertex shader's layout location '1'
    m_normalBuffer.bind();
    m_shaderProgram.enableAttributeArray( 1 );      // layout location
    m_shaderProgram.setAttributeBuffer( 1,          // layout location
                                        GL_FLOAT,   // data's type
                                        0,          // Offset to data in buffer
                                        3);         // number of components (3 for x,y,z)
    if(!m_textureUVBuffer.isCreated())
        return;
    m_textureUVBuffer.bind();
    m_shaderProgram.enableAttributeArray( 2 );      // layout location
    m_shaderProgram.setAttributeBuffer( 2,          // layout location
                                        GL_FLOAT,   // data's type
                                        0,          // Offset to data in buffer
                                        2);         // number of components (2 for u,v)
}

void OpenGLWidget2::setupLightingAndMatrices()
{
    m_view.setToIdentity();
    m_view.lookAt(
                QVector3D(0.0f, 0.0f, 1.2f),    // Camera Position
                QVector3D(0.0f, 0.0f, 0.0f),    // Point camera looks towards
                QVector3D(0.0f, 1.0f, 0.0f));   // Up vector

    float aspect = 4.0f/3.0f;
    m_projection.setToIdentity();
    m_projection.perspective(
                60.0f,          // field of vision
                aspect,         // aspect ratio
                0.3f,           // near clipping plane
                1000.0f);       // far clipping plane

    m_lightInfo.Position = QVector4D( -1.0f, 1.0f, 1.0f, 1.0f );
    m_lightInfo.Intensity = QVector3D( 1.0f, 1.0f, 1.0f);

    m_materialInfo.Ambient = QVector3D( 0.1f, 0.05f, 0.0f );
    m_materialInfo.Diffuse = QVector3D( .9f, .9f, .9f );
    m_materialInfo.Specular = QVector3D( .2f, .2f, .2f );
    m_materialInfo.Shininess = 50.0f;
}

void OpenGLWidget2::resizeGL(int w, int h)
{
    int side = qMin(w, h);
    glViewport((w - side) / 2, (h - side) / 2, side, side);
    m_viewPortAspectRatio = float(w)/float(h);

    m_projection.setToIdentity();
    m_projection.perspective(60.0f, (float)w/h, .3f, 1000);
}

void OpenGLWidget2::paintGL()
{
    if(m_error)
        return;

    // Clear color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind shader program
    m_shaderProgram.bind();

    // Set the model matrix
    // Translate and rotate it a bit to get a better view of the model
    m_model.setToIdentity();
    m_model.translate(-0.0f, 0.0f, -0.5f);
    m_model.rotate(float(m_counter++), 0.0f, 1.0f, 1.0f);
    m_model.translate(-0.3f, 0.0f, -0.0f);

    // Set shader uniforms for light information
    if (m_drawSquare) {
        m_shaderProgram.setUniformValue( "lightPosition", m_straightLight.Position );
        m_shaderProgram.setUniformValue( "lightIntensity", m_straightLight.Intensity );
    }
    else {
        m_shaderProgram.setUniformValue( "lightPosition", m_lightInfo.Position );
        m_shaderProgram.setUniformValue( "lightIntensity", m_lightInfo.Intensity );
    }

    // Bind VAO and draw everything
    m_vao.bind();
    if (m_drawSquare) {
        drawSquare(m_rootNode.data(), QMatrix4x4());
    }
    else {
        drawNode(m_rootNode.data(), QMatrix4x4());
    }
    m_vao.release();

}

void OpenGLWidget2::drawNode(const Node *node, QMatrix4x4 objectMatrix)
{
    // Prepare matrices
    objectMatrix *= node->transformation;

        QMatrix4x4 modelMatrix = m_model * objectMatrix;
        QMatrix4x4 modelViewMatrix = m_view * modelMatrix;
        QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
        QMatrix4x4 mvp = m_projection * modelViewMatrix;

        m_shaderProgram.setUniformValue( "MV", modelViewMatrix );// Transforming to eye space
        m_shaderProgram.setUniformValue( "N", normalMatrix );    // Transform normal to Eye space
        m_shaderProgram.setUniformValue( "MVP", mvp );           // Matrix for transforming to Clip space

    // Draw each mesh in this node
    for(int imm = 0; imm<node->meshes.size(); ++imm)
    {
        if(node->meshes[imm]->material->Name == QString("DefaultMaterial")){
            setMaterialUniforms(m_materialInfo);
        }
        else {
            setMaterialUniforms(*node->meshes[imm]->material);
        }
        glDrawElements( GL_TRIANGLES, node->meshes[imm]->indexCount, GL_UNSIGNED_INT, (const void*)(node->meshes[imm]->indexOffset * sizeof(unsigned int)) );
    }

    // Recursively draw this nodes children nodes
    for(int inn = 0; inn<node->nodes.size(); ++inn)
        drawNode(&node->nodes[inn], objectMatrix);
}

void OpenGLWidget2::drawSquare(const Node *node, QMatrix4x4 objectMatrix)
{
    // Prepare matrices
    objectMatrix *= node->transformation;

    float verticalScaling = m_frameAspectRatio/m_viewPortAspectRatio;
    float horizontalScaling = 1.0/verticalScaling;
    if (verticalScaling < 1.0) {
        verticalScaling = 1.0;
    } 
    else {
        horizontalScaling = 1.0;
    }
    QMatrix4x4 m;
    m.translate(1.5 - 1/horizontalScaling, 1.5 - 1/verticalScaling, 0.0f);
    QMatrix4x4 modelMatrix = m * objectMatrix;
    QMatrix4x4 modelViewMatrix;
    modelViewMatrix.ortho(-0, horizontalScaling, -0, verticalScaling, -1.0f, 1.0f);
    QMatrix3x3 normalMatrix = modelMatrix.normalMatrix();
    QMatrix4x4 mvp = modelMatrix * modelViewMatrix;

    m_shaderProgram.setUniformValue( "MV", modelViewMatrix );// Transforming to eye space
    m_shaderProgram.setUniformValue( "N", normalMatrix );    // Transform normal to Eye space
    m_shaderProgram.setUniformValue( "MVP", mvp );           // Matrix for transforming to Clip space

    setMaterialUniforms(m_noMaterial);

    Node inner = node->nodes[0];
    glDrawElements( GL_TRIANGLES, inner.meshes[0]->indexCount, GL_UNSIGNED_INT, (const void*)(inner.meshes[0]->indexOffset * sizeof(unsigned int)) );

}

void OpenGLWidget2::setMaterialUniforms(MaterialInfo &mater)
{
    m_shaderProgram.setUniformValue( "Ka", mater.Ambient );
    m_shaderProgram.setUniformValue( "Kd", mater.Diffuse );
    m_shaderProgram.setUniformValue( "Ks", mater.Specular );
    m_shaderProgram.setUniformValue( "shininess", mater.Shininess);

}

void OpenGLWidget2::cleanup()
{
    m_texture->destroy();
    delete m_texture;
}

void OpenGLWidget2::initTextures()
{
    m_shaderProgram.setUniformValue("layer", 0);

    // TODO: If textures not found, create image with text indicating it
    if (m_frames.size() == 0) {
        qDebug() << "No frames, cannot initialize textures";
        return;
    }
    if (m_frames[0].isNull()) {
        qDebug() << "failed to load texture";
        return;
    } 

    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2DArray);
    m_texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
    m_texture->setMipLevels(1);
    m_texture->setLayers(m_maxFrames);
    m_texture->setSize(m_frames[0].width(), m_frames[0].height());

    m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
    m_texture->setWrapMode(QOpenGLTexture::ClampToBorder);
    m_texture->allocateStorage();
    m_texture->bind();

    m_frameAspectRatio = float(m_frames[0].width()) / float(m_frames[0].height());

    for (int i = 0; i < m_frames.size(); i++){
        const uchar* data = m_frames.at(i).bits();
        m_texture->setData(0, i, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, data);
    }
}

void OpenGLWidget2::setFrameIndex(int idx)
{
    if (!m_initDone) {
        return;
    }
    if (idx < 0 || idx > m_frameCount) {
        qDebug() << "Requested frame index out of bounds. Index:" << idx << "max value:" << m_frameCount;
        return;
    }
    makeCurrent();
    m_shaderProgram.setUniformValue("layer", idx);
}

void OpenGLWidget2::setFrames(const QVector<QImage>& frames)
{
    if (m_initDone) {
        makeCurrent();
        m_frames = frames;
        m_texture->destroy();
        delete m_texture;
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2DArray);
        m_texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        m_texture->setMipLevels(1);
        m_texture->setLayers(m_maxFrames);
        m_texture->setSize(m_frames[0].width(), m_frames[0].height());

        m_texture->setMinificationFilter(QOpenGLTexture::Nearest);
        m_texture->setMagnificationFilter(QOpenGLTexture::Nearest);
        m_texture->setWrapMode(QOpenGLTexture::ClampToBorder);
        m_texture->allocateStorage();
        m_texture->bind();

        m_frameAspectRatio = float(m_frames[0].width()) / float(m_frames[0].height());

        for (int i = 0; i < m_frames.size(); i++){
            const uchar* data = m_frames.at(i).bits();
            m_texture->setData(0, i, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, data);
        }
    }
    else {
        m_frames = frames;
    }
    m_frameCount = m_frames.size();
}

void OpenGLWidget2::replaceCurrentFrames(const QVector<QImage>& frames)
{
    qDebug() << "replacecurrentframes";
    if (m_initDone) {
        makeCurrent();
        m_frames = frames;
        
        QOpenGLTexture* newTexture = new QOpenGLTexture(QOpenGLTexture::Target2DArray);
        newTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        newTexture->setMipLevels(1);
        newTexture->setLayers(m_maxFrames);
        newTexture->setSize(m_frames[0].width(), m_frames[0].height());

        newTexture->setMinificationFilter(QOpenGLTexture::Nearest);
        newTexture->setMagnificationFilter(QOpenGLTexture::Nearest);
        newTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
        newTexture->allocateStorage();
        newTexture->bind();

        m_frameAspectRatio = float(m_frames[0].width()) / float(m_frames[0].height());

        for (int i = 0; i < m_frames.size(); i++){
            const uchar* data = m_frames.at(i).bits();
            newTexture->setData(0, i, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, data);
        }

        auto old = m_texture;
        m_texture = newTexture;
        old->destroy();
        delete old;
    }
    else {
        m_frames = frames;
    }
    m_frameCount = m_frames.size();
}