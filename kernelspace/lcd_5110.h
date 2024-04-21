/* This file based on source code of LCD5110 for STM32F4
** https://stm32f4-discovery.net/pcd8544-nokia-33105110-lcd-stm32f429-discovery-library/
** and has modified to run on linux OS
*/

#ifndef __LCD_5110
#define __LCD_5110

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>

/*============================	LCD CONFIG	==========================*/


#define LCD_WIDTH			84
#define LCD_HEIGHT			48

#define LCD_POWERDOWN			0x04
#define LCD_ENTRYMODE			0x02
#define LCD_EXTENDEDINSTRUCTION		0x01
#define LCD_DISPLAYBLANK		0x00
#define LCD_DISPLAYNORMAL		0x04
#define LCD_DISPLAYALLON		0x01
#define LCD_DISPLAYINVERTED		0x05

#define LCD_FUNCTIONSET			0x20
#define LCD_DISPLAYCONTROL		0x08
#define LCD_SETYADDR			0x40
#define LCD_SETXADDR			0x80
#define LCD_SETTEMP			0x04
#define LCD_SETBIAS			0x10
#define LCD_SETVOP			0x80

#define LCD_BIAS			0x03
#define LCD_TEMP			0x02
#define LCD_CONTRAST			0x46

#define LCD_CHAR5x7_WIDTH		6
#define LCD_CHAR5x7_HEIGHT		8
#define LCD_CHAR3x5_WIDTH		4
#define LCD_CHAR3x5_HEIGHT		6

#define LCD_BUFFER_SIZE			(LCD_WIDTH * LCD_HEIGHT / 8)

typedef unsigned char			unit8_t;

typedef enum {
	PIN_LOW = 0,
	PIN_HIGH = !PIN_LOW
} Pin_State_t;

typedef enum {
	LCD_COMMAND = 0,
	LCD_DATA = !LCD_COMMAND
} LCD_WriteType_t;

typedef enum {
	LCD_State_Low = 0,
	LCD_State_High = !LCD_State_Low
} LCD_State_t;

typedef enum {
	LCD_Pin_DC = 1,
	LCD_Pin_RST = 2
} LCD_Pin_t;

typedef enum {
	LCD_Pixel_Clear = 0,
	LCD_Pixel_Set = !LCD_Pixel_Clear
} LCD_Pixel_t;

typedef enum {
	LCD_FontSize_5x7 = 0,
	LCD_FontSize_3x5 = !LCD_FontSize_5x7
} LCD_FontSize_t;

typedef enum {
	LCD_Invert_Yes,
	LCD_Invert_No
} LCD_Invert_t;

/*==========================	LCD IOCTL 	================================*/

#define MAJIC_NO			100
#define IOCTL_CLEAR			_IO(MAJIC_NO, 0)
#define IOCTL_HOME			_IO(MAJIC_NO, 1)
#define IOCTL_SET_CONTRAST		_IOW(MAJIC_NO, 2, unsigned char)

typedef struct position {
	unsigned char x;
	unsigned char y;
} Position_t;
#define IOCTL_GOTOXY			_IOW(MAJIC_NO, 3, Position_t)

typedef enum {
	Pixel_Clear = 0,
	Pixel_Set = !Pixel_Clear
} Pixel_t;

typedef struct draw_pixel {
	unsigned char x;
	unsigned char y;
	Pixel_t pixel;
} Draw_Pixel_t;
#define IOCTL_DRAW_PIXEL		_IOW(MAJIC_NO, 4, Draw_Pixel_t)

typedef struct draw_shape {
	unsigned char x0;
	unsigned char y0;
	unsigned char x1;
	unsigned char y1;
	Pixel_t pixel;
} Draw_Shape_t;
#define IOCTL_DRAW_LINE			_IOW(MAJIC_NO, 5, Draw_Shape_t)
#define IOCTL_DRAW_RECT			_IOW(MAJIC_NO, 6, Draw_Shape_t)
#define IOCTL_DRAW_FILL_RECT	_IOW(MAJIC_NO, 7, Draw_Shape_t)

typedef struct draw_circle {
	char x;
	char y;
	char r;
	Pixel_t pixel;
} Draw_Circle_t;
#define IOCTL_DRAW_CIRCLE		_IOW(MAJIC_NO, 8, Draw_Circle_t)
#define IOCTL_DRAW_FILL_CIRCLE		_IOW(MAJIC_NO, 9, Draw_Circle_t)

#define IOCTL_SEND_BUFF			_IO(MAJIC_NO, 10)

typedef enum {
	FontSize_5x7 = 0,
	FontSize_3x5 = !FontSize_5x7
} FontSize_t;

typedef struct draw_string {
	char message[25];
	Pixel_t pixel;
	FontSize_t font;
} Draw_String_t;


/*=============================================================================*/

void gpio_set_pin(struct gpio_desc *gpio, Pin_State_t state);
int spi_send(unsigned char data);
int LCD_send(unsigned char data);
void LCD_Pin(LCD_Pin_t pin, LCD_State_t state);
void LCD_Write(LCD_WriteType_t type, unsigned char data);
void LCD_Home(void);
void LCD_UpdateArea(unsigned char xMin, unsigned char yMin, unsigned char xMax,
			unsigned char yMax);
void LCD_Refresh(void);
void LCD_Clear(void);
void LCD_GotoXY(unsigned char x, unsigned char y);
void LCD_Init(unsigned char contrast);
void LCD_SetContrast(unsigned char contrast);
void LCD_DrawPixel(unsigned char x, unsigned char y, LCD_Pixel_t pixel);
void LCD_Invert(LCD_Invert_t invert);
void LCD_Putc(char c, LCD_Pixel_t color, LCD_FontSize_t size);
void LCD_Puts(char *s, LCD_Pixel_t color, LCD_FontSize_t size);
void LCD_DrawLine(unsigned char x0, unsigned char y0, unsigned char x1,
			unsigned char y1, LCD_Pixel_t color);
void LCD_DrawRectangle(unsigned char x0, unsigned char y0, unsigned char x1,
			unsigned char y1, LCD_Pixel_t color);
void LCD_DrawFilledRectangle(unsigned char x0, unsigned char y0, unsigned char x1,
			unsigned char y1, LCD_Pixel_t color);
void LCD_DrawCircle(char x0, char y0, char r, LCD_Pixel_t color);
void LCD_DrawFilledCircle(char x0, char y0, char r, LCD_Pixel_t color);

#endif