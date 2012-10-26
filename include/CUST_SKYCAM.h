#ifndef __CUST_SKYCAM_H__
#define __CUST_SKYCAM_H__


/*------------------------------------------------------------------------------

(1)  하드웨어 구성

EF13S/L : MSM7x27, MT9P111(5M SOC), YACBAC1SDDAS(VGA)
EF31S   : MSM7x27, MT9T111(3M SOC), YACBAC1SDDAS(VGA)
EF32K   : MSM7x27, MT9T111(3M SOC), YACBAC1SDDAS(VGA)


(2)  카메라 관련 모든 kernel/userspace/android 로그를 제거

kernel/arch/arm/config/qsd8650-perf_defconfig 를 수정한다.

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

CUST_SKYCAM.h 에서 F_SKYCAM_LOG_PRINTK 을 #undef 한다. 

	#define F_SKYCAM_LOG_PRINTK (default)

모든 소스 파일에서 F_SKYCAM_LOG_OEM 을 찾아 주석 처리한다. 
	선언 된 경우, 해당 파일에 사용된 SKYCDBG/SKYCERR 매크로를 이용한 
	메시지들을 활성화 시킨다. (user-space only)

모든 소스 파일에서 F_SKYCAM_LOG_CDBG 를 찾아 주석 처리한다. 
	선언 된 경우, 해당 파일에 사용된 CDBG 매크로를 이용한 메시지들을 
	활성화 시킨다. (user-space only)

모든 소스 파일에서 F_SKYCAM_LOG_VERBOSE 를 찾아 주석 처리한다.
	선언 된 경우, 해당 파일에 사용된 LOGV/LOGD/LOGI 매크로를 이용한 
	메시지들을 활성화 시킨다. (user-space only)


(3)  MV9337/MV9335 관련 빌드 환경 (kernel/user-space)

<KERNEL>

kernel/arch/arm/config/msm7627-EF31S-perf_defconfig (for EF31S)
kernel/arch/arm/config/msm7627-EF32K-perf_defconfig (for EF32K) 를
각각 수정한다.

#CONFIG_MT9P111 is not set       미포함
CONFIG_MT9T111=y                 포함
CONFIG_YACBAC1SDDAS=y            포함

msm7627-XXXX-perf_defconfig 가 Kconfig (default y) 보다 우선 순위가 높다.

<USERSPACE>

vendor/qcom/proprietary/mm-camera/Android.mk (for Android)
vendor/qcom/proprietary/mm-camera/camera.mk (for Linux) 를 수정한다.

SYS_MODEL_NAME 설정에 따라 값을 각각 수정한다.
SKYCAM_MT9P111=no        미포함
SKYCAM_MT9T111=yes       포함
SKYCAM_YACBAC1SDDAS=yes  포함


(4)  안면인식 관련 기능 빌드 환경

vendor/qcom/android-open/libcamera2/Android.mk 를 수정한다.
	3rd PARTY 솔루션 사용 여부를 결정한다.

	SKYCAM_FD_ENGINE := 0		미포함
	SKYCAM_FD_ENGINE := 1		올라웍스 솔루션 사용
	SKYCAM_FD_ENGINE := 2		기타 솔루션 사용

CUST_SKYCAM.h 에서 F_SKYCAM_ADD_CFG_FACE_FILTER 를 #undef 한다.
	안면인식 기능 관련 인터페이스 포함 여부를 결정한다.

libOlaEngine.so 를 기존 libcamera.so 에 링크하므로 기존 대비 libcamera.so 의
크기가 증가하여 링크 오류가 발생 가능하며, 이 경우 아래 파일들에서 
liboemcamera.so 의 영역을 줄여 libcamera.so 의 영역을 확보할 수 있다.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)	

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC                                                            */
/*  해당 모델에만 적용되는 또는 해당 모델에서만 검증된 FEATURE 목록           */
/*----------------------------------------------------------------------------*/
/*#define F_SKYCAM_TARGET_EF13*/
#define F_SKYCAM_TARGET_EF31
/*EF32에서 EF31과 달리 적용 되는 부분에 대한 정의.
 * EF32의 경우 EF31도 같이 define 하여 사용하여야 한다.
 * EF31의 경우에는 EF32를 define 하지 않는다. */
#ifndef MODEL_EF32K
#define MODEL_EF32K		0x3002
#endif
#if (MODEL_ID == MODEL_EF32K)
#define F_SKYCAM_TARGET_EF32
#endif

#ifdef F_SKYCAM_TARGET_EF13
#define F_SKYCAM_SENSOR_MT9P111
#define F_SKYCAM_SENSOR_YACBAC1SDDAS
#define F_SKYCAM_I2C_GPIO 
#endif

#ifdef F_SKYCAM_TARGET_EF31
#define F_SKYCAM_SENSOR_MT9T111
#define F_SKYCAM_SENSOR_YACBAC1SDDAS
#define F_SKYCAM_I2C_GPIO 

#ifdef F_SKYCAM_TARGET_EF32	/*EF32K만 적용된 feature*/
/* 
 * MEDIA_RECORDER_INFO_FILESIZE_PROGRESS CallBack, Only Use KT Phone 
 * KT폰에서는 비디오 파일 사이즈를 기록하는데 파일 사이즈를 APP에 알려주기 위해서
 * 추가 
 */
#define F_SKYCAM_ADD_EVT_RECSIZE

#else						/*EF31S만 적용된 feature*/

/* F_SKYCAM_TODO, SKT FOTA DCMO (Device Control Management Object)
 * SKT 향에만 적용되며, UI VOB에서만 define을 연다.
 * "pantech/development/sky_fota/sky_fota.h" 파일이 있어야 한다.
*/
#define F_SKYCAM_FOTA_DCMO_CAMERA_LOCK

#endif

#endif


/*
 * 카메라 드라이버가 어플이 종료되지 않았을 때, suspend 되는 것을 막는다.
 * power control 에 의해 커널 드라이버가 suspend 되는 것을 막는다.
 * 일반적인 경우 카메라 어플이 카메라 드라이버를 종료 시키며, 이 때 커널 드라이버도 내려간다.
 * HD 영상통화의 경우 조도 센서의 control이 불가능해 LCD가 OFF 되는 상황에서 suspend가 발생한다.
 * 이 때 커널 드라이버가 suspend 되지 않도록 한다.
*/
#define F_SKYCAM_FIX_SUSPENDLOCK_ADD

