#include "weathernum.h"

#include <TJpg_Decoder.h>

#include "tal_kv.h"
#include "lfs.h"

#include "tal_memory.h"
#include "Log.h"
//int numx;
//int numy;
//int numw;

#define UF_READ_BUFFER_SIZE 3500

int _jpeg_x = 0, _jpeg_y = 0;

lfs_t *lfsHandle = NULL;
lfs_file_t *sg_img_file = NULL;

static unsigned int tuya_uf_draw_input(JDEC* jdec, uint8_t* buf, unsigned int len)
{
  lfs_ssize_t readSize;

  if (lfsHandle == NULL) {
    Serial.println("Failed to get lfs handle");
    return 0;
  }

  readSize = lfs_file_read(lfsHandle, sg_img_file, buf, len);

  return readSize;
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

static int tuya_uf_draw_output(JDEC* jdec, void* bitmap, JRECT* jrect)
{
  jdec = jdec; // Supress warning as ID is not used

  // Retrieve rendering parameters and add any offset
  int16_t  x = jrect->left + _jpeg_x;
  int16_t  y = jrect->top  + _jpeg_y;
  uint16_t w = jrect->right  + 1 - jrect->left;
  uint16_t h = jrect->bottom + 1 - jrect->top;

  return tft_output(x, y, w, h, (uint16_t*)bitmap);
}

void tuya_uf_draw_jpg(int x, int y, const char *img_name)
{
  int result;
  JDEC jdec;
  JRESULT jresult = JDR_OK;

  lfs_file_t file;

  _jpeg_x = x;
  _jpeg_y = y;

  uint8_t *workPool = (uint8_t *)tal_malloc(UF_READ_BUFFER_SIZE);
  if (workPool == NULL) {
    PR_ERR("Failed to malloc workPool");
    return;
  }
  memset(workPool, 0, UF_READ_BUFFER_SIZE);

  lfsHandle = tal_lfs_get();
  if (lfsHandle == NULL) {
    Serial.println("Failed to get lfs handle");
    tal_free(workPool);
    return;
  }

  result = lfs_file_open(lfsHandle, &file, img_name, LFS_O_RDONLY);
  if (result < 0) {
    Serial.println("Failed to open file");
    tal_free(workPool);
    return;
  }
  sg_img_file = &file;

  jdec.swap = true;

  // Analyse input data
  jresult = jd_prepare(&jdec, tuya_uf_draw_input, workPool, UF_READ_BUFFER_SIZE, 0);

  // Extract image and render
  if (jresult == JDR_OK) {
    jresult = jd_decomp(&jdec, tuya_uf_draw_output, 0);
  }

  lfs_file_close(lfsHandle, &file);
  sg_img_file = NULL;

  tal_free(workPool);

  return;
}

//显示天气图标
void WeatherNum::printfweather(int numx,int numy,int numw)
{
  // https://developer.tuya.com/cn/docs/iot/weather-function-description?id=Ka6dcs2cw4avp#title-15-%E9%99%84%E5%BD%95%E5%9B%9B%EF%BC%9A%E4%B8%AD%E6%96%87%E5%A4%A9%E6%B0%94%20UTF-8%20%E7%BC%96%E7%A0%81

  if (120 == numw || 146 == numw) {  // 晴天
    tuya_uf_draw_jpg(numx, numy ,"img_t0");
  } else if (129 == numw || 142 == numw) { // 多云
    tuya_uf_draw_jpg(numx, numy ,"img_t1");
  } else if (132 == numw) { // 阴
    tuya_uf_draw_jpg(numx, numy ,"img_t2");
  } else if (108 == numw || 111 == numw  || 122 == numw || 123 == numw) { // 阵雨
    tuya_uf_draw_jpg(numx, numy ,"img_t3");
  } else if (143 == numw) { // 雷阵雨
    tuya_uf_draw_jpg(numx, numy ,"img_t4");
  } else if (136 == numw) { // 雷阵雨伴有冰雹
    tuya_uf_draw_jpg(numx, numy ,"img_t5");
  } else if (113 == numw) { // 雨夹雪
    tuya_uf_draw_jpg(numx, numy ,"img_t6");
  } else if (112 == numw || 118 == numw || 139 == numw || 141 == numw || 144 == numw) {  // 雨 小到中雨，小雨，中雨，中到大雨
    tuya_uf_draw_jpg(numx, numy ,"img_t7");
  } else if (141 == numw || 101 == numw || 144 == numw || 145 == numw) { // 中雨 大雨 中到大雨 大到暴雨
    tuya_uf_draw_jpg(numx, numy ,"img_t9");
  } else if (107 == numw || 134 == numw || 125 == numw) { // 暴雨 大暴雨 特大暴雨
    tuya_uf_draw_jpg(numx, numy ,"img_t11");
  } else if (138 == numw || 130 == numw) { // 阵雪 小阵雪
    tuya_uf_draw_jpg(numx, numy ,"img_t13");
  } else if (105 == numw || 104 == numw || 128 == numw) { // 雪 小雪 小到中雪
    tuya_uf_draw_jpg(numx, numy ,"img_t14");
  } else if (131 == numw) { // 中雪
    tuya_uf_draw_jpg(numx, numy ,"img_t15");
  } else if (124 == numw || 126 == numw) { // 大雪 暴雪
    tuya_uf_draw_jpg(numx, numy ,"img_t16");
  } else if (106 == numw || 121 == numw) { // 冻雾 雾
    tuya_uf_draw_jpg(numx, numy ,"img_t18");
  } else if (137 == numw) { // 冻雨
    tuya_uf_draw_jpg(numx, numy ,"img_t19");
  } else if (103 == numw) { // 沙尘暴 
    tuya_uf_draw_jpg(numx, numy ,"img_t20");
  }
  // else if (109 == numw) { // 浮尘
  //   tuya_uf_draw_jpg(numx, numy ,"img_t29");
  // } else if (117 == numw) { // 扬沙
  //   tuya_uf_draw_jpg(numx, numy ,"img_t30");
  // } else if (116 == numw) { // 强沙尘暴
  //   tuya_uf_draw_jpg(numx, numy ,"img_t31");
  // }
  else if (140 == numw) { // 霾
    tuya_uf_draw_jpg(numx, numy ,"img_t53");
  }
  else {
    tuya_uf_draw_jpg(numx, numy ,"img_t99");
  }
}
