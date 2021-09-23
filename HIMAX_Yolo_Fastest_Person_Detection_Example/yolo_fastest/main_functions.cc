#include "main_functions.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <algorithm>
#include <math.h>
#include <stdint.h>
#include <forward_list>
#include "hx_drv_tflm.h" 
#define MAX_TRACKED_ALGO_RES  10
#define COLOR_DEPTH	1 // 8bit per pixel FU
#define COCO_DATASET 1
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


typedef struct branch {
    int resolution;
    int num_box;
    float *anchor;
    int8_t *tf_output;
    float scale;
    int zero_point;
    size_t size;
    float scale_x_y;
} branch;

typedef struct network {
    int input_w;
    int input_h;
    int num_classes;
    int num_branch;
    branch *branchs;
    int topN;
} network;


typedef struct box {
    float x, y, w, h;
} box;

typedef struct detection{
    box bbox;
    float *prob;
    float objectness;
} detection;

struct_algoResult algoresult;
hx_drv_sensor_image_config_t g_pimg_config;
// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output[2] = {nullptr,nullptr};

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 399 * 1024;
#if (defined(__GNUC__) || defined(__GNUG__)) && !defined (__CCAC__)
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
#else
#pragma Bss(".tensor_arena")
static uint8_t tensor_arena[kTensorArenaSize];
#pragma Bss()
#endif // if defined (_GNUC_) && !defined (_CCAC_)
}  // namespace



static int sort_class;

void free_dets(std::forward_list<detection> &dets){
    std::forward_list<detection>::iterator it;
    for ( it = dets.begin(); it != dets.end(); ++it ){
        free(it->prob);
    }
}

float sigmoid(float x)
{
    return 1.f/(1.f + exp(-x));
} 

bool det_objectness_comparator(detection &pa, detection &pb)
{
    return pa.objectness < pb.objectness;
}

void insert_topN_det(std::forward_list<detection> &dets, detection det){
    std::forward_list<detection>::iterator it;
    std::forward_list<detection>::iterator last_it;
    for ( it = dets.begin(); it != dets.end(); ++it ){
        if(it->objectness > det.objectness)
            break;
        last_it = it;
    }
    if(it != dets.begin()){
        dets.emplace_after(last_it, det);
        free(dets.begin()->prob);
        dets.pop_front();
    }
    else{
        free(det.prob);
    }
}

