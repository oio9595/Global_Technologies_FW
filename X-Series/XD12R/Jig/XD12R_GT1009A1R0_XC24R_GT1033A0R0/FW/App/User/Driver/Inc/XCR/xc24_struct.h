#define XC_TYPE_XC24R       (0U)
#define XC_TYPE_IC603       (1U)

#define XC_REVISION_ES      (0U)
#define XC_REVISION_CS      (1U)

#define XC_MODEL_TYPE       (XC_TYPE_XC24R)
#define XC_REVISION         (XC_REVISION_ES)  /* ES: Engineering Sample, CS: Customer Sample */

#if (XC_MODEL_TYPE == XC_TYPE_XC24R)    /* XCR24 for VD */
    #if (XC_REVISION == XC_REVISION_ES)
        #include "xc24r_es_struct.h"
    #elif (XC_REVISION == XC_REVISION_CS)
        #include "xc24r_cs_struct.h"
    #else
        #error "Invalid XC24R_REVISION"
    #endif

#elif (XC_MODEL_TYPE == XC_TYPE_IC603)  /* IC603 for Mobis */
    #if (XC_REVISION == XC_REVISION_ES)
        #include "ic603_es_struct.h"
    #elif (XC_REVISION == XC_REVISION_CS)
        #include "ic603_cs_struct.h"
    #else
        #error "Invalid IC603_REVISION"
    #endif

#else
    #error "Invalid XC_MODEL_TYPE"
#endif