/*
 * pantech VT 메인카메라 실행시 90 rotate하게 수정
 * preview buffer는 수정하지 않으며 video buffer의 data를 preview buffer 기준으로
 * 90도 rotation 시킨다.
 * 해당 기능을 사용하기 위해 먼저 F_SKYCAM_CFG_VT에서 추가한 "pantech-vt"
 * parameter를 "on"으로 설정 하여야 한다.
*/
#define F_SKYCAM_VT_FIX_MAIN_CAM_ROT

/* 
 * pantech VT 카메라에 따른 적용을 위해 "pantech-vt" 파라미터를 추가 하였다.
 * "pantech-vt"를 "on"으로 설정 함에 따라 VT에서 main 카메라를 rotation 하는 부분과
 * sub camera의 video 버퍼를 flip 하지 않도록 한다.
*/
#define F_SKYCAM_CFG_VT

/*
 * pantech VT는 호가 연결되면 전송 버퍼를 video 버퍼로부터 얻기 위해 start recording
 * 을 시작하며 따라서 connect/disconnect 시에 촬영음이 발생한다.
 * pantech VT에서 촬영음이 발생하는 것을 막기 위해 CameraService에 
 * CAMERA_CMD_SET_SHUTTER_DISABLE commad를 추가 하였다.
*/
#define F_SKYCAM_VT_SHUTTER_DISABLE 


/*
 * 전면 카메라로 video recording을 하기 위해 video buffer에 LR flip을 적용한다.
 * Froyo 에서는 전면카메라의 경우 LR flip을 적용하여 입력받아 preview와 recording을
 * 그대로 적용 하였으나, Gingerbread에서는 display를 하기 위해 display쪽에 flip을 적용
 * 하므로, 카메라 입력은 normal로 받는다. 따라서 preview와 동일하게 recording을 하기
 * 위해서는 video buffer에 LR flip을 적용한다.
*/
//F_SKYCAM_FAKE_FRONT_CAMERA로 아래 두개 막음 
//#define F_SKYCAM_FRONT_CAM_VBUF_LR
//#define F_SKYCAM_USE_IPLLIB

/*
 * preview size 와 capture size를 정의한다.
 * MSM7x27 에서 preview size와 capture size를 세로가 긴 size도 가능하도록 한다.
*/
#define F_SKYCAM_PORTRAIT_PREVIEW_CAPTURE

/* 카메라 튜닝 파일 로드를 위해...*/
/*#define F_SKYCAM_SENSOR_TUNEUP
*/
#ifdef F_SKYCAM_SENSOR_TUNEUP
#define F_SKYCAM_TUP_LOAD_FILE
#ifdef F_SKYCAM_SENSOR_MT9P111
#define MT9P111_TUP_FNAME       "/sdcard/EF31_5M.txt"
#endif
#ifdef F_SKYCAM_SENSOR_MT9T111
#define MT9T111_TUP_FNAME       "/sdcard/EF31_3M.txt"
#endif
#ifdef F_SKYCAM_SENSOR_YACBAC1SDDAS
#define YACBAC1SDDAS_TUP_FNAME "/sdcard/EF31_VT.txt"
#endif
#endif

/*----------------------------------------------------------------------------*/
/*  MODEL-COMMON                                                              */
/*  안드로이드 기반 모든 모델에 공통 적용되어야 할 FEATURE 목록               */
/*----------------------------------------------------------------------------*/


/* 내수 CS 부서에서는 소비자 시료 분석을 위해 별도 PC 프로그램을 사용하여 
 * 카메라 구동 시간 정보를 PC 로 추출한다. 
 *
 * 구현 방법은 공정 커맨드 사양서에 명시되어 있으므로 관련 코드들은 공정 커맨드 
 * 관련 모듈에 포함되어 있으나, 공정 커맨드 용 PC 프로그램을 사용하지 않고 별도
 * 프로그램을 사용하여, 시료의 DIAG 포트로부터 구동 시간 정보를 확인할 수 있다.
 *
 * 공정 커맨드 사양서 v10.35 기반 구현
 * PhoneInfoDisplay v4.0 프로그램으로 확인
 * 사양서와 프로그램은 DS2팀 박경호 선임에게 문의 */
#define F_SKYCAM_FACTORY_PROC_CMD


/* 카메라 장치 파일 OPEN 에 실패한 경우 (단순 I2C 버스 R/W 오류, 카메라 미장착) 
 * 오류 처리를 위해 수정한다. 
 *
 * 장치 파일을 OPEN 하는 과정에서 VFE 초기화 이후 카메라 HW 초기화가 이루어 
 * 지는데, HW 초기화에 실패할 경우 VFE 는 초기화 된 상태로 남게되고 다음
 * OPEN 시도 시 HW 초기화에 성공한다 하더라도 이미 VFE 가 초기화된 상태이므로 
 * VFE 초기화 시 오류가 발생한다.
 * 
 * 호출순서 : vfefn.vfe_init() -> sctrl.s_init()
 *
 * HW 초기화에 실패할 경우, 이미 초기화된 VFE 를 RELEASE (vfe_release) 시켜 
 * 다음 열기 시도 시 정상 동작하도록 수정한다. 
 *
 * ECLAIR 버전에서는 위와 같은 에러 처리에도 불구하고 센서가 연결되어 있지
 * 않거나 센서 하드웨어에 이상이 발생한 경우 카메라 응용이 ANR 오류로 인해 
 * 비정상 종료되고 이후 재부팅 전까지는 지속하여 재진입이 불가능하다.
 *
 * 센서가 비 정상인 경우, ISP 초기화 시 ISP 와 센서를 프리뷰 모드로 설정하는 
 * 과정에서 3초 간 POLLING 수행하며, 이로 인해 타임아웃 발생하고 ANR 오류로 
 * 이어진다. 비 정상 종료 이후 카메라 재진입 시 센서가 정상이라 하더라도 ANR 
 * 오류 이후 응용이 비 정상적으로 종료되었으므로 FRAMEWORK 내부는 비 정상인 
 * 상태로 유지되고, 이로 인해 재부팅 전까지는 카메라 응용 진입 시 "Unable to 
 * connect camera device" 팝업과 함께 무조건 진입에 실패한다.
 *
 * ISP 초기화 시 프리뷰 모드 설정 이전에, ISP 를 통해 센서의 특정 레지스터를 
 * 1회 READ 하고 비 정상일 경우, 이를 FRAMWORK 을 통해 응용으로 전달하여 
 * 정상적으로 종료되도록 수정한다. 
 *
 * 또한 ISP 자체에 이상이 발생한 경우에도, PROBE 시에 오류 발생하여 해당 
 * 디바이스 파일들을 생성할 수 없으므로 FRAMWORK 내부에서 함께 처리 가능하다. 
 *
 * EF10S 의 경우, BAYER 센서만 커넥터로 연결되어 있고, MV9337 은 ON-BOARD
 * 되어 있으므로, BAYER 센서가 연결되어 있지 않아도, MV9337 만 정상이라면,
 * PROBE 시 정상 동작하였으나, EF12S 의 경우, 카메라 모듈에 MV9335 가 함께
 * 인스톨되어 있어, 커넥터에 모듈이 연결되지 않으면 PROBE 시 I2C R/W 실패가
 * 지속 발생, RETRY 수행하면서 부팅 시간이 10초 이상 지연되고, 이로 인해
 * 다른 모듈들의 초기화에 직접적인 영향을 미친다. */
