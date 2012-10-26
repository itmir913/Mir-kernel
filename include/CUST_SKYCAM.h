#ifndef __CUST_SKYCAM_H__
#define __CUST_SKYCAM_H__


/*------------------------------------------------------------------------------

(1)  �ϵ���� ����

EF13S/L : MSM7x27, MT9P111(5M SOC), YACBAC1SDDAS(VGA)
EF31S   : MSM7x27, MT9T111(3M SOC), YACBAC1SDDAS(VGA)
EF32K   : MSM7x27, MT9T111(3M SOC), YACBAC1SDDAS(VGA)


(2)  ī�޶� ���� ��� kernel/userspace/android �α׸� ����

kernel/arch/arm/config/qsd8650-perf_defconfig �� �����Ѵ�.

	# CONFIG_MSM_CAMERA_DEBUG is not set (default)

CUST_SKYCAM.h ���� F_SKYCAM_LOG_PRINTK �� #undef �Ѵ�. 

	#define F_SKYCAM_LOG_PRINTK (default)

��� �ҽ� ���Ͽ��� F_SKYCAM_LOG_OEM �� ã�� �ּ� ó���Ѵ�. 
	���� �� ���, �ش� ���Ͽ� ���� SKYCDBG/SKYCERR ��ũ�θ� �̿��� 
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)

��� �ҽ� ���Ͽ��� F_SKYCAM_LOG_CDBG �� ã�� �ּ� ó���Ѵ�. 
	���� �� ���, �ش� ���Ͽ� ���� CDBG ��ũ�θ� �̿��� �޽������� 
	Ȱ��ȭ ��Ų��. (user-space only)

��� �ҽ� ���Ͽ��� F_SKYCAM_LOG_VERBOSE �� ã�� �ּ� ó���Ѵ�.
	���� �� ���, �ش� ���Ͽ� ���� LOGV/LOGD/LOGI ��ũ�θ� �̿��� 
	�޽������� Ȱ��ȭ ��Ų��. (user-space only)


(3)  MV9337/MV9335 ���� ���� ȯ�� (kernel/user-space)

<KERNEL>

kernel/arch/arm/config/msm7627-EF31S-perf_defconfig (for EF31S)
kernel/arch/arm/config/msm7627-EF32K-perf_defconfig (for EF32K) ��
���� �����Ѵ�.

#CONFIG_MT9P111 is not set       ������
CONFIG_MT9T111=y                 ����
CONFIG_YACBAC1SDDAS=y            ����

msm7627-XXXX-perf_defconfig �� Kconfig (default y) ���� �켱 ������ ����.

<USERSPACE>

vendor/qcom/proprietary/mm-camera/Android.mk (for Android)
vendor/qcom/proprietary/mm-camera/camera.mk (for Linux) �� �����Ѵ�.

SYS_MODEL_NAME ������ ���� ���� ���� �����Ѵ�.
SKYCAM_MT9P111=no        ������
SKYCAM_MT9T111=yes       ����
SKYCAM_YACBAC1SDDAS=yes  ����


(4)  �ȸ��ν� ���� ��� ���� ȯ��

vendor/qcom/android-open/libcamera2/Android.mk �� �����Ѵ�.
	3rd PARTY �ַ�� ��� ���θ� �����Ѵ�.

	SKYCAM_FD_ENGINE := 0		������
	SKYCAM_FD_ENGINE := 1		�ö���� �ַ�� ���
	SKYCAM_FD_ENGINE := 2		��Ÿ �ַ�� ���

CUST_SKYCAM.h ���� F_SKYCAM_ADD_CFG_FACE_FILTER �� #undef �Ѵ�.
	�ȸ��ν� ��� ���� �������̽� ���� ���θ� �����Ѵ�.

libOlaEngine.so �� ���� libcamera.so �� ��ũ�ϹǷ� ���� ��� libcamera.so ��
ũ�Ⱑ �����Ͽ� ��ũ ������ �߻� �����ϸ�, �� ��� �Ʒ� ���ϵ鿡�� 
liboemcamera.so �� ������ �ٿ� libcamera.so �� ������ Ȯ���� �� �ִ�.

build/core/prelink-linux-arm-2G.map (for 2G/2G)
build/core/prelink-linux-arm.map (for 1G/3G)	

------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC                                                            */
/*  �ش� �𵨿��� ����Ǵ� �Ǵ� �ش� �𵨿����� ������ FEATURE ���           */
/*----------------------------------------------------------------------------*/
/*#define F_SKYCAM_TARGET_EF13*/
#define F_SKYCAM_TARGET_EF31
/*EF32���� EF31�� �޸� ���� �Ǵ� �κп� ���� ����.
 * EF32�� ��� EF31�� ���� define �Ͽ� ����Ͽ��� �Ѵ�.
 * EF31�� ��쿡�� EF32�� define ���� �ʴ´�. */
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

#ifdef F_SKYCAM_TARGET_EF32	/*EF32K�� ����� feature*/
/* 
 * MEDIA_RECORDER_INFO_FILESIZE_PROGRESS CallBack, Only Use KT Phone 
 * KT�������� ���� ���� ����� ����ϴµ� ���� ����� APP�� �˷��ֱ� ���ؼ�
 * �߰� 
 */
#define F_SKYCAM_ADD_EVT_RECSIZE

#else						/*EF31S�� ����� feature*/

/* F_SKYCAM_TODO, SKT FOTA DCMO (Device Control Management Object)
 * SKT �⿡�� ����Ǹ�, UI VOB������ define�� ����.
 * "pantech/development/sky_fota/sky_fota.h" ������ �־�� �Ѵ�.
*/
#define F_SKYCAM_FOTA_DCMO_CAMERA_LOCK

#endif

#endif


/*
 * ī�޶� ����̹��� ������ ������� �ʾ��� ��, suspend �Ǵ� ���� ���´�.
 * power control �� ���� Ŀ�� ����̹��� suspend �Ǵ� ���� ���´�.
 * �Ϲ����� ��� ī�޶� ������ ī�޶� ����̹��� ���� ��Ű��, �� �� Ŀ�� ����̹��� ��������.
 * HD ������ȭ�� ��� ���� ������ control�� �Ұ����� LCD�� OFF �Ǵ� ��Ȳ���� suspend�� �߻��Ѵ�.
 * �� �� Ŀ�� ����̹��� suspend ���� �ʵ��� �Ѵ�.
*/
#define F_SKYCAM_FIX_SUSPENDLOCK_ADD

