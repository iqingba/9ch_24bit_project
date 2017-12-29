#include "stm32f4xx.h"
#include "./adc/bsp_adc.h"
#include "./delay/delay.h"
#include "./sys/sys.c"
#include "./lcd/bsp_ili9806g_lcd.h"
#include "./usart/bsp_debug_usart.h"
uchar dat,a,ds[11],i,ch_flag=0;
unsigned long AD_DATA;
char result[3];
unsigned int Ad_Result[3] = {0,0,0};

/*******************************************************************************
 ADS1256_Write_Byte(unsigned char d)

*******************************************************************************/
void ADS1256_Write_Chip1_Byte(unsigned char d)
{
	
	unsigned char i=8;	
	ClrADS1256_CLK1 ;
   	while(i--)
  	{
		if(d&0X80)
	    SetADS1256_IN1  ;
		else  
			ClrADS1256_IN1  ;
    
		delay_us(50); 	  	  
		SetADS1256_CLK1;
	  delay_us(50);
	  ClrADS1256_CLK1 ;
	  delay_us(50);
	  d <<= 1;
		
	}
}

void ADS1256_Write_Chip2_Byte(unsigned char d)
{
	unsigned char i=8;	
	ClrADS1256_CLK2 ;
   	while(i--)
  	{
		if(d&0X80)
	    SetADS1256_IN2  ;
		else  
			ClrADS1256_IN2  ;
      delay_us(50); 	  	  
		  SetADS1256_CLK2;
	  	delay_us(50);
	  	ClrADS1256_CLK2 ;
	  	delay_us(50);
	  	d <<= 1;
	}
}

void ADS1256_Write_Chip3_Byte(unsigned char d)
{
	unsigned char i=8;	
	ClrADS1256_CLK3 ;
   	while(i--)
  	{
		if(d&0X80)
	    SetADS1256_IN3  ;
		else  
			ClrADS1256_IN3  ;
      delay_us(50); 	  	  
		  SetADS1256_CLK3;
	  	delay_us(50);
	  	ClrADS1256_CLK3 ;
	  	delay_us(50);
	  	d <<= 1;
	}
}

/*******************************************************************************
 ADS1256_Read_Byte(void)
 
*******************************************************************************/

char ADS1256_Read_Chip1_Byte(void)
{  
	char i=8,d=0;   
	ClrADS1256_CLK1;	 
  while(i--)
  {
      	d <<=1;	
	  	  SetADS1256_CLK1;  
      	delay_us(50);
      	ClrADS1256_CLK1;   
		    delay_us(50);
		    SetADS1256_DO1;
	  	 if(ADS1256_DO1)
 	    	d |= 0x01;
	  	 else 
 	    	d &= 0xfe;     	   	  	  
	} 
  delay_us(10);		
  return d;

}

char ADS1256_Read_Chip2_Byte(void)
{  
	char i=8,d=0;   
	ClrADS1256_CLK2;	 
  while(i--)
  {
      	d <<=1;	
	  	  SetADS1256_CLK2;  
      	delay_us(50);
      	ClrADS1256_CLK2;   
		    delay_us(50);
		    SetADS1256_DO2;
	  	 if(ADS1256_DO2)
 	    	d |= 0x01;
	  	 else 
 	    	d &= 0xfe;     	   	  	  
	} 
  delay_us(10);		
  return d;

}

char ADS1256_Read_Chip3_Byte(void)
{  
	char i=8,d=0;   
	ClrADS1256_CLK3;	 
  while(i--)
  {
      	d <<=1;	
	  	  SetADS1256_CLK3;  
      	delay_us(50);
      	ClrADS1256_CLK3;   
		    delay_us(50);
		    SetADS1256_DO3;
	  	 if(ADS1256_DO3)
 	    	d |= 0x01;
	  	 else 
 	    	d &= 0xfe;     	   	  	  
	} 
  delay_us(10);		
  return d;

}


/******************************************************************************
 ADS1256_Write_Reg(unsigned char reg_name, unsigned char reg_data)

*******************************************************************************/

