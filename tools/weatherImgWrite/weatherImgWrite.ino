#include <Arduino.h>

#include "tal_kv.h"
#include "lfs.h"

#include "Log.h"

#include "./tianqi/t0.h"
#include "./tianqi/t1.h"
#include "./tianqi/t2.h"
#include "./tianqi/t3.h"
#include "./tianqi/t4.h"
#include "./tianqi/t5.h"
#include "./tianqi/t6.h"
#include "./tianqi/t7.h"
#include "./tianqi/t9.h"
#include "./tianqi/t11.h"
#include "./tianqi/t13.h"
#include "./tianqi/t14.h"
#include "./tianqi/t15.h"
#include "./tianqi/t16.h"
#include "./tianqi/t18.h"
#include "./tianqi/t19.h"
#include "./tianqi/t20.h"
#include "./tianqi/t29.h"
#include "./tianqi/t30.h"
#include "./tianqi/t31.h"
#include "./tianqi/t53.h"
#include "./tianqi/t99.h"

void img_write(const char *name, const uint8_t *img, size_t size) {
  int result;
  lfs_file_t file;

  lfs_t *lfsHandle = tal_lfs_get();
  if (lfsHandle == NULL) {
    Serial.println("Failed to get lfs handle");
    return;
  }

  result = lfs_file_open(lfsHandle, &file, name, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
  if (result < 0) {
    Serial.println("Failed to open file");
    return;
  }

  lfs_file_rewind(lfsHandle, &file);
  result = lfs_file_write(lfsHandle, &file, img, size);
  if (result < 0) {
    Serial.println("Failed to write file");
  }
  lfs_file_close(lfsHandle, &file);

  return;
}

#define IMG_SIZE 1024*3
uint8_t imgRead[IMG_SIZE] = {0};

void img_read(const char *name) {
  int result;
  lfs_file_t file;
  lfs_ssize_t readSize;

  lfs_t *lfsHandle = tal_lfs_get();
  if (lfsHandle == NULL) {
    Serial.println("Failed to get lfs handle");
    return;
  }

  result = lfs_file_open(lfsHandle, &file, name, LFS_O_RDONLY);
  if (result < 0) {
    Serial.println("Failed to open file");
    return;
  }

  lfs_ssize_t totalSize = 0;

  do {
    readSize = lfs_file_read(lfsHandle, &file, imgRead, IMG_SIZE);
    Serial.print("Read size: "); Serial.println(readSize);

    if (readSize < 0) {
      Serial.println("Failed to read file");
      break;
    }

    if (readSize == 0) {
      break;
    }

    PR_HEXDUMP_DEBUG("read", imgRead, readSize);

    totalSize += readSize;

  } while (readSize > 0);

  Serial.print("Total size: "); Serial.println(totalSize);

  lfs_file_close(lfsHandle, &file);

  return;
}

// 定义函数
uint32_t get_free_space(lfs_t *lfs) {
  struct lfs_fsinfo fs_info;

  if (lfs == NULL) {
      return 0;
  }

  int rt = lfs_fs_stat(lfs, &fs_info);
  if (rt < 0) {
      return 0;
  }

  uint32_t total = fs_info.block_size * fs_info.block_count;

  lfs_ssize_t fs_size = lfs_fs_size(lfs);

  Serial.print("Total: "); Serial.println(total);
  Serial.print("fs_size: "); Serial.println(fs_size);

  return total - fs_size;
}


void setup() {

  // 在 T2 上使用 Benken Write 擦除以下地址：
  // 应用区域: start: 0x11000, length: 0x119000
  // 文件系统: start: 0x1d2000, length: 0x2e000 // 会将文件系统中存储的授权信息，联网信息给擦除掉

  Serial.begin(115200);

  img_write("img_t0", t0, sizeof(t0));    Serial.println("Write img_t0  done");
  img_write("img_t1", t1, sizeof(t1));    Serial.println("Write img_t1  done");
  img_write("img_t2", t2, sizeof(t2));    Serial.println("Write img_t2  done");
  img_write("img_t3", t3, sizeof(t3));    Serial.println("Write img_t3  done");
  img_write("img_t4", t4, sizeof(t4));    Serial.println("Write img_t4  done");
  img_write("img_t5", t5, sizeof(t5));    Serial.println("Write img_t5  done");
  img_write("img_t6", t6, sizeof(t6));    Serial.println("Write img_t6  done");
  img_write("img_t7", t7, sizeof(t7));    Serial.println("Write img_t7  done");
  img_write("img_t9", t9, sizeof(t9));    Serial.println("Write img_t9  done");
  img_write("img_t11", t11, sizeof(t11)); Serial.println("Write img_t11 done");
  img_write("img_t13", t13, sizeof(t13)); Serial.println("Write img_t13 done");
  img_write("img_t14", t14, sizeof(t14)); Serial.println("Write img_t14 done");
  img_write("img_t15", t15, sizeof(t15)); Serial.println("Write img_t15 done");
  img_write("img_t16", t16, sizeof(t16)); Serial.println("Write img_t16 done");
  img_write("img_t18", t18, sizeof(t18)); Serial.println("Write img_t18 done");
  img_write("img_t19", t19, sizeof(t19)); Serial.println("Write img_t19 done");
  img_write("img_t20", t20, sizeof(t20)); Serial.println("Write img_t20 done");
  img_write("img_t53", t53, sizeof(t53)); Serial.println("Write img_t53 done");
  img_write("img_t99", t99, sizeof(t99)); Serial.println("Write img_t99 done");
}

void loop() {
  delay(1000);
}