#define F_SKYCAM_INVALIDATE_CAMERA_CLIENT


/* 단말에서 촬영된 사진의 EXIF TAG 정보 중 제조사 관련 정보를 수정한다. */
#define F_SKYCAM_OEM_EXIF_TAG


/* 지원 가능한 촬영 해상도 테이블을 수정한다. 
 *
 * HAL 에서는 유효한 촬영 해상도를 테이블 형태로 관리하고 테이블에 포함된 
 * 해상도 이외의 설정 요청은 오류로 처리한다. */
#define F_SKYCAM_CUST_PICTURE_SIZES


/* 지원 가능한 프리뷰 해상도 테이블을 수정한다. 
 *
 * HAL 에서는 유효한 프리뷰 해상도를 테이블 형태로 관리하고 테이블에 포함된 
 * 해상도 이외의 설정 요청은 오류로 처리한다. */
#define F_SKYCAM_CUST_PREVIEW_SIZES


/* 카메라 IOCTL 커맨드 MSM_CAM_IOCTL_SENSOR_IO_CFG 를 확장한다. 
 *
 * 추가한 기능 및 SOC 카메라를 감안하지 않고 미구현된 부분들을 수정 및 추가 
 * 구현한다. */
#define F_SKYCAM_CUST_MSM_CAMERA_CFG


/* 리눅스 테스트 응용 (/system/bin/mm-qcamera-test) 중 필요 부분을 수정한다. 
 *
 * 수정이 완벽하지 않으므로 일부 기능은 정상 동작을 보장하지 못한다.
 * 안드로이드 플랫폼이 초기화되기 직전에 adb shell stop 명령으로 시스템을 
 * 중지시키고, adb shell 을 통해 접근 후 실행한다. */
#define F_SKYCAM_CUST_LINUX_APP


/* "ro.product.device" 설정 값 변경으로 인한 코드 오동작을 막기 위해 카메라
 * 관련 코드들에서는 이 값을 읽지 않거나 'TARGET_XXXXXXX' 으로 고정한다. 
 * 릴리즈 버전의 경우 이 값은 "xxxxxx_surf" 이다. */
/*#define F_SKYCAM_CUST_TARGET_TYPE_8X50*/
#define F_SKYCAM_CUST_TARGET_TYPE_7X27


/* SKAF 의 경우, 국내 제조사들에 공통 적용되는 내용이므로 QUALCOMM JPEG 성능 
 * 향상 기능을 사용하지 않도록 수정한다. (DS7팀 요청사항)
 *
 * SKAF 의 경우, SKIA 라이브러리의 표준 라이브러리 헤더 'jpeglib.h' 를
 * 수정 없이 사용하고, QUALCOMM 은 JPEG 성능 향상을 위해 'jpeglib.h' 파일의 
 * 'jpeg_decompress_struct' 구조체에 'struct jpeg_qc_routines * qcroutines'
 * 멤버를 추가하였다. 이로 인해 'jpeg_CreateDecompress()' 실행 시 구조체 크기 
 * 비교 부분에서 오류가 발생하고 해당 디코드 세션은 실패한다.
 *
 * QUALCOMM JPEG 성능 향상은 SNAPDRAGON 코어에서만 지원되고, 2MP 이상의 
 * JPEG 파일 디코드 시 18~32% 의 성능이 향상된다. 
 * (android/external/jpeg/Android.mk, R4070 release note 'D.5 LIBJPEG' 참고) 
 * F_SKYCAM_TODO, QUALCOMM 은 왜 공용 구조체를 수정했는가? 별도 구조체로 
 * 분리하여 구현했다면, 이런 문제는 피할 수 있다. 
 *
 * FROYO (R5025) 에서 삭제되었다. */
/* #define F_SKYCAM_CUST_QC_JPG_OPTIMIZATION */



/*----------------------------------------------------------------------------*/
/*  SENSOR CONFIGURATION                                                      */
/*  모델 별 사용 센서(ISP)를 위해 수정/추가된 FEATURE 목록                    */
/*----------------------------------------------------------------------------*/


/* 카메라의 개수에 상관 없이 오직 SOC 카메라(들)만 사용할 경우 선언한다.
 *
 * 영상통화를 위해 두 개의 카메라를 사용하고, 하나는 SOC 타입, 다른 하나는
 * BAYER 타입인 경우에는 선언하지 않는다. 선언할 경우, BAYER 카메라를 위한
 * 일부 코드들이 실행되지 않는다.
 *
 * EF10S/EF12S 에서는 하나의 SOC 카메라만 사용하므로, BAYER 관련 코드들을 
 * 검증하지 않았고, 일부는 아래 FEATURE 들을 사용하여 주석 처리하였다. */
#define F_SKYCAM_YUV_SENSOR_ONLY


#ifdef F_SKYCAM_YUV_SENSOR_ONLY

