#include <stdint.h>

//DRDY pin
#define DRDY			8//14//24//				
#define SPI_SS_PIN 2
#define SPI_MISO_PIN 6
#define SPI_MOSI_PIN 5
#define SPI_SCK_PIN 3  

//command
#define ADC32_Cmd_NOP 						((uint8_t) 	(0x00))	//No operation
#define ADC32_Cmd_RESET 					((uint8_t)	(0x06)) //Reset the ADC
#define ADC32_Cmd_START1 					((uint8_t)	(0x08)) //Start ADC1 conversions
#define ADC32_Cmd_STOP1 					((uint8_t) 	(0x0A)) //Stop ADC1 conversions
#define ADC32_Cmd_RDATA1 					((uint8_t) 	(0x12)) //Read ADC1 data

//adress
#define ADC32_Adr_ID 				 	((uint8_t)	(0x00)) //ID 
#define ADC32_Adr_POWER			  ((uint8_t)  (0x01)) 
#define ADC32_Adr_INTERFACE   ((uint8_t)	(0x02)) //conversion data format
#define ADC32_Adr_MODE0       ((uint8_t)	(0x03))
#define ADC32_Adr_MODE1       ((uint8_t)	(0x04))
#define ADC32_Adr_MODE2       ((uint8_t)	(0x05))
#define ADC32_Adr_INPMUX      ((uint8_t)	(0x06))
#define ADC32_Adr_REFMUX      ((uint8_t)	(0x0F))

//value
#define ADC32_POWER_VBIASInRef    					((uint8_t) (0x13)) //enable VBIAS output through AINCOM, internal reference enable

#define ADC32_INTERFACE_NoStatusNoCrc				((uint8_t) (0x00)) //no status, no crc
#define ADC32_INTERFACE_NoStatusCheckSum  	((uint8_t) (0x01)) //no status, checksum verify

#define ADC32_MODE0_ConChopDe69 						((uint8_t) (0x14)) //Continous conversion,chop mode,delay 69us
#define ADC32_MODE0_ConDe0  	      				((uint8_t) (0x00)) //Pulse conversion,no chop or idac,delay 0
#define ADC32_MODE0_PulChopDe1100						((uint8_t) (0x58)) //Pulse conversion,chop mode,delay 1.1ms
#define ADC32_MODE0_PulDe0  	      				((uint8_t) (0x40)) //Pulse conversion,no chop or idac,delay 0

#define ADC32_MODE1_FIRNoBias		         		((uint8_t) (0x80))	//FIR filter, no bias current or resister
#define ADC32_MODE1_Sinc1NoBias		         	((uint8_t) (0x00))	//Sinc1 filter, no bias current or resister
#define ADC32_MODE1_Sinc3NoBias		         	((uint8_t) (0x40))	//Sinc3 filter, no bias current or resister
#define ADC32_MODE1_Sinc4NoBias		         	((uint8_t) (0x60))	//Sinc3 filter, no bias current or resister

#define ADC32_MODE2_PassRate400	          	((uint8_t) (0x88))	//PGA enabled,Gain16,5SPS
#define ADC32_MODE2_Gain1Rate5	          	((uint8_t) (0x01))	//PGA enabled,Gain16,5SPS
#define ADC32_MODE2_Gain32Rate5	          	((uint8_t) (0x51))	//PGA enabled,Gain16,5SPS
#define ADC32_MODE2_Gain1Rate400	          ((uint8_t) (0x08))	//PGA enabled,Gain1,400SPS
#define ADC32_MODE2_Gain32Rate400	          ((uint8_t) (0x58))	//PGA enabled,Gain32,400SPS
#define ADC32_MODE2_Gain16Rate400	          ((uint8_t) (0x48))	//PGA enabled,Gain32,400SPS
#define ADC32_MODE2_PassRate1200	          ((uint8_t) (0x89))	//PGA enabled,Gain16,5SPS
#define ADC32_MODE2_Gain32Rate1200	        ((uint8_t) (0x59))	//PGA enabled,Gain16,5SPS

#define ADC32_INPMUX_AIN0AIN1               ((uint8_t) (0x01))	//channel selection:AIN0,AIN1
#define ADC32_INPMUX_AIN0COM                ((uint8_t) (0x0A))	//channel selection:AIN0,AINCOM


void hal_spi_init(void);
bool ADC32_WriteRegister(uint8_t adress,uint8_t value);
uint8_t ADC32_ReadRegister(uint8_t adress);
uint8_t ADC32_Init(void);
uint8_t ADC32_ReadID(void);
void SpiADC32_WriteOneByte(uint8_t Dat);
void ADC32_PulseConversion(uint8_t *Data);
void ADC32_ReadConversionData(uint8_t *Data);
void ADC32_StopConversion(void);
void ADC32_StartConversion(void);
void ADC32_ReadConversionDataCRC(uint8_t *Data);
void ADC32_ReadConversionData(uint8_t *Data);
void hal_spi_uninit(void);