std::forward_list<detection> get_network_boxes(network *net, int image_w, int image_h, float thresh, int *num)
{
    std::forward_list<detection> dets;
    int i;
    int num_classes = net->num_classes;
    *num = 0;

    for (i = 0; i < net->num_branch; ++i) {
        int height  = net->branchs[i].resolution;
        int width = net->branchs[i].resolution;
        int channel  = net->branchs[i].num_box*(5+num_classes);

        for (int h = 0; h < net->branchs[i].resolution; h++) {
            for (int w = 0; w < net->branchs[i].resolution; w++) {
                for (int anc = 0; anc < net->branchs[i].num_box; anc++) {
                    
                    // objectness score
                    int bbox_obj_offset = h * width * channel + w * channel + anc * (num_classes + 5) + 4;
                    float objectness = sigmoid(((float)net->branchs[i].tf_output[bbox_obj_offset] - net->branchs[i].zero_point) * net->branchs[i].scale);

                    if(objectness > thresh){
                        detection det;
                        det.prob = (float*)calloc(num_classes, sizeof(float));
                        det.objectness = objectness;
                        //get bbox prediction data for each anchor, each feature point
                        int bbox_x_offset = bbox_obj_offset -4;
                        int bbox_y_offset = bbox_x_offset + 1;
                        int bbox_w_offset = bbox_x_offset + 2;
                        int bbox_h_offset = bbox_x_offset + 3;
                        int bbox_scores_offset = bbox_x_offset + 5;
                        //int bbox_scores_step = 1;
                        det.bbox.x = ((float)net->branchs[i].tf_output[bbox_x_offset] - net->branchs[i].zero_point) * net->branchs[i].scale;
                        det.bbox.y = ((float)net->branchs[i].tf_output[bbox_y_offset] - net->branchs[i].zero_point) * net->branchs[i].scale;
                        det.bbox.w = ((float)net->branchs[i].tf_output[bbox_w_offset] - net->branchs[i].zero_point) * net->branchs[i].scale;
                        det.bbox.h = ((float)net->branchs[i].tf_output[bbox_h_offset] - net->branchs[i].zero_point) * net->branchs[i].scale;

                        float bbox_x, bbox_y;

                        // Eliminate grid sensitivity trick involved in YOLOv4
                        bbox_x = sigmoid(det.bbox.x); //* net->branchs[i].scale_x_y - (net->branchs[i].scale_x_y - 1) / 2;
                        bbox_y = sigmoid(det.bbox.y); //* net->branchs[i].scale_x_y - (net->branchs[i].scale_x_y - 1) / 2;
                        det.bbox.x = (bbox_x + w) / width;
                        det.bbox.y = (bbox_y + h) / height;

                        det.bbox.w = exp(det.bbox.w) * net->branchs[i].anchor[anc*2] / net->input_w;
                        det.bbox.h = exp(det.bbox.h) * net->branchs[i].anchor[anc*2+1] / net->input_h;
                        
                        for (int s = 0; s < num_classes; s++) {
                            det.prob[s] = sigmoid(((float)net->branchs[i].tf_output[bbox_scores_offset + s] - net->branchs[i].zero_point) * net->branchs[i].scale)*objectness;
                            det.prob[s] = (det.prob[s] > thresh) ? det.prob[s] : 0;
                        }

                        //correct_yolo_boxes 
                        det.bbox.x *= image_w;
                        det.bbox.w *= image_w;
                        det.bbox.y *= image_h;
                        det.bbox.h *= image_h;

                        if (*num < net->topN || net->topN <=0){
                            dets.emplace_front(det);
                            *num += 1;
                        }
                        else if(*num ==  net->topN){
                            dets.sort(det_objectness_comparator);
                            insert_topN_det(dets,det);
                            *num += 1;
                        }else{
                            insert_topN_det(dets,det);
                        }
                    }
                }
            }
        }
    }
    if(*num > net->topN)
        *num -=1;
    return dets;
}

// init part

branch create_brach(int resolution, int num_box, float *anchor, int8_t *tf_output, size_t size, float scale, int zero_point){
    branch b;
    b.resolution = resolution;
    b.num_box = num_box;
    b.anchor = anchor;
    b.tf_output = tf_output;
    b.size = size;
    b.scale = scale;
    b.zero_point = zero_point;
    return b;
}

network creat_network(int input_w, int input_h, int num_classes, int num_branch, branch* branchs, int topN){
    network net;
    net.input_w = input_w;
    net.input_h = input_h;
    net.num_classes = num_classes;
    net.num_branch = num_branch;
    net.branchs = branchs;
    net.topN = topN;
    return net;
}

// NMS part

float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float box_intersection(box a, box b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0) return 0;
    float area = w*h;
    return area;
}

float box_union(box a, box b)
{
    float i = box_intersection(a, b);
    float u = a.w*a.h + b.w*b.h - i;
    return u;
}

float box_iou(box a, box b)
{
    float I = box_intersection(a, b);
    float U = box_union(a, b);
    if (I == 0 || U == 0) {
        return 0;
    }
    return I / U;
}

bool det_comparator(detection &pa, detection &pb)
{
    return pa.prob[sort_class] > pb.prob[sort_class];
}

void do_nms_sort(std::forward_list<detection> &dets, int classes, float thresh)
{
    int k;
    
    for (k = 0; k < classes; ++k) {
        sort_class = k;
        dets.sort(det_comparator);
        
        for (std::forward_list<detection>::iterator it=dets.begin(); it != dets.end(); ++it){
            if (it->prob[k] == 0) continue;
            for (std::forward_list<detection>::iterator itc=std::next(it, 1); itc != dets.end(); ++itc){
                if (itc->prob[k] == 0) continue;
                if (box_iou(it->bbox, itc->bbox) > thresh) {
                    itc->prob[k] = 0;
                }
            }
        }
    }
}


boxabs box_c(box a, box b) {
    boxabs ba;//
    ba.top = 0;
    ba.bot = 0;
    ba.left = 0;
    ba.right = 0;
    ba.top = fmin(a.y - a.h / 2, b.y - b.h / 2);
    ba.bot = fmax(a.y + a.h / 2, b.y + b.h / 2);
    ba.left = fmin(a.x - a.w / 2, b.x - b.w / 2);
    ba.right = fmax(a.x + a.w / 2, b.x + b.w / 2);
    return ba;
}