/* ISP 자체에서 지원 센서 ZOOM 을 설정하기 위한 인터페이스를 추가한다. 
 * EF10S/EF12S 에서는 QUALCOMM ZOOM 을 사용하며, 참고용으로 코드들은 남겨둔다.
 *
 * ISP 자체 ZOOM 의 경우, 프리뷰/스냅샷 모드에서 이미 ZOOM 이 적용된 이미지가 
 * 출력되며 두 가지 모드를 지원한다.
 *
 * 1) DIGITAL (SUBSAMPLE & RESIZE)
 *     프리뷰/스냅샷 해상도별로 동일한 배율을 지원한다. ISP 내부에서 
 *     이미지를 SUBSAMPLE 하여 RESIZE 후 출력하며, 이로 인해 ZOOM 레벨이
 *     0 이 아닌 값으로 설정된 경우 프리뷰 FPS 가 1/2 로 감소된다.
 * 2) SUBSAMPLE ONLY
 *     프리뷰/스냅샷 해상도별로 상이한 배율을 지원한다. ISP 내부에서 
 *     SUBSAMPLE 만 적용하므로 낮은 해상도에서는 높은 배율을 지원하고 최대 
 *     해상도에서는 ZOOM 자체가 불가능하다. 프리뷰 FPS 는 감소되지 않는다.
 *
 * QUALCOMM ZOOM 적용 시, 카메라의 경우 모든 해상도에서 동일 배율 ZOOM 이 
 * 가능하므로 이를 사용하며, 추후 참고를 위해 해당 코드들은 남겨둔다. 
 *
 * 관련 FEATURE : F_SKYCAM_FIX_CFG_ZOOM, F_SKYCAM_ADD_CFG_DIMENSION */
/* #define F_SKYCAM_ADD_CFG_SZOOM */


/* ISP 에서 지원되는 손떨림 보정 기능 (Digital Image Stabilization) 을 위한
 * 인터페이스를 추가한다. 
 *
 * 상하/좌우 일정 패턴으로 흔들릴 경우만 보정 가능하다. 
 * 장면 모드를 OFF 이외의 모드로 설정할 경우, 기존 손떨림 보정 설정은 
 * 무시된다. */
#define F_SKYCAM_ADD_CFG_ANTISHAKE


/* AF WINDOW 설정을 위한 인터페이스를 수정한다. SPOT FOCUS 기능 구현 시 
 * 사용한다.
 *
 * ISP 에서는 프리뷰 모드 출력 해상도를 기준으로 가로/세로를 각각 16 개의 
 * 구간으로 총 256 개 블럭으로 나누어 블럭 단위로 AF WINDOW 설정이 가능하다. 
 * 응용에서는 프리뷰 해상도를 기준으로 사용자가 터치한 지점의 좌표를 HAL 로 
 * 전달하고, HAL 에서는 이를 블럭 좌표로 변환하여 ISP 에 설정한다. 
 * 이후 AF 수행 시 이 WINDOW 에 포함된 이미지에서만 FOCUS VALUE 를 측정하여
 * 렌즈의 위치를 결정한다.
 *
 * ISP 의 출력은 고정된 상태에서 QUALCOMM ZOOM 을 사용하여 SUBSAMPLE/RESIZE
 * 하기 때문에 ZOOM 이 0 레벨 이상으로 설정된 경우, HAL 에서 좌표-to-블록
 * 변환식이 복잡해지고, 특정 ZOOM 레벨 이상일 경우 몇 개의 블록 안에 전체
 * 프리뷰 영역이 포함되어 버리므로 설정 자체가 의미가 없다.
 * 그러므로, 응용은 SPOT FOCUS 기능 사용 시에는 ZOOM 기능을 사용할 수 없도록 
 * 처리 해야한다. */
#define F_SKYCAM_FIX_CFG_FOCUS_RECT


/* QUALCOMM BAYER 솔루션 기반의 화이트밸런스 설정 인터페이스를 수정한다. 
 *
 * 장면 모드를 OFF 이외의 모드로 설정할 경우, 기존 화이트밸런스 설정은 
 * 무시된다. */
#define F_SKYCAM_FIX_CFG_WB


/* QUALCOMM BAYER 솔루션 기반의 노출 설정 인터페이스를 수정한다. 
 *
 * 장면 모드를 OFF 이외의 모드로 설정할 경우, 기존 노출 설정은 무시된다. */
#define F_SKYCAM_FIX_CFG_EXPOSURE


/* 장면 모드 설정을 위한 인터페이스를 추가한다. 
 *
 * 장면 모드를 OFF 이외의 값으로 설정할 경우 기존 화이트밸런스/측광/손떨림보정/
 * ISO 설정은 무시된다. 응용에서 장면 모드를 다시 OFF 로 초기화 하는 경우, 
 * 화이트밸런스/측광/손떨림보정/ISO 는 HAL 에서 기존 설정대로 자동 복구되므로,
 * 응용은 복구할 필요 없다. (HW 제약사항이므로, HAL 에서 제어한다.) */
#define F_SKYCAM_FIX_CFG_SCENE_MODE


/* 플리커 설정을 위한 인터페이스를 수정한다.
 *
 * 2.1 SDK 에는 총 네 가지 모드 (OFF/50Hz/60Hz/AUTO) 가 명시되어 있으나, 
 * ISP 의 경우 OFF/AUTO 가 지원되지 않는다. 그러므로, 응용이 OFF 로 설정 
 * 시에는 커널 드라이버에서 60Hz 로 설정하고, AUTO 로 설정할 경우 HAL 에서 
 * 시스템 설정 값 중 국가 코드 ("gsm.operator.numeric", 앞 3자리 숫자) 를 읽고, 
 * 국가별 Hz 값으로 변환하여 해당 값으로 설정한다.
 *
 * 기획팀 문의 결과, 플리커는 일반적인 기능이 아니므로, 국가 코드를 인식하여 
 * 자동으로 설정할 수 있도록 하고, 수동 설정 메뉴는 삭제 처리한다. */
#define F_SKYCAM_FIX_CFG_ANTIBANDING

#ifdef F_SKYCAM_TARGET_EF13
/* 플래쉬 LED 설정을 위한 인터페이스를 수정한다.
 *
 * QUALCOMM 에서는 별도의 IOCTL (MSM_CAM_IOCTL_FLASH_LED_CFG) 커맨드를 
 * 사용하여 구현되어 있으며, PMIC 전원을 사용하는 LED 드라이버를 제공한다.
 * EF10S/EF12S 에서는 이를 사용하지 않으며, MSM_CAM_IOCTL_SENSOR_IO_CFG 을 
 * 확장하여 구현한다.
 *
 * AUTO 모드로 설정할 경우, 저조도 일 경우에만 AF 수행 중 AF/AE 를 위해
 * 잠시 ON 되고, 실제 스냅샷 시점에서 한 번 더 ON 된다. */
