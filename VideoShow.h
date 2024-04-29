#ifndef VIDEOSHOW_H
#define VIDEOSHOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
extern "C" {
#include "libavutil/frame.h"
}
class VideoShow : public QOpenGLWidget
                , public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit VideoShow(QWidget* parent = nullptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void UpdataTexture(AVFrame* frame);

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLFramebufferObject* m_fbo{nullptr};
    QOpenGLShaderProgram* m_shaderProgram{nullptr};
    QOpenGLShader* m_vertShader{nullptr};
    QOpenGLShader* m_fragShader{nullptr};
    QOpenGLTexture* m_Ytexture{nullptr};
    QOpenGLTexture* m_Utexture{nullptr};
    QOpenGLTexture* m_Vtexture{nullptr};
    QSize m_size;
};

#endif // VIDEOSHOW_H
