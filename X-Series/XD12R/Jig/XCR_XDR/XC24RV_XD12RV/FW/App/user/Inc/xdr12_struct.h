

#define XD_TYPE_XDR12R      (0U)
#define XD_TYPE_XDR12D      (1U)
#define XD_TYPE_IC601       (2U)


#if (XD_MODEL_TYPE == XD_TYPE_XDR12R)    /* XDR12R for VD */

    #if (XD_REVISION == 0)
        #include "xdr12r_es0_struct.h"
    #elif (XD_REVISION == 1)
        #include "xdr12r_es1_struct.h"
    #else
        #error "Invalid XD12R_REVIRSION"
    #endif

#elif (XD_MODEL_TYPE == XD_TYPE_XDR12D)  /* XDR12D for VD */

    #if (XD_REVISION == 0)
        #include "xdr12d_es0_struct.h"
    #elif (XD_REVISION == 1)
        #include "xdr12d_es1_struct.h"
    #else
        #error "Invalid XD12D_REVIRSION"
    #endif

#elif (XD_MODEL_TYPE == XD_TYPE_IC601)  /* IC601 for Mobis*/

    #if (XD_REVISION == 0)
        #include "ic601_es0_struct.h"
    #elif (XD_REVISION == 1)
        #include "ic601_es1_struct.h"
    #else
        #error "Invalid IC601_REVIRSION"
    #endif

#else
    #error "Invlid XD_MODEL_TYPE"
#endif

