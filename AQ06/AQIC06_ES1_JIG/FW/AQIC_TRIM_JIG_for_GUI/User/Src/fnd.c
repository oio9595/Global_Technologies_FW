/** @file fnd.c
 * 
 * @brief 
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#include "main.h"
#include "as1105.h"

#include "fnd.h"

void fnd_init(void)
{
    //as1105_init();
}

void fnd_test_mode_update(test_mode_t t_test_mode)
{
    uint8_t n_fnd_digit[3] = {0,};

    /* FND display */
    n_fnd_digit[0] = t_test_mode;
    n_fnd_digit[1] = CHAR_DASH;   /* '-' */
    n_fnd_digit[2] = CHAR_DASH;   /* '-' */

    as1105_write_digit_multi(0, n_fnd_digit, 3);
}

void fnd_pwr_state_update(power_state_t t_power_state, auto_manual_t t_manual_test_state)
{
    /* FND display */
    /* power off        : oFF */
    /* power on(auto)   : on  */
    /* power on(manual) : on- */
    uint8_t n_fnd_digit[3] = {0,};

    if(t_power_state == PWR_ON)
    {
        n_fnd_digit[0] = CHAR_o;
        n_fnd_digit[1] = CHAR_n;
        if(t_manual_test_state == TEST_MANUAL)
        {
            n_fnd_digit[2] = CHAR_DASH;
        }
        else
        {
            n_fnd_digit[2] = CHAR_BLANK;
        }
    }
    else
    {
        n_fnd_digit[0] = CHAR_o;
        n_fnd_digit[1] = CHAR_F;
        n_fnd_digit[2] = CHAR_F;
    }
    
    as1105_write_digit_multi(3, n_fnd_digit, 3);
}

void fnd_test_result_update(test_mode_t t_mode, uint8_t n_result)
{
    /* FND display */
    /* power off        : oFF */
    /* power on(auto)   : on  */
    /* power on(manual) : on- */
    uint8_t n_fnd_digit[3] = {0,};

    if(TEST_MODE_RESULT == t_mode)
    {
        n_fnd_digit[0] = CHAR_BLANK;
    }
    else
    {
        if(9 < t_mode)
        {
            if(10 == t_mode)
            {
                n_fnd_digit[0] = CHAR_A;
            }
            else if(11 == t_mode)
            {
                n_fnd_digit[0] = CHAR_P;
            }
            else if(12 == t_mode)
            {
                n_fnd_digit[0] = CHAR_L;
            }
            else
            {
                n_fnd_digit[0] = CHAR_E;
            }
        }
        else
        {
            n_fnd_digit[0] = t_mode;
        }
    }

    if(n_result == 0) /* OK */
    {
        n_fnd_digit[1] = 0;
        n_fnd_digit[2] = 0;
    }
    else
    {
        n_fnd_digit[1] = CHAR_DASH;
        n_fnd_digit[2] = CHAR_DASH;
    }

    as1105_write_digit_multi(0, n_fnd_digit, 3);
}

/*** end of file ***/

