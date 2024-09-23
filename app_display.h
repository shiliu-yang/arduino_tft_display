#pragma once

#include "Arduino.h"

#include <TFT_eSPI.h>

/***********************************************************
************************macro define************************
***********************************************************/


/***********************************************************
***********************typedef define***********************
***********************************************************/
extern TFT_eSPI tft;

/***********************************************************
********************function declaration********************
***********************************************************/
void app_display_init(void);

void app_display_page_change(void);

void app_display_weather_icon_set(uint32_t index);

void app_display_weather_real_feel_set(int8_t temp);

void app_display_weather_cur_temp_high_set(int8_t temp);

void app_display_weather_cur_temp_low_set(int8_t temp);

void app_display_weather_cur_humi_set(int8_t humi);

void app_display_onoff_set(uint8_t onoff);

uint8_t app_display_onoff_get(void);

void app_display_colck(uint8_t is_first);
