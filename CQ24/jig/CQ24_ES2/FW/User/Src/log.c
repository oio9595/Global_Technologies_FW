/** @file log.c
 * 
 * @brief uart log message for test result
 *
 * @par       
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#include "main.h"
#include "log.h"
#include "types.h"
#include "ADS124S08.h"


extern UART_HandleTypeDef huart2;

extern test_mode_t gt_jig_test_mode;

extern uint32_t gn_test_tickcount;

extern uint16_t gn_auto_test_count;
uint16_t gn_auto_test_ok_count;
uint16_t gn_auto_test_ng_count;

char msg_buffer[256];

static const char* gn_mode_string[TEST_MODE_MAX] =
{
    "TEST NONE  ",
		
    "TEST_MODE_1_BY_6_CZ_1",
    "TEST_MODE_1_BY_6_CZ_2",
    "TEST_MODE_1_BY_6_CZ_3",
    "TEST_MODE_1_BY_6_CZ_4",
    
    "TEST_MODE_2_BY_3_CZ_1",
    "TEST_MODE_2_BY_3_CZ_2",
    "TEST_MODE_2_BY_3_CZ_3",
    "TEST_MODE_2_BY_3_CZ_4",
    
    "TEST_MODE_3_BY_2_CZ_1",
    "TEST_MODE_3_BY_2_CZ_2",
    "TEST_MODE_3_BY_2_CZ_3",
    "TEST_MODE_3_BY_2_CZ_4",
    
    "TEST RESULT",
};

static const char* get_test_mode_string(test_mode_t t_test_mode)
{
    if(t_test_mode >= TEST_MODE_MAX)
    {
        t_test_mode = TEST_MODE_NONE;
    }

    return gn_mode_string[t_test_mode];
}

static const char* get_result_string(uint8_t n_result)
{
    if(n_result)
    {
        return "NG";
    }
    else
    {
        return "OK";
    }
}

static const char* get_result_pos_string(uint8_t n_result, uint8_t n_pos)
{
    if((n_result & (0x01<<n_pos)) == (0x01<<n_pos))
    {
        return "NG";
    }
    else
    {
        return "OK";
    }
}

void print(const char* str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

void log_send_auto_test_start(uint16_t n_test_count)
{
    snprintf(msg_buffer, sizeof msg_buffer, "===                             RUN... AUTO TEST %5d                       ===\r\n", n_test_count);
    print(msg_buffer);
}

void log_send_manual_test_start(void)
{
    snprintf(msg_buffer, sizeof msg_buffer, "===                             RUN... MANUAL TEST(%2d)                       ===\r\n", gt_jig_test_mode);
    print(msg_buffer);
}

void log_send_function_test_min_max(min_max_t t_manual_function_test_min_max[FUNCTION_TEST_REPEAT_CNT][AMIC_O_MAX])
{
    snprintf(msg_buffer, sizeof msg_buffer, "===                             O# [MIN, MAX]                             ===\r\n");
    print(msg_buffer);
    for(uint16_t cnt=0 ; cnt < 1/*FUNCTION_TEST_REPEAT_CNT*/ ; ++cnt)
    {
        snprintf(msg_buffer, sizeof msg_buffer, "= REPEAT #%2u O1[%5u/%5u], O2[%5u/%5u], O3[%5u/%5u], O4[%5u/%5u], O5[%5u/%5u], O6[%5u/%5u] =\r\n", cnt, t_manual_function_test_min_max[cnt][AMIC_O1].min, t_manual_function_test_min_max[cnt][AMIC_O1].max, t_manual_function_test_min_max[cnt][AMIC_O2].min, t_manual_function_test_min_max[cnt][AMIC_O2].max, t_manual_function_test_min_max[cnt][AMIC_O3].min, t_manual_function_test_min_max[cnt][AMIC_O3].max, t_manual_function_test_min_max[cnt][AMIC_O4].min, t_manual_function_test_min_max[cnt][AMIC_O4].max, t_manual_function_test_min_max[cnt][AMIC_O5].min, t_manual_function_test_min_max[cnt][AMIC_O5].max, t_manual_function_test_min_max[cnt][AMIC_O6].min, t_manual_function_test_min_max[cnt][AMIC_O6].max);
        print(msg_buffer);
        snprintf(msg_buffer, sizeof msg_buffer, "=               [%5u],       [%5u],       [%5u],       [%5u],       [%5u],      [%5u]      =\r\n", (t_manual_function_test_min_max[cnt][AMIC_O1].max - t_manual_function_test_min_max[cnt][AMIC_O1].min), (t_manual_function_test_min_max[cnt][AMIC_O2].max - t_manual_function_test_min_max[cnt][AMIC_O2].min), (t_manual_function_test_min_max[cnt][AMIC_O3].max - t_manual_function_test_min_max[cnt][AMIC_O3].min), (t_manual_function_test_min_max[cnt][AMIC_O4].max - t_manual_function_test_min_max[cnt][AMIC_O4].min), (t_manual_function_test_min_max[cnt][AMIC_O5].max - t_manual_function_test_min_max[cnt][AMIC_O5].min), (t_manual_function_test_min_max[cnt][AMIC_O6].max - t_manual_function_test_min_max[cnt][AMIC_O6].min));
        print(msg_buffer);
    }
}

