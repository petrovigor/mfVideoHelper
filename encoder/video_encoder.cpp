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






# include "video_encoder.h"
# include "avcodec_api.h"
# include <iostream>

VideoEncoder::~VideoEncoder() {
  close();
}

bool VideoEncoder::encode_frame(uint8_t *data_ptr, uint64_t size) {return false;}
void VideoEncoder::close() {}

inline void fill_data(AVFrame **frame_ptr, unsigned width, unsigned height, int i) {
  AVFrame *const p = *frame_ptr;

  for(int y = 0; y < height; y++)
    for(int x = 0; x < width; x++)
        p->data[0][y * p->linesize[0] + x] = x + y + i * 3;

  for(int y = 0; y < height / 2; y++)
    for(int x = 0; x < width / 2; x++) {
        p->data[1][y * p->linesize[1] + x] = 128 + y + i * 2;
        p->data[2][y * p->linesize[2] + x] = 64 + x + i * 5;
    }
}

VideoEncoder::VideoEncoder(const std::string &output_file, unsigned width, unsigned height, unsigned fps) {
  const unsigned seconds = 5;
  AVFormatContext *fmt_ctx = avformat_alloc_context();

  if(!fmt_ctx)
    throw std::runtime_error("err");

  fmt_ctx->oformat = av_guess_format(0, output_file.c_str(), 0);

  if(avio_open2(&fmt_ctx->pb, output_file.c_str(), AVIO_FLAG_WRITE, 0, 0) < 0)
    throw std::runtime_error("err");

  AVCodec *encoder = avcodec_find_encoder_by_name("libx264");
  AVStream *outst = avformat_new_stream(fmt_ctx, encoder);

  outst->codec->width = width;
  outst->codec->height = height;
  outst->codec->pix_fmt = AV_PIX_FMT_YUV420P;
  outst->codec->bit_rate = 400000;
  outst->codec->gop_size = 0;
  //outst->codec->time_base = {1, fps};
  /*outst->time_base = */
  outst->codec->time_base = {1, fps};

  if(av_opt_set(outst->codec->priv_data, "preset", "fast", 0) < 0)
    throw std::runtime_error("err");

  if(av_opt_set(outst->codec->priv_data, "crf", "10", 0) < 0)
    throw std::runtime_error("err");

  //outst->nb_frames = (fps * seconds);

  if(avcodec_open2(outst->codec, encoder, 0) < 0)
    throw std::runtime_error("err");

  if(avformat_write_header(fmt_ctx, 0) < 0)
    throw std::runtime_error("err");

  AVFrame *frame = av_frame_alloc();

  int frame_counter = 0;
  int counter = 0,
    counter1 = 0;

  {
    int got_output, i;
    if(!frame)
      throw std::runtime_error("err");

    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;

    if(av_image_alloc(frame->data, frame->linesize, width, height, AV_PIX_FMT_YUV420P, 32) < 0)
      throw std::runtime_error("err");
    
    int64_t pts = 0;
    for(int j = 0; j < seconds; ++j) {
      for(i = 0; i < fps; ++i) {
        fill_data(&frame, width, height, i);

        //frame->pts = frame_counter++;

        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL;
        pkt.size = 0;

        frame->pts = pts;
        pts += 1;

        if(avcodec_encode_video2(outst->codec, &pkt, frame, &got_output) < 0)
          throw std::runtime_error("err");

        //if(pkt.pts != AV_NOPTS_VALUE)
        //  pkt.pts =  av_rescale_q(pkt.pts, outst->codec->time_base, outst->time_base);
        //
        //if(pkt.dts != AV_NOPTS_VALUE)
        //  pkt.dts = av_rescale_q(pkt.dts, outst->codec->time_base, outst->time_base);

       // got_output = 0;
        if(got_output) {
          ++counter;
          
          av_interleaved_write_frame(fmt_ctx, &pkt);
          //av_write_frame(fmt_ctx, &pkt);
          av_free_packet(&pkt);
        }
      }
    }

    av_freep(&frame->data[0]);
    av_frame_free(&frame);

    for(got_output = 1; got_output; i++) {
      AVPacket pkt;
      av_init_packet(&pkt);
      pkt.data = NULL;
      pkt.size = 0;

      if(avcodec_encode_video2(outst->codec, &pkt, NULL, &got_output) < 0)
        throw std::runtime_error("err");
      
      if(pkt.pts != AV_NOPTS_VALUE)
        pkt.pts =  av_rescale_q(pkt.pts, outst->codec->time_base, outst->time_base);

      if(pkt.dts != AV_NOPTS_VALUE)
        pkt.dts = av_rescale_q(pkt.dts, outst->codec->time_base, outst->time_base);

      if(got_output) {
        ++counter1;

        av_interleaved_write_frame(fmt_ctx, &pkt);
        //av_write_frame(fmt_ctx, &pkt);
        av_free_packet(&pkt);
      }
    }
  }

  if(av_write_trailer(fmt_ctx) < 0)
    throw std::runtime_error("err");

  avio_close(fmt_ctx->pb);
  for(int i = 0; i < fmt_ctx->nb_streams; ++i)
    if(fmt_ctx->streams[i]->codec)
      avcodec_close(fmt_ctx->streams[i]->codec);

  avformat_free_context(fmt_ctx);

  std::cout << "Frames = " << counter << " Frames2 = " << counter1;
  std::cout << std::endl;
}
















