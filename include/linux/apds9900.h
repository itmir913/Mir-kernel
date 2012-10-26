#ifndef _APDS9900_H_
#define _APDS9900_H_

#define APDS9900_TYPE_PROXIMITY         1
#define APDS9900_TYPE_LIGHT                     2
#define APDS9900_TYPE_ALL                       (APDS9900_TYPE_PROXIMITY | APDS9900_TYPE_LIGHT)

#define SENSOR_APDS9900_ISR_VER2
#define SENSOR_APDS9900_ISR_VER3
#define SENSOR_APDS9900_ISR_VER4 //ST_LIM_110422 - 근접센서 인터럽트 설정 변경

typedef struct {
        int x;
        int y;
        int z;
} axes_t;

typedef enum {
        E_ACTIVE_NONE,
        E_ACTIVE_PROXIMITY = APDS9900_TYPE_PROXIMITY,
        E_ACTIVE_LIGHT = APDS9900_TYPE_LIGHT,
        E_ACTIVE_ALL,
} active_e;

int apds9900_control_enable(active_e sensor_type, int enable);
int apds9900_ps_measure(axes_t *val);
int apds9900_ls_measure(axes_t *val);

#define IOCTL_SKY_PROX_DISABLE		0
#define IOCTL_SKY_PROX_ENABLE		1


/*-------------------------------------------------------------
**QE문제 
EF32K No.39, EF31S No.32 근접센서 데이터 전송시 이전 값과 같을 경우,
EVENT 무시되어 APP에서 정상적인 동작이 되지 않는 현상으로 인하여,
최초 EVENT 전달시 ABSX : 0 전달 후  ABSX : 2로 전달하여 초기 설정을 함.
-------------------------------------------------------------*/
//#define FEATURE_PLM_PROBLEM_NO_32 //ST_LIM_20110112
#ifdef FEATURE_PLM_PROBLEM_NO_32
extern int is_first_start_apds9900;
#endif

#endif // _APDS9900_H_