/*
 * pantech VT ����ī�޶� ����� 90 rotate�ϰ� ����
 * preview buffer�� �������� ������ video buffer�� data�� preview buffer ��������
 * 90�� rotation ��Ų��.
 * �ش� ����� ����ϱ� ���� ���� F_SKYCAM_CFG_VT���� �߰��� "pantech-vt"
 * parameter�� "on"���� ���� �Ͽ��� �Ѵ�.
*/
#define F_SKYCAM_VT_FIX_MAIN_CAM_ROT

/* 
 * pantech VT ī�޶� ���� ������ ���� "pantech-vt" �Ķ���͸� �߰� �Ͽ���.
 * "pantech-vt"�� "on"���� ���� �Կ� ���� VT���� main ī�޶� rotation �ϴ� �κа�
 * sub camera�� video ���۸� flip ���� �ʵ��� �Ѵ�.
*/
#define F_SKYCAM_CFG_VT

/*
 * pantech VT�� ȣ�� ����Ǹ� ���� ���۸� video ���۷κ��� ��� ���� start recording
 * �� �����ϸ� ���� connect/disconnect �ÿ� �Կ����� �߻��Ѵ�.
 * pantech VT���� �Կ����� �߻��ϴ� ���� ���� ���� CameraService�� 
 * CAMERA_CMD_SET_SHUTTER_DISABLE commad�� �߰� �Ͽ���.
*/
#define F_SKYCAM_VT_SHUTTER_DISABLE 


/*
 * ���� ī�޶�� video recording�� �ϱ� ���� video buffer�� LR flip�� �����Ѵ�.
 * Froyo ������ ����ī�޶��� ��� LR flip�� �����Ͽ� �Է¹޾� preview�� recording��
 * �״�� ���� �Ͽ�����, Gingerbread������ display�� �ϱ� ���� display�ʿ� flip�� ����
 * �ϹǷ�, ī�޶� �Է��� normal�� �޴´�. ���� preview�� �����ϰ� recording�� �ϱ�
 * ���ؼ��� video buffer�� LR flip�� �����Ѵ�.
*/
//F_SKYCAM_FAKE_FRONT_CAMERA�� �Ʒ� �ΰ� ���� 
//#define F_SKYCAM_FRONT_CAM_VBUF_LR
//#define F_SKYCAM_USE_IPLLIB

/*
 * preview size �� capture size�� �����Ѵ�.
 * MSM7x27 ���� preview size�� capture size�� ���ΰ� �� size�� �����ϵ��� �Ѵ�.
*/
#define F_SKYCAM_PORTRAIT_PREVIEW_CAPTURE

/* ī�޶� Ʃ�� ���� �ε带 ����...*/
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
/*  �ȵ���̵� ��� ��� �𵨿� ���� ����Ǿ�� �� FEATURE ���               */
/*----------------------------------------------------------------------------*/


/* ���� CS �μ������� �Һ��� �÷� �м��� ���� ���� PC ���α׷��� ����Ͽ� 
 * ī�޶� ���� �ð� ������ PC �� �����Ѵ�. 
 *
 * ���� ����� ���� Ŀ�ǵ� ��缭�� ��õǾ� �����Ƿ� ���� �ڵ���� ���� Ŀ�ǵ� 
 * ���� ��⿡ ���ԵǾ� ������, ���� Ŀ�ǵ� �� PC ���α׷��� ������� �ʰ� ����
 * ���α׷��� ����Ͽ�, �÷��� DIAG ��Ʈ�κ��� ���� �ð� ������ Ȯ���� �� �ִ�.
 *
 * ���� Ŀ�ǵ� ��缭 v10.35 ��� ����
 * PhoneInfoDisplay v4.0 ���α׷����� Ȯ��
 * ��缭�� ���α׷��� DS2�� �ڰ�ȣ ���ӿ��� ���� */
#define F_SKYCAM_FACTORY_PROC_CMD


/* ī�޶� ��ġ ���� OPEN �� ������ ��� (�ܼ� I2C ���� R/W ����, ī�޶� ������) 
 * ���� ó���� ���� �����Ѵ�. 
 *
 * ��ġ ������ OPEN �ϴ� �������� VFE �ʱ�ȭ ���� ī�޶� HW �ʱ�ȭ�� �̷�� 
 * ���µ�, HW �ʱ�ȭ�� ������ ��� VFE �� �ʱ�ȭ �� ���·� ���Եǰ� ����
 * OPEN �õ� �� HW �ʱ�ȭ�� �����Ѵ� �ϴ��� �̹� VFE �� �ʱ�ȭ�� �����̹Ƿ� 
 * VFE �ʱ�ȭ �� ������ �߻��Ѵ�.
 * 
 * ȣ����� : vfefn.vfe_init() -> sctrl.s_init()
 *
 * HW �ʱ�ȭ�� ������ ���, �̹� �ʱ�ȭ�� VFE �� RELEASE (vfe_release) ���� 
 * ���� ���� �õ� �� ���� �����ϵ��� �����Ѵ�. 
 *
 * ECLAIR ���������� ���� ���� ���� ó������ �ұ��ϰ� ������ ����Ǿ� ����
 * �ʰų� ���� �ϵ��� �̻��� �߻��� ��� ī�޶� ������ ANR ������ ���� 
 * ������ ����ǰ� ���� ����� �������� �����Ͽ� �������� �Ұ����ϴ�.
 *
 * ������ �� ������ ���, ISP �ʱ�ȭ �� ISP �� ������ ������ ���� �����ϴ� 
 * �������� 3�� �� POLLING �����ϸ�, �̷� ���� Ÿ�Ӿƿ� �߻��ϰ� ANR ������ 
 * �̾�����. �� ���� ���� ���� ī�޶� ������ �� ������ �����̶� �ϴ��� ANR 
 * ���� ���� ������ �� ���������� ����Ǿ����Ƿ� FRAMEWORK ���δ� �� ������ 
 * ���·� �����ǰ�, �̷� ���� ����� �������� ī�޶� ���� ���� �� "Unable to 
 * connect camera device" �˾��� �Բ� ������ ���Կ� �����Ѵ�.
 *
 * ISP �ʱ�ȭ �� ������ ��� ���� ������, ISP �� ���� ������ Ư�� �������͸� 
 * 1ȸ READ �ϰ� �� ������ ���, �̸� FRAMWORK �� ���� �������� �����Ͽ� 
 * ���������� ����ǵ��� �����Ѵ�. 
 *
 * ���� ISP ��ü�� �̻��� �߻��� ��쿡��, PROBE �ÿ� ���� �߻��Ͽ� �ش� 
 * ����̽� ���ϵ��� ������ �� �����Ƿ� FRAMWORK ���ο��� �Բ� ó�� �����ϴ�. 
 *
 * EF10S �� ���, BAYER ������ Ŀ���ͷ� ����Ǿ� �ְ�, MV9337 �� ON-BOARD
 * �Ǿ� �����Ƿ�, BAYER ������ ����Ǿ� ���� �ʾƵ�, MV9337 �� �����̶��,
 * PROBE �� ���� �����Ͽ�����, EF12S �� ���, ī�޶� ��⿡ MV9335 �� �Բ�
 * �ν���Ǿ� �־�, Ŀ���Ϳ� ����� ������� ������ PROBE �� I2C R/W ���а�
 * ���� �߻�, RETRY �����ϸ鼭 ���� �ð��� 10�� �̻� �����ǰ�, �̷� ����
 * �ٸ� ������ �ʱ�ȭ�� �������� ������ ��ģ��. */
