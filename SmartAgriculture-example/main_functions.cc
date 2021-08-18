#include "hx_drv_tflm.h"
#include "main_functions.h"
#include "settings.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace
{
  bool lora_is_initialized = false;
}

bool read_AT_and_cmp(char *cmp_str)
{
  uint8_t recdata[10] = {};
  memset(recdata, '\0', sizeof(recdata));
  int count = 0;

  while (1)
  {
    if (hx_drv_swuart_single_read(recdata + count) == HX_DRV_LIB_PASS)
    {
      if (*(recdata + count) == 0xa)
      {
        count++;
        break;
      }
      count++;
    }
  }

  if (strcmp((char *)recdata, cmp_str) != 0)
  {
    return false;
  }

  return true;
}

bool send_AT_and_retry(char *senddata, int len, char *ack)
{
  int retry = 3;
  bool valid = false;
  for (int i = 0; i < retry; i++)
  {
    hx_drv_swuart_write((uint8_t *)senddata, len);
    if (read_AT_and_cmp(ack))
    {
      valid = true;
      break;
    }
  }
  return valid;
}

void lora_initial()
{
  if (!lora_is_initialized)
  {

    // init swuart
    hx_drv_swuart_initial(HX_DRV_PGPIO_0, HX_DRV_PGPIO_1, UART_BR_115200);

    // init loar
    if (!send_AT_and_retry((char *)"AT\x0d\x0a", 4, (char *)"+OK\x0d\x0a"))
    {
      hx_drv_uart_print("loar initial fail.");
      return;
    }
    if (!send_AT_and_retry((char *)"AT+ADDRESS=1\x0d\x0a", 14, (char *)"+OK\x0d\x0a"))
    {
      hx_drv_uart_print("loar initial fail.");
      return;
    }
    if (!send_AT_and_retry((char *)"AT+CRFOP=7\x0d\x0a", 12, (char *)"+OK\x0d\x0a"))
    {
      hx_drv_uart_print("loar initial fail.");
      return;
    }
    lora_is_initialized = true;
  }
}

void setup()
{
  hx_drv_share_switch(SHARE_MODE_I2CM);
  // init uart
  hx_drv_uart_initial(UART_BR_115200);
  
  if(flag==1)
  {	//init ms8607
  	if (hx_drv_qwiic_ms8607_initial() != HX_DRV_LIB_PASS)
    	{
        hx_drv_uart_print("hx_drv_ms8607_initial fail ");
        return;
    	}
   }
  if(flag==0)
  {	// init ccs811
  	if (hx_drv_qwiic_ccs811_initial(HX_DRV_QWIIC_CCS811_I2C_0X5B) != HX_DRV_LIB_PASS)
  	{
    	hx_drv_uart_print("hx_drv_ccs811_initial fail.");
    	return;
  	}
  	// init bme280
  	if (hx_drv_qwiic_bme280_initial(HX_DRV_QWIIC_BME280_I2C_0X77) != HX_DRV_LIB_PASS)
  	{
    	hx_drv_uart_print("hx_drv_bme280_initial fail ");
    	return;
  	}
   }
} 

void loop()
{
  float t = 0, h = 0;
  uint32_t p=0; 
  float p1 = 0;
  uint16_t co2 = 0, tvoc = 0;
  char senddata[35] = {};
 if(flag==0)
 {
  if (hx_drv_qwiic_ccs811_get_data(&co2, &tvoc) == HX_DRV_LIB_PASS)
  {
    hx_drv_uart_print("CO2: %u ppm tVOC: %u ppb\n", co2, tvoc);
    if (hx_drv_qwiic_bme280_get_data(&t, &p, &h) == HX_DRV_LIB_PASS)
      hx_drv_uart_print("p:%d Pa, t:%d Celsius, h:%d %%RH\n", (uint32_t)p, (uint32_t)t, (uint32_t)h);

        lora_initial();
        if (lora_is_initialized)
        {
          sprintf((char *)senddata, (char *)"AT+SEND=2,20,%04d|%03d|%03d|%07d\x0d\x0a", co2, (uint32_t)t, (uint32_t)h, (uint32_t)p);
          hx_drv_swuart_write((uint8_t *)senddata, 35);
          hx_drv_uart_print("send: %s", senddata);
          memset(senddata, '\0', sizeof(senddata));
        }
  }
 }
 if(flag==1)
 {
  if (hx_drv_qwiic_ms8607_get_data(&t, &p1, &h) == HX_DRV_LIB_PASS)
  {
  	hx_drv_uart_print("p:%d mbar, t:%d Celsius, h:%d %%RH\n", (uint32_t)p1, (int32_t)t, (uint32_t)h);
  		lora_initial();
        	if (lora_is_initialized)
        	{
          	sprintf((char *)senddata, (char *)"AT+SEND=2,20,%04d|%03d|%03d|%07d\x0d\x0a", co2, (uint32_t)t, (uint32_t)h, (uint32_t)p1);
          	hx_drv_swuart_write((uint8_t *)senddata, 35);
          	hx_drv_uart_print("send: %s", senddata);
          	memset(senddata, '\0', sizeof(senddata));
        	}
  }
 }
}
