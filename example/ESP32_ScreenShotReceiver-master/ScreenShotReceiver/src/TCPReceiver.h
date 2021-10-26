#ifndef _TCPRECEIVER_H_
#define _TCPRECEIVER_H_

#pragma GCC optimize ("O3")

#ifndef LOVYANGFX_HPP_
#include <LovyanGFX.hpp>  // https://github.com/lovyan03/LovyanGFX/
#endif

#include <WiFi.h>
#include <WiFiServer.h>
#include <esp_heap_caps.h>
#include "tjpgdClass.h"

#undef min

class TCPReceiver
{
public:
  bool setup(LovyanGFX* lcd)
  {
    _lcd = lcd;
    _lcd->startWrite();
    Serial.println("TCPReceiver setup.");

    _lcd_width = lcd->width();
    _lcd_height = lcd->height();
    _bytesize = lcd->getColorConverter()->bytes;
    _fp_jpgWrite = (_bytesize == 2)
                 ? jpgWrite16
                 : jpgWrite24
                 ;
    for (int i = 0; i < 2; ++i) _dmabufs[i] = (uint8_t*)heap_caps_malloc(_lcd_width * 48 * _bytesize, MALLOC_CAP_DMA);

    _dmabuf = _dmabufs[0];

    _tcp.setNoDelay(true);
    _tcp.begin(63333);

    _jdec.multitask_begin();

    Serial.println("TCPReceiver setup done.");
    return true;
  }

  bool loop()
  {
    static uint32_t waitCount = 0;

    if (SENDER_PREFIX_SIZE <= _client.available()
     && SENDER_PREFIX_SIZE == _client.read(_tcpBuf, SENDER_PREFIX_SIZE)) {
      waitCount = 0;
      if (_tcpBuf[0] == 'J'
       && _tcpBuf[1] == 'P'
       && _tcpBuf[2] == 'G') {
        _recv_remain = _tcpBuf[3] | _tcpBuf[4] << 8 | _tcpBuf[5] << 16 | _tcpBuf[6] << 24;
        if (_recv_remain > 600) {
          if (drawJpg()) {
            ++_drawCount;
          }
        } else {
          Serial.println("jpg too short");
        }
        if (0 < _recv_remain) {
          Serial.printf("clear remain data:%d\r\n", _recv_remain);
          int r;
          for (uint32_t retry = 1000; retry; --retry) {
            r = _client.read(_tcpBuf, _recv_remain < TCP_BUF_LEN ? _recv_remain : TCP_BUF_LEN);
            if (r > 0) {
              _recv_remain -= r;
              if (!_recv_remain) break;
            } else {
              delay(1);
              ++_delayCount;
            }
          }
        }
      } else {
        Serial.println("broken data");
        delay(10);
        while (0 < _client.read(_tcpBuf, TCP_BUF_LEN)) delay(10);
      }
    } else
    if (!_client.connected()) {
      _client = _tcp.available();
      _recv_requested = false;
      waitCount = 0;
    } else {
      if (++waitCount > 1000) {
        _recv_requested = false;
      } else {
        delay(1);
        ++_delayCount;
      }
      if (!_recv_requested)   {
        while (0 < _client.read(_tcpBuf, TCP_BUF_LEN));
        _recv_requested = true;
        waitCount = 0;
        Serial.println("data request");
        _client.print("JPG\n");
      }
    }

    if (_sec != millis() / 1000) {
      _sec = millis() / 1000;
      Serial.printf("%2d fps", _drawCount);
      if (_delayCount) Serial.printf(" / delay%3d", _delayCount);
      Serial.print("\r\n");
      _drawCount = 0;
      _delayCount = 0;
    }

    return true;
  }
private:
  static constexpr int TCP_BUF_LEN = 512;
  static constexpr int SENDER_PREFIX_SIZE = 7;
  static constexpr uint32_t _rowskip = 2;

  LovyanGFX* _lcd;
  uint_fast8_t _bytesize;
  uint8_t* _dmabufs[2];
  uint8_t* _dmabuf;