void  ADS1256_Write_Chip1_Reg(unsigned char reg_name, unsigned char reg_data)
{
	  ClrADS1256_CS1;
	  
  	while(ADS1256_DRDY1);
	  
	  ADS1256_Write_Chip1_Byte(CMD_WREG|reg_name);
	  
   	ADS1256_Write_Chip1_Byte(0x00);
	
   	ADS1256_Write_Chip1_Byte(reg_data);
	  
	  SetADS1256_CS1;
	 
	
}

void  ADS1256_Write_Chip2_Reg(unsigned char reg_name, unsigned char reg_data)
{
	  ClrADS1256_CS2;
  	while(ADS1256_DRDY2);
	  ADS1256_Write_Chip2_Byte(CMD_WREG|reg_name);
   	ADS1256_Write_Chip2_Byte(0x00);
   	ADS1256_Write_Chip2_Byte(reg_data);
	  SetADS1256_CS2;
	
}

void  ADS1256_Write_Chip3_Reg(unsigned char reg_name, unsigned char reg_data)
{
	  ClrADS1256_CS3;
  	while(ADS1256_DRDY3);
	  ADS1256_Write_Chip3_Byte(CMD_WREG|reg_name);
   	ADS1256_Write_Chip3_Byte(0x00);
   	ADS1256_Write_Chip3_Byte(reg_data);
	  SetADS1256_CS3;
	
}

/*******************************************************************************
 ADS1256_Read_dat(void)

*******************************************************************************/
void ADS1256_Read_dat(void)
{
	
  Ad_Result[0] = Ad_Result[1] = Ad_Result[2] =0;
	ClrADS1256_CS1;
	//while(ADS1256_DRDY==0);
	while(ADS1256_DRDY1==0 && ADS1256_DRDY2==0 && ADS1256_DRDY3==0);	 //DRDY??????AD????
	ADS1256_Write_Chip1_Byte(CMD_SYNC);
	ADS1256_Write_Chip2_Byte(CMD_SYNC);
	ADS1256_Write_Chip3_Byte(CMD_SYNC);
	ADS1256_Write_Chip1_Byte(CMD_WAKEUP);
	ADS1256_Write_Chip2_Byte(CMD_WAKEUP);
	ADS1256_Write_Chip3_Byte(CMD_WAKEUP);
  ADS1256_Write_Chip1_Byte(CMD_RDATA);
	ADS1256_Write_Chip2_Byte(CMD_RDATA);
	ADS1256_Write_Chip3_Byte(CMD_RDATA);
	//delay_ms(70);   //min=50*(1/fCLKIN)=50*(1/7.68MHZ)=6500ns;max=whatever
	//ADS1256_Write_Byte(0xff);
	
  Ad_Result[0] |= (ADS1256_Read_Chip1_Byte() << 16);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[0] |= (ADS1256_Read_Chip1_Byte() << 8);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[0] |= ADS1256_Read_Chip1_Byte();
	
	Ad_Result[1] |= (ADS1256_Read_Chip2_Byte() << 16);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[1] |= (ADS1256_Read_Chip2_Byte() << 8);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[1] |= ADS1256_Read_Chip2_Byte();
	
	Ad_Result[2] |= (ADS1256_Read_Chip3_Byte() << 16);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[2] |= (ADS1256_Read_Chip3_Byte() << 8);
	//ADS1256_Write_Byte(0xff);
  Ad_Result[2] |= ADS1256_Read_Chip3_Byte();
	
	SetADS1256_CS1;
}

/*******************************************************************************
Write_Reg_Mux()
*******************************************************************************/

void Write_Reg_Mux(uchar x)   //
{	 	  
	  ADS1256_Write_Chip1_Reg(REG_MUX,x);//
	  ADS1256_Write_Chip2_Reg(REG_MUX,x);//
	  ADS1256_Write_Chip3_Reg(REG_MUX,x);//
	  ADS1256_Read_dat();
}

