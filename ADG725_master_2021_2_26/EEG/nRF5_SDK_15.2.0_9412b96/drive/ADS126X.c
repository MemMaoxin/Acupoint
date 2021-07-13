#include <string.h>
#include "nrf_drv_common.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "ADS126X.h"
#include "boards.h"
#include "nrf_delay.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */


static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static uint8_t    spi_tx_buf[256];   /**< TX buffer. */
static uint8_t    spi_rx_buf[256];   /**< RX buffer. */

//初始化引脚和中断函数等
void hal_spi_init(void)
{
	  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;  
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
	  spi_config.frequency = NRF_DRV_SPI_FREQ_4M;		//时钟频率为2MHz
		spi_config.mode = NRF_DRV_SPI_MODE_1;					//CPOL=0, CPHA=1,active high and sampling at the trailing edge
    
		APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));
}

void hal_spi_uninit(void)
{
	nrf_drv_spi_uninit(&spi);
}

//初始化芯片
uint8_t ADC32_Init(void)
{
	uint8_t testFlag=0x00;
	
  nrf_delay_ms(40); 												   //等待电源和内部参考信号稳定
	SpiADC32_WriteOneByte(ADC32_Cmd_RESET);		   //重置ADS1263	
	nrf_delay_ms(9);
	
	//设置寄存器值
	
	if(true==ADC32_WriteRegister(ADC32_Adr_POWER,ADC32_POWER_VBIASInRef)) //设置电源属性
		testFlag |= 0x80;			//第1位置1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_INTERFACE,ADC32_INTERFACE_NoStatusCheckSum)) //设置数据格式
		testFlag |= 0x40;			//第2位置1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE0,ADC32_MODE0_PulDe0)) //设置采样模式
		testFlag |= 0x20;			//第3位置1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE1,ADC32_MODE1_Sinc1NoBias))	//设置数字滤波模式，和输入端偏置
		testFlag |= 0x10;			//第4位置1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE2,ADC32_MODE2_Gain32Rate1200)) //设置放大倍数、采样速率
		testFlag |= 0x08;			//第5位置1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_INPMUX,ADC32_INPMUX_AIN0AIN1)) //设置信号输入
		testFlag |= 0x04;			//第6位置0		
	
	testFlag |=0x03;    //余下三位置1
	nrf_delay_ms(40);
	
	return testFlag;
}

//读取芯片ID
uint8_t ADC32_ReadID(void)
{
	uint8_t id;
	//读取ID
	id=ADC32_ReadRegister(ADC32_Adr_ID);
	return id;
}

/******************
写寄存器
输入：adress 地址, value 要写入的寄存器的值
输出：成功为ture，失败返回false
***********/
bool ADC32_WriteRegister(uint8_t adress,uint8_t value)
{
	uint8_t len=3;
	
	adress &= 0x1F;//前三位置0
	adress |= 0x40;//第二位置1
	
	spi_tx_buf[0]=adress;
	spi_tx_buf[1]=0x00;		//写一个寄存器
	spi_tx_buf[2]=value;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	nrf_delay_us(10);
	
	//校验寄存器
	if(value==ADC32_ReadRegister(adress))  return true;
	else return false;
}

/******************
读寄存器
adress 地址，返回寄存器的值
***********/
uint8_t ADC32_ReadRegister(uint8_t adress)
{
	uint8_t len=2;
	
	adress &= 0x3F;//前两位置0
	adress |= 0x20;//第三位置1
	spi_tx_buf[0]=adress;
	spi_tx_buf[1]=0x00;
	//发送读命令
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	
	//获取寄存器值
	len=1;
	spi_tx_buf[0]=0xFF;
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	
	nrf_delay_us(10);
	
	return spi_rx_buf[0];
}

/*****************************************************************************
** 描  述：写入一个字节
** 入  参：Dat：待写入的数据
** 返回值：无
******************************************************************************/
void SpiADC32_WriteOneByte(uint8_t Dat)
{
    uint8_t len = 1;
	
    spi_tx_buf[0] = Dat;
	  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
}

/******************
采集单次转换数据
输入：读取数据输出值
***********/
void ADC32_PulseConversion(uint8_t *Data)
{
	//启动转换
	ADC32_StartConversion();
	while(nrf_gpio_pin_read(DRDY)==1);	//等待数据转换完成，DRDY拉低
	//nrf_delay_us(10);
	//读取数据，如果校验位有误则重新读数
	ADC32_ReadConversionDataCRC(Data);
}

/******************
开始ADC1的转换
***********/
void ADC32_StartConversion(void)
{
	//启动转换
	SpiADC32_WriteOneByte(ADC32_Cmd_START1);
	nrf_delay_us(5);
}

/******************
停止ADC1的转换
***********/
void ADC32_StopConversion(void)
{
	//停止转换
	SpiADC32_WriteOneByte(ADC32_Cmd_STOP1);
	nrf_delay_us(5);
}

/******************
读取转换结果，带校验位
输入：读取数据输出值
***********/
void ADC32_ReadConversionDataCRC(uint8_t *Data)
{
	uint8_t len=6;
	bool crcFalg=false;
	uint8_t checksumByte;
	uint16_t sum;
	uint8_t cntV=0;//读数验证
	uint8_t cntMax=3;//最多读几次
	//读取数据，如果校验位有误则重新读数
	
	while(crcFalg==false)
	{
		spi_tx_buf[0]=ADC32_Cmd_RDATA1;
		spi_tx_buf[1] = 0xFF;
		spi_tx_buf[2] = 0xFF;
		spi_tx_buf[3] = 0xFF;
		spi_tx_buf[4] = 0xFF;
	  spi_tx_buf[5] = 0xFF;
		
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, Data, len));
	
		sum=Data[1]+Data[2]+Data[3]+Data[4]+0x9B;
		checksumByte=sum;
		cntV++;
		//检查计算是否正确
//		for(int i=1;i<5;i++) printf("%x",Data[i]);
//	  printf(" checksum:%x,%x\r\n",checksumByte,Data[5]);
	
		if(checksumByte==Data[5] || cntV>=cntMax)  
			crcFalg=true;
		
	}
}

/******************
读取转换结果，无校验位
输入：读取数据输出值
***********/

void ADC32_ReadConversionData(uint8_t *Data)
{
		uint8_t len=6;
//		uint8_t checksumByte;
//		uint16_t sum;
	//	SpiADC32_WriteOneByte(ADC32_Cmd_RDATA1);
		spi_tx_buf[0]=ADC32_Cmd_RDATA1;
		spi_tx_buf[1] = 0xFF;
		spi_tx_buf[2] = 0xFF;
		spi_tx_buf[3] = 0xFF;
		spi_tx_buf[4] = 0xFF;
	  spi_tx_buf[5] = 0xFF;
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, Data, len));
	//  nrf_drv_spi_transfer(&spi,ff_buf, len, Data, len);
	//		sum=Data[1]+Data[2]+Data[3]+Data[4]+0x9B;
	//	  checksumByte=sum;

		//检查计算是否正确
		 //printf("%x",Data[i]);
		 //printf(" checksum:%x,%x\r\n",checksumByte,Data[5]);
}


