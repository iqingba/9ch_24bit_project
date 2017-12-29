/**
  ******************************************************************************
  * @file    bsp_ili9806g_lcd.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ILI9806G液晶屏驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火  STM32 F407 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 

#include "./lcd/bsp_ili9806g_lcd.h"
#include "./font/fonts.h"	

//根据液晶扫描方向而变化的XY像素宽度
//调用ILI9806G_GramScan函数设置方向时会自动更改
uint16_t LCD_X_LENGTH = ILI9806G_MORE_PIXEL;
uint16_t LCD_Y_LENGTH = ILI9806G_LESS_PIXEL;

//液晶屏扫描模式，本变量主要用于方便选择触摸屏的计算参数
//参数可选值为0-7
//调用ILI9806G_GramScan函数设置方向时会自动更改
//LCD刚初始化完成时会使用本默认值
uint8_t LCD_SCAN_MODE =6;


static sFONT *LCD_Currentfonts = &Font16x32;  //英文字体
static uint16_t CurrentTextColor   = WHITE;//前景色
static uint16_t CurrentBackColor   = BLACK;//背景色

__inline void                 ILI9806G_Write_Cmd           ( uint16_t usCmd );
__inline void                 ILI9806G_Write_Data          ( uint16_t usData );
__inline uint16_t             ILI9806G_Read_Data           ( void );
static void                   ILI9806G_Delay               ( __IO uint32_t nCount );
static void                   ILI9806G_GPIO_Config         ( void );
static void                   ILI9806G_FSMC_Config         ( void );
static void                   ILI9806G_REG_Config          ( void );
static void                   ILI9806G_SetCursor           ( uint16_t usX, uint16_t usY );
static __inline void          ILI9806G_FillColor           ( uint32_t ulAmout_Point, uint16_t usColor );
static uint16_t               ILI9806G_Read_PixelData      ( void );


/**
  * @brief  简单延时函数
  * @param  nCount ：延时计数值
  * @retval 无
  */	
static void Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
	
}

/**
  * @brief  向ILI9806G写入命令
  * @param  usCmd :要写入的命令（表寄存器地址）
  * @retval 无
  */	
__inline void ILI9806G_Write_Cmd ( uint16_t usCmd )
{
	* ( __IO uint16_t * ) ( FSMC_Addr_ILI9806G_CMD ) = usCmd;
	
}


/**
  * @brief  向ILI9806G写入数据
  * @param  usData :要写入的数据
  * @retval 无
  */	
__inline void ILI9806G_Write_Data ( uint16_t usData )
{
	* ( __IO uint16_t * ) ( FSMC_Addr_ILI9806G_DATA ) = usData;
	
}


/**
  * @brief  从ILI9806G读取数据
  * @param  无
  * @retval 读取到的数据
  */	
__inline uint16_t ILI9806G_Read_Data ( void )
{
	return ( * ( __IO uint16_t * ) ( FSMC_Addr_ILI9806G_DATA ) );
	
}


/**
  * @brief  用于 ILI9806G 简单延时函数
  * @param  nCount ：延时计数值
  * @retval 无
  */	
static void ILI9806G_Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
	
}


/**
  * @brief  初始化ILI9806G的IO引脚
  * @param  无
  * @retval 无
  */
