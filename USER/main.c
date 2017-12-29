/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   TCP Client����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:����  STM32 F407 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "Bsp/led/bsp_led.h" 
#include "Bsp/usart/bsp_debug_usart.h"
#include "Bsp/systick/bsp_SysTick.h"
#include "Bsp/key/bsp_key.h"
#include "lwip/tcp.h"
#include "netconf.h"
#include "tcp_echoclient.h"
#include "stm32f4x7_phy.h"


#include "./adc/bsp_adc.h"
#include "delay.h"
#include "./lcd/bsp_ili9806g_lcd.h"
#include "ff.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t EthLinkStatus;
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
/* Private function prototypes -----------------------------------------------*/
static void TIM3_Config(uint16_t period,uint16_t prescaler);
static void Delay ( __IO uint32_t nCount );
static void mount_filesystem(void);
void mcu_connect_server(void);
void LCD_Display(long double result[3]);
void LCD_Display_InitInfo(char info[100]);
void LCD_Display_RecvInfo(char info[200], int len);
void LCD_Display_Init(void);
void mcu_xmit_ads1256data(void);
void mcu_open_savefile(void);
void mcu_save_ads1256data(void);

void Data_Collection_And_Xmit(int data_num);
/* Private functions ---------------------------------------------------------*/

/******************************/

FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FIL adsfnew[3];													/* �ļ����� */
FRESULT res_sd;                /* �ļ�������� */
UINT fnum;   
FRESULT ads1256_sd;

extern char result[3];
extern uchar a;
extern unsigned long AD_DATA;
extern unsigned int Ad_Result[3];


#define AD_DATA_NUM 100
long double ADSVolutage[AD_DATA_NUM];
long double ReadADSVolutage[AD_DATA_NUM];

static int LCD_LINE = 0;

extern struct tcp_pcb *echoclient_pcb;
/******************************/

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
	//uint8_t flag=0;
	//int i = 0;
	/* ��ʼ��LED */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);	
	LED_GPIO_Config();
	
	/* ��ʼ������ */
	Key_GPIO_Config();
	
	/* ��ʼ�����Դ��ڣ�һ��Ϊ����1 */
	Debug_USART_Config();
	
	ILI9806G_Init ();         //LCD ��ʼ��
	ILI9806G_GramScan ( 6 );
	LCD_Display_Init();
	LCD_Display_InitInfo("Init Adc1256 Gpio Start...");
	Init_ADS_GPIO_NO_SPI();
	LCD_Display_InitInfo("Init Adc1256 Gpio Success...");
	printf("\r\n **************init ads gpio ok********** \r\n");
	//GPIO_SetBits(GPIOB,GPIO_Pin_14);//reset 
	Delay(0x1ffFF);
	//SetADS1256_RESET1;
	//ADS1256_Init();
	LCD_Display_InitInfo("Init Adc1256 Register Start...");
	ADS1256_Init_No_SPI();
	LCD_Display_InitInfo("Init Adc1256 Register Success...");
	printf("\r\n **************init ads ok********** \r\n");
	
	/* ��ʼ��ϵͳ�δ�ʱ�� */	
	SysTick_Init();
	
	TIM3_Config(999,899);//10ms��ʱ��
	
	/* Configure ethernet (GPIOs, clocks, MAC, DMA) */
	LCD_Display_InitInfo("Init Eth Bsp Configure Start...");
  ETH_BSP_Config();	
	LCD_Display_InitInfo("Init Eth Bsp Configure Success...");
  printf("PHY��ʼ������\n");
	
  /* Initilaize the LwIP stack */
	LCD_Display_InitInfo("Init LwIP Start...");
  LwIP_Init();	
	LCD_Display_InitInfo("Init LwIP Success...");
	Delay(0xfffFF);
  //ILI9806G_Init ();         //LCD ��ʼ��
	//ILI9806G_GramScan ( 6 );
	
	printf("ILI9806G_GramScan OK\n");
	
	mount_filesystem();
	LCD_Display_InitInfo("Mount Filesystem Success...");
	Delay(0xffFF);
	mcu_open_savefile();
	LCD_Display_InitInfo("Mcu Open Save File Success...");
  
  /* IP��ַ�Ͷ˿ڿ���netconf.h�ļ��޸ģ�����ʹ��DHCP�����Զ���ȡIP
	(��Ҫ·����֧��)*/
  printf("����IP�Ͷ˿�: %d.%d.%d.%d\n",IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  printf("Զ��IP�Ͷ˿�: %d.%d.%d.%d:%d\n",DEST_IP_ADDR0, DEST_IP_ADDR1,DEST_IP_ADDR2, DEST_IP_ADDR3,DEST_PORT);
  mcu_connect_server();
	//printf("connect server: %d.%d.%d.%d:%d success\n",DEST_IP_ADDR0, DEST_IP_ADDR1,DEST_IP_ADDR2, DEST_IP_ADDR3,DEST_PORT);
	
	/* wait server cmd */
  while(1)
	{
		/* if not connect ,connect server again */
		if(echoclient_pcb == NULL)
		    mcu_connect_server();

    if (ETH_CheckFrameReceived())
    { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);
		
	}
	
}

