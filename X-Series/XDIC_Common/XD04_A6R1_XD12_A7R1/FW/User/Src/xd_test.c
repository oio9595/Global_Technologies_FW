
/** @file xd_test.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) XXX. All rights reserved.
 */
#define __XDIC_TEST_C__

#include "main.h"
#include "config.h"
#include "xdic.h"
#include "xc24.h"
#include "xd_test.h"
#include "JigBd_IF.h"
#include "ads124s08.h"

typedef void (*func_t)(void);

/* XDIC Common Spec */
#define XDIC_ERROR_RATE         (0.026f) // 2.6%
#define XDIC_VREF_TARGET        (2.2f)
#define XDIC_OSC_TARGET         (39.36f)
#define XDIC_OSC_ERROR_RATE     (0.03f) // 3.0%
#define XDIC_LDO_SWEEP_TARGET   (0.0015f) // 1.5mV

/* XD12 Current Spec */
#define XD12_GAIN_P1            (1000U)
#define XD12_GAIN_P2            (3000U)
#define XD12_GAIN_TARGET        (3.907f) // Delta

#define XD12_OFS_P1             (350U)
#define XD12_OFS_P2             (600U)
#define XD12_OFS_TARGET         (1.00f) // Average

#define XD12_ICC_TARGET         (2.95f)

/* XD04 Current Spec */
#define XD04_ICTL_L_P1          (500U)
#define XD04_ICTL_L_P2          (900U)
#define XD04_ICTL_L_TARGET      (4.102f) // Average

#define XD04_ICTL_H_P1          (400U)
#define XD04_ICTL_H_P2          (1300U)
#define XD04_ICTL_H_TARGET      (26.57f) // Average

#define XD04_ICC_TARGET         (1.8f)

static const char* gs_xdic_test_mode[XDIC_TEST_MAX] =
{
    "XDIC_TEST_VREF_CTL        [V]",
    "XDIC_TEST_OSC_FREQUENCY [MHz]",
    "XDIC_TEST_CURRENT_TYPE_A [mA]",
    "XDIC_TEST_CURRENT_TYPE_B [mA]",
    "XDIC_TEST_ICC            [mA]",
    "XDIC_TEST_LDO_SWEEP       [V]",
};

static const char* gs_xdic_test_result[2] =
{
    "\033[32m(O)\033[0m",
    "\033[31m(X)\033[0m",
};

static const char* gs_xdic_red[2] =
{
    "\033[31m",
    "\033[0m",
};

typedef struct tag_XDIC_TEST_CONDITION_T
{
    float f_target_min;
    float f_target_max;
    uint16_t vref_p1;
    uint16_t vref_p2;
    func_t p_func;
    current_gain_t current_gain;
} xdic_test_condition_t;

static xdic_test_step_t gt_xdic_test_step;
static xdic_test_mode_t gt_xdic_test_mode;

static uint8_t gn_xdic_channel;
static uint8_t gn_xdic_measure_count;
static uint16_t gn_xdic_temp_adc[2];
static uint16_t gn_task_delay;

static float gf_xdic_measured_vref;
static float gf_xdic_measured_osc;
static float gf_xdic_measured_icc;

static float gf_xdic_measured_current_A[XDIC_CH_MAX];
static float gf_xdic_measured_current_B[XDIC_CH_MAX];

static float gf_xdic_measured_ldo_sweep[64];
static xdic_test_condition_t gt_xdic_trim_condition[XDIC_TEST_MAX];

