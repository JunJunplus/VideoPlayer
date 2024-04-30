#include "VideoDecoder.h"
#include <QDebug>
#include <QImage>
#include <thread>
#include <cmath>

VideoDecoder::VideoDecoder(QObject *parent)
    : QObject{parent}
{
         //operate信号发射后启动线程工作
    m_mutex = new std::mutex;
    m_lock;
}

void VideoDecoder::openVideo(const QString &path)
{
    m_videoPath = path;
    m_formatContext = avformat_alloc_context();

    //1 打开视频文件
    int ret = avformat_open_input(&m_formatContext, m_videoPath.toStdString().c_str(), nullptr, nullptr);
    if (ret) {
        qDebug() << "avformat_open_input failed";
    } else {
        qDebug() << "avformat_open_input successed";
    }

    //2 查找视频流信息
    ret = avformat_find_stream_info(m_formatContext, nullptr);
    if (ret) {
        qDebug() << "avformat_find_stream_info failed";
        return;
    } else {
        qDebug() << "avformat_find_stream_info successed";
    }

    //3 查找解码器
    for (int i = 0; i < m_formatContext->nb_streams; ++i) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_streamIndex = i;
            break;
        }
    }

    if (m_streamIndex == -1) {
        qDebug() << "find streams index failed";
        return;
    }
    ret = 0;
    if ((ret = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, m_streamIndex, -1, nullptr, 0)) < 0)
    {
        qDebug() << "Could not find stream" << av_get_media_type_string(AVMEDIA_TYPE_VIDEO);
        return ;
    }

    m_stream = m_formatContext->streams[ret];
    m_codec = const_cast<AVCodec*>(avcodec_find_decoder(m_stream->codecpar->codec_id));
    if (m_codec == nullptr) {
        qDebug() << "avcodec_find_decoder failed";
    } else {
        qDebug() << "avcodec_find_decoder successed";
    }

    //4 获取解码器上下文
    m_codecContext = avcodec_alloc_context3(m_codec);
    if (avcodec_parameters_to_context(m_codecContext, m_stream->codecpar) < 0) {
        qDebug() << "avcodec_parameters_to_context failed";
    } else {
        qDebug() << "avcodec_parameters_to_context successed";
    }

    //5 打开解码器
    if (avcodec_open2(m_codecContext, m_codec, nullptr) != 0) {
        qDebug() << "avcodec_open2 failed";
    } else {
        qDebug() << "avcodec_open2 successed";
    }

    //6 获取格式上下文

    //解码的状态类型(0:表示解码完毕，非0:表示正在解码)
    //    int current_frame_index = 0;

    //用于转码（缩放）的参数，转之前的宽高，转之后的宽高，格式等
    //准备一个视频像素数据格式上下文
    //参数一：输入帧数据宽
    //参数二：输入帧数据高
    //参数三：输入帧数据格式
    //参数四：输出帧数据宽
    //参数五：输出帧数据高
    //参数六：输出帧数据格式->AV_PIX_FMT_RGB32
    //参数七：视频像素数据格式转换算法类型
    //参数八：字节对齐类型(C/C++里面)->提高读取效率
    m_swsContext = sws_getContext(m_codecContext->width,
                                             m_codecContext->height,
                                             m_codecContext->pix_fmt,
                                             m_codecContext->width,
                                             m_codecContext->height,
                                             AV_PIX_FMT_YUV420P,
                                             SWS_BICUBIC, NULL,NULL,NULL);



    m_packet = av_packet_alloc();
    //av_new_packet(m_packet, m_codecContext->width * m_codecContext->height);
    m_frame = av_frame_alloc();

    qDebug()<<"视频详细信息输出";
    //此函数自动打印输入或输出的详细信息
    av_dump_format(m_formatContext, 0, nullptr, 0);
    //fflush(stderr);
    qDebug() << "start: " << m_formatContext->duration / AV_TIME_BASE << "." << m_formatContext->duration % AV_TIME_BASE;
    m_total = m_formatContext->duration / AV_TIME_BASE;
    m_thread = new std::thread(&VideoDecoder::doWork, this);
    m_isEnd = false;
}