unsigned char SPI_WriteByte(unsigned char TxData)
{
  unsigned char RxData=0;
  while(SPI_I2S_GetFlagStatus(ADS1256_SPI,SPI_I2S_FLAG_TXE)==RESET); //                                                   
  SPI_I2S_SendData(ADS1256_SPI,TxData);

   while(SPI_I2S_GetFlagStatus(ADS1256_SPI,SPI_I2S_FLAG_RXNE)==RESET);

   RxData=SPI_I2S_ReceiveData(ADS1256_SPI);
	
	 delay_us(10);
	/*
	printf("\r\n*****************SPI Receive Data: ");
	 for(i = 0; i < 8; i++){
		   if((RxData << i)&(0x80))
           printf("1");
			 else
				   printf("0");
	 }
	 printf("*********************\r\n");
	*/
    return RxData;
} 

//-----------------------------------------------------------------//
//	��    �ܣ�ADS1256 д����
//	��ڲ���: /
//	���ڲ���: /
//	ȫ�ֱ���: /
//	��    ע: ��ADS1256�е�ַΪregaddr�ļĴ���д��һ���ֽ�databyte
//-----------------------------------------------------------------//
void ADS1256WREG(unsigned char regaddr,unsigned char databyte)
{
    ClrADS1256_CS1;
	
	  while(ADS1256_DRDY1);//��ADS1256_DRDYΪ��ʱ����д�Ĵ���
	//��Ĵ���д�����ݵ�ַ
    SPI_WriteByte(ADS1256_CMD_WREG | (regaddr & 0x0F));
    //д�����ݵĸ���n-1
    SPI_WriteByte(0x00);
    //��regaddr��ַָ��ļĴ���д������databyte
    SPI_WriteByte(databyte);
	  SetADS1256_CS1;
}

//��ʼ��ADS1256
void ADS1256_Init(void)
{
	ADS1256WREG(ADS1256_STATUS,0x06);               // ��λ��ǰ��У׼��ʹ�û���
//	ADS1256WREG(ADS1256_MUX,0x08);                  // ��ʼ���˿�A0Ϊ��+����AINCOMλ��-��
	ADS1256WREG(ADS1256_ADCON,0x00);                // �Ŵ���1
	ADS1256WREG(ADS1256_DRATE,DATARATE_30K);  // ����30sps
	ADS1256WREG(ADS1256_IO,0x00);   	
}
//��ȡADֵ
unsigned int ADS1256ReadData()  
{
  //unsigned char i=0;
  unsigned int sum=0;
	//unsigned int r=0;
	ClrADS1256_CS1;

	while(ADS1256_DRDY1);               //��ADS1256_DRDYΪ��ʱ����д�Ĵ��� 
//	ADS1256WREG(ADS1256_MUX,channel);		//����ͨ��
	SPI_WriteByte(ADS1256_CMD_SYNC);
	SPI_WriteByte(ADS1256_CMD_WAKEUP);	               
	SPI_WriteByte(ADS1256_CMD_RDATA);

  sum |= (SPI_WriteByte(0xff) << 16);
	sum |= (SPI_WriteByte(0xff) << 8);
	sum |= SPI_WriteByte(0xff);

	SetADS1256_CS1;
  return sum;
}

/*��ȡADоƬ������*/
unsigned int Get_chx_dat(unsigned char channel)
{   
	 //AD_DATA=Write_Reg_Mux(channel);//�л�ģ�⿪��
	 ADS1256WREG(ADS1256_MUX,channel);
	 return ADS1256ReadData();
}

void Get_Ads1256_Data_No_SPI(unsigned char channel)
{
	  ADS1256_Write_Chip1_Reg(REG_MUX,channel);
	  ADS1256_Write_Chip2_Reg(REG_MUX,channel);
	  ADS1256_Write_Chip3_Reg(REG_MUX,channel);
    ADS1256_Read_dat();//�л�ģ�⿪��
}

/**
  * @brief  SPI_ADS1256��ʼ��
  * @param  ��
  * @retval ��
  */