#define F_SKYCAM_FIX_CFG_LED_MODE
#endif

/* ISO 설정을 위한 인터페이스를 수정한다.
 *
 * 기획팀 문의 결과, AUTO 모드에서의 화질에 큰 이상이 없으므로 수동으로
 * ISO 를 변경할 수 있는 메뉴는 삭제 처리한다.
 * 장면 모드를 OFF 이외의 모드로 설정할 경우, 기존 ISO 설정은 무시된다. */
/*#define F_SKYCAM_FIX_CFG_ISO*/

/* 특수효과 설정을 위한 인터페이스를 수정한다.
 *
 * SDK 2.1 에 명시된 효과들 중 일부만 지원한다. MV9337/MV9335 의 경우 SDK 에
 * 명시되지 않은 효과들도 지원하지만 응용에서 사용하지 않으므로 별도 추가는
 * 하지 않는다. */
#define F_SKYCAM_FIX_CFG_EFFECT


/* MANUAL FOCUS 설정을 위한 인터페이스를 수정한다. 
 *
 * MANUAL FOCUS 모드로 설정할 경우, 즉 응용이 임의로 렌즈의 위치를
 * 이동시킬 경우, 스냅샷 직전에 AF 가 수행되지 않는 것이 정상이며, 이로 인해 
 * ISP 에서 조도를 측정하여 보정할 수 있는 시간이 없으므로 플래쉬가 ON 될
 * 경우, 프리뷰/스냅샷 이미지가 상이할 수 있다. 그러므로 응용은 MANUAL FOCUS 
 * 모드 진입 시 반드시 플래쉬 모드를 OFF 로 설정해야 하고, AUTO FOCUS 또는 
 * SPOT FOCUS 모드 진입 시 원래 모드로 복구시켜야 한다. */
#define F_SKYCAM_FIX_CFG_FOCUS_STEP


/* 밝기 설정을 위한 인터페이스를 수정한다. */
#define F_SKYCAM_FIX_CFG_BRIGHTNESS


/* LENS SHADE 설정을 위한 인터페이스를 수정한다. 
 *
 * MV9337/MV9335 의 경우 별도의 LENS SHADE 기능을 지원하지 않고, 
 * 프리뷰/스냅샷 시 항상 LENS SHADE 가 적용된 이미지를 출력한다. */
/*#define F_SKYCAM_FIX_CFG_LENSSHADE*/


/* 프리뷰 회전각 설정을 위한 인터페이스를 수정한다.
 *
 * 스냅샷 이후 JPEG 인코드 시와 안면인식 필터 (셀프샷) 적용 시 카메라의 
 * 회전 상태를 입력하여야 한다. 응용은 OrientationListener 등록 후 아래와 같은
 * 시점에 HAL 에 회전각 값을 설정 해주어야 한다.
 * 
 * JPEG 인코드
 *     인코드 직전에 설정
 * 셀프샷 모드
 *     변경 시 매번 설정
*/
#define F_SKYCAM_FIX_CFG_ROTATION


/* AF 동작을 위한 인터페이스를 수정한다. 
 *
 * QUICK SEARCH 알고리즘을 사용하며, 렌즈 이동 시마다 FOCUS VALUE 를 측정하고,
 * 연속해서 FOCUS VALUE 가 감소할 경우, 가장 최근에 최고의 FOCUS VALUE 값을
 * 갖는 위치로 렌즈를 이동시킨다.
 * MV9337/MV9335 의 경우, 무한거리의 피사체를 촬영할 경우 렌즈를 매크로 모드까지
 * 이동시키지 않으므로 넥서스원 대비 AF 속도가 우수하다. */
#define F_SKYCAM_FIX_CFG_AF

/* spot select focus 
위치 지정하여 터치시 동작하도록 추가 수정
ui에서는 'normal' 과 'spot' 스트링을 이용하여 AF모드 선택 
EF31S 32K는 rect의 x -1, y -1 일때 center적용되도록 한다. 
autofocus모드가 rect이후에 와서 focus mode의 의미가 없다.
*/
#define F_SKYCAM_CFG_AF_SPOT

#define F_SKYCAM_FIX_CFG_REFLECT


/* ZOOM 설정을 위한 인터페이스를 수정한다.
 *
 * QUALCOMM ZOOM 의 경우, 최대 ZOOM 레벨은 HAL 이하에서 프리뷰/스냅샷 해상도에 
 * 따라 결정되며, 응용은 이 값 ("max-zoom") 을 읽어 최소/최대 ZOOM 레벨을
 * 결정하고 그 범위 안의 값들로만 설정해야 한다.
 * 만약 해상도/모델에 관계없이 일정한 ZOOM 레벨 범위를 제공해야 할 경우, HAL
 * 에서는 "max-zoom" 값을 일정 ZOOM 레벨 범위로 나누어 설정할 수 있어야 한다.
 *
 * 관련 FEATURE : F_SKYCAM_ADD_CFG_SZOOM, F_SKYCAM_ADD_CFG_DIMENSION */
/* #define F_SKYCAM_FIX_CFG_ZOOM */


#endif /* F_SKYCAM_YUV_SENSOR_ONLY */


/* 안면인식 기반 이미지 필터 설정을 위한 인터페이스를 추가한다.
 *
 * EF10S/EF12S 에서는 올라웍스 솔루션을 사용하며, 프리뷰/스냅샷 이미지에서 
 * 얼굴 위치를 검출하여 얼굴 영역에 마스크나 특수효과를 적용할 수 있다. 
 *
 * vendor/qcom/android-open/libcamera2/Android.mk 에서 올라웍스 라이브러리를
 * 링크시켜야만 동작한다. SKYCAM_FD_ENGINE 를 1 로 설정할 경우 올라웍스 
 * 솔루션을 사용하고, 0 으로 설정할 경우 카메라 파라미터만 추가된 상태로 관련 
 * 기능들은 동작하지 않는다. 다른 솔루션을 사용할 경우 이 값을 확장하여 
 * 사용한다. */
