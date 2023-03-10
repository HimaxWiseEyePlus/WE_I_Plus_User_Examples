/*
 * bodydetect_meta.h
 *
 *  Created on: 2019¦~12¤ë29¤é
 *      Author: 902447
 */

#ifndef SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_
#define SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_

#define MAX_TRACKED_ALGO_RES  10
#define COLOR_DEPTH	1 // 8bit per pixel FU
typedef enum
{
    MONO_FRAME=0,
    RAWBAYER_FRAME,
    YUV_FRAME
}cust_frameFormat;

typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
}struct_det_box;
typedef struct
{
    struct_det_box bbox;
    uint32_t time_of_existence;
    uint32_t is_reliable;
}struct_MotionTarget;

typedef struct
{
    struct_det_box upper_body_bbox;
    uint32_t upper_body_scale;
    uint32_t upper_body_score;
    uint32_t upper_body_num_frames_since_last_redetection_attempt;
    struct_det_box head_bbox;
    uint32_t head_scale;
    uint32_t head_score;
    uint32_t head_num_frames_since_last_redetection_attempt;
    uint32_t octave;
    uint32_t time_of_existence;
    uint32_t isVerified;
    uint32_t face_id;
}struct_DetHuman;

typedef struct
{
    struct_det_box leftEyesBox;  //LEFT EYE
    struct_det_box rightEyesBox; //RIGHT EYE
    struct_det_box mouthBox;     //MOUTH
}struct_DetFacialParts;

typedef struct
{
    struct_det_box bodyDetectBox; //body detection
    int32_t bodyConfidence; //Confidence
}struct_DetBody;

typedef struct
{
    int num_hot_pixels ;
    struct_MotionTarget Emt[MAX_TRACKED_ALGO_RES]; ; //ecv::motion::Target* *tracked_moving_targets;
    int frame_count ;
    short num_tracked_moving_targets;
    short num_tracked_human_targets ;
    bool humanPresence ;
    struct_DetHuman ht[MAX_TRACKED_ALGO_RES];  //TrackedHumanTarget* *tracked_human_targets;
    int num_reliable_moving_targets;
    int verifiedHumansExist;
    struct_DetFacialParts fp[MAX_TRACKED_ALGO_RES]; //Facial Parts Detection
    struct_DetBody bd[MAX_TRACKED_ALGO_RES]; //body detection
}struct_algoResult;

#endif /* SCENARIO_APP_AIOT_BODYDETECT_ALLON_BODYDETECT_META_H_ */
