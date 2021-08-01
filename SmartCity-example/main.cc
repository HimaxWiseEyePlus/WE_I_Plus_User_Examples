#include "hx_drv_tflm.h"
#include "SparkFunBME280.h"
#include "SparkFunCCS811.h"
#include "tflite-model/trained_model_compiled.h"

#define CCS811_ADDR 0x5B //Default I2C Address
//Global sensor objects
    CCS811 myCCS811(CCS811_ADDR);
    BME280 myBME280;

namespace
{
    hx_drv_sensor_image_config_t g_pimg_config;
    hx_drv_gpio_config_t gpio_config;
    int input_width = 96;
    int input_height = 96;
    int input_channels = 1;
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

    //send jpeg image data out through SPI
    hx_drv_spim_send(g_pimg_config.jpeg_address, g_pimg_config.jpeg_size,
                     SPI_TYPE_JPG);

    hx_drv_image_rescale((uint8_t *)g_pimg_config.raw_address,
                         g_pimg_config.img_width, g_pimg_config.img_height,
                         image_data, image_width, image_height);

    return kTfLiteOk;
}

void RespondToDetection(int8_t *score)
{
    // get the index with the highest score
    
    int maxindex = 0;
    int maxvalue = -128;
    float per;
    for (int i = 0; i < 2; i++)
    {
        if (score[i] > maxvalue)
        {
            maxvalue = score[i];
            maxindex = i;
        }
    }
    per=(maxvalue*100)/128;    
    // init gpio pin to 0
    gpio_config.gpio_pin = HX_DRV_PGPIO_0;
    gpio_config.gpio_data = 0;
    hx_drv_gpio_set(&gpio_config);
    gpio_config.gpio_pin = HX_DRV_PGPIO_1;
    gpio_config.gpio_data = 0;
    hx_drv_gpio_set(&gpio_config);
    
    
    // show the scores to UART
   
    	if(0<per && per<40 && maxindex ==0)
    		{
    		// if nocar, turn on LED RED and output 1 to PGPIO_1
        	hx_drv_led_off(HX_DRV_LED_GREEN);
        	hx_drv_led_on(HX_DRV_LED_RED);
        	gpio_config.gpio_pin = HX_DRV_PGPIO_1;
        	gpio_config.gpio_data = 1;
        	hx_drv_gpio_set(&gpio_config);
        	hx_drv_uart_print("NO CAR DETECTED: %d %% confidence\n",(int)per);
    		}
    	else if(0<per && per<40 && maxindex ==1)
    		{
    		// if car, turn on LED GREEN and output 1 to PGPIO_0
        	hx_drv_led_on(HX_DRV_LED_GREEN);
        	hx_drv_led_off(HX_DRV_LED_RED);
        	gpio_config.gpio_pin = HX_DRV_PGPIO_0;
        	gpio_config.gpio_data = 1;
        	hx_drv_gpio_set(&gpio_config);
        	hx_drv_uart_print("CAR DETECTED: %d %% confidence\n",(int)per);
        	}
        else if(60<per && per<100 && maxindex ==0)
    		{
    		// if car, turn on LED GREEN and output 1 to PGPIO_0
        	hx_drv_led_on(HX_DRV_LED_GREEN);
        	hx_drv_led_off(HX_DRV_LED_RED);
        	gpio_config.gpio_pin = HX_DRV_PGPIO_0;
        	gpio_config.gpio_data = 1;
        	hx_drv_gpio_set(&gpio_config);
        	hx_drv_uart_print("CAR DETECTED: %d %% confidence\n",(int)per);
    		}
    	else if(60<per && per<100 && maxindex ==1)
    		{
    		// if nocar, turn on LED RED and output 1 to PGPIO_1
        	hx_drv_led_off(HX_DRV_LED_GREEN);
        	hx_drv_led_on(HX_DRV_LED_RED);
        	gpio_config.gpio_pin = HX_DRV_PGPIO_1;
        	gpio_config.gpio_data = 1;
        	hx_drv_gpio_set(&gpio_config);
        	hx_drv_uart_print("NO CAR DETECTED: %d %% confidence\n",(int)per);
        	}
        else
        	{
        	hx_drv_led_off(HX_DRV_LED_GREEN);
        	hx_drv_led_off(HX_DRV_LED_RED);
        	}
    hx_drv_uart_print("\t [car] %d, [nocar] %d\n", score[0], score[1]);
}

int main(void)
{   
    // initial uart and gpio
    hx_drv_uart_initial(UART_BR_115200);


    
    //for sensors
    hx_drv_share_switch(SHARE_MODE_I2CM);
    //This begins the CCS811 sensor and prints error status of .begin()
    if (!myCCS811.begin())
    {
        hx_drv_uart_print("Problem with CCS811\n");
    }
    else
    {
        hx_drv_uart_print("CCS811 online\n");
    }

    //Initialize BME280
    //For I2C, enable the following and disable the SPI section
    myBME280.settings.commInterface = I2C_MODE;
    myBME280.settings.I2CAddress = 0x77;
    myBME280.settings.runMode = 3; //Normal mode
    myBME280.settings.tStandby = 0;
    myBME280.settings.filter = 4;
    myBME280.settings.tempOverSample = 5;
    myBME280.settings.pressOverSample = 5;
    myBME280.settings.humidOverSample = 5;

    //Calling .begin() causes the settings to be loaded
    hx_util_delay_ms(10);            //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
    uint8_t id = myBME280.begin(); //Returns ID of 0x60 if successful
    if (id != 0x60)
    {
        hx_drv_uart_print("Problem with BME280\n");
    }
    else
    {
        hx_drv_uart_print("BME280 online\n");
    }

    myCCS811.setDriveMode(1); //constant power mode
    



    
    gpio_config.gpio_pin = HX_DRV_PGPIO_0;
    gpio_config.gpio_direction = HX_DRV_GPIO_OUTPUT;
    hx_drv_gpio_initial(&gpio_config);
    gpio_config.gpio_pin = HX_DRV_PGPIO_1;
    hx_drv_gpio_initial(&gpio_config);
    

    // init model
    TfLiteStatus init_status = trained_model_init(NULL);
    if (init_status != kTfLiteOk)
    {
        hx_drv_uart_print("init fail\n");
        return 0;
    }

    TfLiteTensor *input = trained_model_input(0);

    // loop step
    while (true)
    {
        if (kTfLiteOk != GetImage(input_width, input_height, input_channels, input->data.int8))
        {
            hx_drv_uart_print("Image capture failed.");
            continue;
        }
        trained_model_invoke();

        TfLiteTensor *output = trained_model_output(0);

        RespondToDetection((int8_t *)output->data.uint8);


        //for sensor
        myCCS811.readAlgorithmResults(); //Read latest from CCS811 and update tVOC and CO2 variables
        hx_drv_uart_print("CO2: %d,TVOC: %d,Humidity: %d\n", myCCS811.getCO2(), myCCS811.getTVOC(),(uint32_t)(int)myBME280.readFloatHumidity());
	hx_drv_uart_print("\n");


    }

    return 0;
}