QImage VideoDecoder::Decode()
{
    static int count = 0;
    QImage image(QSize(800, 600), QImage::Format_RGB888);
    if(!m_frameQue.isEmpty())
    {
        m_mutex->lock();
        AVFrame *frame = m_frameQue.dequeue();
        m_mutex->unlock();

        // 创建
        if (frame) {
            for(int h = 0; h < 600; h++)
            {
                for(int w = 0; w < 800; w ++)
                {
                    int hh = h >> 1;
                    int ww = w >> 1;
                    int Y = frame->data[0][h * frame->linesize[0] + w];
                    int U = frame->data[1][hh * (frame->linesize[1]) + ww];
                    int V = frame->data[2][hh * (frame->linesize[2]) + ww];

                    int C = Y - 16;
                    int D = U - 128;
                    int E = V - 128;

                    int r = 298 * C           + 409 * E + 128;
                    int g = 298 * C - 100 * D - 208 * E + 128;
                    int b = 298 * C + 516 * D           + 128;

                    r = qBound(0, r >> 8, 255);
                    g = qBound(0, g >> 8, 255);
                    b = qBound(0, b >> 8, 255);

                    r = qBound(0, r, 255);
                    g = qBound(0, g, 255);
                    b = qBound(0, b, 255);

                    QRgb rgb = qRgb(r, g, b);
                    image.setPixel(QPoint(w, h), rgb);
                }
            }
            av_frame_unref(frame);
            count++;
            qDebug() << "dequeue: " << count;
            return image;
        }
    }
    return QImage();
}

AVFrame *VideoDecoder::PopFrame()
{
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (!m_frameQue.isEmpty()) {
        return m_frameQue.dequeue();
    } else {
        m_condition.notify_one();
        return nullptr;
    }
    return nullptr;
}

void VideoDecoder::SetSize(int w, int h)
{
    m_swsContext = sws_getContext(m_codecContext->width,
                                  m_codecContext->height,
                                  m_codecContext->pix_fmt,
                                  w,
                                  h,
                                  AV_PIX_FMT_YUV420P,
                                  SWS_BICUBIC, NULL,NULL,NULL);
}

qint64 VideoDecoder::GetTotalTime()
{
    return m_total;
}

qint64 VideoDecoder::GetCurStamp(AVFrame* frame)
{

    AVRational ration {1, AV_TIME_BASE};
    int64_t dts = frame->pkt_dts;
    int64_t pts = frame->pts;

    double dts_in_seconds = dts * av_q2d(m_stream->time_base);
    double pts_in_seconds = pts * av_q2d(m_stream->time_base);
    qDebug() << "CurStamp: " << dts_in_seconds << "--" << pts_in_seconds;
    //floor(pts_in_seconds);
    return (qint64)floor(pts_in_seconds);;
}

void VideoDecoder::Seek(qint64 time)
{
    qDebug() << "Seek : " << time;
    int64_t target_pts = time * AV_TIME_BASE;
    AVRational rat{1, AV_TIME_BASE};
    int64_t target_timestamp = av_rescale_q(target_pts, rat, m_stream->time_base);
    std::unique_lock<std::mutex> lock(m_seekMutex);
    m_isSeek = true;
    av_seek_frame(m_formatContext, m_streamIndex, target_timestamp, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(m_codecContext);
    for (auto i : m_frameQue) {
        av_frame_unref(i);
    }
    m_frameQue.clear();
    m_condition.notify_one();
}

void VideoDecoder::doWork()
{
    while (true) {
        std::unique_lock<std::mutex> lock(m_seekMutex);
        int ret = av_read_frame(m_formatContext, m_packet);
        if (ret >= 0) {
            if (m_stream && m_packet->stream_index == m_streamIndex) {

                if (avcodec_send_packet(m_codecContext, m_packet) == 0)
                {
                    if (avcodec_receive_frame(m_codecContext, m_frame) == 0) {
                        // if (!m_frame->key_frame && m_isSeek) {
                        //     continue;
                        // }
                        m_swsFrame = av_frame_alloc();
                        sws_scale_frame(m_swsContext, m_swsFrame, m_frame);
                        m_swsFrame->pkt_dts = m_packet->dts;
                        m_swsFrame->pts = m_packet->pts;
                        std::unique_lock<std::mutex> lock(*m_mutex);
                        //m_condition.wait(lock, [this]() { return m_frameQue.size() < 100; });

                        m_frameQue.enqueue(m_swsFrame);
                        m_isSeek = false;

                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                }
            }
        } else {
            std::unique_lock<std::mutex> lock(*m_mutex);
            m_condition.wait(lock);
        }
    }
}
