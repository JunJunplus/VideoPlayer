#include "VideoShow.h"
#include <QOpenGLPixelTransferOptions>
// extern "C" {
// _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
// }

VideoShow::VideoShow(QWidget *parent)
    : QOpenGLWidget(parent)
{

}

void VideoShow::initializeGL()
{
    initializeOpenGLFunctions();


    m_vertShader = new QOpenGLShader(QOpenGLShader::Vertex);
    m_vertShader->compileSourceFile(":/shader/screen.vert");

    m_fragShader = new QOpenGLShader(QOpenGLShader::Fragment);
    m_fragShader->compileSourceFile(":/shader/screen.frag");

    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShader(m_vertShader);
    m_shaderProgram->addShader(m_fragShader);
    m_shaderProgram->link();

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    m_vao.create();
    m_vao.bind();
    m_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(quadVertices, sizeof(quadVertices));

    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 4);
    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->setAttributeBuffer(1, GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 4);
    m_shaderProgram->enableAttributeArray(1);

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("Ytexture", 0);
    m_shaderProgram->setUniformValue("Utexture", 1);
    m_shaderProgram->setUniformValue("Vtexture", 2);
    m_shaderProgram->release();

    // QOpenGLFramebufferObjectFormat format;
    // format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    // format.setMipmap(true);
    // format.setTextureTarget(GL_TEXTURE_2D);
    // format.setInternalTextureFormat(GL_RGBA);

    // m_fbo = new QOpenGLFramebufferObject(size() * devicePixelRatioF(), format);
    // m_fbo->bind();
    // m_texture->bind();
    // m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    // m_fbo->addColorAttachment(size(), GL_RGBA);
    // m_fbo->release();
}

void VideoShow::paintGL()
{
    // 渲染到帧缓冲
    //m_fbo->bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0, 0.0, 0.0, 1);
    if (m_isFirst) {
        return;
    }
    m_vao.bind();
    m_shaderProgram->bind();
    if (m_type == AVPixelFormat::AV_PIX_FMT_YUV420P) {
        glActiveTexture(GL_TEXTURE0);
        m_textures[YUVFrameType::Ytexture]->bind();
        glActiveTexture(GL_TEXTURE1);
        m_textures[YUVFrameType::Utexture]->bind();
        glActiveTexture(GL_TEXTURE2);
        m_textures[YUVFrameType::Vtexture]->bind();
    } else if (m_type == AVPixelFormat::AV_PIX_FMT_NV12) {
        glActiveTexture(GL_TEXTURE0);
        m_textures[NV12FrameType::Ytexture]->bind();
        glActiveTexture(GL_TEXTURE1);
        m_textures[NV12FrameType::UVtexture]->bind();
    }


    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao.release();
    m_shaderProgram->release();
    for (auto texture : m_textures.values()) {
        texture->release();
    }

    // 渲染到屏幕
   //  m_fbo->bindDefault();
   //  glClear(GL_COLOR_BUFFER_BIT);
   // // glClearColor(0, 0.5, 0.7, 1);
   //  m_vao.bind();
   //  m_shaderProgram->bind();
   //  glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
   //  //m_fbo->takeTexture();
   //  glDrawArrays(GL_TRIANGLES, 0, 6);

   //  // 保存显存中的帧
   //  glPixelStorei(GL_PACK_ALIGNMENT, 1);  /* default is 4 */
   //  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   //  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
   //  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   //  uchar *data = (uchar*)malloc(size().height() * size().width() * 32);  // CPU memory
   //  GLint lastBuffer;
   //  glGetIntegerv(GL_READ_BUFFER, &lastBuffer);
   //  // 如果已经通过glBindFramebuffer(GL_READ_FRAMEBUFFER, ...)指定了读取源，则不需要再通过glReadBuffer指定
   //  //glReadBuffer(GL_FRONT_LEFT);
   //  glReadPixels(0, 0, size().width(), size().height(), GL_RGBA, GL_UNSIGNED_BYTE, data);
   //  glReadBuffer(lastBuffer);
   //  QImage image{data, size().width(), size().height(), QImage::Format::Format_RGBA8888};
   //  QImage ima = image.mirrored(false, true);
   //  ima.save("./test.jpg");
}