float box_diou(box a, box b)
{
    boxabs ba = box_c(a, b);
    float w = ba.right - ba.left;
    float h = ba.bot - ba.top;
    float c = w * w + h * h;
    float iou = box_iou(a, b);
    if (c == 0) {
        return iou;
    }
    float d = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    float u = pow(d / c, 0.6);
    float diou_term = u;

    return iou - diou_term;
}


void diounms_sort(std::forward_list<detection> &dets, int classes, float thresh)
{
    int k;
    
    for (k = 0; k < classes; ++k) {
        sort_class = k;
        dets.sort(det_comparator);
        
        for (std::forward_list<detection>::iterator it=dets.begin(); it != dets.end(); ++it){
            if (it->prob[k] == 0) continue;
            for (std::forward_list<detection>::iterator itc=std::next(it, 1); itc != dets.end(); ++itc){
                if (itc->prob[k] == 0) continue;
                if (box_diou(it->bbox, itc->bbox) > thresh) {
                    itc->prob[k] = 0;
                }
            }
        }
    }
}

void setup() {
    hx_drv_uart_initial(UART_BR_115200);

    //flash init 
    //must do before hx_drv_flash_get_Model_address()
    if (hx_drv_flash_init() != HX_DRV_LIB_PASS)
    {
        hx_drv_uart_print("Spi 0 for flash initial fail.");
        return ;
    }
    if (hx_drv_spim_init() != HX_DRV_LIB_PASS)
    {
        hx_drv_uart_print("Spi 1 for ft4222 initial fail.");
        return ;
    }
    
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    //get model (.tflite) from flash
    model = tflite::GetModel((unsigned char*)hx_drv_flash_get_Model_address());
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }


  static tflite::MicroMutableOpResolver<12> micro_op_resolver;
		micro_op_resolver.AddAveragePool2D();
		micro_op_resolver.AddConv2D();
		micro_op_resolver.AddDepthwiseConv2D();
		micro_op_resolver.AddReshape();
		micro_op_resolver.AddSoftmax();

		micro_op_resolver.AddPad();
		micro_op_resolver.AddRelu6();
		micro_op_resolver.AddAdd();
		micro_op_resolver.AddMaxPool2D();
		micro_op_resolver.AddConcatenation();
		micro_op_resolver.AddResizeNearestNeighbor();
		micro_op_resolver.AddQuantize();
  
  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;
  
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
  
  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
  
}

