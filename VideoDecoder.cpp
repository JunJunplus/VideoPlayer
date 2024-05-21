#include "VideoDecoder.h"
#include <QDebug>
#include <QImage>
#include <thread>
#include <cmath>
/*********************************** FFmpeg获取GPU硬件解码帧格式的回调函数 *****************************************/
static enum AVPixelFormat g_pixelFormat;
/**
 * @brief      回调函数，获取GPU硬件解码帧的格式
 * @param s
 * @param fmt
 * @return
 */
AVPixelFormat get_hw_format(AVCodecContext* s, const enum AVPixelFormat* fmt)
{
    Q_UNUSED(s)
    const enum AVPixelFormat* p;

    for (p = fmt; *p != -1; p++)
    {
        if(*p == g_pixelFormat)
        {
            return *p;
        }
    }

    qDebug() << "无法获取硬件表面格式.";         // 当同时打开太多路视频时，如果超过了GPU的能力，可能会返回找不到解码帧格式
    return AV_PIX_FMT_NONE;
}

VideoDecoder::VideoDecoder(QObject *parent)
    : QObject{parent}
{
         //operate信号发射后启动线程工作
    m_mutex = new std::mutex;
    m_lock;

    AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;      // ffmpeg支持的硬件解码器
    QStringList strTypes;
    while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)       // 遍历支持的设备类型。
    {
        m_HWDeviceTypes.append(type);
        const char* ctype = av_hwdevice_get_type_name(type);  // 获取AVHWDeviceType的字符串名称。
        if(ctype)
        {
            strTypes.append(QString(ctype));
        }
    }
    qDebug() << "支持的硬解码器： " << strTypes;
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
    //m_codec = const_cast<AVCodec*>(avcodec_find_decoder(m_stream->codecpar->codec_id));
    m_codec = const_cast<AVCodec*>(avcodec_find_decoder_by_name("h264_qsv"));
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
    m_codecContext->flags2 |= AV_CODEC_FLAG2_FAST;    // 允许不符合规范的加速技巧。
    m_codecContext->thread_count = 8;                 // 使用8线程解码

    //InitHWDecoder(m_codec);
    //5 打开解码器
    if (avcodec_open2(m_codecContext, nullptr, nullptr) != 0) {
        qDebug() << "avcodec_open2 failed";
    } else {
        qDebug() << "avcodec_open2 successed";
    }
    qDebug() << "解码器： " << m_codecContext->codec->long_name;
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
                                             AV_PIX_FMT_NV12,
                                             m_codecContext->width,
                                             m_codecContext->height,
                                             AV_PIX_FMT_YUV420P,
                                             SWS_BICUBIC, NULL,NULL,NULL);



    m_packet = av_packet_alloc();
    //av_new_packet(m_packet, m_codecContext->width * m_codecContext->height);
    m_frame = av_frame_alloc();
    m_frameHW = av_frame_alloc();
    qDebug()<<"视频详细信息输出";
    //此函数自动打印输入或输出的详细信息
    av_dump_format(m_formatContext, 0, nullptr, 0);
    //fflush(stderr);
    qDebug() << "start: " << m_formatContext->duration / AV_TIME_BASE << "." << m_formatContext->duration % AV_TIME_BASE;
    m_total = m_formatContext->duration / AV_TIME_BASE;
    m_thread = new std::thread(&VideoDecoder::doWork, this);
    m_isEnd = false;
    AVRational frameRate = m_stream->avg_frame_rate;
    m_fps = av_q2d(frameRate);
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
    // m_swsContext = sws_getContext(m_codecContext->width,
    //                               m_codecContext->height,
    //                               m_codecContext->pix_fmt,
    //                               w,
    //                               h,
    //                               AV_PIX_FMT_YUV420P,
    //                               SWS_BICUBIC, NULL,NULL,NULL);
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

    //floor(pts_in_seconds);
    return (qint64)floor(dts_in_seconds);;
}

