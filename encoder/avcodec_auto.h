//--------------------------------------------------------------------------------
# pragma once
//--------------------------------------------------------------------------------
# ifndef __AVCODEC_AUTO_H__
# define __AVCODEC_AUTO_H__
//--------------------------------------------------------------------------------
# include "avcodec_api.h"
//--------------------------------------------------------------------------------
class AvcodecAuto {
public:
  AvcodecAuto() {
    init();
  }

  inline void init() {
    av_register_all();
  }
};
//--------------------------------------------------------------------------------
# endif //__AVCODEC_AUTO_H__
//--------------------------------------------------------------------------------