#define F_SKYCAM_INVALIDATE_CAMERA_CLIENT


/* �ܸ����� �Կ��� ������ EXIF TAG ���� �� ������ ���� ������ �����Ѵ�. */
#define F_SKYCAM_OEM_EXIF_TAG


/* ���� ������ �Կ� �ػ� ���̺��� �����Ѵ�. 
 *
 * HAL ������ ��ȿ�� �Կ� �ػ󵵸� ���̺� ���·� �����ϰ� ���̺� ���Ե� 
 * �ػ� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_SKYCAM_CUST_PICTURE_SIZES


/* ���� ������ ������ �ػ� ���̺��� �����Ѵ�. 
 *
 * HAL ������ ��ȿ�� ������ �ػ󵵸� ���̺� ���·� �����ϰ� ���̺� ���Ե� 
 * �ػ� �̿��� ���� ��û�� ������ ó���Ѵ�. */
#define F_SKYCAM_CUST_PREVIEW_SIZES


/* ī�޶� IOCTL Ŀ�ǵ� MSM_CAM_IOCTL_SENSOR_IO_CFG �� Ȯ���Ѵ�. 
 *
 * �߰��� ��� �� SOC ī�޶� �������� �ʰ� �̱����� �κе��� ���� �� �߰� 
 * �����Ѵ�. */
#define F_SKYCAM_CUST_MSM_CAMERA_CFG


/* ������ �׽�Ʈ ���� (/system/bin/mm-qcamera-test) �� �ʿ� �κ��� �����Ѵ�. 
 *
 * ������ �Ϻ����� �����Ƿ� �Ϻ� ����� ���� ������ �������� ���Ѵ�.
 * �ȵ���̵� �÷����� �ʱ�ȭ�Ǳ� ������ adb shell stop ������� �ý����� 
 * ������Ű��, adb shell �� ���� ���� �� �����Ѵ�. */
#define F_SKYCAM_CUST_LINUX_APP


/* "ro.product.device" ���� �� �������� ���� �ڵ� �������� ���� ���� ī�޶�
 * ���� �ڵ�鿡���� �� ���� ���� �ʰų� 'TARGET_XXXXXXX' ���� �����Ѵ�. 
 * ������ ������ ��� �� ���� "xxxxxx_surf" �̴�. */
/*#define F_SKYCAM_CUST_TARGET_TYPE_8X50*/
#define F_SKYCAM_CUST_TARGET_TYPE_7X27


/* SKAF �� ���, ���� ������鿡 ���� ����Ǵ� �����̹Ƿ� QUALCOMM JPEG ���� 
 * ��� ����� ������� �ʵ��� �����Ѵ�. (DS7�� ��û����)
 *
 * SKAF �� ���, SKIA ���̺귯���� ǥ�� ���̺귯�� ��� 'jpeglib.h' ��
 * ���� ���� ����ϰ�, QUALCOMM �� JPEG ���� ����� ���� 'jpeglib.h' ������ 
 * 'jpeg_decompress_struct' ����ü�� 'struct jpeg_qc_routines * qcroutines'
 * ����� �߰��Ͽ���. �̷� ���� 'jpeg_CreateDecompress()' ���� �� ����ü ũ�� 
 * �� �κп��� ������ �߻��ϰ� �ش� ���ڵ� ������ �����Ѵ�.
 *
 * QUALCOMM JPEG ���� ����� SNAPDRAGON �ھ���� �����ǰ�, 2MP �̻��� 
 * JPEG ���� ���ڵ� �� 18~32% �� ������ ���ȴ�. 
 * (android/external/jpeg/Android.mk, R4070 release note 'D.5 LIBJPEG' ����) 
 * F_SKYCAM_TODO, QUALCOMM �� �� ���� ����ü�� �����ߴ°�? ���� ����ü�� 
 * �и��Ͽ� �����ߴٸ�, �̷� ������ ���� �� �ִ�. 
 *
 * FROYO (R5025) ���� �����Ǿ���. */
/* #define F_SKYCAM_CUST_QC_JPG_OPTIMIZATION */



/*----------------------------------------------------------------------------*/
/*  SENSOR CONFIGURATION                                                      */
/*  �� �� ��� ����(ISP)�� ���� ����/�߰��� FEATURE ���                    */
/*----------------------------------------------------------------------------*/


/* ī�޶��� ������ ��� ���� ���� SOC ī�޶�(��)�� ����� ��� �����Ѵ�.
 *
 * ������ȭ�� ���� �� ���� ī�޶� ����ϰ�, �ϳ��� SOC Ÿ��, �ٸ� �ϳ���
 * BAYER Ÿ���� ��쿡�� �������� �ʴ´�. ������ ���, BAYER ī�޶� ����
 * �Ϻ� �ڵ���� ������� �ʴ´�.
 *
 * EF10S/EF12S ������ �ϳ��� SOC ī�޶� ����ϹǷ�, BAYER ���� �ڵ���� 
 * �������� �ʾҰ�, �Ϻδ� �Ʒ� FEATURE ���� ����Ͽ� �ּ� ó���Ͽ���. */
#define F_SKYCAM_YUV_SENSOR_ONLY


#ifdef F_SKYCAM_YUV_SENSOR_ONLY

