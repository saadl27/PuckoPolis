#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define RAD2DEG                     57.2957795131f

/* A* constants */
#define NUM_NODES                   15
#define MAX_EDGES_PER_NODE          4


/* Constants for FSM and Obstacle Detection threads */
#define FSM_THD_LOOP_MS             100
#define FSM_STACK_SIZE              8192
#define RST_STACK_SIZE              256
#define OBS_STACK_SIZE              1024
#define OBSTACLE_THRESHOLD_MM       ((uint16_t) 25)


/* Constants used by camera threads */
#define USED_LINE                   400   // Must be inside [0..478]
#define IMAGE_BUFFER_SIZE		    640
#define WIDTH_SLOPE				    5
#define MIN_LINE_WIDTH			    40
#define NO_LINE                     0

#define CAPTURE_STACK_SIZE          256
#define PROCESS_STACK_SIZE          16384

#define INDEX_OFFSET                50

#define RED_THRESHOLD_SCALE         1.4f
#define GREEN_THRESHOLD_SCALE       1.2f
#define BLUE_THRESHOLD_SCALE        1.2f

#define POLLING_COUNT               7


/* Constants used by ToF thread */
#define TOF_THD_PERIOD_MS           100
#define TOF_INIT_TIME_MS            300
#define DIST_STACK_SIZE             512


/* Kalman calibration (all measured from empirical data) */
#define DIST_OFFSET_MM              50
#define PROC_NOISE                  7.50f  // process noise (variance)
#define MEAS_NOISE                  8.52f // measurement noise (variance)


/* Constants used by Kalman orientation estimation thread */
#define IMU_STACK_SIZE              512
#define IMU_THD_PERIOD_MS           4
#define IMU_DT                      IMU_THD_PERIOD_MS / 1000.0f

#define IMU_STATE_SIZE              2       // [theta, bgz]
#define GZ_MEAS_NOISE               0.0000051610f


/* Constants used for motors */
#define PID_STACK_SIZE		        256
#define PID_LOOP_MS			        10

#define ROT_SPEED                   250
#define FWD_SPEED                   500
#define ROTATION_THRESHOLD		    10
#define ERROR_THRESHOLD			    0.1f	//[cm] because of the noise of the camera
#define KP						    3.2f
#define KI 						    (0.001f * 0)
#define KD                          (0.01f * 0)
#define MAX_D_ERROR                 100.0f
#define LOW_PASS_FACTOR             0.8f
#define MAX_SUM_ERROR 			    (MOTOR_SPEED_LIMIT/KI)
#define ERROR_ANGLE                 0.05f    // rad (roughly +/- 3 deg)

#define WHEEL_PERIMETER		        13 //cm
#define STEPS_ONE_TURN		        1000
#define FWD_DISP			        4 // forward displacement in cm


/* Constant used by telemetry reset thread */
#define RECEIVE_RESET_STACK_SIZE    256


/* Constants used by LED functions */
#define RED_INTENSITY               255 // max uint8_t
#define GREEN_INTENSITY             255
#define BLUE_INTENSITY              255


#endif /* __CONSTANTS_H__ */