  WiFiServer _tcp;
  WiFiClient _client;
  TJpgD _jdec;
  int32_t _recv_remain = 0;
  uint32_t _sec = 0;
  int32_t _lcd_width;
  int32_t _lcd_height;
  int32_t _out_width;
  int32_t _out_height;
  int32_t _off_x;
  int32_t _off_y;
  int32_t _jpg_x;
  int32_t _jpg_y;
  uint32_t _drawCount = 0;
  uint32_t _delayCount = 0;
  uint8_t _tcpBuf[TCP_BUF_LEN];
  bool _recv_requested = false;

  uint32_t(*_fp_jpgWrite)(TJpgD*,void*,TJpgD::JRECT*);

  static uint32_t jpgRead(TJpgD *jdec, uint8_t *buf, uint32_t len) {
    TCPReceiver* me = (TCPReceiver*)jdec->device;
    WiFiClient* client = &me->_client;
    uint32_t retry;

    if (len == TJPGD_SZBUF) {
      if (me->_recv_requested) {
        me->_recv_requested = false;
      } else
      if (me->_recv_remain < TJPGD_SZBUF*2 && TJPGD_SZBUF < me->_recv_remain) { // dataend read tweak
        len = me->_recv_remain - len;
      }
    } else if (client->available() < len) {
      for (retry = 1000; client->available() < len && retry; --retry) {
        delay(1);
        ++me->_delayCount;
      }
    }

    int l = client->read(buf ? buf : me->_tcpBuf, len);
    if (l <= 0) {
      for (retry = 1000; retry; --retry) {
        delay(1);
        ++me->_delayCount;
        l = client->read(buf ? buf : me->_tcpBuf, len);
        if (l > 0) break;
      }
    }
    if (l <= 0) {
      Serial.printf("jpgRead error:%d:%d:%d\r\n", l, len, client->available());
      return 0;
    }
    me->_recv_remain -= l;
    if (!me->_recv_requested && me->_recv_remain < 3) {
      if (me->_recv_remain >= client->available()) {
        client->print("JPG\n"); // request the next image from the client
        me->_recv_requested = true;
      } else {
        Serial.println("excessive request");
        me->_recv_requested = true;
      }
    }
    return l;
  }

  static uint32_t jpgWrite24(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect) {
    TCPReceiver* me = (TCPReceiver*)jdec->device;

    auto dst = me->_dmabuf;

    uint_fast16_t x = rect->left;
    uint_fast16_t y = rect->top;
    uint_fast16_t w = rect->right + 1 - x;
    uint_fast16_t h = rect->bottom + 1 - y;
    uint_fast16_t outWidth = me->_out_width;
    uint_fast16_t outHeight = me->_out_height;
    uint8_t *src = (uint8_t*)bitmap;
    uint_fast16_t oL = 0, oR = 0;

    if (rect->right < me->_off_x)      return 1;
    if (x >= (me->_off_x + outWidth))  return 1;
    if (rect->bottom < me->_off_y)     return 1;
    if (y >= (me->_off_y + outHeight)) return 1;

    int32_t src_idx = 0;
    int32_t dst_idx = 0;

    uint_fast16_t tmpy = y % ((1 + _rowskip) * jdec->msy << 3);
    if (me->_off_y > y) {
      uint_fast16_t linesToSkip = me->_off_y - y;
      src_idx += linesToSkip * w;
      h -= linesToSkip;
      dst_idx -= tmpy * outWidth;
    } else 
    if (me->_off_y > (y - tmpy)) {
      uint_fast16_t linesToSkip = me->_off_y - (y - tmpy);
      dst_idx -= linesToSkip * outWidth;
    }

    if (me->_off_x > x) {
      oL = me->_off_x - x;
    }
    if (rect->right >= (me->_off_x + outWidth)) {
      oR = (rect->right + 1) - (me->_off_x + outWidth);
    }

    int_fast16_t line = (w - ( oL + oR )) * 3;
    dst_idx += oL + x - me->_off_x + outWidth * tmpy;
    src_idx += oL;
    do {
      memcpy(&dst[dst_idx * 3], &src[src_idx * 3], line);
      dst_idx += outWidth;
      src_idx += w;
    } while (--h);

    return 1;
  }