static void ILI9806G_GPIO_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能FSMC对应相应管脚时钟*/
	RCC_AHB1PeriphClockCmd ( 	
													/*控制信号*/
													ILI9806G_CS_CLK|ILI9806G_DC_CLK|ILI9806G_WR_CLK|
													ILI9806G_RD_CLK	|ILI9806G_BK_CLK|ILI9806G_RST_CLK|
													/*数据信号*/
													ILI9806G_D0_CLK|ILI9806G_D1_CLK|	ILI9806G_D2_CLK | 
													ILI9806G_D3_CLK | ILI9806G_D4_CLK|ILI9806G_D5_CLK|
													ILI9806G_D6_CLK | ILI9806G_D7_CLK|ILI9806G_D8_CLK|
													ILI9806G_D9_CLK | ILI9806G_D10_CLK|ILI9806G_D11_CLK|
													ILI9806G_D12_CLK | ILI9806G_D13_CLK|ILI9806G_D14_CLK|
													ILI9806G_D15_CLK	, ENABLE );
		
	
	/* 配置FSMC相对应的数据线,FSMC-D0~D15 */	
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D0_PIN; 
    GPIO_Init(ILI9806G_D0_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D0_PORT,ILI9806G_D0_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D1_PIN; 
    GPIO_Init(ILI9806G_D1_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D1_PORT,ILI9806G_D1_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D2_PIN; 
    GPIO_Init(ILI9806G_D2_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D2_PORT,ILI9806G_D2_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D3_PIN; 
    GPIO_Init(ILI9806G_D3_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D3_PORT,ILI9806G_D3_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D4_PIN; 
    GPIO_Init(ILI9806G_D4_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D4_PORT,ILI9806G_D4_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D5_PIN; 
    GPIO_Init(ILI9806G_D5_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D5_PORT,ILI9806G_D5_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D6_PIN; 
    GPIO_Init(ILI9806G_D6_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D6_PORT,ILI9806G_D6_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D7_PIN; 
    GPIO_Init(ILI9806G_D7_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D7_PORT,ILI9806G_D7_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D8_PIN; 
    GPIO_Init(ILI9806G_D8_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D8_PORT,ILI9806G_D8_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D9_PIN; 
    GPIO_Init(ILI9806G_D9_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D9_PORT,ILI9806G_D9_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D10_PIN; 
    GPIO_Init(ILI9806G_D10_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D10_PORT,ILI9806G_D10_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D11_PIN; 
    GPIO_Init(ILI9806G_D11_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D11_PORT,ILI9806G_D11_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D12_PIN; 
    GPIO_Init(ILI9806G_D12_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D12_PORT,ILI9806G_D12_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D13_PIN; 
    GPIO_Init(ILI9806G_D13_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D13_PORT,ILI9806G_D13_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D14_PIN; 
    GPIO_Init(ILI9806G_D14_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D14_PORT,ILI9806G_D14_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_D15_PIN; 
    GPIO_Init(ILI9806G_D15_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_D15_PORT,ILI9806G_D15_PinSource,FSMC_AF);

	/* 配置FSMC相对应的控制线
	 * FSMC_NOE   :LCD-RD
	 * FSMC_NWE   :LCD-WR
	 * FSMC_NE1   :LCD-CS
	 * FSMC_A0    :LCD-DC
	 */
    GPIO_InitStructure.GPIO_Pin = ILI9806G_RD_PIN; 
    GPIO_Init(ILI9806G_RD_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_RD_PORT,ILI9806G_RD_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_WR_PIN; 
    GPIO_Init(ILI9806G_WR_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_WR_PORT,ILI9806G_WR_PinSource,FSMC_AF);

    GPIO_InitStructure.GPIO_Pin = ILI9806G_CS_PIN; 
    GPIO_Init(ILI9806G_CS_PORT, &GPIO_InitStructure);   
    GPIO_PinAFConfig(ILI9806G_CS_PORT,ILI9806G_CS_PinSource,FSMC_AF);  

    GPIO_InitStructure.GPIO_Pin = ILI9806G_DC_PIN; 
    GPIO_Init(ILI9806G_DC_PORT, &GPIO_InitStructure);
    GPIO_PinAFConfig(ILI9806G_DC_PORT,ILI9806G_DC_PinSource,FSMC_AF);
	

  /* 配置LCD复位RST控制管脚*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = ILI9806G_RST_PIN; 
	GPIO_Init ( ILI9806G_RST_PORT, & GPIO_InitStructure );
		
	/* 配置LCD背光控制管脚BK*/
	GPIO_InitStructure.GPIO_Pin = ILI9806G_BK_PIN; 
	GPIO_Init ( ILI9806G_BK_PORT, & GPIO_InitStructure );

}


 /**
  * @brief  LCD  FSMC 模式配置
  * @param  无
  * @retval 无
  */
static void ILI9806G_FSMC_Config ( void )
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 	
	
	/* 使能FSMC时钟*/
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);

	//地址建立时间（ADDSET）为1个HCLK 5/168M=30ns
	readWriteTiming.FSMC_AddressSetupTime      = 0x04;	 //地址建立时间
	//数据保持时间（DATAST）+ 1个HCLK = 5/168M=30ns	
	readWriteTiming.FSMC_DataSetupTime         = 0x04;	 //数据建立时间
	//选择控制的模式
	//模式B,异步NOR FLASH模式，与ILI9806G的8080时序匹配
	readWriteTiming.FSMC_AccessMode            = FSMC_AccessMode_B;	
	
	/*以下配置与模式B无关*/
	//地址保持时间（ADDHLD）模式A未用到
	readWriteTiming.FSMC_AddressHoldTime       = 0x00;	 //地址保持时间
	//设置总线转换周期，仅用于复用模式的NOR操作
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	//设置时钟分频，仅用于同步类型的存储器
	readWriteTiming.FSMC_CLKDivision           = 0x00;
	//数据保持时间，仅用于同步型的NOR	
	readWriteTiming.FSMC_DataLatency           = 0x00;	

	
	FSMC_NORSRAMInitStructure.FSMC_Bank                  = FSMC_Bank1_NORSRAMx;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &readWriteTiming;  
	
	FSMC_NORSRAMInit ( & FSMC_NORSRAMInitStructure ); 
	
	
	/* 使能 FSMC_Bank1_NORSRAM3 */
	FSMC_NORSRAMCmd ( FSMC_Bank1_NORSRAMx, ENABLE );  
		
		
}


/**
 * @brief  初始化ILI9806G寄存器
 * @param  无
 * @retval 无
 */
static void ILI9806G_REG_Config ( void )
{	
		/* EXTC Command Set enable register */
		DEBUG_DELAY  ();
		ILI9806G_Write_Cmd ( 0xFF  );
		ILI9806G_Write_Data ( 0xFF  );
		ILI9806G_Write_Data ( 0x98  );
		ILI9806G_Write_Data ( 0x06  );

		/* GIP 1(BCh)  */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd(0xBC);
		ILI9806G_Write_Data(0x01); 
		ILI9806G_Write_Data(0x0E); 
		ILI9806G_Write_Data(0x61); 
		ILI9806G_Write_Data(0xFB); 
		ILI9806G_Write_Data(0x10); 
		ILI9806G_Write_Data(0x10); 
		ILI9806G_Write_Data(0x0B); 
		ILI9806G_Write_Data(0x0F); 
		ILI9806G_Write_Data(0x2E); 
		ILI9806G_Write_Data(0x73); 
		ILI9806G_Write_Data(0xFF); 
		ILI9806G_Write_Data(0xFF); 
		ILI9806G_Write_Data(0x0E); 
		ILI9806G_Write_Data(0x0E); 
		ILI9806G_Write_Data(0x00); 
		ILI9806G_Write_Data(0x03); 
		ILI9806G_Write_Data(0x66); 
		ILI9806G_Write_Data(0x63); 
		ILI9806G_Write_Data(0x01); 
		ILI9806G_Write_Data(0x00); 
		ILI9806G_Write_Data(0x00);

		/* GIP 2 (BDh) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd(0xBD);
		ILI9806G_Write_Data(0x01); 
		ILI9806G_Write_Data(0x23); 
		ILI9806G_Write_Data(0x45); 
		ILI9806G_Write_Data(0x67); 
		ILI9806G_Write_Data(0x01); 
		ILI9806G_Write_Data(0x23); 
		ILI9806G_Write_Data(0x45); 
		ILI9806G_Write_Data(0x67); 

		/* GIP 3 (BEh) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd(0xBE);
		ILI9806G_Write_Data(0x00); 
		ILI9806G_Write_Data(0x21); 
		ILI9806G_Write_Data(0xAB); 
		ILI9806G_Write_Data(0x60); 
		ILI9806G_Write_Data(0x22); 
		ILI9806G_Write_Data(0x22); 
		ILI9806G_Write_Data(0x22); 
		ILI9806G_Write_Data(0x22); 
		ILI9806G_Write_Data(0x22); 

		/* Vcom  (C7h) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xC7 );
		ILI9806G_Write_Data ( 0x6F );

		/* EN_volt_reg (EDh)*/
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xED );
		ILI9806G_Write_Data ( 0x7F );
		ILI9806G_Write_Data ( 0x0F );
		ILI9806G_Write_Data ( 0x00 );

		/* Power Control 1 (C0h) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xC0 );
		ILI9806G_Write_Data ( 0x37 );
		ILI9806G_Write_Data ( 0x0B );
		ILI9806G_Write_Data ( 0x0A );

		/* LVGL (FCh) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xFC );
		ILI9806G_Write_Data ( 0x0A );

		/* Engineering Setting (DFh) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xDF );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x20 );

		/* DVDD Voltage Setting(F3h) */
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0xF3 );
		ILI9806G_Write_Data ( 0x74 );

		/* Display Inversion Control (B4h) */
		ILI9806G_Write_Cmd ( 0xB4 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x00 );

		/* 480x854 (F7h)  */
		ILI9806G_Write_Cmd ( 0xF7 );
		ILI9806G_Write_Data ( 0x89 );

		/* Frame Rate (B1h) */
		ILI9806G_Write_Cmd ( 0xB1 );
		ILI9806G_Write_Data ( 0x00 );
		ILI9806G_Write_Data ( 0x12 );
		ILI9806G_Write_Data ( 0x10 );

		/* Panel Timing Control (F2h) */
		ILI9806G_Write_Cmd ( 0xF2 );
		ILI9806G_Write_Data ( 0x80 );
		ILI9806G_Write_Data ( 0x5B );
		ILI9806G_Write_Data ( 0x40 );
		ILI9806G_Write_Data ( 0x28 );

		DEBUG_DELAY ();

		/* Power Control 2 (C1h) */
		ILI9806G_Write_Cmd ( 0xC1 ); 
		ILI9806G_Write_Data ( 0x17 );
		ILI9806G_Write_Data ( 0x7D );
		ILI9806G_Write_Data ( 0x7A );
		ILI9806G_Write_Data ( 0x20 );

		DEBUG_DELAY ();

		ILI9806G_Write_Cmd(0xE0); 
		ILI9806G_Write_Data(0x00); //P1 
		ILI9806G_Write_Data(0x11); //P2 
		ILI9806G_Write_Data(0x1C); //P3 
		ILI9806G_Write_Data(0x0E); //P4 
		ILI9806G_Write_Data(0x0F); //P5 
		ILI9806G_Write_Data(0x0C); //P6 
		ILI9806G_Write_Data(0xC7); //P7 
		ILI9806G_Write_Data(0x06); //P8 
		ILI9806G_Write_Data(0x06); //P9 
		ILI9806G_Write_Data(0x0A); //P10 
		ILI9806G_Write_Data(0x10); //P11 
		ILI9806G_Write_Data(0x12); //P12 
		ILI9806G_Write_Data(0x0A); //P13 
		ILI9806G_Write_Data(0x10); //P14 
		ILI9806G_Write_Data(0x02); //P15 
		ILI9806G_Write_Data(0x00); //P16 

		DEBUG_DELAY ();

		ILI9806G_Write_Cmd(0xE1); 
		ILI9806G_Write_Data(0x00); //P1 
		ILI9806G_Write_Data(0x12); //P2 
		ILI9806G_Write_Data(0x18); //P3 
		ILI9806G_Write_Data(0x0C); //P4 
		ILI9806G_Write_Data(0x0F); //P5 
		ILI9806G_Write_Data(0x0A); //P6 
		ILI9806G_Write_Data(0x77); //P7 
		ILI9806G_Write_Data(0x06); //P8 
		ILI9806G_Write_Data(0x07); //P9 
		ILI9806G_Write_Data(0x0A); //P10 
		ILI9806G_Write_Data(0x0E); //P11 
		ILI9806G_Write_Data(0x0B); //P12 
		ILI9806G_Write_Data(0x10); //P13 
		ILI9806G_Write_Data(0x1D); //P14 
		ILI9806G_Write_Data(0x17); //P15 
		ILI9806G_Write_Data(0x00); //P16  

		/* Tearing Effect ON (35h)  */
		ILI9806G_Write_Cmd ( 0x35 );
		ILI9806G_Write_Data ( 0x00 );

		ILI9806G_Write_Cmd ( 0x3A );
		ILI9806G_Write_Data ( 0x55 );

		ILI9806G_Write_Cmd ( 0x11 );
		DEBUG_DELAY ();
		ILI9806G_Write_Cmd ( 0x29 );
	
}


/**
 * @brief  ILI9806G初始化函数，如果要用到lcd，一定要调用这个函数
 * @param  无
 * @retval 无
 */
void ILI9806G_Init ( void )
{
	ILI9806G_GPIO_Config ();
	ILI9806G_FSMC_Config ();


	ILI9806G_Rst ();
	ILI9806G_REG_Config ();

	//设置默认扫描方向，其中 6 模式为大部分液晶例程的默认显示方向  
	ILI9806G_GramScan(LCD_SCAN_MODE);

	ILI9806G_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏，显示全黑 */
	ILI9806G_BackLed_Control ( ENABLE );      //点亮LCD背光灯
}


/**
 * @brief  ILI9806G背光LED控制
 * @param  enumState ：决定是否使能背光LED
  *   该参数为以下值之一：
  *     @arg ENABLE :使能背光LED
  *     @arg DISABLE :禁用背光LED
 * @retval 无
 */
void ILI9806G_BackLed_Control ( FunctionalState enumState )
{
	if ( enumState )
		 GPIO_SetBits( ILI9806G_BK_PORT, ILI9806G_BK_PIN );	
	else
		 GPIO_ResetBits( ILI9806G_BK_PORT, ILI9806G_BK_PIN );
		
}



/**
 * @brief  ILI9806G 软件复位
 * @param  无
 * @retval 无
 */
void ILI9806G_Rst ( void )
{			
	GPIO_ResetBits ( ILI9806G_RST_PORT, ILI9806G_RST_PIN );	 //低电平复位

	ILI9806G_Delay ( 0xAFF ); 					   

	GPIO_SetBits ( ILI9806G_RST_PORT, ILI9806G_RST_PIN );		 	 

	ILI9806G_Delay ( 0xAFF ); 	
	
}




/**
 * @brief  设置ILI9806G的GRAM的扫描方向 
 * @param  ucOption ：选择GRAM的扫描方向 
 *     @arg 0-7 :参数可选值为0-7这八个方向
 *
 *	！！！其中0、3、5、6 模式适合从左至右显示文字，
 *				不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果			
 *		
 *	其中0、2、4、6 模式的X方向像素为480，Y方向像素为854
 *	其中1、3、5、7 模式下X方向像素为854，Y方向像素为480
 *
 *	其中 6 模式为大部分液晶例程的默认显示方向
 *	其中 3 模式为摄像头例程使用的方向
 *	其中 0 模式为BMP图片显示例程使用的方向
 *
 * @retval 无
 * @note  坐标图例：A表示向上，V表示向下，<表示向左，>表示向右
					X表示X轴，Y表示Y轴

------------------------------------------------------------
模式0：				.		模式1：		.	模式2：			.	模式3：					
					A		.					A		.		A					.		A									
					|		.					|		.		|					.		|							
					Y		.					X		.		Y					.		X					
					0		.					1		.		2					.		3					
	<--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->	
------------------------------------------------------------	
模式4：				.	模式5：			.	模式6：			.	模式7：					
	<--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->	
					4		.					5		.		6					.		7	
					Y		.					X		.		Y					.		X						
					|		.					|		.		|					.		|							
					V		.					V		.		V					.		V		
---------------------------------------------------------				
											 LCD屏示例
								|-----------------|
								|			秉火Logo		|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|-----------------|
								屏幕正面（宽480，高854）

 *******************************************************/
void ILI9806G_GramScan ( uint8_t ucOption )
{	
	//参数检查，只可输入0-7
	if(ucOption >7 )
		return;
	
	//根据模式更新LCD_SCAN_MODE的值，主要用于触摸屏选择计算参数
	LCD_SCAN_MODE = ucOption;
	
	//根据模式更新XY方向的像素宽度
	if(ucOption%2 == 0)	
	{
		//0 2 4 6模式下X方向像素宽度为480，Y方向为854
		LCD_X_LENGTH = ILI9806G_LESS_PIXEL;
		LCD_Y_LENGTH =	ILI9806G_MORE_PIXEL;
	}
	else				
	{
		//1 3 5 7模式下X方向像素宽度为854，Y方向为480
		LCD_X_LENGTH = ILI9806G_MORE_PIXEL;
		LCD_Y_LENGTH =	ILI9806G_LESS_PIXEL; 
	}

	//0x36命令参数的高3位可用于设置GRAM扫描方向	
	ILI9806G_Write_Cmd ( 0x36 ); 
	ILI9806G_Write_Data (0x00 | (ucOption<<5));//根据ucOption的值设置LCD参数，共0-7种模式
	ILI9806G_Write_Cmd ( CMD_SetCoordinateX ); 
	ILI9806G_Write_Data ( 0x00 );		/* x 起始坐标高8位 */
	ILI9806G_Write_Data ( 0x00 );		/* x 起始坐标低8位 */
	ILI9806G_Write_Data ( ((LCD_X_LENGTH-1)>>8)&0xFF ); /* x 结束坐标高8位 */	
	ILI9806G_Write_Data ( (LCD_X_LENGTH-1)&0xFF );				/* x 结束坐标低8位 */

	ILI9806G_Write_Cmd ( CMD_SetCoordinateY ); 
	ILI9806G_Write_Data ( 0x00 );		/* y 起始坐标高8位 */
	ILI9806G_Write_Data ( 0x00 );		/* y 起始坐标低8位 */
	ILI9806G_Write_Data ( ((LCD_Y_LENGTH-1)>>8)&0xFF );	/* y 结束坐标高8位 */	 
	ILI9806G_Write_Data ( (LCD_Y_LENGTH-1)&0xFF );				/* y 结束坐标低8位 */

	/* write gram start */
	ILI9806G_Write_Cmd ( CMD_SetPixel );	
}



/**
 * @brief  在ILI9806G显示器上开辟一个窗口
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @retval 无
 */
void ILI9806G_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{	
	ILI9806G_Write_Cmd ( CMD_SetCoordinateX ); 				 /* 设置X坐标 */
	ILI9806G_Write_Data ( usX >> 8  );	 /* 先高8位，然后低8位 */
	ILI9806G_Write_Data ( usX & 0xff  );	 /* 设置起始点和结束点*/
	ILI9806G_Write_Data ( ( usX + usWidth - 1 ) >> 8  );
	ILI9806G_Write_Data ( ( usX + usWidth - 1 ) & 0xff  );

	ILI9806G_Write_Cmd ( CMD_SetCoordinateY ); 			     /* 设置Y坐标*/
	ILI9806G_Write_Data ( usY >> 8  );
	ILI9806G_Write_Data ( usY & 0xff  );
	ILI9806G_Write_Data ( ( usY + usHeight - 1 ) >> 8 );
	ILI9806G_Write_Data ( ( usY + usHeight - 1) & 0xff );
	
}


/**
 * @brief  设定ILI9806G的光标坐标
 * @param  usX ：在特定扫描方向下光标的X坐标
 * @param  usY ：在特定扫描方向下光标的Y坐标
 * @retval 无
 */
static void ILI9806G_SetCursor ( uint16_t usX, uint16_t usY )	
{
	ILI9806G_OpenWindow ( usX, usY, 1, 1 );
}


/**
 * @brief  在ILI9806G显示器上以某一颜色填充像素点
 * @param  ulAmout_Point ：要填充颜色的像素点的总数目
 * @param  usColor ：颜色
 * @retval 无
 */
static __inline void ILI9806G_FillColor ( uint32_t ulAmout_Point, uint16_t usColor )
{
	uint32_t i = 0;
	
	
	/* memory write */
	ILI9806G_Write_Cmd ( CMD_SetPixel );	
		
	for ( i = 0; i < ulAmout_Point; i ++ )
		ILI9806G_Write_Data ( usColor );
		
	
}


/**
 * @brief  对ILI9806G显示器的某一窗口以某种颜色进行清屏
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_Clear ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{
	ILI9806G_OpenWindow ( usX, usY, usWidth, usHeight );

	ILI9806G_FillColor ( usWidth * usHeight, CurrentBackColor );		
	
}


/**
 * @brief  对ILI9806G显示器的某一点以某种颜色进行填充
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_SetPointPixel ( uint16_t usX, uint16_t usY )	
{	
	if ( ( usX < LCD_X_LENGTH ) && ( usY < LCD_Y_LENGTH ) )
  {
		ILI9806G_SetCursor ( usX, usY );
		
		ILI9806G_FillColor ( 1, CurrentTextColor );
	}
	
}


/**
 * @brief  读取ILI9806G GRAN 的一个像素数据
 * @param  无
 * @retval 像素数据
 */
static uint16_t ILI9806G_Read_PixelData ( void )	
{	
	uint16_t usR=0, usG=0, usB=0 ;

	
	ILI9806G_Write_Cmd ( 0x2E );   /* 读数据 */
	
	usR = ILI9806G_Read_Data (); 	/*FIRST READ OUT DUMMY DATA*/
	
	usR = ILI9806G_Read_Data ();  	/*READ OUT RED DATA  */
	usB = ILI9806G_Read_Data ();  	/*READ OUT BLUE DATA*/
	usG = ILI9806G_Read_Data ();  	/*READ OUT GREEN DATA*/	
	
  return ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
	
}


/**
 * @brief  获取 ILI9806G 显示器上某一个坐标点的像素数据
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @retval 像素数据
 */
uint16_t ILI9806G_GetPointPixel ( uint16_t usX, uint16_t usY )
{ 
	uint16_t usPixelData;

	
	ILI9806G_SetCursor ( usX, usY );
	
	usPixelData = ILI9806G_Read_PixelData ();
	
	return usPixelData;
	
}


/**
 * @brief  在 ILI9806G 显示器上使用 Bresenham 算法画线段 
 * @param  usX1 ：在特定扫描方向下线段的一个端点X坐标
 * @param  usY1 ：在特定扫描方向下线段的一个端点Y坐标
 * @param  usX2 ：在特定扫描方向下线段的另一个端点X坐标
 * @param  usY2 ：在特定扫描方向下线段的另一个端点Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DrawLine ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2 )
{
	uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y; 	
	
	
	lDelta_X = usX2 - usX1; //计算坐标增量 
	lDelta_Y = usY2 - usY1; 
	
	usX_Current = usX1; 
	usY_Current = usY1; 
	
	
	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; //设置单步方向 
	
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;//垂直线 
	
	else 
  { 
    lIncrease_X = -1;
    lDelta_X = - lDelta_X;
  } 

	
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;//水平线 
	
	else 
  {
    lIncrease_Y = -1;
    lDelta_Y = - lDelta_Y;
  } 

	
	if (  lDelta_X > lDelta_Y )
		lDistance = lDelta_X; //选取基本增量坐标轴 
	
	else 
		lDistance = lDelta_Y; 

	
	for ( us = 0; us <= lDistance + 1; us ++ )//画线输出 
	{  
		ILI9806G_SetPointPixel ( usX_Current, usY_Current );//画点 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if ( lError_X > lDistance ) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  
	
	
}   


/**
 * @brief  在 ILI9806G 显示器上画一个矩形
 * @param  usX_Start ：在特定扫描方向下矩形的起始点X坐标
 * @param  usY_Start ：在特定扫描方向下矩形的起始点Y坐标
 * @param  usWidth：矩形的宽度（单位：像素）
 * @param  usHeight：矩形的高度（单位：像素）
 * @param  ucFilled ：选择是否填充该矩形
  *   该参数为以下值之一：
  *     @arg 0 :空心矩形
  *     @arg 1 :实心矩形 
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DrawRectangle ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled )
{
	if ( ucFilled )
	{
		ILI9806G_OpenWindow ( usX_Start, usY_Start, usWidth, usHeight );
		ILI9806G_FillColor ( usWidth * usHeight ,CurrentTextColor);	
	}
	else
	{
		ILI9806G_DrawLine ( usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start );
		ILI9806G_DrawLine ( usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );
		ILI9806G_DrawLine ( usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1 );
		ILI9806G_DrawLine ( usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1 );		
	}

}


/**
 * @brief  在 ILI9806G 显示器上使用 Bresenham 算法画圆
 * @param  usX_Center ：在特定扫描方向下圆心的X坐标
 * @param  usY_Center ：在特定扫描方向下圆心的Y坐标
 * @param  usRadius：圆的半径（单位：像素）
 * @param  ucFilled ：选择是否填充该圆
  *   该参数为以下值之一：
  *     @arg 0 :空心圆
  *     @arg 1 :实心圆
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DrawCircle ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled )
{
	int16_t sCurrentX, sCurrentY;
	int16_t sError;
	
	
	sCurrentX = 0; sCurrentY = usRadius;	  
	
	sError = 3 - ( usRadius << 1 );     //判断下个点位置的标志
	
	
	while ( sCurrentX <= sCurrentY )
	{
		int16_t sCountY;
		
		
		if ( ucFilled ) 			
			for ( sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++ ) 
			{                      
				ILI9806G_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCountY );           //1，研究对象 
				ILI9806G_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCountY );           //2       
				ILI9806G_SetPointPixel ( usX_Center - sCountY,   usY_Center + sCurrentX );           //3
				ILI9806G_SetPointPixel ( usX_Center - sCountY,   usY_Center - sCurrentX );           //4
				ILI9806G_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCountY );           //5    
        ILI9806G_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCountY );           //6
				ILI9806G_SetPointPixel ( usX_Center + sCountY,   usY_Center - sCurrentX );           //7 	
        ILI9806G_SetPointPixel ( usX_Center + sCountY,   usY_Center + sCurrentX );           //0				
			}
		
		else
		{          
			ILI9806G_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCurrentY );             //1，研究对象
			ILI9806G_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCurrentY );             //2      
			ILI9806G_SetPointPixel ( usX_Center - sCurrentY, usY_Center + sCurrentX );             //3
			ILI9806G_SetPointPixel ( usX_Center - sCurrentY, usY_Center - sCurrentX );             //4
			ILI9806G_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCurrentY );             //5       
			ILI9806G_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCurrentY );             //6
			ILI9806G_SetPointPixel ( usX_Center + sCurrentY, usY_Center - sCurrentX );             //7 
			ILI9806G_SetPointPixel ( usX_Center + sCurrentY, usY_Center + sCurrentX );             //0
    }			
		
		
		sCurrentX ++;

		
		if ( sError < 0 ) 
			sError += 4 * sCurrentX + 6;	  
		
		else
		{
			sError += 10 + 4 * ( sCurrentX - sCurrentY );   
			sCurrentY --;
		} 	
		
		
	}
	
	
}

/**
 * @brief  在 ILI9806G 显示器上显示一个英文字符
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下该点的起始Y坐标
 * @param  cChar ：要显示的英文字符
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DispChar_EN ( uint16_t usX, uint16_t usY, const char cChar )
{
	uint8_t  byteCount, bitCount,fontLength;	
	uint16_t ucRelativePositon;
	uint8_t *Pfont;
	
	//对ascii码表偏移（字模表不包含ASCII表的前32个非图形符号）
	ucRelativePositon = cChar - ' ';
	
	//每个字模的字节数
	fontLength = (LCD_Currentfonts->Width*LCD_Currentfonts->Height)/8;
		
	//字模首地址
	/*ascii码表偏移值乘以每个字模的字节数，求出字模的偏移位置*/
	Pfont = (uint8_t *)&LCD_Currentfonts->table[ucRelativePositon * fontLength];
	
	//设置显示窗口
	ILI9806G_OpenWindow ( usX, usY, LCD_Currentfonts->Width, LCD_Currentfonts->Height);
	
	ILI9806G_Write_Cmd ( CMD_SetPixel );			

	//按字节读取字模数据
	//由于前面直接设置了显示窗口，显示数据会自动换行
	for ( byteCount = 0; byteCount < fontLength; byteCount++ )
	{
			//一位一位处理要显示的颜色
			for ( bitCount = 0; bitCount < 8; bitCount++ )
			{
					if ( Pfont[byteCount] & (0x80>>bitCount) )
						ILI9806G_Write_Data ( CurrentTextColor );			
					else
						ILI9806G_Write_Data ( CurrentBackColor );
			}	
	}	
}


/**
 * @brief  在 ILI9806G 显示器上显示英文字符串
 * @param  line ：在特定扫描方向下字符串的起始Y坐标
  *   本参数可使用宏LINE(0)、LINE(1)等方式指定文字坐标，
  *   宏LINE(x)会根据当前选择的字体来计算Y坐标值。
	*		显示中文且使用LINE宏时，需要把英文字体设置成Font8x16
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DispStringLine_EN (  uint16_t line,  char * pStr )
{
	uint16_t usX = 0;
	
	while ( * pStr != '\0' )
	{
		if ( ( usX - ILI9806G_DispWindow_X_Star + LCD_Currentfonts->Width ) > LCD_X_LENGTH )
		{
			usX = ILI9806G_DispWindow_X_Star;
			line += LCD_Currentfonts->Height;
		}
		
		if ( ( line - ILI9806G_DispWindow_Y_Star + LCD_Currentfonts->Height ) > LCD_Y_LENGTH )
		{
			usX = ILI9806G_DispWindow_X_Star;
			line = ILI9806G_DispWindow_Y_Star;
		}
		
		ILI9806G_DispChar_EN ( usX, line, * pStr);
		
		pStr ++;
		
		usX += LCD_Currentfonts->Width;
		
	}
	
}


/**
 * @brief  在 ILI9806G 显示器上显示英文字符串
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下字符的起始Y坐标
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DispString_EN ( 	uint16_t usX ,uint16_t usY,  char * pStr )
{
	while ( * pStr != '\0' )
	{
		if ( ( usX - ILI9806G_DispWindow_X_Star + LCD_Currentfonts->Width ) > LCD_X_LENGTH )
		{
			usX = ILI9806G_DispWindow_X_Star;
			usY += LCD_Currentfonts->Height;
		}
		
		if ( ( usY - ILI9806G_DispWindow_Y_Star + LCD_Currentfonts->Height ) > LCD_Y_LENGTH )
		{
			usX = ILI9806G_DispWindow_X_Star;
			usY = ILI9806G_DispWindow_Y_Star;
		}
		
		ILI9806G_DispChar_EN ( usX, usY, * pStr);
		
		pStr ++;
		
		usX += LCD_Currentfonts->Width;
		
	}
	
}


/**
 * @brief  在 ILI9806G 显示器上显示英文字符串(沿Y轴方向)
 * @param  usX ：在特定扫描方向下字符的起始X坐标
 * @param  usY ：在特定扫描方向下字符的起始Y坐标
 * @param  pStr ：要显示的英文字符串的首地址
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void ILI9806G_DispString_EN_YDir (	 uint16_t usX,uint16_t usY ,  char * pStr )
{	
	while ( * pStr != '\0' )
	{
		if ( ( usY - ILI9806G_DispWindow_Y_Star + LCD_Currentfonts->Height ) >LCD_Y_LENGTH  )
		{
			usY = ILI9806G_DispWindow_Y_Star;
			usX += LCD_Currentfonts->Width;
		}
		
		if ( ( usX - ILI9806G_DispWindow_X_Star + LCD_Currentfonts->Width ) >  LCD_X_LENGTH)
		{
			usX = ILI9806G_DispWindow_X_Star;
			usY = ILI9806G_DispWindow_Y_Star;
		}
		
		ILI9806G_DispChar_EN ( usX, usY, * pStr);
		
		pStr ++;
		
		usY += LCD_Currentfonts->Height;		
	}	
}


/**
  * @brief  设置英文字体类型
  * @param  fonts: 指定要选择的字体
	*		参数为以下值之一
  * 	@arg：Font24x32;
  * 	@arg：Font16x24;
  * 	@arg：Font8x16;
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  获取当前字体类型
  * @param  None.
  * @retval 返回当前字体类型
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}


/**
  * @brief  设置LCD的前景(字体)及背景颜色,RGB565
  * @param  TextColor: 指定前景(字体)颜色
  * @param  BackColor: 指定背景颜色
  * @retval None
  */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor) 
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief  获取LCD的前景(字体)及背景颜色,RGB565
  * @param  TextColor: 用来存储前景(字体)颜色的指针变量
  * @param  BackColor: 用来存储背景颜色的指针变量
  * @retval None
  */
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  设置LCD的前景(字体)颜色,RGB565
  * @param  Color: 指定前景(字体)颜色 
  * @retval None
  */
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  设置LCD的背景颜色,RGB565
  * @param  Color: 指定背景颜色 
  * @retval None
  */
void LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  清除某行文字
  * @param  Line: 指定要删除的行
  *   本参数可使用宏LINE(0)、LINE(1)等方式指定要删除的行，
  *   宏LINE(x)会根据当前选择的字体来计算Y坐标值，并删除当前字体高度的第x行。
  * @retval None
  */
void ILI9806G_ClearLine(uint16_t Line)
{
  ILI9806G_Clear(0,Line,LCD_X_LENGTH,((sFONT *)LCD_GetFont())->Height);	/* 清屏，显示全黑 */

}
/*********************end of file*************************/



