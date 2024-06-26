#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <memory>
#include <QSharedPointer>
#include <QThread>
#include <QQueue>
#include <mutex>
#include <condition_variable>
#include <QThread>

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
    AVFrame* PopFrame();
    void SetSize(int w, int h);
    qint64 GetTotalTime();
    qint64 GetCurStamp(AVFrame* frame);
    void InitHWDecoder(const AVCodec* codec);
    bool FrameDataCopy();
    double GetFps();
public slots:
    void Seek(qint64 time);
private:
    void doWork();
    void SaveCurPack();
private:
    AVFormatContext* m_formatContext = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    SwsContext* m_swsContext = nullptr;
    AVStream* m_stream = nullptr;
    AVCodec* m_codec = nullptr;

    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_frameHW = nullptr;
    AVFrame* m_swsFrame = nullptr;
    AVPacket* m_curPacket = nullptr;
    AVFrame* m_curFrame = nullptr;
    double m_fps = 0.0;
    uint8_t* m_outBuffer;
    QList<int> m_HWDeviceTypes;
    AVBufferRef* hw_device_ctx = nullptr;         // 对数据缓冲区的引用

    QString m_videoPath;
    int m_streamIndex = -1;
    QQueue<AVFrame *> m_frameQue;

    std::thread* m_thread;
    std::mutex* m_mutex;
    std::mutex m_seekMutex;
    std::condition_variable m_seekCondition;
    std::condition_variable m_queCondition;
    std::unique_lock<std::mutex> m_lock;

    QThread m_qthread;
    bool m_isEnd = false;
    qint64 m_total = 0;
    bool m_isSeek = false;
};

#endif // VIDEODECODER_H