#define F_SKYCAM_ADD_CFG_FACE_FILTER


/* ISP 의 출력 해상도를 설정할 수 있는 인터페이스를 추가한다.
 *
 * QUALCOMM ZOOM 기능은 VFE/CAMIF SUBSAMPLE 과 MDP RESIZE 를 사용하며, 모든
 * 프리뷰/스냅샷 해상도에 대해 동일한 ZOOM 배율을 지원한다. 이를 위해 ISP 의
 * 출력 해상도를 프리뷰/스냅샷 모드에서 각각 별도의 값으로 고정하고 
 * VFE/CAMIF/MDP 에서 후처리 (SUBSAMPLE/RESIZE) 하는 구조이다.
 *
 * 그러나, ISP 의 ZOOM 기능은 출력 자체가 ZOOM 이 적용되어 출력되므로, 
 * ISP 자체 출력을 응용에서 요구하는 해상도로 설정하고 VFE/CAMIF/MDP 
 * 기능들은 모두 DISABLE 시켜야 한다.
 *
 * EF10S/EF12S 에서는 사용하지 않으며 참고용으로 코드들은 남겨둔다.
 * 관련 FEATURE : F_SKYCAM_FIX_CFG_ZOOM, F_SKYCAM_ADD_CFG_SZOOM */
/* #define F_SKYCAM_ADD_CFG_DIMENSION */


/* 센서 인스톨 방향 설정을 위한 인터페이스를 수정한다.
 *
 * EF10S/EF12S 의 경우 응용에서 UI 를 가로모드로 고정하여 사용하며, HAL 에서 
 * 연동하여 처리해야 할 부분이 있을 경우 이 값을 사용해야 한다.
 *
 * F_SKYCAM_TODO 7K 에서는 해당 인터페이스 추가되었으며 확인 필요하다.
 * EF10S/EF12S 에서는 실제 사용되는 부분은 없으나 참고용으로 코드들은 남겨둔다. */
#define F_SKYCAM_FIX_CFG_ORIENTATION


/* 프리뷰 FPS 설정을 위한 인터페이스를 변경한다. 
 *
 * 1 ~ 30 까지 설정 가능하며 의미는 다음과 같다.
 *
 * 5 ~ 29 : fixed fps (조도에 관계 없이 고정) ==> 캠코더 프리뷰 시 사용
 * 30 : 8 ~ 30 variable fps (조도에 따라 자동 조절) ==> 카메라 프리뷰 시 사용
 *
 * MV9337/MV9335 은 프리뷰 모드에서 고정 1 ~ 30 프레임과 변동 8 ~ 30 프레임을 
 * 지원하며, EF10S/EF12S 에서는 동영상 녹화 시 24fps (QVGA MMS 의 경우 15fps) 으로
 * 설정하고, 카메라 프리뷰 시에는 가변 8 ~ 30fps 으로 설정한다. */
#define F_SKYCAM_FIX_CFG_PREVIEW_FPS


/* 멀티샷 설정을 위한 인터페이스를 추가한다.
 * 
 * 연속촬영, 분할촬영 4컷/9컷 모드에서 매 촬영 시마다 센서 모드를 변경하는 
 * 것을 방지한다. 첫 번째 촬영에서 센서를 스냅샷 모드로 변경하고, VFE/CAMIF
 * 설정까지 완료된 상태이므로, 두 번째 촬영부터는 스냅샷/썸네일/JPEG 버퍼만
 * 초기화하고 VFE/CAMIF 에 스냅샷 명령만 송신한다.
 *
 * 제약사항 : 저조도에서 플래쉬 모드를 자동으로 설정하고 연속촬영, 분할촬영 
 * 4컷/9컷 모드로 설정 후 촬영 시, 첫 번째 촬영에서만 플래쉬가 ON 되고, 
 * 두 번째부터는 ON 되지 않으므로 연속촬영, 분할촬영 4컷/9컷 모드에서는
 * 응용이 반드시 플래쉬를 OFF 해야하며, 이외 모드로 설정 시 이전 설정을
 * 스스로 복구시켜야 한다. */
#define F_SKYCAM_ADD_CFG_MULTISHOT


/* 다이나믹 클럭 적용시에 항상 카메라에서는 항상 MAX 클럭으로 사용한다.
 */
#define F_SKYCAM_CUST_CPU_CLK_FREQ

/* SKYCAM_PSJ_100817
 * OTP모듈의 불량률을 줄이기 위해 메모리 zone의 2개를 detecting하여 
 * 현재 사용해야 될 zone을 찾아낸 후 해당 zone의 메모리를 사용 할 수 있도록 추가
*/
#define F_SKYCAM_ADD_CFG_OTP_MODULE


/*  Camcoder일 경우 Continuous AF를 ON, OFF를 할 수 있다
 */
#define F_SKYCAM_ADD_CFG_CAF


/* Pantech의 MIRROR 어플리케이션을 위해 Preview 를 CROP 한다.
 * MIRROR 어플리케이션은 거울 기능으로 전면 카메라 PREVIEW만 가능하다.
 * PREVIEW 설정은 640x480 으로 제한하며, 3:4 비율인 360x480으로 한다.
 * EF31 에서는 UI에서 320x426 으로 Surface를 설정하여 preview 한다.
 * pantech-mirror-crop 을 "on" 으로 설정하고, pewview size 640x480 인 경우로 제한한다.
*/
#define F_SKYCAM_ADD_CFG_MIRROR_CROP

#ifdef F_SKYCAM_ADD_CFG_FACE_FILTER
/* 안면인식 솔루션 사용 시, 검출된 다수의 얼굴들에 선택적으로 필터를 적용할
 * 수 있는 인터페이스를 추가한다.
 *
 * 프리뷰 이미지 상 다수의 얼굴 영역들을 검출하고 해당 영역을 터치할 경우
 * 해당 얼굴 영역에 필터를 적용할 지 여부를 ON/OFF 방식으로 선택할 수 있다.
 *
*/
#define F_SKYCAM_ADD_CFG_FACE_FILTER_RECT
#endif


/* 
 * Gingerbread에서 OpenCore에서 Stagefright를 사용하도록 수정 되었다.
 * EF31S와 EF32K에서 비디오 recording시 stagefright 사용이 문제가 되어
 * 우선 OpenCore를 사용하도록 한다.
 * Stagefright에 대해서는 디버깅 진행 후 적용 한다.
*/
#define F_SKYCAM_CFG_OPENCORE