/* ISP ��ü���� ���� ���� ZOOM �� �����ϱ� ���� �������̽��� �߰��Ѵ�. 
 * EF10S/EF12S ������ QUALCOMM ZOOM �� ����ϸ�, ��������� �ڵ���� ���ܵд�.
 *
 * ISP ��ü ZOOM �� ���, ������/������ ��忡�� �̹� ZOOM �� ����� �̹����� 
 * ��µǸ� �� ���� ��带 �����Ѵ�.
 *
 * 1) DIGITAL (SUBSAMPLE & RESIZE)
 *     ������/������ �ػ󵵺��� ������ ������ �����Ѵ�. ISP ���ο��� 
 *     �̹����� SUBSAMPLE �Ͽ� RESIZE �� ����ϸ�, �̷� ���� ZOOM ������
 *     0 �� �ƴ� ������ ������ ��� ������ FPS �� 1/2 �� ���ҵȴ�.
 * 2) SUBSAMPLE ONLY
 *     ������/������ �ػ󵵺��� ������ ������ �����Ѵ�. ISP ���ο��� 
 *     SUBSAMPLE �� �����ϹǷ� ���� �ػ󵵿����� ���� ������ �����ϰ� �ִ� 
 *     �ػ󵵿����� ZOOM ��ü�� �Ұ����ϴ�. ������ FPS �� ���ҵ��� �ʴ´�.
 *
 * QUALCOMM ZOOM ���� ��, ī�޶��� ��� ��� �ػ󵵿��� ���� ���� ZOOM �� 
 * �����ϹǷ� �̸� ����ϸ�, ���� ���� ���� �ش� �ڵ���� ���ܵд�. 
 *
 * ���� FEATURE : F_SKYCAM_FIX_CFG_ZOOM, F_SKYCAM_ADD_CFG_DIMENSION */
/* #define F_SKYCAM_ADD_CFG_SZOOM */


/* ISP ���� �����Ǵ� �ն��� ���� ��� (Digital Image Stabilization) �� ����
 * �������̽��� �߰��Ѵ�. 
 *
 * ����/�¿� ���� �������� ��鸱 ��츸 ���� �����ϴ�. 
 * ��� ��带 OFF �̿��� ���� ������ ���, ���� �ն��� ���� ������ 
 * ���õȴ�. */
#define F_SKYCAM_ADD_CFG_ANTISHAKE


/* AF WINDOW ������ ���� �������̽��� �����Ѵ�. SPOT FOCUS ��� ���� �� 
 * ����Ѵ�.
 *
 * ISP ������ ������ ��� ��� �ػ󵵸� �������� ����/���θ� ���� 16 ���� 
 * �������� �� 256 �� ������ ������ �� ������ AF WINDOW ������ �����ϴ�. 
 * ���뿡���� ������ �ػ󵵸� �������� ����ڰ� ��ġ�� ������ ��ǥ�� HAL �� 
 * �����ϰ�, HAL ������ �̸� �� ��ǥ�� ��ȯ�Ͽ� ISP �� �����Ѵ�. 
 * ���� AF ���� �� �� WINDOW �� ���Ե� �̹��������� FOCUS VALUE �� �����Ͽ�
 * ������ ��ġ�� �����Ѵ�.
 *
 * ISP �� ����� ������ ���¿��� QUALCOMM ZOOM �� ����Ͽ� SUBSAMPLE/RESIZE
 * �ϱ� ������ ZOOM �� 0 ���� �̻����� ������ ���, HAL ���� ��ǥ-to-���
 * ��ȯ���� ����������, Ư�� ZOOM ���� �̻��� ��� �� ���� ��� �ȿ� ��ü
 * ������ ������ ���ԵǾ� �����Ƿ� ���� ��ü�� �ǹ̰� ����.
 * �׷��Ƿ�, ������ SPOT FOCUS ��� ��� �ÿ��� ZOOM ����� ����� �� ������ 
 * ó�� �ؾ��Ѵ�. */
#define F_SKYCAM_FIX_CFG_FOCUS_RECT


/* QUALCOMM BAYER �ַ�� ����� ȭ��Ʈ�뷱�� ���� �������̽��� �����Ѵ�. 
 *
 * ��� ��带 OFF �̿��� ���� ������ ���, ���� ȭ��Ʈ�뷱�� ������ 
 * ���õȴ�. */
#define F_SKYCAM_FIX_CFG_WB


/* QUALCOMM BAYER �ַ�� ����� ���� ���� �������̽��� �����Ѵ�. 
 *
 * ��� ��带 OFF �̿��� ���� ������ ���, ���� ���� ������ ���õȴ�. */
#define F_SKYCAM_FIX_CFG_EXPOSURE


/* ��� ��� ������ ���� �������̽��� �߰��Ѵ�. 
 *
 * ��� ��带 OFF �̿��� ������ ������ ��� ���� ȭ��Ʈ�뷱��/����/�ն�������/
 * ISO ������ ���õȴ�. ���뿡�� ��� ��带 �ٽ� OFF �� �ʱ�ȭ �ϴ� ���, 
 * ȭ��Ʈ�뷱��/����/�ն�������/ISO �� HAL ���� ���� ������� �ڵ� �����ǹǷ�,
 * ������ ������ �ʿ� ����. (HW ��������̹Ƿ�, HAL ���� �����Ѵ�.) */
#define F_SKYCAM_FIX_CFG_SCENE_MODE


/* �ø�Ŀ ������ ���� �������̽��� �����Ѵ�.
 *
 * 2.1 SDK ���� �� �� ���� ��� (OFF/50Hz/60Hz/AUTO) �� ��õǾ� ������, 
 * ISP �� ��� OFF/AUTO �� �������� �ʴ´�. �׷��Ƿ�, ������ OFF �� ���� 
 * �ÿ��� Ŀ�� ����̹����� 60Hz �� �����ϰ�, AUTO �� ������ ��� HAL ���� 
 * �ý��� ���� �� �� ���� �ڵ� ("gsm.operator.numeric", �� 3�ڸ� ����) �� �а�, 
 * ������ Hz ������ ��ȯ�Ͽ� �ش� ������ �����Ѵ�.
 *
 * ��ȹ�� ���� ���, �ø�Ŀ�� �Ϲ����� ����� �ƴϹǷ�, ���� �ڵ带 �ν��Ͽ� 
 * �ڵ����� ������ �� �ֵ��� �ϰ�, ���� ���� �޴��� ���� ó���Ѵ�. */
#define F_SKYCAM_FIX_CFG_ANTIBANDING

#ifdef F_SKYCAM_TARGET_EF13
/* �÷��� LED ������ ���� �������̽��� �����Ѵ�.
 *
 * QUALCOMM ������ ������ IOCTL (MSM_CAM_IOCTL_FLASH_LED_CFG) Ŀ�ǵ带 
 * ����Ͽ� �����Ǿ� ������, PMIC ������ ����ϴ� LED ����̹��� �����Ѵ�.
 * EF10S/EF12S ������ �̸� ������� ������, MSM_CAM_IOCTL_SENSOR_IO_CFG �� 
 * Ȯ���Ͽ� �����Ѵ�.
 *
 * AUTO ���� ������ ���, ������ �� ��쿡�� AF ���� �� AF/AE �� ����
 * ��� ON �ǰ�, ���� ������ �������� �� �� �� ON �ȴ�. */