double cnv_adc_to_current(test_mode_t t_test_mode, uint8_t n_id, uint16_t n_mode_adc_reault[TEST_MODE_MAX][AMIC_O_MAX])
{
	float ret = 0;
	
	ret = ((n_mode_adc_reault[t_test_mode][n_id] * ADC_VOLT_PER_STEP * CURRENT_SENSE_C_HIGH * CURRENT_SENSE_RIN) / (1000000 * SAMPLE_TIME * CURRENT_SENSE_R));	/* mA */	

	return ret;
}

void log_send_auto_test_result(uint8_t n_msg_type, uint8_t n_test_mode_result[TEST_MODE_MAX], uint16_t n_mode_adc_reault[TEST_MODE_MAX][AMIC_O_MAX])
{
    uint8_t n_color_changed = 0;

    if(n_msg_type == MSG_NONE)
    {
    }
    else if(n_msg_type == MSG_DETAIL)
    {
        test_mode_t t_test_mode = TEST_MODE_NONE;

        //print("\033[2J\033[H"); 
        print("=================================================================================\r\n");
        snprintf(msg_buffer, sizeof msg_buffer, "===                AMIC TEST RESULT (%5u|%5u)/%5u (%2.2f sec)            ===\r\n", gn_auto_test_ok_count, gn_auto_test_ng_count, gn_auto_test_count, ((HAL_GetTick()-gn_test_tickcount)/1000.0));
        print(msg_buffer);
        print("=================================================================================\r\n");

        for(t_test_mode = TEST_MODE_1_BY_6_CZ_1 ; t_test_mode<TEST_MODE_RESULT ; ++t_test_mode)
        {
            if(n_test_mode_result[t_test_mode])
            {
                print(FOREGROUND_RED_COLOR);
                n_color_changed = 1;
            }

            switch(t_test_mode)
            {
				case TEST_MODE_1_BY_6_CZ_1:
				case TEST_MODE_1_BY_6_CZ_2:
				case TEST_MODE_1_BY_6_CZ_3:
				case TEST_MODE_1_BY_6_CZ_4:				
					
				case TEST_MODE_2_BY_3_CZ_1:
				case TEST_MODE_2_BY_3_CZ_2:
				case TEST_MODE_2_BY_3_CZ_3:
				case TEST_MODE_2_BY_3_CZ_4:
					
				case TEST_MODE_3_BY_2_CZ_1:
				case TEST_MODE_3_BY_2_CZ_2:
				case TEST_MODE_3_BY_2_CZ_3:
				case TEST_MODE_3_BY_2_CZ_4:

                    snprintf(msg_buffer, sizeof msg_buffer, "= %s (%s) [O1(%s) : %4u, O2(%s) : %4u, O3(%s) : %4u, O4(%s) : %4u, O5(%s) : %4u, O6(%s) : %4u] =\r\n", get_test_mode_string(t_test_mode), get_result_string(n_test_mode_result[t_test_mode]), get_result_pos_string(n_test_mode_result[t_test_mode], 0), n_mode_adc_reault[t_test_mode][0], get_result_pos_string(n_test_mode_result[t_test_mode], 1), n_mode_adc_reault[t_test_mode][1], get_result_pos_string(n_test_mode_result[t_test_mode], 2), n_mode_adc_reault[t_test_mode][2], get_result_pos_string(n_test_mode_result[t_test_mode], 3), n_mode_adc_reault[t_test_mode][3], get_result_pos_string(n_test_mode_result[t_test_mode], 4), n_mode_adc_reault[t_test_mode][4], get_result_pos_string(n_test_mode_result[t_test_mode], 5), n_mode_adc_reault[t_test_mode][5] );
                    print(msg_buffer);
                    snprintf(msg_buffer, sizeof msg_buffer, "=                           %4.3f,         %4.3f,         %4.3f,         %4.3f,         %4.3f,         %4.3f  =\r\n", cnv_adc_to_current(t_test_mode, 0, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 1, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 2, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 3, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 4, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 5, n_mode_adc_reault) );
                    print(msg_buffer);
                    break;
            }

            if(n_color_changed)
            {
                print(FOREGROUND_DEFAULT_COLOR);
                n_color_changed = 0;
            }
        }

        print("=================================================================================\r\n");
    }
    else if(n_msg_type == MSG_CVS)
    {
        test_mode_t t_test_mode = TEST_MODE_1_BY_6_CZ_1;

        print("COUNT,MODE,RESULT,O1,,O2,,O3,,O4,\r\n");
        for(t_test_mode = TEST_MODE_1_BY_6_CZ_1 ; t_test_mode<TEST_MODE_RESULT ; ++t_test_mode)
        {
            if(n_test_mode_result[t_test_mode])
            {
                print(FOREGROUND_RED_COLOR);
                n_color_changed = 1;
            }

            switch(t_test_mode)
            {
				case TEST_MODE_1_BY_6_CZ_1:
				case TEST_MODE_1_BY_6_CZ_2:
				case TEST_MODE_1_BY_6_CZ_3:
				case TEST_MODE_1_BY_6_CZ_4:
				
				case TEST_MODE_2_BY_3_CZ_1:
				case TEST_MODE_2_BY_3_CZ_2:
				case TEST_MODE_2_BY_3_CZ_3:
				case TEST_MODE_2_BY_3_CZ_4:
				
				case TEST_MODE_3_BY_2_CZ_1:
				case TEST_MODE_3_BY_2_CZ_2:
				case TEST_MODE_3_BY_2_CZ_3:
				case TEST_MODE_3_BY_2_CZ_4:

                    snprintf(msg_buffer, sizeof msg_buffer, "%u,%s,%s,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f\r\n", gn_auto_test_count, get_test_mode_string(t_test_mode), get_result_string(n_test_mode_result[t_test_mode]), n_mode_adc_reault[t_test_mode][0], cnv_adc_to_current(t_test_mode, 0, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][1], cnv_adc_to_current(t_test_mode, 1, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][2], cnv_adc_to_current(t_test_mode, 2, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][3], cnv_adc_to_current(t_test_mode, 3, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][4], cnv_adc_to_current(t_test_mode, 4, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][5], cnv_adc_to_current(t_test_mode, 5, n_mode_adc_reault));
                    print(msg_buffer);
                    break;
            }

            if(n_color_changed)
            {
                print(FOREGROUND_DEFAULT_COLOR);
                n_color_changed = 0;
            }
        }
    }
    else
    {
    }
}

