# Touch Screen Camera

```c++
/*
Version:		V1.0
Author:			Vincent
Create Date:	2020/8/19
Note:
	
*/
```

[toc]

# OVERVIEW

## TFT screen
### ILI9488

9488没有arduino下的官方库，根据github上的第三方驱动，修改了一个版本。可以实现Adafruit GFX库的基本功能：画点，线，几何图形，方向调整，位图显示等。S

- /Project_Touch-Screen-Camra/ILI9488 ：ILI9488驱动
- /Project_Touch-Screen-Camra/9488test：ILI9488屏幕测试，SD卡测试
- /Project_Touch-Screen-Camra/SD2TFT ：从SD卡显示图片



## Touch Screen

### NS2009

NS2009是一款I2C协议的电阻屏驱动IC，输出x，y，z1，z2四个轴的12位数据。

### FT6236

FT6236是一款I2C协议的电容屏驱动IC，直接输出滤波后的坐标值。

- /Project_Touch-Screen-Camra/touch_screen_driver_example/ft6236 :FT6236驱动示范
- /Project_Touch-Screen-Camra/touch_screen_driver_example/ns2009 ：NS2009驱动示范
- /Project_Touch-Screen-Camra/touch_screen_driver_example/i2c_scanner ：I2C扫描
- /Project_Touch-Screen-Camra/touch_draw ：触屏画图测试，兼容NS2009和FT6236



## Camera

### OV2640

