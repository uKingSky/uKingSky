/**************************************************
**	文件名:lcd.h
**	版本号:V 1.0
**	文件说明:配置LCD相关参数
**		使用不同的屏请修改第13行的预定义中
***************************************************/

#define PT035			1		// 群创3.5寸屏(320X240)
#define WD_F3224	2		// 胜华3.5寸屏(320X240)
#define TX11D			3		// 日立4.3寸屏(480X272)
#define A070			4		// 群创7.0寸屏(800X480)

#define LCD_TYPE	WD_F3224		//显示类型选择


#define M5D(n) ((n) & 0x1fffff)	// To get lower 21bits

#if(LCD_TYPE == PT035)

#define MVAL				(13)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN			(1)		//0=normal       1=inverted
#define BSWP				(0)		//Byte swap control
#define HWSWP				(1)		//Half word swap control

//TFT 320240
#define LCD_XSIZE_TFT 	(320)	
#define LCD_YSIZE_TFT 	(240)

#define SCR_XSIZE_TFT 	(320)
#define SCR_YSIZE_TFT 	(240)

//Timing parameter for LCD PT035
#define VBPD                      (11)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (5)		//垂直同步信号的脉宽

#define HBPD                      (69)		//水平同步信号的后肩
#define HFPD                      (10)		//水平同步信号的前肩
#define HSPW                      (5)		//水平同步信号的脉宽

#define CLKVAL_TFT           (7) 	

#elif(LCD_TYPE == WD_F3224)

#define MVAL				(13)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN			(1)		//0=normal       1=inverted
#define BSWP				(0)		//Byte swap control
#define HWSWP				(1)		//Half word swap control

//TFT 320240
#define LCD_XSIZE_TFT 	(320)	
#define LCD_YSIZE_TFT 	(240)

#define SCR_XSIZE_TFT 	(320)
#define SCR_YSIZE_TFT 	(240)

//Timing parameter for LCD WD_F3224
#define VBPD                      (12)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (5)		//垂直同步信号的脉宽

#define HBPD                      (69)		//水平同步信号的后肩
#define HFPD                      (10)		//水平同步信号的前肩
#define HSPW                      (5)		//水平同步信号的脉宽

#define CLKVAL_TFT           (4) 	

#elif(LCD_TYPE == TX11D)				// 4.3

#define MVAL				(13)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN			(1)		//0=normal       1=inverted
#define BSWP				(0)		//Byte swap control
#define HWSWP				(1)		//Half word swap control

//TFT 480272
#define LCD_XSIZE_TFT 	(480)	
#define LCD_YSIZE_TFT 	(272)

#define SCR_XSIZE_TFT 	(480)
#define SCR_YSIZE_TFT 	(272)

//Timing parameter for LCD TX11D
#define VBPD                      (5)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (5)		//垂直同步信号的脉宽

#define HBPD                      (5)		//水平同步信号的后肩
#define HFPD                      (5)		//水平同步信号的前肩
#define HSPW                      (5)		//水平同步信号的脉宽

#define CLKVAL_TFT			(5)

#elif(LCD_TYPE == A070)		// 日立4.3存屏(480X272)

#define MVAL				(13)
#define MVAL_USED 	(0)			//0=each frame   1=rate by MVAL
#define INVVDEN			(1)			//0=normal       1=inverted
#define BSWP				(0)			//Byte swap control
#define HWSWP				(1)			//Half word swap control

//TFT_SIZE
#define LCD_XSIZE_TFT 	(800)	
#define LCD_YSIZE_TFT 	(480)

#define SCR_XSIZE_TFT 	(800)
#define SCR_YSIZE_TFT 	(480)

//Timing parameter for 4.3' LCD
#define VBPD 		(25)			//垂直同步信号的后肩
#define VFPD 		(5)			//垂直同步信号的前肩
#define VSPW 		(1)			//垂直同步信号的脉宽

#define HBPD 		(67)			//水平同步信号的后肩
#define HFPD 		(40)			//水平同步信号的前肩
#define HSPW 		(31)			//水平同步信号的脉宽

#define CLKVAL_TFT 	(1)  	

#endif

#define HOZVAL_TFT          (LCD_XSIZE_TFT -1)
#define LINEVAL_TFT         (LCD_YSIZE_TFT -1)


#if((LCD_TYPE == PT035)||(LCD_TYPE == WD_F3224))
	extern const unsigned char phone[];
#elif(LCD_TYPE == TX11D)
	extern const  unsigned char LCD43_ucos[];
#elif(LCD_TYPE == A070)
	extern const  unsigned char LCD70_ucos[];
#endif

