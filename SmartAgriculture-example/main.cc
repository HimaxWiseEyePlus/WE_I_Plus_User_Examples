#include "hx_drv_tflm.h"
#include "edge-impulse-sdk/tensorflow/lite/schema/schema_generated.h"
#include "tflite-model/trained_model_compiled.h"
#include "settings.h"

namespace
{
    hx_drv_sensor_image_config_t g_pimg_config;
    int input_width = 96;
    int input_height = 96;
    int input_channels = 1;
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

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t *image_data)
{
    static bool is_initialized = false;

    if (!is_initialized)
    {
        if (hx_drv_sensor_initial(&g_pimg_config) != HX_DRV_LIB_PASS)
        {
            return kTfLiteError;
        }

        if (hx_drv_spim_init() != HX_DRV_LIB_PASS)
        {
            return kTfLiteError;
        }

        is_initialized = true;
    }

    //capture image by sensor
    hx_drv_sensor_capture(&g_pimg_config);

    hx_drv_image_rescale((uint8_t *)g_pimg_config.raw_address,
                         g_pimg_config.img_width, g_pimg_config.img_height,
                         image_data, image_width, image_height);

    return kTfLiteOk;
}

void RespondToDetection(int8_t *crop)
{
    hx_drv_uart_print("[wheat] %d, [sugarcane] %d, [maize] %d\n", crop[0], crop[1], crop[2]);
}

int main(int argc, char* argv[]) 
{
  hx_drv_share_switch(SHARE_MODE_I2CM);
  // init uart
  hx_drv_uart_initial(UART_BR_115200);
  
  if(flag==0)
  {	// init ccs811
  	if (hx_drv_qwiic_ccs811_initial(HX_DRV_QWIIC_CCS811_I2C_0X5B) != HX_DRV_LIB_PASS)
  	{
    	hx_drv_uart_print("hx_drv_ccs811_initial fail.");
    	return 0;
  	}
  	// init bme280
  	if (hx_drv_qwiic_bme280_initial(HX_DRV_QWIIC_BME280_I2C_0X77) != HX_DRV_LIB_PASS)
  	{
    	hx_drv_uart_print("hx_drv_bme280_initial fail ");
    	return 0;
  	}
   }
   
  if(flag==1)
  {	//init ms8607
  	if (hx_drv_qwiic_ms8607_initial() != HX_DRV_LIB_PASS)
    	{
        hx_drv_uart_print("hx_drv_ms8607_initial fail ");
        return 0;
    	}
   }

    // init model
    TfLiteStatus init_status = trained_model_init(NULL);
    if (init_status != kTfLiteOk)
    {
        hx_drv_uart_print("init fail\n");
        return 0;
    }

    TfLiteTensor *input = trained_model_input(0);
    
  while (true) 
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

		//lora_initial
		  if (!lora_is_initialized)
			  {
			    // init swuart
			    hx_drv_swuart_initial(HX_DRV_PGPIO_0, HX_DRV_PGPIO_1, UART_BR_115200);

			    // init loar
			    if (!send_AT_and_retry((char *)"AT\x0d\x0a", 4, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			    if (!send_AT_and_retry((char *)"AT+ADDRESS=1\x0d\x0a", 14, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			    if (!send_AT_and_retry((char *)"AT+CRFOP=7\x0d\x0a", 12, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			   lora_is_initialized = true;
			  }

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
	  	
	  	//lora_initial
		  if (!lora_is_initialized)
			  {
			    // init swuart
			    hx_drv_swuart_initial(HX_DRV_PGPIO_0, HX_DRV_PGPIO_1, UART_BR_115200);

			    // init loar
			    if (!send_AT_and_retry((char *)"AT\x0d\x0a", 4, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			    if (!send_AT_and_retry((char *)"AT+ADDRESS=1\x0d\x0a", 14, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			    if (!send_AT_and_retry((char *)"AT+CRFOP=7\x0d\x0a", 12, (char *)"+OK\x0d\x0a"))
			    {
			      hx_drv_uart_print("loar initial fail.");
			      return 0;
			    }
			   lora_is_initialized = true;
			  }
	  	
	    if (lora_is_initialized)
	      {
	       sprintf((char *)senddata, (char *)"AT+SEND=2,20,0000|%03d|%03d|%07d\x0d\x0a", (uint32_t)t, (uint32_t)h, (uint32_t)p1);
	       hx_drv_swuart_write((uint8_t *)senddata, 35);
	       hx_drv_uart_print("send: %s", senddata);
	       memset(senddata, '\0', sizeof(senddata));
	      }
	  }
	 }
	 
	 GetImage(input_width, input_height, input_channels, input->data.int8);
	 trained_model_invoke();

	 TfLiteTensor *output = trained_model_output(0);
	 RespondToDetection((int8_t *)output->data.uint8);
  }
}