// The name of this function is important for Arduino compatibility.
void loop() {
    // Get image start.
    static bool is_initialized = false;
    if (!is_initialized) {
        if (hx_drv_sensor_initial(&g_pimg_config) != HX_DRV_LIB_PASS) {
            TF_LITE_REPORT_ERROR(error_reporter, "Image capture failed.");
            return;
        }
        is_initialized = true;
    }

    hx_drv_sensor_capture(&g_pimg_config);
    ////Camera image send by SPI
    if(hx_drv_share_switch(SHARE_MODE_SPIM) != HX_DRV_LIB_PASS)
    {
        hx_drv_uart_print("Switch to SPI fail.");
        return ;
    }
    if(hx_drv_spim_send(g_pimg_config.jpeg_address ,g_pimg_config.jpeg_size, SPI_TYPE_JPG ) != HX_DRV_LIB_PASS)
    {
        hx_drv_uart_print("jpeg Image capture failed.");
        return ;
    }

    int input_w = 160;
	int input_h = 160;
    //resize image to 160 x 160
    hx_drv_image_rescale((uint8_t*)g_pimg_config.raw_address,
                        g_pimg_config.img_width, g_pimg_config.img_height,
                        input->data.int8, input_w, input_h);

    TfLiteStatus invoke_status = interpreter->Invoke();
    // Run the model on this input and make sure it succeeds.
    if (kTfLiteOk != invoke_status) {
        TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed.");
    }
    for(int anchor=0;anchor<2;anchor++)
    {
         output[anchor] = interpreter->output(anchor);
    }

    // init postprocessing 	
	int num_classes = 1;
	int num_branch = 2;
	int topN = 0;
		

	branch* branchs = (branch*)calloc(num_branch, sizeof(branch));
    #ifdef COCO_DATASET
        float anchor1[] = {32, 93, 62,106, 113, 132};
        float anchor2[] = {4, 9, 10, 25, 23, 49};
    #else
        float anchor1[] = {19, 72, 34, 103, 73, 109};
	    float anchor2[] = {6, 25, 12, 43, 34, 33};
    #endif

	branchs[0] = create_brach(5, 3, anchor1, output[0]->data.int8, output[0]->bytes, ((TfLiteAffineQuantization*)(output[0]->quantization.params))->scale->data[0], ((TfLiteAffineQuantization*)(output[0]->quantization.params))->zero_point->data[0]);
		
	branchs[1] = create_brach(10, 3, anchor2, output[1]->data.int8, output[1]->bytes, ((TfLiteAffineQuantization*)(output[1]->quantization.params))->scale->data[0],((TfLiteAffineQuantization*)(output[1]->quantization.params))->zero_point->data[0]);
		
	network net = creat_network(input_w, input_h, num_classes, num_branch, branchs,topN);
	// end init

	// start postprocessing
    int nboxes=0;
    float thresh = .5;//50%
    float nms = .45;
    int image_width = g_pimg_config.img_width;
    int image_height =  g_pimg_config.img_height;
    std::forward_list<detection> dets = get_network_boxes(&net, image_width, image_height, thresh, &nboxes);
    hx_drv_uart_print("image_width: %d, image_height: %d, nboxes: %d\n",image_width,image_height, nboxes );
    // do nms
    diounms_sort(dets, net.num_classes, nms);
	uint8_t temp_unsuppressed_counter = 0;
    int j;
    for (std::forward_list<detection>::iterator it=dets.begin(); it != dets.end(); ++it){
        float xmin = it->bbox.x - it->bbox.w / 2.0f;
        float xmax = it->bbox.x + it->bbox.w / 2.0f;
        float ymin = it->bbox.y - it->bbox.h / 2.0f;
        float ymax = it->bbox.y + it->bbox.h / 2.0f;

        if (xmin < 0) xmin = 0;
        if (ymin < 0) ymin = 0;
        if (xmax > image_width) xmax = image_width;
        if (ymax > image_height) ymax = image_height;

        float bx = xmin;
        float by = ymin;
        float bw = xmax - xmin;
        float bh = ymax - ymin;

        for (j = 0; j <  net.num_classes; ++j) {
            if (it->prob[j] > 0) {
                hx_drv_uart_print("{\"bbox\":[x0 = %d, y0 = %d, w = %d, h = %d]@640x480, \"score\":%d},\n", (uint32_t)bx, (uint32_t)by, (uint32_t)bw, (uint32_t)bh, (uint32_t)(it->prob[j]*100));
                 algoresult.ht[temp_unsuppressed_counter].upper_body_score = (uint32_t)(it->prob[j]*100);
                 algoresult.ht[temp_unsuppressed_counter].upper_body_bbox.x = (uint32_t)(bx);
                 algoresult.ht[temp_unsuppressed_counter].upper_body_bbox.y = (uint32_t)(by);
                 algoresult.ht[temp_unsuppressed_counter].upper_body_bbox.width = (uint32_t)(bw);
                 algoresult.ht[temp_unsuppressed_counter].upper_body_bbox.height = (uint32_t)(bh);
                temp_unsuppressed_counter++;
            }
        }
    }

	free_dets(dets);
	free(branchs);

	if(nboxes)error_reporter->Report("bbox process ok.\n");
	
    uint8_t  bbox_quantity = temp_unsuppressed_counter;
	if(bbox_quantity > 0)
	{
		algoresult.humanPresence = true;
		algoresult.num_tracked_human_targets = bbox_quantity;
	}
	else
	{
		algoresult.humanPresence = false;
        algoresult.num_tracked_human_targets = 0;
	}
	//end postprocessing
    
    // Send the inference results by SPI.
    if(hx_drv_spim_send((uint32_t)&algoresult, sizeof(struct_algoResult), SPI_TYPE_META_DATA ) != HX_DRV_LIB_PASS)
    {
        hx_drv_uart_print("spi send bbox failed.");
        return ;
    }
  
}
