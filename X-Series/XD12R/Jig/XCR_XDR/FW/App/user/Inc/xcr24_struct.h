

#define XC_TYPE_XC24R       (0U)
#define XC_TYPE_IC603       (1U)

#if (XC_REVISION == 0)
#elif (XC_REVISION == 1)
#else
#endif

#if (XC_MODEL_TYPE == XC_TYPE_XC24R)    /* XCR24 for VD */
    #include "xcr_struct.h"
#elif (XC_MODEL_TYPE == XC_TYPE_IC603)  /* IC603 for Mobis */
    #include "ic603_struct.h"
#else
    #error "Invlid XC_MODEL_TYPE"
#endif

