#include "hx_drv_tflm.h"
#include "tflite-model/trained_model_compiled.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "model-parameters/model_variables.h"
#define MAX_TRACKED_ALGO_RES  10
typedef enum
{
	MONO_FRAME=0,
	RAWBAYER_FRAME,
	YUV_FRAME
}enum_frameFormat;


typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
}struct__box;

typedef struct
{
	struct__box bbox;
    uint32_t time_of_existence;
    uint32_t is_reliable;
}struct_MotionTarget;



typedef struct
{
	struct__box upper_body_bbox;
    uint32_t upper_body_scale;
    uint32_t upper_body_score;
    uint32_t upper_body_num_frames_since_last_redetection_attempt;
    struct__box head_bbox;
    uint32_t head_scale;
    uint32_t head_score;
    uint32_t head_num_frames_since_last_redetection_attempt;
    uint32_t octave;
    uint32_t time_of_existence;
    uint32_t isVerified;
}struct_Human;

typedef struct
{
    int num_hot_pixels ;
    struct_MotionTarget Emt[MAX_TRACKED_ALGO_RES]; ; //ecv::motion::Target* *tracked_moving_targets;
    int frame_count ;
    short num_tracked_moving_targets;
    short num_tracked_human_targets ;
    bool humanPresence ;
    struct_Human ht[MAX_TRACKED_ALGO_RES];  //TrackedHumanTarget* *tracked_human_targets;
    int num_reliable_moving_targets;
    int verifiedHumansExist;
}struct_algoResult;

typedef struct boxabs {
    float left, right, top, bot;
} boxabs;
struct_algoResult algoresult;
namespace
{
    hx_drv_sensor_image_config_t g_pimg_config;
    hx_drv_gpio_config_t gpio_config;
    int input_width = 160;
    int input_height = 160;
    int input_channels = 1;
}

// Callback function declaration
static int get_signal_data(size_t offset, size_t length, float *out_ptr);
int8_t inference_image[25600]={0};//input_channels*input_width*input_height
TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t *image_data)
{
    static bool is_initialized = false;

    if (!is_initialized)
    {
        g_pimg_config.sensor_type = HX_DRV_SENSOR_TYPE_HM0360_MONO;
        g_pimg_config.format = HX_DRV_VIDEO_FORMAT_YUV400;
        g_pimg_config.img_width = 640;
        g_pimg_config.img_height = 480;
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

    //rescale image to model input size
    hx_drv_image_rescale((uint8_t *)g_pimg_config.raw_address,
                         g_pimg_config.img_width, g_pimg_config.img_height,
                         image_data, image_width, image_height);

    return kTfLiteOk;
}
extern "C" void print_out(const char *format, va_list args){
    hx_drv_uart_print("%s", format);
};
//link to label define array
extern const char* ei_classifier_inferencing_categories [];
int main(void)
{
    // initial uart
    hx_drv_uart_initial(UART_BR_115200);
    // loop step
    while (true)
    {
         if (kTfLiteOk != GetImage(input_width, input_height, input_channels,inference_image))
        {
            hx_drv_uart_print("Image capture failed.");
            continue;
        }
        signal_t signal;            // Wrapper for raw input buffer
        ei_impulse_result_t result; // Used to store inference output
        EI_IMPULSE_ERROR res;       // Return code from inference

        // Assign callback function to fill buffer used for preprocessing/inference
        signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
        signal.get_data = &get_signal_data;

        // Perform DSP pre-processing and inference
        res = run_classifier(&signal, &result, false);
        int bbox_quantity = 0;
        // Print the prediction results (object detection)
        hx_drv_uart_print("Object detection: \r\n");
        for (uint32_t i = 0; i < EI_CLASSIFIER_OBJECT_DETECTION_COUNT; i++) {
            ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
            if (bb.value == 0) {
                continue;
            }
            int score = bb.value*100;
        #if 1
            algoresult.ht[i].upper_body_score = (uint32_t)(score);
            algoresult.ht[i].upper_body_bbox.x = (uint32_t)(bb.x  * g_pimg_config.img_width / input_width);
            algoresult.ht[i].upper_body_bbox.y = (uint32_t)(bb.y * g_pimg_config.img_height / input_height);
            // set 50 x 50 box to show on PC_TOOL
            algoresult.ht[i].upper_body_bbox.width = 50;
            algoresult.ht[i].upper_body_bbox.height = 50;
            hx_drv_uart_print(" %s score: (%d) [ x: %u, y: %u]\r\n", 
                    bb.label,
                    score, 
                    algoresult.ht[i].upper_body_bbox.x, 
                    algoresult.ht[i].upper_body_bbox.y);
                
            bbox_quantity++;
        #endif
        
        }

        if (bbox_quantity>0)
        {
            algoresult.humanPresence = true;
            algoresult.num_tracked_human_targets = bbox_quantity;
        }
        else
        {
            hx_drv_uart_print(" no object\r\n");
            algoresult.humanPresence = false;
            algoresult.num_tracked_human_targets = 0;
        }
        if(hx_drv_spim_send((uint32_t)&algoresult, sizeof(struct_algoResult), SPI_TYPE_META_DATA ) != HX_DRV_LIB_PASS)
        {
            hx_drv_uart_print("spi send bbox failed.");
            return 0;
        }

    }

    return 0;
}

void grayscale_to_rgb(uint8_t gray, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = gray;
    *g = gray;
    *b = gray;
}

int get_signal_data(size_t offset, size_t length, float *out_ptr) {
    size_t bytes_left = length;
    size_t out_ptr_ix = 0;

    // read byte for byte
    while (bytes_left != 0) {

        // grab the value and convert to r/g/b
        uint8_t pixel = inference_image[offset]+128;

        uint8_t r, g, b;
        grayscale_to_rgb(pixel, &r, &g, &b);
        // then convert to out_ptr format
        float pixel_f = (r << 16) + (g << 8) + b;
        out_ptr[out_ptr_ix] = pixel_f;

        // and go to the next pixel
        out_ptr_ix++;
        offset++;
        bytes_left--;
    }

    // and done!
    return 0;
}