void log_send_manual_test_result(test_mode_t t_test_mode, uint8_t n_msg_type, uint8_t n_test_mode_result[TEST_MODE_MAX], uint16_t n_mode_adc_reault[TEST_MODE_MAX][AMIC_O_MAX])
{
    uint8_t n_color_changed = 0;

    if(n_msg_type == MSG_NONE)
    {
        if(n_test_mode_result[t_test_mode])
        {
            print(FOREGROUND_RED_COLOR);
            n_color_changed = 1;
        }

        snprintf(msg_buffer, sizeof msg_buffer, "[MANUAL] %s test result (%s)[%u] =\r\n", get_test_mode_string(t_test_mode), get_result_string(n_test_mode_result[t_test_mode]), n_test_mode_result[t_test_mode]);
        print(msg_buffer);

        if(n_color_changed)
        {
            print(FOREGROUND_DEFAULT_COLOR);
            n_color_changed = 0;
        }
    }
    else if(n_msg_type == MSG_DETAIL)
    {
        //print("\033[2J\033[H"); 
        print("================================================================================\r\n");
        print("===                         AMIC MANUAL TEST RESULT                          ===\r\n");
        print("================================================================================\r\n");
        if(n_test_mode_result[t_test_mode])
        {
            print(FOREGROUND_RED_COLOR);
            n_color_changed = 1;
        }

        switch(t_test_mode)
        {
			case TEST_MODE_1_BY_6_CZ_1:
			case TEST_MODE_1_BY_6_CZ_2:
			case TEST_MODE_1_BY_6_CZ_3:
			case TEST_MODE_1_BY_6_CZ_4:
			
			case TEST_MODE_2_BY_3_CZ_1:
			case TEST_MODE_2_BY_3_CZ_2:
			case TEST_MODE_2_BY_3_CZ_3:
			case TEST_MODE_2_BY_3_CZ_4:
			
			case TEST_MODE_3_BY_2_CZ_1:
			case TEST_MODE_3_BY_2_CZ_2:
			case TEST_MODE_3_BY_2_CZ_3:
			case TEST_MODE_3_BY_2_CZ_4:

                snprintf(msg_buffer, sizeof msg_buffer, "= %s (%s) [O1(%s) : %4u, O2(%s) : %4u, O3(%s) : %4u, O4(%s) : %4u, O5(%s) : %4u, O6(%s) : %4u] =\r\n", get_test_mode_string(t_test_mode), get_result_string(n_test_mode_result[t_test_mode]), get_result_pos_string(n_test_mode_result[t_test_mode], 0), n_mode_adc_reault[t_test_mode][0], get_result_pos_string(n_test_mode_result[t_test_mode], 1), n_mode_adc_reault[t_test_mode][1], get_result_pos_string(n_test_mode_result[t_test_mode], 2), n_mode_adc_reault[t_test_mode][2], get_result_pos_string(n_test_mode_result[t_test_mode], 3), n_mode_adc_reault[t_test_mode][3], get_result_pos_string(n_test_mode_result[t_test_mode], 4), n_mode_adc_reault[t_test_mode][4], get_result_pos_string(n_test_mode_result[t_test_mode], 5), n_mode_adc_reault[t_test_mode][5] );
                print(msg_buffer);
                snprintf(msg_buffer, sizeof msg_buffer, "=                           %4.3f,         %4.3f,         %4.3f,         %4.3f,         %4.3f,         %4.3f  =\r\n", cnv_adc_to_current(t_test_mode, 0, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 1, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 2, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 3, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 4, n_mode_adc_reault), cnv_adc_to_current(t_test_mode, 5, n_mode_adc_reault) );
                print(msg_buffer);
                break;
            default:
                print("===                         INVALID TEST MODE ID                         ===\r\n");
                break;
        }

        if(n_color_changed)
        {
            print(FOREGROUND_DEFAULT_COLOR);
            n_color_changed = 0;
        }

        print("================================================================================\r\n");
    }
    else if(n_msg_type == MSG_CVS)
    {
        if(n_test_mode_result[t_test_mode])
        {
            print(FOREGROUND_RED_COLOR);
            n_color_changed = 1;
        }

        switch(t_test_mode)
        {
			case TEST_MODE_1_BY_6_CZ_1:
			case TEST_MODE_1_BY_6_CZ_2:
			case TEST_MODE_1_BY_6_CZ_3:
			case TEST_MODE_1_BY_6_CZ_4:
			
			case TEST_MODE_2_BY_3_CZ_1:
			case TEST_MODE_2_BY_3_CZ_2:
			case TEST_MODE_2_BY_3_CZ_3:
			case TEST_MODE_2_BY_3_CZ_4:
			
			case TEST_MODE_3_BY_2_CZ_1:
			case TEST_MODE_3_BY_2_CZ_2:
			case TEST_MODE_3_BY_2_CZ_3:
			case TEST_MODE_3_BY_2_CZ_4:

                print("MODE,RESULT,O1,O2,O3,O4,O5,O6\r\n");
                snprintf(msg_buffer, sizeof msg_buffer, "%s,%s,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f,%4u,%4.3f\r\n", get_test_mode_string(t_test_mode), get_result_string(n_test_mode_result[t_test_mode]), n_mode_adc_reault[t_test_mode][0], cnv_adc_to_current(t_test_mode, 0, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][1], cnv_adc_to_current(t_test_mode, 1, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][2], cnv_adc_to_current(t_test_mode, 2, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][3], cnv_adc_to_current(t_test_mode, 3, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][4], cnv_adc_to_current(t_test_mode, 4, n_mode_adc_reault), n_mode_adc_reault[t_test_mode][5], cnv_adc_to_current(t_test_mode, 5, n_mode_adc_reault));
                print(msg_buffer);
                break;
        }
        
        if(n_color_changed)
        {
            print(FOREGROUND_DEFAULT_COLOR);
            n_color_changed = 0;
        }
    }
    else
    {
    }
}

/*** end of file ***/

