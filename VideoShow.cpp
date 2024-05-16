#include "VideoShow.h"
#include <QOpenGLPixelTransferOptions>

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
    //m_shaderProgram->bind();
    m_Ytexture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    m_Ytexture->setFormat(QOpenGLTexture::R8_UNorm);
    //m_Ytexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_Ytexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
    m_Ytexture->setWrapMode(QOpenGLTexture::ClampToEdge);

    m_Utexture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    m_Utexture->setFormat(QOpenGLTexture::R8_UNorm);
    //m_Utexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_Utexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
    m_Utexture->setWrapMode(QOpenGLTexture::ClampToEdge);

    m_Vtexture = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);
    m_Vtexture->setFormat(QOpenGLTexture::R8_UNorm);
    //m_Vtexture->setMinificationFilter(QOpenGLTexture::Nearest);
    m_Vtexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear,QOpenGLTexture::Linear);
    m_Vtexture->setWrapMode(QOpenGLTexture::ClampToEdge);

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

    m_vao.bind();
    m_shaderProgram->bind();
    glActiveTexture(GL_TEXTURE0);
    m_Ytexture->bind();
    glActiveTexture(GL_TEXTURE1);
    m_Utexture->bind();
    glActiveTexture(GL_TEXTURE2);
    m_Vtexture->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vao.release();
    m_shaderProgram->release();
    m_Ytexture->release();
    m_Utexture->release();
    m_Vtexture->release();
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
    static bool first = true;
    if (first) {
        m_Ytexture->setSize(frame->width, frame->height);
        m_Ytexture->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
        m_Utexture->setSize(frame->width / 2, frame->height / 2);
        m_Utexture->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
        m_Vtexture->setSize(frame->width / 2, frame->height / 2);
        m_Vtexture->allocateStorage(QOpenGLTexture::Red,QOpenGLTexture::UInt8);
        first = false;
    }

    QOpenGLPixelTransferOptions options;
    options.setImageHeight(frame->height);

    options.setRowLength(frame->linesize[0]);
    m_Ytexture->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, reinterpret_cast<const void *>(frame->data[0]), &options);

    options.setRowLength(frame->linesize[1]);
    m_Utexture->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, reinterpret_cast<const void *>(frame->data[1]), &options);

    options.setRowLength(frame->linesize[2]);
    m_Vtexture->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, reinterpret_cast<const void *>(frame->data[2]), &options);

    this->update();
}