#define F_SKYCAM_FIX_CFG_LED_MODE
#endif

/* ISO ������ ���� �������̽��� �����Ѵ�.
 *
 * ��ȹ�� ���� ���, AUTO ��忡���� ȭ���� ū �̻��� �����Ƿ� ��������
 * ISO �� ������ �� �ִ� �޴��� ���� ó���Ѵ�.
 * ��� ��带 OFF �̿��� ���� ������ ���, ���� ISO ������ ���õȴ�. */
/*#define F_SKYCAM_FIX_CFG_ISO*/

/* Ư��ȿ�� ������ ���� �������̽��� �����Ѵ�.
 *
 * SDK 2.1 �� ��õ� ȿ���� �� �Ϻθ� �����Ѵ�. MV9337/MV9335 �� ��� SDK ��
 * ��õ��� ���� ȿ���鵵 ���������� ���뿡�� ������� �����Ƿ� ���� �߰���
 * ���� �ʴ´�. */
#define F_SKYCAM_FIX_CFG_EFFECT


/* MANUAL FOCUS ������ ���� �������̽��� �����Ѵ�. 
 *
 * MANUAL FOCUS ���� ������ ���, �� ������ ���Ƿ� ������ ��ġ��
 * �̵���ų ���, ������ ������ AF �� ������� �ʴ� ���� �����̸�, �̷� ���� 
 * ISP ���� ������ �����Ͽ� ������ �� �ִ� �ð��� �����Ƿ� �÷����� ON ��
 * ���, ������/������ �̹����� ������ �� �ִ�. �׷��Ƿ� ������ MANUAL FOCUS 
 * ��� ���� �� �ݵ�� �÷��� ��带 OFF �� �����ؾ� �ϰ�, AUTO FOCUS �Ǵ� 
 * SPOT FOCUS ��� ���� �� ���� ���� �������Ѿ� �Ѵ�. */
#define F_SKYCAM_FIX_CFG_FOCUS_STEP


/* ��� ������ ���� �������̽��� �����Ѵ�. */
#define F_SKYCAM_FIX_CFG_BRIGHTNESS


/* LENS SHADE ������ ���� �������̽��� �����Ѵ�. 
 *
 * MV9337/MV9335 �� ��� ������ LENS SHADE ����� �������� �ʰ�, 
 * ������/������ �� �׻� LENS SHADE �� ����� �̹����� ����Ѵ�. */
/*#define F_SKYCAM_FIX_CFG_LENSSHADE*/


/* ������ ȸ���� ������ ���� �������̽��� �����Ѵ�.
 *
 * ������ ���� JPEG ���ڵ� �ÿ� �ȸ��ν� ���� (������) ���� �� ī�޶��� 
 * ȸ�� ���¸� �Է��Ͽ��� �Ѵ�. ������ OrientationListener ��� �� �Ʒ��� ����
 * ������ HAL �� ȸ���� ���� ���� ���־�� �Ѵ�.
 * 
 * JPEG ���ڵ�
 *     ���ڵ� ������ ����
 * ������ ���
 *     ���� �� �Ź� ����
*/
#define F_SKYCAM_FIX_CFG_ROTATION


/* AF ������ ���� �������̽��� �����Ѵ�. 
 *
 * QUICK SEARCH �˰����� ����ϸ�, ���� �̵� �ø��� FOCUS VALUE �� �����ϰ�,
 * �����ؼ� FOCUS VALUE �� ������ ���, ���� �ֱٿ� �ְ��� FOCUS VALUE ����
 * ���� ��ġ�� ��� �̵���Ų��.
 * MV9337/MV9335 �� ���, ���ѰŸ��� �ǻ�ü�� �Կ��� ��� ��� ��ũ�� ������
 * �̵���Ű�� �����Ƿ� �ؼ����� ��� AF �ӵ��� ����ϴ�. */
#define F_SKYCAM_FIX_CFG_AF

/* spot select focus 
��ġ �����Ͽ� ��ġ�� �����ϵ��� �߰� ����
ui������ 'normal' �� 'spot' ��Ʈ���� �̿��Ͽ� AF��� ���� 
EF31S 32K�� rect�� x -1, y -1 �϶� center����ǵ��� �Ѵ�. 
autofocus��尡 rect���Ŀ� �ͼ� focus mode�� �ǹ̰� ����.
*/
#define F_SKYCAM_CFG_AF_SPOT

#define F_SKYCAM_FIX_CFG_REFLECT


/* ZOOM ������ ���� �������̽��� �����Ѵ�.
 *
 * QUALCOMM ZOOM �� ���, �ִ� ZOOM ������ HAL ���Ͽ��� ������/������ �ػ󵵿� 
 * ���� �����Ǹ�, ������ �� �� ("max-zoom") �� �о� �ּ�/�ִ� ZOOM ������
 * �����ϰ� �� ���� ���� ����θ� �����ؾ� �Ѵ�.
 * ���� �ػ�/�𵨿� ������� ������ ZOOM ���� ������ �����ؾ� �� ���, HAL
 * ������ "max-zoom" ���� ���� ZOOM ���� ������ ������ ������ �� �־�� �Ѵ�.
 *
 * ���� FEATURE : F_SKYCAM_ADD_CFG_SZOOM, F_SKYCAM_ADD_CFG_DIMENSION */
/* #define F_SKYCAM_FIX_CFG_ZOOM */


#endif /* F_SKYCAM_YUV_SENSOR_ONLY */


/* �ȸ��ν� ��� �̹��� ���� ������ ���� �������̽��� �߰��Ѵ�.
 *
 * EF10S/EF12S ������ �ö���� �ַ���� ����ϸ�, ������/������ �̹������� 
 * �� ��ġ�� �����Ͽ� �� ������ ����ũ�� Ư��ȿ���� ������ �� �ִ�. 
 *
 * vendor/qcom/android-open/libcamera2/Android.mk ���� �ö���� ���̺귯����
 * ��ũ���Ѿ߸� �����Ѵ�. SKYCAM_FD_ENGINE �� 1 �� ������ ��� �ö���� 
 * �ַ���� ����ϰ�, 0 ���� ������ ��� ī�޶� �Ķ���͸� �߰��� ���·� ���� 
 * ��ɵ��� �������� �ʴ´�. �ٸ� �ַ���� ����� ��� �� ���� Ȯ���Ͽ� 
 * ����Ѵ�. */
#define F_SKYCAM_ADD_CFG_FACE_FILTER


