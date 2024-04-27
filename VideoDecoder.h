#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <memory>
#include <QSharedPointer>
#include <QThread>
#include <QQueue>
#include <mutex>

extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/samplefmt.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/codec.h"
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavdevice/avdevice.h"
}
class VideoDecoder : public QObject
{
    Q_OBJECT
public:
    VideoDecoder(QObject *parent = nullptr);
    void openVideo(const QString& path);
    QImage Decode();

private:
    void doWork();
private:
    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    SwsContext* m_swsContext = nullptr;
    AVStream* m_stream = nullptr;
    AVCodec* m_codec = nullptr;

    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_swsFrame = nullptr;
    uint8_t* m_outBuffer;

    QString m_videoPath;
    int m_streamIndex = -1;
    QQueue<AVFrame *> m_frameQue;

    std::thread* m_thread;
    std::mutex* m_mutex;
};

#endif // VIDEODECODER_H
