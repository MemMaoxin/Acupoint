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

//��ʼ�����ź��жϺ�����
void hal_spi_init(void)
{
	  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;  
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
	  spi_config.frequency = NRF_DRV_SPI_FREQ_4M;		//ʱ��Ƶ��Ϊ2MHz
		spi_config.mode = NRF_DRV_SPI_MODE_1;					//CPOL=0, CPHA=1,active high and sampling at the trailing edge
    
		APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));
}

void hal_spi_uninit(void)
{
	nrf_drv_spi_uninit(&spi);
}

//��ʼ��оƬ
uint8_t ADC32_Init(void)
{
	uint8_t testFlag=0x00;
	
  nrf_delay_ms(40); 												   //�ȴ���Դ���ڲ��ο��ź��ȶ�
	SpiADC32_WriteOneByte(ADC32_Cmd_RESET);		   //����ADS1263	
	nrf_delay_ms(9);
	
	//���üĴ���ֵ
	
	if(true==ADC32_WriteRegister(ADC32_Adr_POWER,ADC32_POWER_VBIASInRef)) //���õ�Դ����
		testFlag |= 0x80;			//��1λ��1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_INTERFACE,ADC32_INTERFACE_NoStatusCheckSum)) //�������ݸ�ʽ
		testFlag |= 0x40;			//��2λ��1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE0,ADC32_MODE0_PulDe0)) //���ò���ģʽ
		testFlag |= 0x20;			//��3λ��1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE1,ADC32_MODE1_Sinc1NoBias))	//���������˲�ģʽ���������ƫ��
		testFlag |= 0x10;			//��4λ��1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_MODE2,ADC32_MODE2_Gain32Rate1200)) //���÷Ŵ�������������
		testFlag |= 0x08;			//��5λ��1
	
	if(true==ADC32_WriteRegister(ADC32_Adr_INPMUX,ADC32_INPMUX_AIN0AIN1)) //�����ź�����
		testFlag |= 0x04;			//��6λ��0		
	
	testFlag |=0x03;    //������λ��1
	nrf_delay_ms(40);
	
	return testFlag;
}

//��ȡоƬID
uint8_t ADC32_ReadID(void)
{
	uint8_t id;
	//��ȡID
	id=ADC32_ReadRegister(ADC32_Adr_ID);
	return id;
}

/******************
д�Ĵ���
���룺adress ��ַ, value Ҫд��ļĴ�����ֵ
������ɹ�Ϊture��ʧ�ܷ���false
***********/
bool ADC32_WriteRegister(uint8_t adress,uint8_t value)
{
	uint8_t len=3;
	
	adress &= 0x1F;//ǰ��λ��0
	adress |= 0x40;//�ڶ�λ��1
	
	spi_tx_buf[0]=adress;
	spi_tx_buf[1]=0x00;		//дһ���Ĵ���
	spi_tx_buf[2]=value;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	nrf_delay_us(10);
	
	//У��Ĵ���
	if(value==ADC32_ReadRegister(adress))  return true;
	else return false;
}

/******************
���Ĵ���
adress ��ַ�����ؼĴ�����ֵ
***********/
uint8_t ADC32_ReadRegister(uint8_t adress)
{
	uint8_t len=2;
	
	adress &= 0x3F;//ǰ��λ��0
	adress |= 0x20;//����λ��1
	spi_tx_buf[0]=adress;
	spi_tx_buf[1]=0x00;
	//���Ͷ�����
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	
	//��ȡ�Ĵ���ֵ
	len=1;
	spi_tx_buf[0]=0xFF;
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
	
	nrf_delay_us(10);
	
	return spi_rx_buf[0];
}

/*****************************************************************************
** ��  ����д��һ���ֽ�
** ��  �Σ�Dat����д�������
** ����ֵ����
******************************************************************************/
void SpiADC32_WriteOneByte(uint8_t Dat)
{
    uint8_t len = 1;
	
    spi_tx_buf[0] = Dat;
	  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buf, len, spi_rx_buf, len));
}

/******************
�ɼ�����ת������
���룺��ȡ�������ֵ
***********/
void ADC32_PulseConversion(uint8_t *Data)
{
	//����ת��
	ADC32_StartConversion();
	while(nrf_gpio_pin_read(DRDY)==1);	//�ȴ�����ת����ɣ�DRDY����
	//nrf_delay_us(10);
	//��ȡ���ݣ����У��λ���������¶���
	ADC32_ReadConversionDataCRC(Data);
}

/******************
��ʼADC1��ת��
***********/
void ADC32_StartConversion(void)
{
	//����ת��
	SpiADC32_WriteOneByte(ADC32_Cmd_START1);
	nrf_delay_us(5);
}

/******************
ֹͣADC1��ת��
***********/
void ADC32_StopConversion(void)
{
	//ֹͣת��
	SpiADC32_WriteOneByte(ADC32_Cmd_STOP1);
	nrf_delay_us(5);
}

/******************
��ȡת���������У��λ
���룺��ȡ�������ֵ
***********/
void ADC32_ReadConversionDataCRC(uint8_t *Data)
{
	uint8_t len=6;
	bool crcFalg=false;
	uint8_t checksumByte;
	uint16_t sum;
	uint8_t cntV=0;//������֤
	uint8_t cntMax=3;//��������
	//��ȡ���ݣ����У��λ���������¶���
	
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
		//�������Ƿ���ȷ
//		for(int i=1;i<5;i++) printf("%x",Data[i]);
//	  printf(" checksum:%x,%x\r\n",checksumByte,Data[5]);
	
		if(checksumByte==Data[5] || cntV>=cntMax)  
			crcFalg=true;
		
	}
}

/******************
��ȡת���������У��λ
���룺��ȡ�������ֵ
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

		//�������Ƿ���ȷ
		 //printf("%x",Data[i]);
		 //printf(" checksum:%x,%x\r\n",checksumByte,Data[5]);
}