/* 
 * Gingerbread의 CameraService에서 lockIfMessageWanted(int32_t msgType) 함수가 추가 되었다.
 * CameraService의 callback 처리 함수에서 mLock을 check하여 LOCK 상태이면, UNLOCK까지 waiting 한다.
 * capture 수행 도중 UI 로부터 command가 내려오면 callback 함수에서 이로 인해 지연이 발생한다.
 * capture 수행 중 카메라 종료시 이로 인해 CameraHAL보다 UI가 먼저 종료 되는 경우가 발생한다.
 * UI가 먼저 종료되고 CameraHAL 종료전에 다시 Camera가 실행되면 정상적으로 Open 하지 못한다.
 * lockIfMessageWanted 함수를 사용 하지 않도록 수정하였다.
*/
#define F_SKYCAM_FIX_CS_TRYLOCK

/*
camera id별로 검색하여 각각 app에서 후면 카메라, 전면 카메라 따로 동작시 진입 가능하게 되어
진입시 open이 비슷한 시기에 되거나(홈키 long 키, 전환), setparameter가 셋팅되는 현상등이 발생하여,
전혀 의도하지 않은 값이 써져 오동작 하는 문제로
froyo와 마찬가지로 전 후면 모든 카메라가 이전 카메라 release 이전에는 진입 불가하도록 수정

HW, QCH 모두 개별의 카메라 동작을 지원한다면 아래를 제거한 후 테스트 할 것.
*/
#define F_SKYCAM_FIX_CS_CONNECT


#define F_SKYCAM_FIX_PREVIEW_ZOOM

#define F_SKYCAM_FIX_PREVIEW_FACE_FILTER

/* CTS qualcomm bug 수정 
 */
#define F_SKYCAM_FIX_CFG_FOCUS_DISTANCES

#define F_SKYCAM_FIX_CFG_FPS_RANGE

#define F_SKYCAM_FIX_CANCEL_AF

#define F_SKYCAM_ADD_CFG_READ_REG

/*촬영음을 재생을 캡쳐 이전에서 캡쳐 이후 JPEG 인코딩 이전으로 변경*/
#define F_SKYCAM_FIX_NOTIFYSHUTTER

/*카메라 재 진입시 이전에 비정상 종료 등의 이유로 카메라 드라이버가 정상 종료 되지 않았을때
를 위한 방어 코드 - 재 진입시 드라이버 비정상 상태이면, 현재 카메라 reject하고 driver 종료*/
#define F_SKYCAM_DEVICE_CONNECT_FAIL_FIXED

/* vfe가 비 정상 상태 일 때, 카메라 종료시 vfe에 잘못 된 명령을 보내 stop등의 명령 처리시 
config_proc.c의 *((int *)(0xc0debadd)) = 0xdeadbeef; 비정상 처리로 인한 mediaserver 죽는 문제에
대한 방어 코드 적용 */
#define F_SKYCAM_DEADBEEF_ERROR_FIX

/*#define F_SKYCAM_HIDDEN_FRONT_CAMERA*/
#define F_SKYCAM_FAKE_FRONT_CAMERA

/*----------------------------------------------------------------------------*/
/*  MISCELLANEOUS / QUALCOMM BUG                                              */
/*  모델 기능 구현/수정과 특별히 관계가 없는 FEATURE 들과 QUALCOMM 버그에     */
/*  대한 임시 수정, SBA 적용 등에 대한 FEATURE 목록                           */
/*----------------------------------------------------------------------------*/


/* 커널 영역 코드에서 SKYCDBG/SKYCERR 매크로를 사용하여 출력되는 메시지들을
 * 활성화 시킨다. 
 * kernel/arch/arm/mach-msm/include/mach/camera.h */
/*#define F_SKYCAM_LOG_PRINTK*/


/* 현재 개발 중이거나 추후 검토가 필요한 내용, 또는 타 모델에서 검토가 필요한
 * 내용들을 마킹하기 위해 사용한다. */
#define F_SKYCAM_TODO

/* 유저 영역(vendor)의 SKYCDBG/SKYCERR 메세지 on off */
#define F_SKYCAM_LOG_OEM		

/* 유저 영역의 LOGV/LOGD/LOGI 메세지 on off */
#define F_SKYCAM_LOG_VERBOSE

/* QUALCOMM 릴리즈 코드에 디버그 용 메시지를 추가한다. */
#define F_SKYCAM_LOG_DEBUG

/*
 * 비디오 레코딩 시작과 종료 음이 끊기는 현상에 대한 디버깅
 * 효과음 재생 중 시스템 부하로 인해 소리가 끊기는 경우 발생
 * 레코딩 시작음 start 이 후 시작음 종료 될 때까지 wating
 * EF31S/EF32K 에서는 Sound쪽 kernel message가 나오는 경우 소리가 끊기며,
 * kernel message를 막거나 release build에서는 현상 발생 안함.
*/
#define F_SKYCAM_QBUG_REC_BEEP_SOUND


/* 1600x1200, 1600x960 해상도에서 "zoom" 파라미터를 21 로 설정 후 스냅샷 시
 * 썸네일 YUV 이미지를 CROP 하는 과정에서 발생하는 BUFFER OVERRUN 문제를 
 * 임시 수정한다. 
 *
 * QualcommCameraHardware::receiveRawPicture() 에서 crop_yuv420(thumbnail) 
 * 호출 시 파라미터는 width=512, height=384, cropped_width=504, 
 * cropped_height=380 이며 memcpy 도중에 SOURCE 주소보다 DESTINATION 주소가 
 * 더 커지는 상황이 발생한다.
 *
 * R5040 에서 QUALCOMM 수정 확인 후 삭제한다. (SR#308616) 
 * - R407705 FIXED ISSUE 6 번 참조 */
/* #define F_SKYCAM_QBUG_ZOOM_CAUSE_BUFFER_OVERRUN */