/* ISP �� ��� �ػ󵵸� ������ �� �ִ� �������̽��� �߰��Ѵ�.
 *
 * QUALCOMM ZOOM ����� VFE/CAMIF SUBSAMPLE �� MDP RESIZE �� ����ϸ�, ���
 * ������/������ �ػ󵵿� ���� ������ ZOOM ������ �����Ѵ�. �̸� ���� ISP ��
 * ��� �ػ󵵸� ������/������ ��忡�� ���� ������ ������ �����ϰ� 
 * VFE/CAMIF/MDP ���� ��ó�� (SUBSAMPLE/RESIZE) �ϴ� �����̴�.
 *
 * �׷���, ISP �� ZOOM ����� ��� ��ü�� ZOOM �� ����Ǿ� ��µǹǷ�, 
 * ISP ��ü ����� ���뿡�� �䱸�ϴ� �ػ󵵷� �����ϰ� VFE/CAMIF/MDP 
 * ��ɵ��� ��� DISABLE ���Ѿ� �Ѵ�.
 *
 * EF10S/EF12S ������ ������� ������ ��������� �ڵ���� ���ܵд�.
 * ���� FEATURE : F_SKYCAM_FIX_CFG_ZOOM, F_SKYCAM_ADD_CFG_SZOOM */
/* #define F_SKYCAM_ADD_CFG_DIMENSION */


/* ���� �ν��� ���� ������ ���� �������̽��� �����Ѵ�.
 *
 * EF10S/EF12S �� ��� ���뿡�� UI �� ���θ��� �����Ͽ� ����ϸ�, HAL ���� 
 * �����Ͽ� ó���ؾ� �� �κ��� ���� ��� �� ���� ����ؾ� �Ѵ�.
 *
 * F_SKYCAM_TODO 7K ������ �ش� �������̽� �߰��Ǿ����� Ȯ�� �ʿ��ϴ�.
 * EF10S/EF12S ������ ���� ���Ǵ� �κ��� ������ ��������� �ڵ���� ���ܵд�. */
#define F_SKYCAM_FIX_CFG_ORIENTATION


/* ������ FPS ������ ���� �������̽��� �����Ѵ�. 
 *
 * 1 ~ 30 ���� ���� �����ϸ� �ǹ̴� ������ ����.
 *
 * 5 ~ 29 : fixed fps (������ ���� ���� ����) ==> ķ�ڴ� ������ �� ���
 * 30 : 8 ~ 30 variable fps (������ ���� �ڵ� ����) ==> ī�޶� ������ �� ���
 *
 * MV9337/MV9335 �� ������ ��忡�� ���� 1 ~ 30 �����Ӱ� ���� 8 ~ 30 �������� 
 * �����ϸ�, EF10S/EF12S ������ ������ ��ȭ �� 24fps (QVGA MMS �� ��� 15fps) ����
 * �����ϰ�, ī�޶� ������ �ÿ��� ���� 8 ~ 30fps ���� �����Ѵ�. */
#define F_SKYCAM_FIX_CFG_PREVIEW_FPS


/* ��Ƽ�� ������ ���� �������̽��� �߰��Ѵ�.
 * 
 * �����Կ�, �����Կ� 4��/9�� ��忡�� �� �Կ� �ø��� ���� ��带 �����ϴ� 
 * ���� �����Ѵ�. ù ��° �Կ����� ������ ������ ���� �����ϰ�, VFE/CAMIF
 * �������� �Ϸ�� �����̹Ƿ�, �� ��° �Կ����ʹ� ������/�����/JPEG ���۸�
 * �ʱ�ȭ�ϰ� VFE/CAMIF �� ������ ��ɸ� �۽��Ѵ�.
 *
 * ������� : ���������� �÷��� ��带 �ڵ����� �����ϰ� �����Կ�, �����Կ� 
 * 4��/9�� ���� ���� �� �Կ� ��, ù ��° �Կ������� �÷����� ON �ǰ�, 
 * �� ��°���ʹ� ON ���� �����Ƿ� �����Կ�, �����Կ� 4��/9�� ��忡����
 * ������ �ݵ�� �÷����� OFF �ؾ��ϸ�, �̿� ���� ���� �� ���� ������
 * ������ �������Ѿ� �Ѵ�. */
#define F_SKYCAM_ADD_CFG_MULTISHOT


/* ���̳��� Ŭ�� ����ÿ� �׻� ī�޶󿡼��� �׻� MAX Ŭ������ ����Ѵ�.
 */
#define F_SKYCAM_CUST_CPU_CLK_FREQ

/* SKYCAM_PSJ_100817
 * OTP����� �ҷ����� ���̱� ���� �޸� zone�� 2���� detecting�Ͽ� 
 * ���� ����ؾ� �� zone�� ã�Ƴ� �� �ش� zone�� �޸𸮸� ��� �� �� �ֵ��� �߰�
*/
#define F_SKYCAM_ADD_CFG_OTP_MODULE


/*  Camcoder�� ��� Continuous AF�� ON, OFF�� �� �� �ִ�
 */
#define F_SKYCAM_ADD_CFG_CAF


/* Pantech�� MIRROR ���ø����̼��� ���� Preview �� CROP �Ѵ�.
 * MIRROR ���ø����̼��� �ſ� ������� ���� ī�޶� PREVIEW�� �����ϴ�.
 * PREVIEW ������ 640x480 ���� �����ϸ�, 3:4 ������ 360x480���� �Ѵ�.
 * EF31 ������ UI���� 320x426 ���� Surface�� �����Ͽ� preview �Ѵ�.
 * pantech-mirror-crop �� "on" ���� �����ϰ�, pewview size 640x480 �� ���� �����Ѵ�.
*/
#define F_SKYCAM_ADD_CFG_MIRROR_CROP

#ifdef F_SKYCAM_ADD_CFG_FACE_FILTER
/* �ȸ��ν� �ַ�� ��� ��, ����� �ټ��� �󱼵鿡 ���������� ���͸� ������
 * �� �ִ� �������̽��� �߰��Ѵ�.
 *
 * ������ �̹��� �� �ټ��� �� �������� �����ϰ� �ش� ������ ��ġ�� ���
 * �ش� �� ������ ���͸� ������ �� ���θ� ON/OFF ������� ������ �� �ִ�.
 *
*/
#define F_SKYCAM_ADD_CFG_FACE_FILTER_RECT
#endif


/* 
 * Gingerbread���� OpenCore���� Stagefright�� ����ϵ��� ���� �Ǿ���.
 * EF31S�� EF32K���� ���� recording�� stagefright ����� ������ �Ǿ�
 * �켱 OpenCore�� ����ϵ��� �Ѵ�.
 * Stagefright�� ���ؼ��� ����� ���� �� ���� �Ѵ�.
*/
#define F_SKYCAM_CFG_OPENCORE