void Data_Collection_And_Xmit(int data_num)
{
  long ulResult;
	long double ldVolutage[3];
	unsigned char channel;
	static int AD_Count = 0;
	while ( 1 )
	{
				
		channel = POSITIVE_AIN0+NEGTIVE_AINCOM;
		Get_Ads1256_Data_No_SPI(channel);

		ulResult = Ad_Result[0];
		ldVolutage[0] = (long double)ulResult*2.5/16777216*4;
		ADSVolutage[AD_Count++] = ldVolutage[0];
		if(AD_Count == AD_DATA_NUM)
			break;
		
		ulResult = Ad_Result[1];
		ldVolutage[1] = (long double)ulResult*2.5/16777216*4;
		ADSVolutage[AD_Count++] = ldVolutage[1];
		if(AD_Count == AD_DATA_NUM)
			break;
		
		ulResult = Ad_Result[2];
		ldVolutage[2] = (long double)ulResult*2.5/16777216*4;
		ADSVolutage[AD_Count++] = ldVolutage[2];
    if(AD_Count == AD_DATA_NUM)
			break;
		
		printf("\r\n The current AD value = %Lf V , %Lf V , %Lf V  %d\r\n",ldVolutage[0],ldVolutage[1],ldVolutage[2],AD_Count);
    
		/*
    if (ETH_CheckFrameReceived())
    { 
      
      LwIP_Pkt_Handle();
    }
    */
    LwIP_Periodic_Handle(LocalTime);
		
		LCD_Display(ldVolutage);

	}
	mcu_save_ads1256data();
	mcu_xmit_ads1256data();
}

/**
  * @brief  ͨ�ö�ʱ��3�жϳ�ʼ��
  * @param  period : �Զ���װֵ��
  * @param  prescaler : ʱ��Ԥ��Ƶ��
  * @retval ��
  * @note   ��ʱ�����ʱ����㷽��:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=��ʱ������Ƶ��,ΪSystemCoreClock/2=90,��λ:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=prescaler;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=period;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ��ʱ��3�жϷ�����
  * @param  ��
  * @retval ��
  */
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		LocalTime+=10;//10ms����
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


/**
  * @brief  ����ʱ����
  * @param  nCount ����ʱ����ֵ
  * @retval ��
  */	
static void Delay ( __IO uint32_t nCount )
{
  for ( ; nCount != 0; nCount -- );
	
}

static void mount_filesystem(void)
{
   /*  �����ļ�ϵͳ������AD������ */
	printf("\r\n**********mount filesystem***********\r\n");
	res_sd = f_mount(&fs,"0:",1);
	if(res_sd == FR_NO_FILESYSTEM)
	{
		printf("��SD����û���ļ�ϵͳ���������и�ʽ��...\r\n");
    /* ��ʽ�� */
		res_sd=f_mkfs("0:",0,0);							
		
		if(res_sd == FR_OK)
		{
			printf("��SD���ѳɹ���ʽ���ļ�ϵͳ��\r\n");
      /* ��ʽ������ȡ������ */
			res_sd = f_mount(NULL,"0:",1);			
      /* ���¹���	*/			
			res_sd = f_mount(&fs,"0:",1);
		}
		else
		{
			LED_RED; // if failed red power on
			printf("������ʽ��ʧ�ܡ�����\r\n");
		}
	}
  else if(res_sd!=FR_OK)
  {
    printf("����SD�������ļ�ϵͳʧ�ܡ�(%d)\r\n",res_sd);
    printf("��������ԭ��SD����ʼ�����ɹ���\r\n");
  }
  else
  {
		LED_GREEN;
    printf("���ļ�ϵͳ���سɹ������Խ��ж�д����\r\n");
  }
	
}