/* 모든 해상도에서 ZOOM 을 특정 레벨 이상으로 설정할 경우, 
 * EXIFTAGID_EXIF_PIXEL_X_DIMENSION, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION 태그
 * 정보에 잘못된 값이 저장되는 문제를 임시 수정한다.
 *
 * R5040 에서 QUALCOMM 수정 확인 후 삭제한다. (SR#307343) 
 * - R4077 FIXED ISSUE 12 번 참조 
 * - vendor/qcom/proprietary/mm-still/jpeg/src/jpeg_writer.c 의 기존 임시 수정
 *   코드는 구조가 변경되어 삭제 */
/* #define F_SKYCAM_QBUG_EXIF_IMAGE_WIDTH_HEIGHT */


/* SKYCAM_PSJ_100610
 * 촬영이 종료되지 않은 상황에서 Stop preview를 하여 메모리가 해제되는 상황 방지
*/
#define F_SKYCAM_QBUG_SNAPSHOT_RELEASE_INTERRUPT

/* 동영상 녹화 시작/종료를 빠르게 반복하거나, 이어잭을 장착한 상태에서 연속촬영
 * 모드로 촬영할 경우, MediaPlayer 가 오동작하면서 HALT 발생한다.
 *
 * MediaPlayer 의 경우, 동일한 음원을 재생 중에 또 다시 재생 시도할 경우 100%
 * 오동작하므로 동일 음원을 연속하여 재생해야 할 경우, 반드시 이전 재생이 완료
 * 되었는지 여부를 확인 후 재생해야 한다. */
#define F_SKYCAM_QBUG_SKIP_CAMERA_SOUND

/*
 * zoom 설정 후 snapshot jpeg encode에서 발생하는 영상 밀팅 현상 수정 
 */
#define F_SKYCAM_QBUG_JPEG_ZOOM

/* 촬영음/녹화음 재생 중에 응용이 종료될 경우, CLIENT 소멸 시에 해당 촬영음/
 * 녹화음 오브젝트가 강제로 disconnect/clear 되면서 MEDIA SERVER 가 죽는 것을
 * 방지한다. */
#define F_SKYCAM_QBUG_STOP_CAMERA_SOUND


/* AF 가 정상적으로 수행되더라도 HAL 에서 반환 값 체크 시 잘못된 값을 사용하므로
 * 응용은 항상 AF 실패로 인식하는 문제를 수정한다. */
#define F_SKYCAM_QBUG_SET_PARM_AUTO_FOCUS


/* capture 후 postview 하기전 black screen, postview 후 preview 하기전 black screen 막음.
*/
#define F_SKYCAM_QBUG_ADD_UNREGISTER2

/* CTS qualcomm bug 수정 
 */
#define F_SKYCAM_QBUG_CTS


/* 비디오 레코딩 중 SDCARD FULL 이 된 경우에 정상적으로 레코딩 파일이 만들어지지 않는 경우 발생
*/
#define F_SKYCAM_QBUG_RECORDING_MAXSIZE
/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                  */
/*  모델 관련 상수 선언                                                       */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_SKYCAM_SELECT_MAIN_CAM 		0
#define C_SKYCAM_SELECT_SUB_CAM 		1


/* 카메라 동작 시간을 저장하기 위한 데이터 파일명이다. */
#ifdef F_SKYCAM_FACTORY_PROC_CMD
#define C_SKYCAM_FNAME_FACTORY_PROC_CMD	"/data/.factorycamera.dat"
#endif


/* 설정 가능한 최소/최대 포커스 레벨이다. */
#ifdef F_SKYCAM_FIX_CFG_FOCUS_STEP
#define C_SKYCAM_MIN_FOCUS_STEP 0	/* 무한대 (default) */
#define C_SKYCAM_MAX_FOCUS_STEP 9	/* 매크로 */
#endif


/* 설정 가능한 최소/최대 밝기 단계이다. */
#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
#define C_SKYCAM_MIN_BRIGHTNESS 0	/* -4 */
#define C_SKYCAM_MAX_BRIGHTNESS 8	/* +4 */
#endif


/* 설정 가능한 최소/최대 ZOOM 단계이다. 응용에서의 사용 편의성을 위해 최소/최대
 * 단계를 고정할 경우 사용한다.
 * EF10S/EF12S 에서는 사용하지 않으며 참고용으로 코드들은 남겨둔다. */
#ifdef F_SKYCAM_FIX_CFG_ZOOM
#define C_SKYCAM_MIN_ZOOM 0
#define C_SKYCAM_MAX_ZOOM 8
#endif


/* 카메라 응용에서 촬영 시 삽입할 EXIF 태그 정보의 제조사 관련 정보를 
 * 수정한다. */
#ifdef F_SKYCAM_OEM_EXIF_TAG
#define C_SKYCAM_EXIF_MAKE		"PANTECH"
#define C_SKYCAM_EXIF_MAKE_LEN		8		/* including NULL */
#ifdef F_SKYCAM_TARGET_EF32
#define C_SKYCAM_EXIF_MODEL		"IM-A750K"
#else
#define C_SKYCAM_EXIF_MODEL		"IM-A740S"
#endif
#define C_SKYCAM_EXIF_MODEL_LEN		9		/* including NULL */
#endif


/* 센서가 프리뷰 모드일 경우, 출력 가능한 FPS 의 최소/최대 값이다. */
#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
#define C_SKYCAM_MIN_PREVIEW_FPS	5
#define C_SKYCAM_MAX_PREVIEW_FPS	30
#endif

#ifdef F_SKYCAM_CUST_CPU_CLK_FREQ
#define C_SKYCAM_CPU_PERF_LOCK_FNAME	"/sys/devices/system/cpu/cpu0/cpufreq/perf_lock"
#define C_SKYCAM_CPU_PERF_UNLOCK_FNAME	"/sys/devices/system/cpu/cpu0/cpufreq/perf_unlock"
#endif


/*
	두개의 카메라 중 한개의 카메라가 구동중일 때, 카메라 전환 중, 현재 구동 중인 카메라의 off 와 
	전환될 카메라의 on의 코드가 겹치는 현상이 발생하여 변수 사용하여 한쪽의 off가  완료되면 다른 카메라의
	start를 시작하도록 수정 함
*/
#define F_SKYCAM_DRIVER_COMPLETE

/*
드라이버가 종료 되지 않아, 카메라 재진입 되지 않는 현상에 대한 방어 코드
*/ 
/*#define F_SKYCAM_FIX_MSM_OPEN_FAIL*/

#endif /* CUST_SKYCAM.h */