void SPI_ADS1256_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* ʹ�� ADS1256_SPI ��GPIO ʱ�� */
  /*!< SPI_ADS1256_SPI_CS_GPIO, SPI_ADS1256_SPI_MOSI_GPIO, 
       SPI_ADS1256_SPI_MISO_GPIO,SPI_ADS1256_SPI_SCK_GPIO ʱ��ʹ�� */
  RCC_AHB1PeriphClockCmd (ADS1256_SPI_SCK_GPIO_CLK | ADS1256_SPI_MISO_GPIO_CLK|ADS1256_SPI_MOSI_GPIO_CLK, ENABLE);

  /*!< SPI_ADS1256_SPI ʱ��ʹ�� */
  ADS1256_SPI_CLK_INIT(ADS1256_SPI_CLK, ENABLE);
 
  //�������Ÿ���
  GPIO_PinAFConfig(ADS1256_SPI_SCK_GPIO_PORT,ADS1256_SPI_SCK_PINSOURCE,ADS1256_SPI_SCK_AF); 
	GPIO_PinAFConfig(ADS1256_SPI_MISO_GPIO_PORT,ADS1256_SPI_MISO_PINSOURCE,ADS1256_SPI_MISO_AF); 
	GPIO_PinAFConfig(ADS1256_SPI_MOSI_GPIO_PORT,ADS1256_SPI_MOSI_PINSOURCE,ADS1256_SPI_MOSI_AF); 
  
  /*!< ���� SPI_ADS1256_SPI ����: SCK */
  GPIO_InitStructure.GPIO_Pin = ADS1256_SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
  
  GPIO_Init(ADS1256_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
  
	/*!< ���� SPI_ADS1256_SPI ����: MISO */
  GPIO_InitStructure.GPIO_Pin = ADS1256_SPI_MISO_PIN;
  GPIO_Init(ADS1256_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
	/*!< ���� SPI_ADS1256_SPI ����: MOSI */
  GPIO_InitStructure.GPIO_Pin = ADS1256_SPI_MOSI_PIN;
  GPIO_Init(ADS1256_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);  

  SetADS1256_CS1;

  /* ADS1256_SPI ģʽ���� */
  // ADS1256оƬ ֧��SPIģʽ0��ģʽ3���ݴ�����CPOL CPHA
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(ADS1256_SPI, &SPI_InitStructure);

  /* ʹ�� ADS1256_SPI  */
  SPI_Cmd(ADS1256_SPI, ENABLE);

} 

void Init_ADS_GPIO()
{

  GPIO_InitTypeDef  GPIO_InitStructure;
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��

  //GPIO��ʼ������
	/*
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9| GPIO_Pin_8;//LED2��LED3��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//ʹ��GPIOBʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //DRDY
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ӦDRDY������ģʽ��Ҫ���ó�����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(GPIOD, &GPIO_InitStructure);//GPIOD_3��ӦAD��DRDY�ܽ�
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); //PE2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦCS������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(GPIOE, &GPIO_InitStructure);//GPIOE_2��ӦAD��CS�ܽ�
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); //PG15
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦRESET������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(GPIOG, &GPIO_InitStructure);//GPIOG_15��ӦAD��RESET�ܽ�
	
	SPI_ADS1256_Init();
}


void Init_ADS_GPIO_NO_SPI()
{
  /*��ʼ��chip1��GPIO*/
  GPIO_InitTypeDef  GPIO_InitStructure;
	
  RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_SCLK_RCC, ENABLE);//ʹ��GPIOBʱ��
  GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_SCLK_PIN;//SCLK�Ͷ�ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP1_SCLK_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_DIN_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_DIN_PIN;//DIN��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP1_DIN_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_DOUT_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_DOUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP1_DOUT_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_DRDY_RCC, ENABLE);//ʹ��GPIODʱ��
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_DRDY_PIN; //DRDY
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ӦDRDY������ģʽ��Ҫ���ó�����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP1_DRDY_GPIO_PORT, &GPIO_InitStructure);//GPIOD_3��ӦAD��DRDY�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_CS_RCC, ENABLE); //PE2
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_CS_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦCS������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP1_CS_GPIO_PORT, &GPIO_InitStructure);//GPIOE_2��ӦAD��CS�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP1_RESET_RCC, ENABLE); //PG15
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP1_RESET_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦRESET������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP1_RESET_GPIO_PORT, &GPIO_InitStructure);//GPIOG_15��ӦAD��RESET�ܽ�
	
	
	/*��ʼ��chip2��GPIO*/
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_SCLK_RCC, ENABLE);//ʹ��GPIOBʱ��
  GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_SCLK_PIN;//SCLK�Ͷ�ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP2_SCLK_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_DIN_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_DIN_PIN;//DIN��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP2_DIN_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_DOUT_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_DOUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP2_DOUT_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_DRDY_RCC, ENABLE);//ʹ��GPIODʱ��
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_DRDY_PIN; //DRDY
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ӦDRDY������ģʽ��Ҫ���ó�����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP2_DRDY_GPIO_PORT, &GPIO_InitStructure);//GPIOD_3��ӦAD��DRDY�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_CS_RCC, ENABLE); //PE2
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_CS_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦCS������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP2_CS_GPIO_PORT, &GPIO_InitStructure);//GPIOE_2��ӦAD��CS�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP2_RESET_RCC, ENABLE); //PG15
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP2_RESET_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦRESET������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP2_RESET_GPIO_PORT, &GPIO_InitStructure);//GPIOG_15��ӦAD��RESET�ܽ�
	
	
	/*��ʼ��chip3��GPIO*/
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_SCLK_RCC, ENABLE);//ʹ��GPIOBʱ��
  GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_SCLK_PIN;//SCLK�Ͷ�ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP3_SCLK_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_DIN_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_DIN_PIN;//DIN��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP3_DIN_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_DOUT_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_DOUT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(ADS1256_CHIP3_DOUT_GPIO_PORT, &GPIO_InitStructure);//��ʼ��GPIOB
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_DRDY_RCC, ENABLE);//ʹ��GPIODʱ��
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_DRDY_PIN; //DRDY
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ӦDRDY������ģʽ��Ҫ���ó�����
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP3_DRDY_GPIO_PORT, &GPIO_InitStructure);//GPIOD_3��ӦAD��DRDY�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_CS_RCC, ENABLE); //PE2
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_CS_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦCS������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP3_CS_GPIO_PORT, &GPIO_InitStructure);//GPIOE_2��ӦAD��CS�ܽ�
	
	RCC_AHB1PeriphClockCmd(ADS1256_CHIP3_RESET_RCC, ENABLE); //PG15
	GPIO_InitStructure.GPIO_Pin = ADS1256_CHIP3_RESET_PIN; //
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ӦRESET������ģʽ��Ҫ���ó�����
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����ģʽ
  GPIO_Init(ADS1256_CHIP3_RESET_GPIO_PORT, &GPIO_InitStructure);//GPIOG_15��ӦAD��RESET�ܽ�
	
}


