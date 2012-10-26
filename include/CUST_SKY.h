#ifndef _CUST_SKY_FILE_
#define _CUST_SKY_FILE_


/* MODEL_ID  */
#if defined (CONFIG_MACH_EF31S)
#define MODEL_EF31S		0x3001
#define MODEL_EF31S_32K MODEL_EF31S
/* BOARD_VER */
//  Added EF11_WS15 by wjmin 2010.3.25
//  EF11_WS15 : 2/26 SMT. Use SP33K LCD
#define EF31S_TG10	(MODEL_EF31S<<8)+0x00
#define EF31S_PT10	(MODEL_EF31S<<8)+0x01
#define EF31S_WS10	(MODEL_EF31S<<8)+0x02
#define EF31S_WS20	(MODEL_EF31S<<8)+0x03
#define EF31S_WS21	(MODEL_EF31S<<8)+0x04
#define EF31S_ES10	(MODEL_EF31S<<8)+0x06
#define EF31S_ES20	(MODEL_EF31S<<8)+0x07
#define EF31S_ES21	(MODEL_EF31S<<8)+0x08


#define EF31S_TP10	(MODEL_EF31S<<8)+0x10
#define EF31S_TP20	(MODEL_EF31S<<8)+0x11

#elif defined(CONFIG_MACH_EF32K)
#define MODEL_EF32K		0x3002
#define MODEL_EF31S_32K MODEL_EF32K

#define EF32K_TG10	(MODEL_EF32K<<8)+0x00
#define EF32K_PT10	(MODEL_EF32K<<8)+0x01
#define EF32K_WS10	(MODEL_EF32K<<8)+0x02
#define EF32K_WS20	(MODEL_EF32K<<8)+0x03
#define EF32K_WS21	(MODEL_EF32K<<8)+0x04
#define EF32K_ES10	(MODEL_EF32K<<8)+0x06
#define EF32K_ES20	(MODEL_EF32K<<8)+0x07
#define EF32K_ES21	(MODEL_EF32K<<8)+0x08


#define EF32K_TP10	(MODEL_EF32K<<8)+0x10
#define EF32K_TP20	(MODEL_EF32K<<8)+0x11
#endif

/* cust features for android c&cpp modules. */
//#define CONFIG_HSUSB_PANTECH_OBEX /* SKT OEM DRIVER */
#define FEATURE_ANDROID_PANTECH_USB_SKY

#define FEATURE_ANDROID_PANTECH_USB_QC_FIX_BUG


#if defined(CONFIG_HSUSB_PANTECH_OBEX)
#define CONFIG_HSUSB_PANTECH_USB_TEST
#endif

#define FEATURE_PANTECH_MDS_MTC  /* MTC */
#if defined(FEATURE_PANTECH_MDS_MTC)
#define FEATURE_PANTECH_MAT      /* MAT */
#endif

#if defined(FEATURE_PANTECH_MDS_MTC)
#define FEATURE_PANTECH_DIAG_LARGE_PACKET
#endif

#define FEATURE_PANTECH_STABILITY  /* STABILITY */

//chjeon20100818@DS2 add
#if defined(FEATURE_AARM_RELEASE_MODE)
#define FEATURE_SKY_DM_MSG_VIEW_DISABLE
#define FEATURE_SW_RESET_RELEASE_MODE // use in release mode
#endif

#define FEATURE_SKY_SELF_UPGRADE_SDCARD     //chjeon20101004@DS2 add

#define CONFIG_SKY_WLAN
#define CONFIG_SKY_BLUETOOTH

/// [ ds2_min.woojung_2010.01.14 ] Added for Power off
#define CONFIG_SKY_POWER_OFF


#define FEATURE_PANTECH_VOLUME_CTL

#define FEATURE_PANTECH_BOOTING_EVENT_DROP

/* Global features for SKY camera framework. */
#include "CUST_SKYCAM.h"
/* [DS3] Kang Seong-Goo framework features about SurfaceFlinger */
#include "cust_sky_surfaceflinger.h"
/*[PS1] ST_Sang Youn Add*/
#define FEATURE_SKY_SAVE_LOG_SDCARD		

/* 2011/03/10 최병주
  PANTECH multimedia engine/codec 개발 관련 feature 정의파일 (최상위 FEATURE_PANTECH_MMP 등)*/
#include "cust_pantech_mmp.h"

#define FEATURE_SKY_SND_MODIFICATION

#if defined(MODEL_EF31S) || defined(MODEL_EF32K)
#define FEATURE_SKY_BOOT_LOGO_WITH_MDDI	// D_PS3_PZ1569_BOOT_LOGO - 20101227
#define FEATURE_SKY_BATT_ID_NO_CHECK_FLAG	// PZ1569_TESTMENU_FEATURE_SKY_BATT_ID_CHECK_SKIP, FEATURE_SKY_BATT_ID_CHECK_SKIP - 20110330 - BSH
#endif
#define FEATURE_SKY_CHG_LOGO //pz1716_test

#define FEATURE_SKY_LOADBAR_INCREASABLE	// D_PS1_PZ1569_SKY_LOADBAR - 20110226 - BSH

#define FEATURE_SW_RESET_CONVERT_HW_RESET //ST_LIM_20101229 Kill -9 servicemanager 실행시 리부팅 기능 피쳐링.

#define SKYMENU_ADD_INITLCD	// novapex_hsj 2011-01-19
#define FEATURE_SKY_YAFFS2_PLC_SUPPLEMENT // PLC TEST - Supplement

/* 특정 SD CARD 인식 관련하여 Power Control을 Sleep으로 진입하더라도 OFF시키지 않는다.*/
#define FAETURE_SKY_SDCARD_DETECT	// jwchoi 2011.03.24

#define FEATURE_SKY_CTS_TEST_2P3_R2	// PZ1569_SYS_PERMISSION_MODIFY_FOR_CTS - 20110413 - BSH
#define FEATURE_SKY_PDIP_MODIFY_FOR_CTS	// PZ1569_PDIP_MODIFY_FOR_CTS - 20110413 - BSH

//#define FEATURE_SKY_USB_STATUS_SUPPLEMENT //PZ1565 USB CONFIGURATION CONNECT STATUS SUPPLEMENT

// FEATURE_SKY_MODEM_TARGET_COMMON [
#include "cust_sky_cp.h"
// ]

/*  [PS11 Team Feature] */
#include "cust_sky_ds.h" //FEATURE_SKY_DS_INIT
#define FEATURE_67023		//[BIH] patch 67023 - for wake-up problem
#define FEATURE_67024		//[BIH] patch 67024 - for wake-up problem
#endif/*_CUST_SKY_FILE_*/

