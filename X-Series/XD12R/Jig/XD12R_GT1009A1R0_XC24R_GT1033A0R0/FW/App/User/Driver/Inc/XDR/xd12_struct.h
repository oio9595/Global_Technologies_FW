#define XD_TYPE_XD12R       (0U)
#define XD_TYPE_XD12D       (1U)
#define XD_TYPE_IC601       (2U)

#define XD_REVISION_ES      (0U)
#define XD_REVISION_CS      (1U)

#define XD_MODEL_TYPE       (XD_TYPE_XD12R)
#define XD_REVISION         (XD_REVISION_CS)  /* ES: Engineering Sample, CS: Customer Sample */

#if (XD_MODEL_TYPE == XD_TYPE_XD12R)    /* XDR12R for VD */
    #if (XD_REVISION == XD_REVISION_ES)
        #include "xd12r_es_struct.h"
    #elif (XD_REVISION == XD_REVISION_CS)
        #include "xd12r_cs_struct.h"
    #else
        #error "Invalid XD12R_REVISION"
    #endif

#elif (XD_MODEL_TYPE == XD_TYPE_XD12D)  /* XDR12D for VD */
    #if (XD_REVISION == XD_REVISION_ES)
        #include "xd12d_es_struct.h"
    #elif (XD_REVISION == XD_REVISION_CS)
        #include "xd12d_cs_struct.h"
    #else
        #error "Invalid XD12D_REVISION"
    #endif

#elif (XD_MODEL_TYPE == XD_TYPE_IC601)  /* IC601 for Mobis*/
    #if (XD_REVISION == XD_REVISION_ES)
        #include "ic601_es_struct.h"
    #elif (XD_REVISION == XD_REVISION_CS)
        #include "ic601_cs_struct.h"
    #else
        #error "Invalid IC601_REVISION"
    #endif

#else
    #error "Invalid XD_MODEL_TYPE"
#endif