void ADS1256_Init_No_SPI()
{
	SetADS1256_RESET1;
	ClrADS1256_RESET2;
	ClrADS1256_RESET3;
	ClrADS1256_CS1;
	ADS1256_Write_Chip1_Reg(REG_STATUS,0x06);
	ADS1256_Write_Chip1_Reg(REG_ADCON,0x00);
	ADS1256_Write_Chip1_Reg(REG_DRATE,DATARATE_30K);//data rate 30k SPS
	ADS1256_Write_Chip1_Reg(REG_IO,0x00);
	delay_ms(10);
	
	ClrADS1256_RESET1;
	SetADS1256_RESET2;
	ClrADS1256_RESET3;
  ADS1256_Write_Chip2_Reg(REG_STATUS,0x06);
	ADS1256_Write_Chip2_Reg(REG_ADCON,0x00);
	ADS1256_Write_Chip2_Reg(REG_DRATE,DATARATE_30K);//data rate 30k SPS
	ADS1256_Write_Chip2_Reg(REG_IO,0x00);
	delay_ms(10);
	
  ClrADS1256_RESET1;
	ClrADS1256_RESET2;
	SetADS1256_RESET3;
	ADS1256_Write_Chip3_Reg(REG_STATUS,0x06);
	ADS1256_Write_Chip3_Reg(REG_ADCON,0x00);
	ADS1256_Write_Chip3_Reg(REG_DRATE,DATARATE_30K);//data rate 30k SPS
	ADS1256_Write_Chip3_Reg(REG_IO,0x00);
	
	delay_ms(10);
	SetADS1256_RESET1;
	SetADS1256_RESET2;
	SetADS1256_RESET3;
}