void mcu_open_savefile()
{
  ads1256_sd = f_open(&adsfnew[0], "0:ADS1256chip1.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if(ads1256_sd == FR_OK)
	{
		  LED_PURPLE;
	    printf("����/����ADS1256chip1.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
	}
	else
	{	
		LED_RED;
		printf("������/�����ļ�ʧ�� ADS1256chip1.txt��\r\n");
	}
	/*
	ads1256_sd = f_open(&adsfnew[1], "0:ADS1256chip2.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if(ads1256_sd == FR_OK)
	{
		  LED_PURPLE;
	    printf("����/����ADS1256chip2.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
	}
	else
	{	
		LED_RED;
		printf("������/�����ļ�ʧ�� ADS1256chip2.txt��\r\n");
	}
	
	ads1256_sd = f_open(&adsfnew[2], "0:ADS1256chip3.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if(ads1256_sd == FR_OK)
	{
		  LED_PURPLE;
	    printf("����/����ADS1256chip3.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
	}
	else
	{	
		LED_RED;
		printf("������/�����ļ�ʧ�� ADS1256chip3.txt��\r\n");
	}
	*/
}


void mcu_save_ads1256data()
{
  res_sd=f_write(&adsfnew[0],ADSVolutage,sizeof(ADSVolutage),&fnum);
	if(res_sd==FR_OK)
  {
      printf("���ļ�д��ɹ���д���ֽ����ݣ�%d\n",fnum);
  }
  else
  {
      printf("�����ļ�д��ʧ�ܣ�(%d)\n",res_sd);
  }    
		/* ���ٶ�д���ر��ļ� */
	LED_GREEN;
  f_close(&adsfnew[0]);
}

void mcu_connect_server()
{
	  if (EthLinkStatus == 0){
			  printf("mcu_connect_server\n");
        tcp_echoclient_connect();
		}
		Delay(0x1ffFF);
		
}

void mcu_xmit_ads1256data()
{
  int i= 0;  
	int flag = 0;
	res_sd = f_open(&adsfnew[0], "0:ADS1256chip1.txt", FA_OPEN_EXISTING | FA_READ); 
  
  if(res_sd == FR_OK)
	{
		LED_GREEN;
		printf("�����ļ��ɹ���\r\n");
		res_sd = f_read(&adsfnew[0], ReadADSVolutage, sizeof(ReadADSVolutage), &fnum); 
    if(res_sd==FR_OK)
    {
      printf("���ļ���ȡ�ɹ�,�����ֽ����ݣ�%d\r\n",fnum);
			for(i=0; i < AD_DATA_NUM; i++)
			{
			    printf("�������ֽ����ݣ� i=%d  %Lf\r\n",i,ReadADSVolutage[i]);
			}
    }
    else
    {
      printf("�����ļ���ȡʧ�ܣ�(%d)\n",res_sd);
    }		
	}
	else
	{
		LED_RED;
		printf("�������ļ�ʧ�ܡ�\r\n");
	}
	/* ���ٶ�д���ر��ļ� */
	f_close(&adsfnew[0]);
	
	for(i = 0; i < AD_DATA_NUM; i++)
	{
	    if(tcp_xmit_ads1256data(echoclient_pcb,ReadADSVolutage[i]) == ERR_OK){
				
				printf("xmit ads data i = %d,  %Lf V success \n",i,ReadADSVolutage[i]);
			}
			else{
				flag = 1;
				//LCD_Display_InitInfo("Xmit Adc Data Failed");
				printf("xmit ads data i = %d,  %Lf V failed\n ",i,ReadADSVolutage[i]);
			}
			
	}
  if(flag)
    LCD_Display_RecvInfo("Xmit Adc Data Failed",10);
   else
    LCD_Display_RecvInfo("Xmit Adc Data Success",10);
	 
	 tcp_echoclient_disconnect();
}

/*���ڲ��Ը���Һ���ĺ���*/

void LCD_Display(long double result[3])
{
	

	static uint8_t testCNT = 0;	
	char dispBuff[100] = "ADC1256 data:";
	
	testCNT++;	
	
	//LCD_SetFont(&Font16x32);
	//LCD_SetColors(RED,BLACK);

  //ILI9806G_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	
  /*
  ILI9806G_DispStringLine_EN(LINE(0),"BH 4.5 inch LCD");
  ILI9806G_DispStringLine_EN(LINE(2),"resolution:480x854px");
  ILI9806G_DispStringLine_EN(LINE(3),"LCD driver:ILI9806G");
  ILI9806G_DispStringLine_EN(LINE(4),"Touch driver:GT5688");
	*/
	
	ILI9806G_DispStringLine_EN(LINE(LCD_LINE),dispBuff);
	sprintf(dispBuff, "chip1:%Lf", result[0]);
	ILI9806G_DispStringLine_EN(LINE(LCD_LINE+1),dispBuff);
	sprintf(dispBuff, "chip2:%Lf", result[1]);
	ILI9806G_DispStringLine_EN(LINE(LCD_LINE+2),dispBuff);
	sprintf(dispBuff, "chip3:%Lf", result[2]);
	ILI9806G_DispStringLine_EN(LINE(LCD_LINE+3),dispBuff);
  
}

void LCD_Display_Init(void)
{
  LCD_SetFont(&Font16x32);
	LCD_SetColors(RED,BLACK);

  ILI9806G_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	

  ILI9806G_DispStringLine_EN(LINE(LCD_LINE++),"9_ch_24bit system start init....");
}

void LCD_Display_InitInfo(char info[100])
{	
	
  ILI9806G_DispStringLine_EN(LINE(LCD_LINE++),info);
}

void LCD_Display_RecvInfo(char info[200], int len)
{	
  ILI9806G_DispStringLine_EN(LINE((LCD_LINE++)+4),info);
}

