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
#include <QVarLengthArray>
#include <variant>
extern "C" {
#include "libavutil/frame.h"
}

enum class  YUVFrameType{
    Ytexture,
    Utexture,
    Vtexture
};
enum class  NV12FrameType{
    Ytexture,
    UVtexture,
};


class VideoShow : public QOpenGLWidget
                , public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    using FrameType = std::variant<YUVFrameType, NV12FrameType>;
    explicit VideoShow(QWidget* parent = nullptr);

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void UpdataTexture(AVFrame* frame);
    void InitTexture(AVPixelFormat type);

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

    QMap<FrameType, QOpenGLTexture*> m_textures;
    AVPixelFormat m_type;
    QSize m_size;
    bool m_isFirst = true;
};

#endif // VIDEOSHOW_H
