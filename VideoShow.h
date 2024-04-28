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

class VideoShow : public QOpenGLWidget
                , public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit VideoShow(QWidget* parent = nullptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLFramebufferObject* m_fbo{nullptr};
    QOpenGLShaderProgram* m_shaderProgram{nullptr};
    QOpenGLShader* m_vertShader{nullptr};
    QOpenGLShader* m_fragShader{nullptr};
    QOpenGLTexture* m_texture{nullptr};
};

#endif // VIDEOSHOW_H
