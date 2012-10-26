//
// lived 2009.11.05
// FEATURE define
//

#ifndef SKY_FRAMEWORK_FEATURE
#define SKY_FRAMEWORK_FEATURE

// Debug Msg
#if 1 //#if TARGET_BUILD_VARIANT == eng
#define TEST_MSG(...)   ((void)LOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))
//#define TEST_MSG(...)   ((void)LOG(LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define SKY_LOG_DEBUG
#else
#define TEST_MSG(...)
#endif

//#define SKY_SURFACE_LR_CHANGE	// for Surface L/R dynamic change	//pantch_lks@PS3_20111118 - prepare ef31s_ef32k 	
#define SKY_ADD_SURFACE_BUFFER_HEAP_270 // for VGA camera
#define SKY_LCD_LOADBAR_ANDROID //leecy.. 
#define SKY_FIX_REFRESH_RATE
#define SKY_SET_BACKLIGHT_BEFORE_BOOTANIM

#define TODO_APP_DISPLAY_TYPE_MDDI  	1		//pantch_lks@PS3_20111118 - prepare ef31s_ef32k 	
//#define SKY_MDDI_DEBUG 							//pantch_lks@PS3_20111118 - prepare ef31s_ef32k 
#define ENABLE_MDDI_MULTI_READ_WRITE			//pantch_lks@PS3_20111118 - prepare ef31s_ef32k 	

//#define SKY_SURFACE_VIEW_LR						//pantch_lks@PS3_20110112 - ef31s_ef32k 	

#define SKYMENU_ADD_INITLCD	// novapex_hsj 2011-01-19

#endif  // SKY_FRAMEWORK_FEATURE