/* 
 * Gingerbread�� CameraService���� lockIfMessageWanted(int32_t msgType) �Լ��� �߰� �Ǿ���.
 * CameraService�� callback ó�� �Լ����� mLock�� check�Ͽ� LOCK �����̸�, UNLOCK���� waiting �Ѵ�.
 * capture ���� ���� UI �κ��� command�� �������� callback �Լ����� �̷� ���� ������ �߻��Ѵ�.
 * capture ���� �� ī�޶� ����� �̷� ���� CameraHAL���� UI�� ���� ���� �Ǵ� ��찡 �߻��Ѵ�.
 * UI�� ���� ����ǰ� CameraHAL �������� �ٽ� Camera�� ����Ǹ� ���������� Open ���� ���Ѵ�.
 * lockIfMessageWanted �Լ��� ��� ���� �ʵ��� �����Ͽ���.
*/
#define F_SKYCAM_FIX_CS_TRYLOCK

/*
camera id���� �˻��Ͽ� ���� app���� �ĸ� ī�޶�, ���� ī�޶� ���� ���۽� ���� �����ϰ� �Ǿ�
���Խ� open�� ����� �ñ⿡ �ǰų�(ȨŰ long Ű, ��ȯ), setparameter�� ���õǴ� ������� �߻��Ͽ�,
���� �ǵ����� ���� ���� ���� ������ �ϴ� ������
froyo�� ���������� �� �ĸ� ��� ī�޶� ���� ī�޶� release �������� ���� �Ұ��ϵ��� ����

HW, QCH ��� ������ ī�޶� ������ �����Ѵٸ� �Ʒ��� ������ �� �׽�Ʈ �� ��.
*/
#define F_SKYCAM_FIX_CS_CONNECT


#define F_SKYCAM_FIX_PREVIEW_ZOOM

#define F_SKYCAM_FIX_PREVIEW_FACE_FILTER

/* CTS qualcomm bug ���� 
 */
#define F_SKYCAM_FIX_CFG_FOCUS_DISTANCES

#define F_SKYCAM_FIX_CFG_FPS_RANGE

#define F_SKYCAM_FIX_CANCEL_AF

#define F_SKYCAM_ADD_CFG_READ_REG

/*�Կ����� ����� ĸ�� �������� ĸ�� ���� JPEG ���ڵ� �������� ����*/
#define F_SKYCAM_FIX_NOTIFYSHUTTER

/*ī�޶� �� ���Խ� ������ ������ ���� ���� ������ ī�޶� ����̹��� ���� ���� ���� �ʾ�����
�� ���� ��� �ڵ� - �� ���Խ� ����̹� ������ �����̸�, ���� ī�޶� reject�ϰ� driver ����*/
#define F_SKYCAM_DEVICE_CONNECT_FAIL_FIXED

/* vfe�� �� ���� ���� �� ��, ī�޶� ����� vfe�� �߸� �� ����� ���� stop���� ��� ó���� 
config_proc.c�� *((int *)(0xc0debadd)) = 0xdeadbeef; ������ ó���� ���� mediaserver �״� ������
���� ��� �ڵ� ���� */
#define F_SKYCAM_DEADBEEF_ERROR_FIX

/*#define F_SKYCAM_HIDDEN_FRONT_CAMERA*/
#define F_SKYCAM_FAKE_FRONT_CAMERA

/*----------------------------------------------------------------------------*/
/*  MISCELLANEOUS / QUALCOMM BUG                                              */
/*  �� ��� ����/������ Ư���� ���谡 ���� FEATURE ��� QUALCOMM ���׿�     */
/*  ���� �ӽ� ����, SBA ���� � ���� FEATURE ���                           */
/*----------------------------------------------------------------------------*/


/* Ŀ�� ���� �ڵ忡�� SKYCDBG/SKYCERR ��ũ�θ� ����Ͽ� ��µǴ� �޽�������
 * Ȱ��ȭ ��Ų��. 
 * kernel/arch/arm/mach-msm/include/mach/camera.h */
/*#define F_SKYCAM_LOG_PRINTK*/


/* ���� ���� ���̰ų� ���� ���䰡 �ʿ��� ����, �Ǵ� Ÿ �𵨿��� ���䰡 �ʿ���
 * ������� ��ŷ�ϱ� ���� ����Ѵ�. */
#define F_SKYCAM_TODO

/* ���� ����(vendor)�� SKYCDBG/SKYCERR �޼��� on off */
#define F_SKYCAM_LOG_OEM		

/* ���� ������ LOGV/LOGD/LOGI �޼��� on off */
#define F_SKYCAM_LOG_VERBOSE

/* QUALCOMM ������ �ڵ忡 ����� �� �޽����� �߰��Ѵ�. */
#define F_SKYCAM_LOG_DEBUG

/*
 * ���� ���ڵ� ���۰� ���� ���� ����� ���� ���� �����
 * ȿ���� ��� �� �ý��� ���Ϸ� ���� �Ҹ��� ����� ��� �߻�
 * ���ڵ� ������ start �� �� ������ ���� �� ������ wating
 * EF31S/EF32K ������ Sound�� kernel message�� ������ ��� �Ҹ��� �����,
 * kernel message�� ���ų� release build������ ���� �߻� ����.
*/
#define F_SKYCAM_QBUG_REC_BEEP_SOUND


/* 1600x1200, 1600x960 �ػ󵵿��� "zoom" �Ķ���͸� 21 �� ���� �� ������ ��
 * ����� YUV �̹����� CROP �ϴ� �������� �߻��ϴ� BUFFER OVERRUN ������ 
 * �ӽ� �����Ѵ�. 
 *
 * QualcommCameraHardware::receiveRawPicture() ���� crop_yuv420(thumbnail) 
 * ȣ�� �� �Ķ���ʹ� width=512, height=384, cropped_width=504, 
 * cropped_height=380 �̸� memcpy ���߿� SOURCE �ּҺ��� DESTINATION �ּҰ� 
 * �� Ŀ���� ��Ȳ�� �߻��Ѵ�.
 *
 * R5040 ���� QUALCOMM ���� Ȯ�� �� �����Ѵ�. (SR#308616) 
 * - R407705 FIXED ISSUE 6 �� ���� */
/* #define F_SKYCAM_QBUG_ZOOM_CAUSE_BUFFER_OVERRUN */