  static uint32_t jpgWrite16(TJpgD *jdec, void *bitmap, TJpgD::JRECT *rect) {
    TCPReceiver* me = (TCPReceiver*)jdec->device;

    uint16_t *dst = (uint16_t *)me->_dmabuf;

    uint_fast16_t x = rect->left;
    uint_fast16_t y = rect->top;
    uint_fast16_t w = rect->right + 1 - x;
    uint_fast16_t h = rect->bottom + 1 - y;
    uint_fast16_t outWidth = me->_out_width;
    uint_fast16_t outHeight = me->_out_height;
    uint8_t *src = (uint8_t*)bitmap;
    uint_fast16_t oL = 0, oR = 0;

    if (rect->right < me->_off_x)      return 1;
    if (x >= (me->_off_x + outWidth))  return 1;
    if (rect->bottom < me->_off_y)     return 1;
    if (y >= (me->_off_y + outHeight)) return 1;

    uint_fast16_t tmpy = y % ((1 + _rowskip) * jdec->msy << 3);
    if (me->_off_y > y) {
      uint_fast16_t linesToSkip = me->_off_y - y;
      src += linesToSkip * w * 3;
      h -= linesToSkip;
      dst -= tmpy * outWidth;
    } else 
    if (me->_off_y > (y - tmpy)) {
      uint_fast16_t linesToSkip = me->_off_y - (y - tmpy);
      dst -= linesToSkip * outWidth;
    }

    if (me->_off_x > x) {
      oL = me->_off_x - x;
    }
    if (rect->right >= (me->_off_x + outWidth)) {
      oR = (rect->right + 1) - (me->_off_x + outWidth);
    }

    int_fast16_t line = (w - ( oL + oR ));
    dst += oL + x - me->_off_x + outWidth * tmpy;
    src += oL * 3;
    do {
      int i = 0;
      do {
        uint_fast8_t r8 = src[i*3+0] & 0xF8;
        uint_fast8_t g8 = src[i*3+1];
        uint_fast8_t b5 = src[i*3+2] >> 3;
        r8 |= g8 >> 5;
        g8 &= 0x1C;
        b5 = (g8 << 3) + b5;
        dst[i] = r8 | b5 << 8;
      } while (++i != line);
      dst += outWidth;
      src += w * 3;
    } while (--h);

    return 1;
  }

  static uint32_t jpgWriteRow(TJpgD *jdec, uint32_t y, uint32_t h) {
    static int flip = 0;
    TCPReceiver* me = (TCPReceiver*)jdec->device;

    if (y == 0)
      me->_lcd->setAddrWindow(me->_jpg_x, me->_jpg_y, me->_out_width, me->_out_height);

    int16_t outY = y - me->_off_y;
    if (outY < 0) {
      if (h <= - outY) return 1;
      h += outY;
      outY = 0;
    }
    if (me->_lcd_height <= me->_jpg_y + outY) return 1;
    if (me->_lcd_height < me->_jpg_y + outY + h) {
      h = me->_lcd_height - (me->_jpg_y + outY);
    }

    me->_lcd->pushPixelsDMA(me->_dmabuf, me->_out_width * h);

    flip = !flip;
    me->_dmabuf = me->_dmabufs[flip];
    return 1;
  }

  bool drawJpg() {
    TJpgD::JRESULT jres = _jdec.prepare(jpgRead, this);
    if (jres != TJpgD::JDR_OK) {
      Serial.printf("prepare failed! %d\r\n", jres);
      return false;
    }
    _out_width = std::min<int32_t>(_jdec.width, _lcd_width);
    _jpg_x = (_lcd_width - _jdec.width) >> 1;
    if (0 > _jpg_x) {
      _off_x = - _jpg_x;
      _jpg_x = 0;
    } else {
      _off_x = 0;
    }
    _out_height = std::min<int32_t>(_jdec.height, _lcd_height);
    _jpg_y = (_lcd_height- _jdec.height) >> 1;
    if (0 > _jpg_y) {
      _off_y = - _jpg_y;
      _jpg_y = 0;
    } else {
      _off_y = 0;
    }

    jres = _jdec.decomp_multitask(_fp_jpgWrite, jpgWriteRow, _rowskip);
    if (jres != TJpgD::JDR_OK) {
      Serial.printf("decomp failed! %d\r\n", jres);
      return false;
    }

    return true;
  }
};

#endif