void XDIC_Test_Param_Init(void)
{
    // VREF
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].f_target_min = 2.0f; //XDIC_VREF_TARGET  * (1.0f - XDIC_ERROR_RATE)
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].f_target_max = XDIC_VREF_TARGET  * (1.0f + XDIC_ERROR_RATE);
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].vref_p1 = 0;
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].vref_p2 = 0;
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].p_func = XDIC_Trim_Init_VREF_CTL;
    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].current_gain = GAIN_LOW; // Don't Care

    // OSC
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].f_target_min = XDIC_OSC_TARGET  * (1.0f - XDIC_OSC_ERROR_RATE);
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].f_target_max = XDIC_OSC_TARGET  * (1.0f + XDIC_OSC_ERROR_RATE);
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].vref_p1 = 0;
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].vref_p2 = 0;
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].p_func = XDIC_Trim_Init_OSC;
    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].current_gain = GAIN_LOW; // Don't Care

    if (gb_xdic_type_is_xd04)
    {
        // Current Type A - XD04 ICTL_L
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].f_target_min = XD04_ICTL_L_TARGET  - 1.0f; // not trimmed
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].f_target_max = XD04_ICTL_L_TARGET  + 1.0f; // not trimmed
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p1 = XD04_ICTL_L_P1;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p2 = XD04_ICTL_L_P2;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].p_func = XDIC_Trim_Init_Current_Type_A;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].current_gain = GAIN_MID;

        // Current Type B - XD04 ICTL_H
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].f_target_min = XD04_ICTL_H_TARGET  * (1.0f - XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].f_target_max = XD04_ICTL_H_TARGET  * (1.0f + XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p1 = XD04_ICTL_H_P1;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p2 = XD04_ICTL_H_P2;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].p_func = XDIC_Trim_Init_Current_Type_B;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].current_gain = GAIN_HIGH;

        gt_xdic_trim_condition[XDIC_TEST_ICC].f_target_min = XD04_ICC_TARGET;
        gt_xdic_trim_condition[XDIC_TEST_ICC].f_target_max = 10.0f;//XD04_ICC_TARGET  * (1.0f + XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_ICC].vref_p1 = 0;
        gt_xdic_trim_condition[XDIC_TEST_ICC].vref_p2 = 0;
        gt_xdic_trim_condition[XDIC_TEST_ICC].p_func = XDIC_Trim_Init_ICC;
        gt_xdic_trim_condition[XDIC_TEST_ICC].current_gain = GAIN_LOW; // Don't Care
    }
    else
    {
        // Current Type A - XD12 GAIN
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].f_target_min = XD12_GAIN_TARGET  * (1.0f - XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].f_target_max = XD12_GAIN_TARGET  * (1.0f + XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p1 = XD12_GAIN_P1;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p2 = XD12_GAIN_P2;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].p_func = XDIC_Trim_Init_Current_Type_A;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].current_gain = GAIN_MID;

        // Current Type B - XD12 OFS
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].f_target_min = XD12_OFS_TARGET  * (1.0f - XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].f_target_max = XD12_OFS_TARGET  * (1.0f + XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p1 = XD12_OFS_P1;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p2 = XD12_OFS_P2;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].p_func = XDIC_Trim_Init_Current_Type_B;
        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].current_gain = GAIN_MID;

        gt_xdic_trim_condition[XDIC_TEST_ICC].f_target_min = XD12_ICC_TARGET;
        gt_xdic_trim_condition[XDIC_TEST_ICC].f_target_max = 10.0f;//XD12_ICC_TARGET  * (1.0f + XDIC_ERROR_RATE);
        gt_xdic_trim_condition[XDIC_TEST_ICC].vref_p1 = 0;
        gt_xdic_trim_condition[XDIC_TEST_ICC].vref_p2 = 0;
        gt_xdic_trim_condition[XDIC_TEST_ICC].p_func = XDIC_Trim_Init_ICC;
        gt_xdic_trim_condition[XDIC_TEST_ICC].current_gain = GAIN_LOW; // Don't Care
    }

    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].f_target_min = XDIC_LDO_SWEEP_TARGET;
    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].f_target_max = 10.0f;
    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].vref_p1 = 0;
    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].vref_p2 = 0;
    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].p_func = XDIC_Trim_Init_LDO_CTL;
    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].current_gain = GAIN_LOW; // Don't Care
}

void XDIC_Test_Start(void)
{
    XDIC_Detect_Type();
    if (gt_xdic_test_step == XDIC_TEST_STEP_NONE)
    {
        gt_xdic_test_step = XDIC_TEST_STEP_IC_INIT;
    }
}

static bool XDIC_Judge_Test_Result(xdic_test_mode_t test_mode, float val)
{
    bool test_result = false;
    if (val > gt_xdic_trim_condition[test_mode].f_target_min && val < gt_xdic_trim_condition[test_mode].f_target_max)
    {
        test_result = true;
    }
    print(LOG_DEBUG, "val = %7.3f, min = %7.3f, max = %7.3f, test_result = %d\r\n", val, gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, test_result);
    return test_result;
}