/* ��� �ػ󵵿��� ZOOM �� Ư�� ���� �̻����� ������ ���, 
 * EXIFTAGID_EXIF_PIXEL_X_DIMENSION, EXIFTAGID_EXIF_PIXEL_Y_DIMENSION �±�
 * ������ �߸��� ���� ����Ǵ� ������ �ӽ� �����Ѵ�.
 *
 * R5040 ���� QUALCOMM ���� Ȯ�� �� �����Ѵ�. (SR#307343) 
 * - R4077 FIXED ISSUE 12 �� ���� 
 * - vendor/qcom/proprietary/mm-still/jpeg/src/jpeg_writer.c �� ���� �ӽ� ����
 *   �ڵ�� ������ ����Ǿ� ���� */
/* #define F_SKYCAM_QBUG_EXIF_IMAGE_WIDTH_HEIGHT */


/* SKYCAM_PSJ_100610
 * �Կ��� ������� ���� ��Ȳ���� Stop preview�� �Ͽ� �޸𸮰� �����Ǵ� ��Ȳ ����
*/
#define F_SKYCAM_QBUG_SNAPSHOT_RELEASE_INTERRUPT

/* ������ ��ȭ ����/���Ḧ ������ �ݺ��ϰų�, �̾����� ������ ���¿��� �����Կ�
 * ���� �Կ��� ���, MediaPlayer �� �������ϸ鼭 HALT �߻��Ѵ�.
 *
 * MediaPlayer �� ���, ������ ������ ��� �߿� �� �ٽ� ��� �õ��� ��� 100%
 * �������ϹǷ� ���� ������ �����Ͽ� ����ؾ� �� ���, �ݵ�� ���� ����� �Ϸ�
 * �Ǿ����� ���θ� Ȯ�� �� ����ؾ� �Ѵ�. */
#define F_SKYCAM_QBUG_SKIP_CAMERA_SOUND

/*
 * zoom ���� �� snapshot jpeg encode���� �߻��ϴ� ���� ���� ���� ���� 
 */
#define F_SKYCAM_QBUG_JPEG_ZOOM

/* �Կ���/��ȭ�� ��� �߿� ������ ����� ���, CLIENT �Ҹ� �ÿ� �ش� �Կ���/
 * ��ȭ�� ������Ʈ�� ������ disconnect/clear �Ǹ鼭 MEDIA SERVER �� �״� ����
 * �����Ѵ�. */
#define F_SKYCAM_QBUG_STOP_CAMERA_SOUND


/* AF �� ���������� ����Ǵ��� HAL ���� ��ȯ �� üũ �� �߸��� ���� ����ϹǷ�
 * ������ �׻� AF ���з� �ν��ϴ� ������ �����Ѵ�. */
#define F_SKYCAM_QBUG_SET_PARM_AUTO_FOCUS


/* capture �� postview �ϱ��� black screen, postview �� preview �ϱ��� black screen ����.
*/
#define F_SKYCAM_QBUG_ADD_UNREGISTER2

/* CTS qualcomm bug ���� 
 */
#define F_SKYCAM_QBUG_CTS


/* ���� ���ڵ� �� SDCARD FULL �� �� ��쿡 ���������� ���ڵ� ������ ��������� �ʴ� ��� �߻�
*/
#define F_SKYCAM_QBUG_RECORDING_MAXSIZE
/*----------------------------------------------------------------------------*/
/*  MODEL-SPECIFIC CONSTANTS                                                  */
/*  �� ���� ��� ����                                                       */
/*----------------------------------------------------------------------------*/

/* camera select enum */
#define C_SKYCAM_SELECT_MAIN_CAM 		0
#define C_SKYCAM_SELECT_SUB_CAM 		1


/* ī�޶� ���� �ð��� �����ϱ� ���� ������ ���ϸ��̴�. */
#ifdef F_SKYCAM_FACTORY_PROC_CMD
#define C_SKYCAM_FNAME_FACTORY_PROC_CMD	"/data/.factorycamera.dat"
#endif


/* ���� ������ �ּ�/�ִ� ��Ŀ�� �����̴�. */
#ifdef F_SKYCAM_FIX_CFG_FOCUS_STEP
#define C_SKYCAM_MIN_FOCUS_STEP 0	/* ���Ѵ� (default) */
#define C_SKYCAM_MAX_FOCUS_STEP 9	/* ��ũ�� */
#endif


/* ���� ������ �ּ�/�ִ� ��� �ܰ��̴�. */
#ifdef F_SKYCAM_FIX_CFG_BRIGHTNESS
#define C_SKYCAM_MIN_BRIGHTNESS 0	/* -4 */
#define C_SKYCAM_MAX_BRIGHTNESS 8	/* +4 */
#endif


/* ���� ������ �ּ�/�ִ� ZOOM �ܰ��̴�. ���뿡���� ��� ���Ǽ��� ���� �ּ�/�ִ�
 * �ܰ踦 ������ ��� ����Ѵ�.
 * EF10S/EF12S ������ ������� ������ ��������� �ڵ���� ���ܵд�. */
#ifdef F_SKYCAM_FIX_CFG_ZOOM
#define C_SKYCAM_MIN_ZOOM 0
#define C_SKYCAM_MAX_ZOOM 8
#endif


/* ī�޶� ���뿡�� �Կ� �� ������ EXIF �±� ������ ������ ���� ������ 
 * �����Ѵ�. */
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


/* ������ ������ ����� ���, ��� ������ FPS �� �ּ�/�ִ� ���̴�. */
#ifdef F_SKYCAM_FIX_CFG_PREVIEW_FPS
#define C_SKYCAM_MIN_PREVIEW_FPS	5
#define C_SKYCAM_MAX_PREVIEW_FPS	30
#endif

#ifdef F_SKYCAM_CUST_CPU_CLK_FREQ
#define C_SKYCAM_CPU_PERF_LOCK_FNAME	"/sys/devices/system/cpu/cpu0/cpufreq/perf_lock"
#define C_SKYCAM_CPU_PERF_UNLOCK_FNAME	"/sys/devices/system/cpu/cpu0/cpufreq/perf_unlock"
#endif


/*
	�ΰ��� ī�޶� �� �Ѱ��� ī�޶� �������� ��, ī�޶� ��ȯ ��, ���� ���� ���� ī�޶��� off �� 
	��ȯ�� ī�޶��� on�� �ڵ尡 ��ġ�� ������ �߻��Ͽ� ���� ����Ͽ� ������ off��  �Ϸ�Ǹ� �ٸ� ī�޶���
	start�� �����ϵ��� ���� ��
*/
#define F_SKYCAM_DRIVER_COMPLETE

/*
����̹��� ���� ���� �ʾ�, ī�޶� ������ ���� �ʴ� ���� ���� ��� �ڵ�
*/ 
/*#define F_SKYCAM_FIX_MSM_OPEN_FAIL*/

#endif /* CUST_SKYCAM.h */
