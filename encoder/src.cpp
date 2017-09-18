//--------------------------------------------------------------------------------
# include "video_encoder.h"
# include "avcodec_auto.h"
# include <memory>
//--------------------------------------------------------------------------------
int main() {
  AvcodecAuto av_auto;
  VideoEncoder encoder("c:\\users\\i.petrov\\desktop\\output.mp4", 640, 480, 30);

  //const unsigned size = 640 * 480 * 3;
  //for(unsigned i = 0; i < 5 * 30; ++i) {
  //  std::unique_ptr<uint8_t[]> data_ptr = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
  //  encoder.encode_frame(data_ptr.get(), size);
  //}

  encoder.close();
  return 0;
}
//--------------------------------------------------------------------------------