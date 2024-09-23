#include "app_display.h"

#include "tal_time_service.h"

#include "Inconsolata_Regular_64.h"

#include <Log.h>

int lastMinute = -1;  // 用于存储上一次的分钟，用来判断是否需要更新
int screenWidth = 240;
int screenHeight = 240;

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

#define CLOCK_DIGIT_Y_POS (-5)


// 61, 102, 55
#define DARK_MOSS_GREEN (0x3B26)  // 暗苔绿
// 38, 42, 43
#define DARK_SLATE_GRAY (0x2145)  // 暗岩灰
// 200, 195, 188
#define LIGHT_GRAY      (0xCE17)  // 亮灰色

#define BACKGROUND_COLOR        DARK_MOSS_GREEN
#define TEXT_COLOR              LIGHT_GRAY
#define TEXT_BACKGROUND_COLOR   DARK_SLATE_GRAY

#define DIGIT_WIDTH   50
#define DIGIT_HEIGHT  72

#define DIGIT_FONT    Inconsolata_Regular_64

void drawTextBackground(int x, int y, int w, int h, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, 5, color);
}

// 显示数字函数，接受数字和位置参数
void drawDigital(int digit, int x, int y, uint16_t color, uint16_t bgColor) {
  // draw text background
  drawTextBackground(x, y, DIGIT_WIDTH, DIGIT_HEIGHT, bgColor);

  // draw text
  tft.setTextColor(color);
  tft.loadFont(DIGIT_FONT);

  int16_t font_h, font_w;
  font_w = tft.textWidth(String(digit).c_str());
  font_h = tft.fontHeight();

  // PR_DEBUG("---> font_h: %d, font_w: %d\n", font_h, font_w);

  // 计算字体居中位置
  int16_t x_pos = x + (DIGIT_WIDTH - font_w) / 2;
  int16_t y_pos = y + (DIGIT_HEIGHT - font_h) / 2;

  tft.setCursor(x_pos, y_pos);
  tft.print(digit);
  tft.unloadFont();
}

// :
void drawColon(int x, int y, uint16_t color) {
  // draw text
  tft.setTextColor(color);
  tft.loadFont(DIGIT_FONT);

  int16_t font_h, font_w;
  font_w = tft.textWidth(":");
  font_h = tft.fontHeight();

  PR_DEBUG("---> font_h: %d, font_w: %d\n", font_h, font_w);

  // 计算字体居中位置
  int16_t x_pos = x + (SCREEN_WIDTH - font_w) / 2;
  int16_t y_pos = y + (SCREEN_HEIGHT - font_h) / 2;

  tft.setCursor(x_pos, y_pos);
  tft.print(":");
  tft.unloadFont();
}

// 翻页动画函数
void flipAnimation(int oldDigit, int newDigit, int x, int y) {
  // // 数字背景色：38, 42, 43
  // // 数字颜色：200, 195, 188

  // // 擦除旧数字
  // drawDigit(oldDigit, x, y, TFT_BLACK);
  
  // // 模拟翻页效果 (简单的闪烁效果)
  // delay(100);
  
  // // 显示新数字
  // drawDigit(newDigit, x, y, TFT_WHITE);
}

void drawClock(int hour, int minute, uint16_t color) {
  // 冒号
  drawColon(-4, 0, TEXT_BACKGROUND_COLOR);

  // 时钟数字
  drawDigital(hour / 10, 5, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, color, TEXT_BACKGROUND_COLOR);
  drawDigital(hour % 10, 5*2 + DIGIT_WIDTH, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, color, TEXT_BACKGROUND_COLOR);

  // 分钟数字
  drawDigital(minute / 10, 5*3+16 + DIGIT_WIDTH*2, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, color, TEXT_BACKGROUND_COLOR);
  drawDigital(minute % 10, 5*4+16 + DIGIT_WIDTH*3, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, color, TEXT_BACKGROUND_COLOR);
}

static int last_hour_h = -1, last_hour_l = -1, last_minute_h = -1, last_minute_l = -1;

void app_display_colck(uint8_t is_first)
{

  POSIX_TM_S now_tm;

  if (is_first) {
    tft.fillScreen(BACKGROUND_COLOR);
    last_hour_h = -1;
    last_hour_l = -1;
    last_minute_h = -1;
    last_minute_l = -1;

    // 冒号
    drawColon(-4, 0, TEXT_BACKGROUND_COLOR);
  }

  // 获取当前时间
  memset(&now_tm, 0, sizeof(POSIX_TM_S));
  // tal_time_get(&now_tm);
  tal_time_get_local_time_custom(0, &now_tm);

  int hour_h = now_tm.tm_hour / 10;
  int hour_l = now_tm.tm_hour % 10;
  int minute_h = now_tm.tm_min / 10;
  int minute_l = now_tm.tm_min % 10;

  if (minute_l != last_minute_l) {
    // clear
    if (last_minute_l > 0) {
      drawDigital(last_minute_l, 5*4+16 + DIGIT_WIDTH*3, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_BACKGROUND_COLOR, TEXT_BACKGROUND_COLOR);
    }
    drawDigital(minute_l, 5*4+16 + DIGIT_WIDTH*3, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_COLOR, TEXT_BACKGROUND_COLOR);
    last_minute_l = minute_l;
  }

  if (minute_h != last_minute_h) {
    // clear
    if (last_minute_h > 0) {
      drawDigital(last_minute_h, 5*3+16 + DIGIT_WIDTH*2, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_BACKGROUND_COLOR, TEXT_BACKGROUND_COLOR);
    }
    drawDigital(minute_h, 5*3+16 + DIGIT_WIDTH*2, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_COLOR, TEXT_BACKGROUND_COLOR);
    last_minute_h = minute_h;
  }

  if (hour_l != last_hour_l) {
    // clear
    if (last_hour_l > 0) {
      drawDigital(last_hour_l, 5*2 + DIGIT_WIDTH, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_BACKGROUND_COLOR, TEXT_BACKGROUND_COLOR);
    }
    drawDigital(hour_l, 5*2 + DIGIT_WIDTH, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_COLOR, TEXT_BACKGROUND_COLOR);
    last_hour_l = hour_l;
  }

  if (hour_h != last_hour_h) {
    // clear
    if (last_hour_h > 0) {
      drawDigital(last_hour_h, 5, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_BACKGROUND_COLOR, TEXT_BACKGROUND_COLOR);
    }
    drawDigital(hour_h, 5, (SCREEN_WIDTH - DIGIT_WIDTH) / 2 + CLOCK_DIGIT_Y_POS, TEXT_COLOR, TEXT_BACKGROUND_COLOR);
    last_hour_h = hour_h;
  }
}