void VideoShow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    // // 窗口resize时重新创建根据大小重新fbo
    // QOpenGLFramebufferObjectFormat format;
    // format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    // format.setMipmap(true);
    // format.setTextureTarget(GL_TEXTURE_2D);
    // format.setInternalTextureFormat(GL_RGBA);

    // m_fbo = new QOpenGLFramebufferObject(size() * devicePixelRatioF(), format);
    // m_fbo->bind();
    // m_texture->bind();
    // m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    // m_fbo->addColorAttachment(size(), GL_RGBA);
    // m_fbo->release();
    m_size.setWidth(w);
    m_size.setHeight(h);

}

void VideoShow::UpdataTexture(AVFrame *frame)
{
    if (m_isFirst) {
        m_size.setWidth(frame->width);
        m_size.setHeight(frame->height);
        InitTexture(static_cast<AVPixelFormat>(frame->format));
        m_isFirst = false;
    }

    QOpenGLPixelTransferOptions options;
    options.setImageHeight(frame->height);
    if (m_type == AVPixelFormat::AV_PIX_FMT_YUV420P) {
        for (int i = 0; i < 3; ++i) {
            YUVFrameType frameType = static_cast<YUVFrameType>(i);
            options.setRowLength(frame->linesize[i]);
            m_textures[frameType]->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, static_cast<const void *>(frame->data[i]), &options);
        }
    } else if (m_type == AVPixelFormat::AV_PIX_FMT_NV12) {
        options.setRowLength(frame->linesize[0]);
        m_textures[NV12FrameType::Ytexture]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, static_cast<const void *>(frame->data[0]), &options);
        options.setImageHeight(frame->height / 2);
        options.setRowLength(frame->linesize[1] / 2);
        m_textures[NV12FrameType::UVtexture]->setData(QOpenGLTexture::RG, QOpenGLTexture::UInt8, static_cast<const void *>(frame->data[1]), &options);

    }
    this->update();
}

void VideoShow::InitTexture(AVPixelFormat type)
{
    m_type = type;
    if (m_type == AVPixelFormat::AV_PIX_FMT_YUV420P) {
        if (m_textures.isEmpty()) {
            m_textures.insert(YUVFrameType::Ytexture, new QOpenGLTexture(QOpenGLTexture::Target::Target2D));
            m_textures.insert(YUVFrameType::Utexture, new QOpenGLTexture(QOpenGLTexture::Target::Target2D));
            m_textures.insert(YUVFrameType::Vtexture, new QOpenGLTexture(QOpenGLTexture::Target::Target2D));
        }

        for (auto textureType : m_textures.keys()) {
            if (type == AVPixelFormat::AV_PIX_FMT_YUV420P) {
                if (std::get<YUVFrameType>(textureType) == YUVFrameType::Ytexture) {
                    m_textures[textureType]->setSize(m_size.width(), m_size.height());
                } else {
                    m_textures[textureType]->setSize(m_size.width() / 2, m_size.height() / 2);
                }
                m_textures[textureType]->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
                m_textures[textureType]->setFormat(QOpenGLTexture::R8_UNorm);
                m_textures[textureType]->allocateStorage();
            }
        }
    } else if (m_type == AVPixelFormat::AV_PIX_FMT_NV12){
        if (m_textures.isEmpty()) {
            m_textures.insert(NV12FrameType::Ytexture, new QOpenGLTexture(QOpenGLTexture::Target::Target2D));
            m_textures.insert(NV12FrameType::UVtexture, new QOpenGLTexture(QOpenGLTexture::Target::Target2D));
        }
        for (auto textureType : m_textures.keys()) {
            if (std::get<NV12FrameType>(textureType) == NV12FrameType::Ytexture) {
                m_textures[textureType]->setSize(m_size.width(), m_size.height());
                m_textures[textureType]->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
                m_textures[textureType]->setFormat(QOpenGLTexture::R8_UNorm);
            } else {
                m_textures[textureType]->setSize(m_size.width() / 2, m_size.height() / 2);
                m_textures[textureType]->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
                m_textures[textureType]->setFormat(QOpenGLTexture::RG8_UNorm);
            }


            m_textures[textureType]->allocateStorage();
        }

    }

    resizeGL(this->width(), this->height());
}
