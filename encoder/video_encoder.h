//--------------------------------------------------------------------------------
# pragma once
//--------------------------------------------------------------------------------
# ifndef __VIDEO_ENCODER_H__
# define __VIDEO_ENCODER_H__
//--------------------------------------------------------------------------------
# include <string>
# include <cstdint>
//--------------------------------------------------------------------------------
class VideoEncoder {
private:
  class Impl;
  Impl *_impl;

public:
  VideoEncoder(const std::string &output_file, unsigned width, unsigned height, unsigned fps);
  ~VideoEncoder();

  bool encode_frame(uint8_t *data_ptr, uint64_t size);
  void close();

private:
  VideoEncoder(const VideoEncoder&) = delete;
  VideoEncoder& operator=(const VideoEncoder&) = delete;
};
//--------------------------------------------------------------------------------
# endif //__VIDEO_ENCODER_H__
//--------------------------------------------------------------------------------