//--------------------------------------------------------------------------------
# include "video_encoder.h"
# include "avcodec_api.h"
//--------------------------------------------------------------------------------
# include <stdio.h>
# include <iostream>
# include <memory>
//--------------------------------------------------------------------------------
class VideoEncoder::Impl {
private:
  FILE *const _strm;
  unsigned _width, _height, _fps;

  AVCodec *codec;
  AVCodecContext *context;
  AVFrame *frame;

public:
  Impl(const std::string &output_file, unsigned width, unsigned height, unsigned fps) :
    _strm(std::fopen(output_file.c_str(), "wb")), _width(width), _height(height), _fps(fps)
  {
    codec = nullptr;
    context = nullptr;
    frame = nullptr;

    if(!_strm || !init())
      throw std::runtime_error("video_encoder failed on init()");
  }

  ~Impl() {
    close();
  }

  bool init() {
    int ret;
    int got_output;
    int i = 0;

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec)
      return false;

    context = avcodec_alloc_context3(codec);
    if(!context)
      return false;

    context->bit_rate = 400000;
    context->width = _width;
    context->height = _height;
    context->time_base = {1, _fps}; //{1, _fps};
    context->gop_size = 10; /* emit one intra frame every ten frames */
    context->max_b_frames = 1;
    context->pix_fmt = AV_PIX_FMT_YUV420P;

    if(avcodec_open2(context, codec, 0) < 0)
      return false;

    av_opt_set(context->priv_data, "preset", "slow", 0);

    frame = av_frame_alloc();
    if(!frame)
      return false;

    frame->format = context->pix_fmt;
    frame->width = context->width;
    frame->height = context->height;

    ret = av_image_alloc(frame->data, frame->linesize, context->width, context->height, context->pix_fmt, 32);
    if(ret < 0)
      return false;

    const unsigned n_frames = _fps * 5;

    AVPacket pkt;
    for(i = 0; i < n_frames; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        //pkt.pts = AV_NOPTS_VALUE;
        //pkt.dts = AV_NOPTS_VALUE;
        //pkt.duration = 1000 / _fps;
        fflush(stdout);

        //pkt.pts = av_rescale_q(m_pVideoStream->pts.val, c->time_base, m_pVideoStream->time_base);
        /* prepare a dummy image */
        /* Y */
        for(int y = 0; y < context->height; y++)
          for(int x = 0; x < context->width; x++)
              frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        /* Cb and Cr */
        for(int y = 0; y < context->height/2; y++)
          for(int x = 0; x < context->width/2; x++) {
              frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
              frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
          }
        frame->pts = i;
        //frame->pts = pkt.pts;
        //frame->pkt_dts = 0;
        /* encode the image */
        
        ret = avcodec_encode_video2(context, &pkt, frame, &got_output);
        if(ret < 0) {
            //fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if(got_output) {
           // printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, _strm);
            fflush(_strm);
            av_free_packet(&pkt);
        }
    }

    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);
        ret = avcodec_encode_video2(context, &pkt, NULL, &got_output);
        if (ret < 0) {
            //fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            //printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, _strm);
            av_free_packet(&pkt);
        }
    }
    /* add sequence end code to have a real mpeg file */
    uint8_t endcode[] = {0, 0, 1, 0xb7};
    fwrite(endcode, 1, sizeof(endcode), _strm);
    avcodec_close(context);
    av_free(context);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
  }

  //bool encode_frame(uint8_t *data_ptr, uint64_t size) {
  //
  //  return false;
  //}

  void close() {
    if(_strm)
      std::fclose(_strm);
  }

private:


};
//--------------------------------------------------------------------------------
VideoEncoder::VideoEncoder(const std::string &output_file, unsigned width, unsigned height, unsigned fps) {
  _impl = new VideoEncoder::Impl(output_file, width, height, fps);
}
//--------------------------------------------------------------------------------
VideoEncoder::~VideoEncoder() {
  delete _impl;
}
//--------------------------------------------------------------------------------
bool VideoEncoder::encode_frame(uint8_t *data_ptr, uint64_t size) {
  //return _impl->encode_frame(data_ptr, size);
  return true;
}
//--------------------------------------------------------------------------------
void VideoEncoder::close() {
  _impl->close();
}
//--------------------------------------------------------------------------------