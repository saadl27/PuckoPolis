#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define RAD2DEG                     57.2957795131f

/* A* constants */
#define MAX_NUM_NODES               15
#define MAX_EDGES_PER_NODE          4

// edge weights
#define WEIGHT_01_05                550 // mm, distance between nodes 1 and 5
#define WEIGHT_01_03                380
#define WEIGHT_01_02                85
#define WEIGHT_02_06                200
#define WEIGHT_02_04                170
#define WEIGHT_04_10                365
#define WEIGHT_04_09                235
#define WEIGHT_05_06                220
#define WEIGHT_05_07                110
#define WEIGHT_07_08                330
#define WEIGHT_06_08                220
#define WEIGHT_07_13                550
#define WEIGHT_08_09                200
#define WEIGHT_11_13                340
#define WEIGHT_13_14                340
#define WEIGHT_14_15                145
#define WEIGHT_09_10                102
#define WEIGHT_10_12                305
#define WEIGHT_03_12                495
#define WEIGHT_12_15                240
#define WEIGHT_03_15                1100
#define WEIGHT_11_14                155
#define WEIGHT_09_11                95


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

#define POOLING_COUNT               7


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
#define IMU_DT                      (IMU_THD_PERIOD_MS / 1000.0f)

#define IMU_STATE_SIZE              2       // [theta, bgz]
#define GZ_MEAS_NOISE               0.0000051610f
#define GZ_PROC_NOISE               1e-2f
#define GZ_BIAS_VARIANCE            1e-7f


/* Constants used for motors */
#define PID_STACK_SIZE		        256
#define PID_LOOP_MS			        10
#define MOT_DT                      (PID_LOOP_MS / 1000.0f)

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
#define FWD_DISP			        3.5f // forward displacement in cm
#define ROT_CORRECTION              0.35f // rad (roughly 20 deg)


/* Constant used by telemetry reset thread */
#define RECEIVE_RESET_STACK_SIZE    256


/* Constants used by LED functions */
#define RED_INTENSITY               100
#define GREEN_INTENSITY             100
#define BLUE_INTENSITY              100


#endif /* __CONSTANTS_H__ */