void VideoDecoder::InitHWDecoder(const AVCodec *codec)
{
    if(!codec) return;

    for(int i = 0; ; i++)
    {
        const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i);    // 检索编解码器支持的硬件配置。
        if(!config)
        {
            qDebug() << "打开硬件解码器失败！";
            return;          // 没有找到支持的硬件配置
        }

        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)       // 判断是否是设备类型
        {
            for(auto i : m_HWDeviceTypes)
            {
                if(config->device_type == AVHWDeviceType(i))                 // 判断设备类型是否是支持的硬件解码器
                {
                    g_pixelFormat = config->pix_fmt;

                    // 打开指定类型的设备，并为其创建AVHWDeviceContext。
                    int ret = av_hwdevice_ctx_create(&hw_device_ctx, config->device_type, nullptr, nullptr, 0);
                    if(ret < 0)
                    {
                        av_buffer_unref(&hw_device_ctx);
                        return ;
                    }
                    qDebug() << "打开硬件解码器：" << av_hwdevice_get_type_name(config->device_type);
                    m_codecContext->hw_device_ctx = av_buffer_ref(hw_device_ctx);  // 创建一个对AVBuffer的新引用。
                    m_codecContext->get_format = get_hw_format;                    // 由一些解码器调用，以选择将用于输出帧的像素格式

                    return;
                }
            }
        }
    }
}

bool VideoDecoder::FrameDataCopy()
{
    if(m_frame->format != g_pixelFormat)
    {

        return false;
    }
#if 1   // av_hwframe_map在ffmpeg3.3以后才有，经过测试av_hwframe_transfer_data的耗时大概是av_hwframe_map的【1.5倍】
    int ret = av_hwframe_map(m_frameHW, m_frame, 0);                   // 映射硬件数据帧
    if(ret < 0)
    {

        return false;
    }
    m_frameHW->width = m_frame->width;
    m_frameHW->height = m_frame->height;
#else
    int ret = av_hwframe_transfer_data(m_frameHW, m_frame, 0);       // 将解码后的数据从GPU复制到CPU(m_frameHW) 这一步比较耗时，在这一步之前硬解码速度比软解码快很多
    if(ret < 0)
    {
        showError(ret);
        av_frame_unref(m_frame);
        return false;
    }
    av_frame_copy_props(m_frameHW, m_frame);                        // 仅将“metadata”字段从src复制到dst。
#endif
    return true;
}

double VideoDecoder::GetFps()
{
    return m_fps;
}
void VideoDecoder::Seek(qint64 time)
{
    qDebug() << "Seek : " << time;

    int64_t target_pts = time * AV_TIME_BASE;
    AVRational rat{1, AV_TIME_BASE};
    int64_t target_timestamp = av_rescale_q(target_pts, rat, m_stream->time_base);
    {
    std::unique_lock<std::mutex> lock(m_seekMutex);
    m_isSeek = true;
    avcodec_flush_buffers(m_codecContext);
    av_seek_frame(m_formatContext, m_streamIndex, target_timestamp, AVSEEK_FLAG_BACKWARD);

    for (auto i : m_frameQue) {
        av_frame_unref(i);
    }
    m_frameQue.clear();

    m_isSeek = false;
    }
    m_condition.notify_all();
}

void VideoDecoder::doWork()
{
    while (true) {
        std::unique_lock<std::mutex> lockSeek(m_seekMutex);

        m_condition.wait(lockSeek, [this](){ return !m_isSeek;});

        int ret = av_read_frame(m_formatContext, m_packet);
        if (ret >= 0) {
            if (m_stream && m_packet->stream_index == m_streamIndex) {

                if (avcodec_send_packet(m_codecContext, m_packet) == 0)
                {
                    if (avcodec_receive_frame(m_codecContext, m_frame) == 0) {
                        // if (!m_frame->key_frame && m_isSeek) {
                        //     continue;
                        // }

                        av_frame_unref(m_frameHW);

                        // if(!m_frame->data[0])               // 如果是硬件解码就进入
                        // {
                        //     FrameDataCopy();
                        // }

                        m_swsFrame = av_frame_alloc();
                        static bool isSw = true;
                        if (isSw)

                        // m_swsContext = sws_getContext(m_frameHW->width,
                        //                               m_frameHW->height,
                        //                               AV_PIX_FMT_NV12,
                        //                               m_frameHW->width,
                        //                               m_frameHW->width,
                        //                               AV_PIX_FMT_YUV420P,
                        //                               SWS_BILINEAR, NULL,NULL,NULL);
                        isSw = false;
                        sws_scale_frame(m_swsContext, m_swsFrame, m_frame);
                        m_swsFrame->pkt_dts = m_packet->dts;
                        m_swsFrame->pts = m_packet->pts;
                        lockSeek.unlock();
                        std::unique_lock<std::mutex> lock(*m_mutex);
                        m_condition.wait(lock, [this]() { return m_frameQue.size() < 100; });

                        m_frameQue.enqueue(m_swsFrame);
                    }
                }
            }
        } else {
            std::unique_lock<std::mutex> lock(*m_mutex);
            m_condition.wait(lock);
        }
    }
}