static void XDIC_Display_Test_Result(void)
{
    bool judge_result = false;
    for (xdic_test_mode_t test_mode = XDIC_TEST_START ; test_mode < XDIC_TEST_MAX ; ++test_mode)
    {
        switch (test_mode)
        {
        case XDIC_TEST_VREF_CTL:
            judge_result = XDIC_Judge_Test_Result(XDIC_TEST_VREF_CTL, gf_xdic_measured_vref);
            if (judge_result)
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_VREF_CTL], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_vref, gs_xdic_test_result[0]);
            }
            else
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_VREF_CTL], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_vref, gs_xdic_test_result[0]);
            }
            break;
        case XDIC_TEST_OSC_FREQUENCY:
            judge_result = XDIC_Judge_Test_Result(XDIC_TEST_OSC_FREQUENCY, gf_xdic_measured_osc);
            if (judge_result)
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_OSC_FREQUENCY], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_osc, gs_xdic_test_result[0]);
            }
            else
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_OSC_FREQUENCY], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_osc, gs_xdic_test_result[1]);
            }
            break;
        case XDIC_TEST_CURRENT_TYPE_A:
            print(LOG_INFO, "%s [%7.4f ~ %7.4f],", gs_xdic_test_mode[XDIC_TEST_CURRENT_TYPE_A], gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max);
            for (uint8_t ch = 0 ; ch < gn_xdic_channel_size ; ++ch)
            {
                judge_result = XDIC_Judge_Test_Result(XDIC_TEST_CURRENT_TYPE_A, gf_xdic_measured_current_A[ch]);
                if (judge_result)
                {
                    print(LOG_INFO, "%7.3f%s,", gf_xdic_measured_current_A[ch], gs_xdic_test_result[0]);
                }
                else
                {
                    print(LOG_INFO, "%7.3f%s,", gf_xdic_measured_current_A[ch], gs_xdic_test_result[1]);
                }
            }
            print(LOG_INFO, "\r\n");
            break;
        case XDIC_TEST_CURRENT_TYPE_B:
            print(LOG_INFO, "%s [%7.4f ~ %7.4f],", gs_xdic_test_mode[XDIC_TEST_CURRENT_TYPE_B], gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max);
            for (uint8_t ch = 0 ; ch < gn_xdic_channel_size ; ++ch)
            {
                judge_result = XDIC_Judge_Test_Result(XDIC_TEST_CURRENT_TYPE_B, gf_xdic_measured_current_B[ch]);
                if (judge_result)
                {
                    print(LOG_INFO, "%7.3f%s,", gf_xdic_measured_current_B[ch], gs_xdic_test_result[0]);
                }
                else
                {
                    print(LOG_INFO, "%7.3f%s,", gf_xdic_measured_current_B[ch], gs_xdic_test_result[1]);
                }
            }
            print(LOG_INFO, "\r\n");
            break;
        case XDIC_TEST_ICC:
            judge_result = XDIC_Judge_Test_Result(XDIC_TEST_ICC, gf_xdic_measured_icc);
            if (judge_result)
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_ICC], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_icc, gs_xdic_test_result[0]);
            }
            else
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f],%7.3f%s\r\n", gs_xdic_test_mode[XDIC_TEST_ICC], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gf_xdic_measured_icc, gs_xdic_test_result[1]);
            }
            break;
        case XDIC_TEST_LDO_SWEEP:
            judge_result = true;
            for (uint8_t i = 2 ; i < 32 ; ++i)
            {
                if (gf_xdic_measured_ldo_sweep[i] - gf_xdic_measured_ldo_sweep[i-1] <= XDIC_LDO_SWEEP_TARGET)
                {
                    judge_result = false;
                    break;
                }
                if (gf_xdic_measured_ldo_sweep[i] > 1.65f)
                {
                    break;
                }
            }

            if (judge_result)
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f] %s\r\n", gs_xdic_test_mode[XDIC_TEST_LDO_SWEEP], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gs_xdic_test_result[0]);
            }
            else
            {
                print(LOG_INFO, "%s [%7.4f ~ %7.4f] %s\r\n", gs_xdic_test_mode[XDIC_TEST_LDO_SWEEP], \
                    gt_xdic_trim_condition[test_mode].f_target_min, gt_xdic_trim_condition[test_mode].f_target_max, gs_xdic_test_result[1]);
            }

            for (uint8_t i = 0; i < 32; ++i)
            {
                bool compare = (i >= 2);
                if (compare && (gf_xdic_measured_ldo_sweep[i] - gf_xdic_measured_ldo_sweep[i - 1] <= XDIC_LDO_SWEEP_TARGET))
                {
                    print(LOG_INFO, "%s%5.3f%s\t", gs_xdic_red[0], gf_xdic_measured_ldo_sweep[i], gs_xdic_red[1]);
                }
                else
                {
                    print(LOG_INFO, "%5.3f\t", gf_xdic_measured_ldo_sweep[i]);
                }
                if ((i + 1) % 16 == 0)
                {
                    print(LOG_INFO, "\r\n");
                }
                if (gf_xdic_measured_ldo_sweep[i] > 1.65f)
                {
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
    print(LOG_INFO, "\r\n");
}

void XDIC_Test_Task(void)
{
    if (gt_xdic_test_step != XDIC_TEST_STEP_NONE)
    {
        if (gn_task_delay)
        {
            --gn_task_delay;
        }
        else
        {
            switch(gt_xdic_test_step)
            {
            case XDIC_TEST_STEP_IC_INIT:
                XDIC_Test_Param_Init();
                if (IS_XC24_Support())
                {
                    XC24_Init();
                }
                XDIC_Init();
                gt_xdic_test_mode = XDIC_TEST_VREF_CTL;
                gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                break;
            case XDIC_TEST_STEP_MODE_INIT:
                switch (gt_xdic_test_mode)
                {
                case XDIC_TEST_VREF_CTL:
                    gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_VREF_CTL].current_gain);
                    break;
                case XDIC_TEST_OSC_FREQUENCY:
                    gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_OSC_FREQUENCY].current_gain);
                    break;
                case XDIC_TEST_CURRENT_TYPE_A:
                    gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].current_gain);
                    break;
                case XDIC_TEST_CURRENT_TYPE_B:
                    gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].current_gain);
                    break;
                case XDIC_TEST_ICC:
                    gt_xdic_trim_condition[XDIC_TEST_ICC].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_ICC].current_gain);
                    break;
                case XDIC_TEST_LDO_SWEEP:
                    gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].p_func();
                    JigBD_IF_Change_Current_Gain(gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].current_gain);
                    break;
                default:
                    break;
                }
                gn_xdic_temp_adc[0] = 0;
                gn_xdic_temp_adc[1] = 0;
                gn_xdic_measure_count = 0;
                gn_xdic_channel = XDIC_CH_01;
                gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                break;
            case XDIC_TEST_STEP_CHANGE_OUTPUT:
                switch (gt_xdic_test_mode)
                {
                case XDIC_TEST_VREF_CTL:
                    break;
                case XDIC_TEST_OSC_FREQUENCY:
                    break;
                case XDIC_TEST_CURRENT_TYPE_A:
                    JigBD_IF_Select_Output_Ch(gn_xdic_channel);
                    if (gn_xdic_measure_count == 0)
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p1);
                    }
                    else
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].vref_p2);
                    }
                    break;
                case XDIC_TEST_CURRENT_TYPE_B:
                    JigBD_IF_Select_Output_Ch(gn_xdic_channel);
                    if (gn_xdic_measure_count == 0)
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p1);
                    }
                    else
                    {
                        XDIC_Set_Max_Curr_Vref(gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].vref_p2);
                    }
                    break;
                case XDIC_TEST_ICC:
                    break;
                case XDIC_TEST_LDO_SWEEP:
                    XDIC_Set_VREF_CTL(gt_xdic_trim_condition[XDIC_TEST_LDO_SWEEP].vref_p1 + gn_xdic_measure_count * 2U);
                    break;
                default:
                    break;
                }
                gt_xdic_test_step = XDIC_TEST_STEP_SET_ADC_CH;
                break;
            case XDIC_TEST_STEP_SET_ADC_CH:
                if (gt_xdic_test_mode != XDIC_TEST_ICC)
                {
                    ADS114S08_Select_Input_CH(ADS114S08_CH_XD_IOUT);
                }
                else
                {
                    if (gn_xdic_measure_count == 0)
                    {
                        ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_P);
                    }
                    else
                    {
                        ADS114S08_Select_Input_CH(ADS114S08_CH_XD_ICC_N);
                    }
                }
                gn_task_delay = 10;
                gt_xdic_test_step = XDIC_TEST_STEP_START_ADC_CONVERSION;
                break;
            case XDIC_TEST_STEP_START_ADC_CONVERSION:
                switch (gt_xdic_test_mode)
                {
                case XDIC_TEST_VREF_CTL:
                    JigBD_IF_Start_MCU_ADC();
                    gn_task_delay = 2;
                    break;
                case XDIC_TEST_OSC_FREQUENCY:
                    JigBD_IF_Start_Input_Capture();
                    gn_task_delay = 110;
                    break;
                case XDIC_TEST_CURRENT_TYPE_A:
                    ADS114S08_Set_Start(1);
                    gn_task_delay = 2;
                    break;
                case XDIC_TEST_CURRENT_TYPE_B:
                    ADS114S08_Set_Start(1);
                    gn_task_delay = 2;
                    break;
                case XDIC_TEST_ICC:
                    ADS114S08_Set_Start(1);
                    gn_task_delay = 2;
                    break;
                case XDIC_TEST_LDO_SWEEP:
                    JigBD_IF_Start_MCU_ADC();
                    gn_task_delay = 2;
                    break;
                default:
                    break;
                }
                gt_xdic_test_step = XDIC_TEST_STEP_GET_ADC_CH;
                break;
            case XDIC_TEST_STEP_GET_ADC_CH:
                switch (gt_xdic_test_mode)
                {
                case XDIC_TEST_VREF_CTL:
                    gn_xdic_temp_adc[0] = JigBD_IF_Get_MCU_ADC();
                    gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    break;
                case XDIC_TEST_OSC_FREQUENCY:
                    //JigBD_IF_Wait_Input_Capture_Done();
                    JigBD_IF_Stop_Input_Capture();
                    gn_xdic_temp_adc[0] = (uint16_t)(JigBD_IF_Get_Input_Capture_Freq());
                    gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    break;
                case XDIC_TEST_CURRENT_TYPE_A:
                    ADS114S08_Wait_Done();
                    gn_xdic_temp_adc[gn_xdic_measure_count] = ADS114S08_Get_ADC_Value();
                    ++gn_xdic_measure_count;
                    if (gn_xdic_measure_count >= 2)
                    {
                        gn_xdic_measure_count = 0;
                        gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    }
                    else
                    {
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    XDIC_Set_Max_Curr_Vref(0);
                    break;
                case XDIC_TEST_CURRENT_TYPE_B:
                    ADS114S08_Wait_Done();
                    gn_xdic_temp_adc[gn_xdic_measure_count] = ADS114S08_Get_ADC_Value();
                    ++gn_xdic_measure_count;
                    if (gn_xdic_measure_count >= 2)
                    {
                        gn_xdic_measure_count = 0;
                        gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    }
                    else
                    {
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    XDIC_Set_Max_Curr_Vref(0);
                    break;
                case XDIC_TEST_ICC:
                    ADS114S08_Wait_Done();
                    gn_xdic_temp_adc[gn_xdic_measure_count] = ADS114S08_Get_ADC_Value();
                    ++gn_xdic_measure_count;
                    if (gn_xdic_measure_count >= 2)
                    {
                        gn_xdic_measure_count = 0;
                        gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    }
                    else
                    {
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    break;
                case XDIC_TEST_LDO_SWEEP:
                    gn_xdic_temp_adc[0] = JigBD_IF_Get_MCU_ADC();
                    gt_xdic_test_step = XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE;
                    break;
                default:
                    break;
                }
                break;
            case XDIC_TEST_STEP_CONVERSION_ADC_TO_VALUE :
                switch (gt_xdic_test_mode)
                {
                case XDIC_TEST_VREF_CTL:
                    gf_xdic_measured_vref = JigBD_IF_Convert_MCU_ADC_To_Volt(gn_xdic_temp_adc[0]);
                    gt_xdic_test_mode = XDIC_TEST_OSC_FREQUENCY;
                    gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                    break;
                case XDIC_TEST_OSC_FREQUENCY:
                    gf_xdic_measured_osc = JigBD_IF_Reconvert_XDIC_Original_Freq((double)gn_xdic_temp_adc[0]);
                    gt_xdic_test_mode = XDIC_TEST_CURRENT_TYPE_A;
                    gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                    break;
                case XDIC_TEST_CURRENT_TYPE_A:
                    if (gb_xdic_type_is_xd04)
                    {
                        // ICTL_L : Average
                        gf_xdic_measured_current_A[gn_xdic_channel] = JigBD_IF_Convert_Adc_To_Current((uint16_t)(((float)(gn_xdic_temp_adc[1] + gn_xdic_temp_adc[0]) / 2.0f) + 0.5f),\
                            gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].current_gain);
                    }
                    else
                    {
                        // GAIN : Delta
                        gf_xdic_measured_current_A[gn_xdic_channel] = JigBD_IF_Convert_Adc_To_Current((gn_xdic_temp_adc[1] - gn_xdic_temp_adc[0]),\
                            gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_A].current_gain);
                    }
                    ++gn_xdic_channel;
                    if (gn_xdic_channel >= gn_xdic_channel_size)
                    {
                        gn_xdic_channel = XDIC_CH_01;
                        gt_xdic_test_mode = XDIC_TEST_CURRENT_TYPE_B;
                        gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                    }
                    else
                    {
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    break;
                case XDIC_TEST_CURRENT_TYPE_B:
                    // Common Average
                    gf_xdic_measured_current_B[gn_xdic_channel] = JigBD_IF_Convert_Adc_To_Current((uint16_t)(((float)(gn_xdic_temp_adc[1] + gn_xdic_temp_adc[0]) / 2.0f) + 0.5f),\
                        gt_xdic_trim_condition[XDIC_TEST_CURRENT_TYPE_B].current_gain);
                    ++gn_xdic_channel;
                    if (gn_xdic_channel >= gn_xdic_channel_size)
                    {
                        gn_xdic_channel = XDIC_CH_01;
                        gt_xdic_test_mode = XDIC_TEST_ICC;
                        gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                    }
                    else
                    {
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    break;
                case XDIC_TEST_ICC:
                    gf_xdic_measured_icc = (JigBD_IF_Convert_Adc_To_milli_Voltage((gn_xdic_temp_adc[0] - gn_xdic_temp_adc[1])) / CURRENT_SENSE_R_ICC);
                    gn_xdic_channel = XDIC_CH_01;
                    gt_xdic_test_mode = XDIC_TEST_LDO_SWEEP;
                    gt_xdic_test_step = XDIC_TEST_STEP_MODE_INIT;
                    break;
                case XDIC_TEST_LDO_SWEEP:
                    gf_xdic_measured_ldo_sweep[gn_xdic_measure_count] = JigBD_IF_Convert_MCU_ADC_To_Volt(gn_xdic_temp_adc[0]);

                    if (gf_xdic_measured_ldo_sweep[gn_xdic_measure_count] > 1.65)
                    {
                        gn_xdic_measure_count = 0;
                        gt_xdic_test_mode = XDIC_TEST_MAX;
                        gt_xdic_test_step = XDIC_TEST_STEP_RESULT;
                    }

                    if (gn_xdic_measure_count < 31)
                    {
                        ++gn_xdic_measure_count;
                        gt_xdic_test_mode = XDIC_TEST_LDO_SWEEP;
                        gt_xdic_test_step = XDIC_TEST_STEP_CHANGE_OUTPUT;
                    }
                    else
                    {
                        gn_xdic_measure_count = 0;
                        gt_xdic_test_mode = XDIC_TEST_MAX;
                        gt_xdic_test_step = XDIC_TEST_STEP_RESULT;
                    }
                    break;
                default:
                    break;
                }
                break;
            case XDIC_TEST_STEP_RESULT:
                XDIC_Display_Test_Result();
                gt_xdic_test_step = XDIC_TEST_STEP_PWR_OFF;
                break;
            case XDIC_TEST_STEP_PWR_OFF:
                JigBD_IF_XD_VCC_EN(PWR_OFF);
                JigBD_IF_XC_VCC_EN(PWR_OFF);
                JigBD_IF_VLED_9V_EN(PWR_OFF);
                gt_xdic_test_step = XDIC_TEST_STEP_NONE;
                break;
            default:
                break;
            }
        }
    }
}
/* END - TRIM_PROCEDURE_RUN   *****************************************/

/*** end of file ***/