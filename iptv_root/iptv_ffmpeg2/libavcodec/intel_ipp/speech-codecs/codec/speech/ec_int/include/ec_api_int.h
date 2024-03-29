/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2005-2007 Intel Corporation. All Rights Reserved.
//
//     Intel(R) Integrated Performance Primitives EC Sample
//
//  By downloading and installing this sample, you hereby agree that the
//  accompanying Materials are being provided to you under the terms and
//  conditions of the End User License Agreement for the Intel(R) Integrated
//  Performance Primitives product previously accepted by you. Please refer
//  to the file ippEULA.rtf or ippEULA.txt located in the root directory of your Intel(R) IPP
//  product installation for more information.
//
//
//  Purpose: Echo Canceller header file.
//
*/

#ifndef __EC_API_INT_H__
#define __EC_API_INT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __EC_API_H__
//#define AEC_SB_8ms_interface
typedef enum op_code {
    EC_COEFFS_ZERO = 0,
    EC_ADAPTATION_ENABLE,
    EC_ADAPTATION_ENABLE_LITE,
    EC_ADAPTATION_DISABLE,
    EC_ADAPTATION_DISABLE_LITE,
    EC_NLP_ENABLE,
    EC_NLP_DISABLE,
    EC_TD_ENABLE,
    EC_TD_DISABLE,
    EC_AH_ENABLE,  /* Enable AH, HD mode 1 - FFT-based shift up*/
    EC_AH_ENABLE3,  /* Enable AH, HD mode 3 - cos shift up*/
    EC_AH_DISABLE,
    EC_ALG1
} ECOpcode;
typedef enum hd_op_code_enum {
    HD_1=0,
    HD_2
} HDOpcode;
typedef enum ah_op_code_enum {
    AH_ENABLE= 0,
    AH_DISABLE,
    AH_ENABLE1,
    AH_ENABLE2,
    AH_ENABLE3
} AHOpcode;
#endif /* __EC_API_H__ */

typedef int (*ec_GetFrameSizeI_ptr)(IppPCMFrequency freq, int taptime_ms, int *s);
typedef int (*ec_GetSizeI_ptr)(IppPCMFrequency freq, int taptime_ms, int *s, int *s1, int *s2);
typedef int (*ec_GetSendPathDelayI_ptr)(int *delay);
typedef int (*ec_InitI_ptr)(void *state,void *hObj, IppPCMFrequency freq, int taptime_ms);
typedef int (*ec_ModeOpI_ptr)(void *state, ECOpcode op);
typedef int (*ec_ProcessFrameI_ptr)(void *state, const Ipp16s *rin, Ipp16s *sin, Ipp16s *sout);
typedef int (*ec_GetInfoI_ptr)(void *state,ECOpcode *isNLP,ECOpcode *isTD,
             ECOpcode *isAdapt,ECOpcode *isAdaptLite, AHOpcode *isAH);
typedef int (*ec_GetSubbandNum_ptr)(void *state);
typedef int (*ec_setNRI_ptr)(void *state, int num);
typedef int (*ec_setCNGI_ptr)(void *state, int num);
typedef int (*ec_setTDThreshold_ptr)(void *state, int num);
typedef int (*ec_setTDSilenceTime_ptr)(void *state, int num);

int ec_ifb_GetSize(IppPCMFrequency freq, int taptime_ms, int *s, int *s1, int *s2);
int ec_ifb_Init(void *state,void *hObj, IppPCMFrequency freq, int taptime_ms);
int ec_ifb_InitBuff(void *state, char *buff);
int ec_ifb_ModeOp(void *state, ECOpcode op);
int ec_ifb_ProcessFrame(void *state, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout);
int ec_ifb_GetFrameSize(IppPCMFrequency freq, int taptime_ms, int *s);
int ec_ifb_GetSendPathDelay(int *delay);
int ec_ifb_GetNLPGain(void *state);
int ec_ifb_GetInfo(void *state,ECOpcode *isNLP,ECOpcode *isTD,
                   ECOpcode *isAdapt,ECOpcode *isAdaptLite, AHOpcode *isAH);
int ec_ifb_GetSubbandNum(void *state);

int ec_isb_GetSize(IppPCMFrequency freq, int taptime_ms, int *s, int *s1, int *s2);
int ec_isb_Init(void *state,void *hObj, IppPCMFrequency freq, int taptime_ms);
int ec_isb_InitBuff(void *state, char *buff);
int ec_isb_ModeOp(void *state, ECOpcode op);
int ec_isb_ModeCheck(void *state, ECOpcode op);
int ec_isb_GetSegNum(void *state);
int ec_isb_GetSubbandNum(void *state);
int ec_isb_GetNLPGain(void *state);

int ec_isb_ProcessFrame(void *state, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout);
int ec_isb_GetFrameSize(IppPCMFrequency freq, int taptime_ms, int *s);
int ec_isb_GetSendPathDelay(int *delay);
int ec_isb_GetInfo(void *stat,ECOpcode *isNLP,ECOpcode *isTD,
                   ECOpcode *isAdapt,ECOpcode *isAdaptLite, ECOpcode *isAH);
int ec_isb_GetSubbandNum(void *state);

int ec_isbf_GetSize(IppPCMFrequency freq, int taptime_ms, int *s, int *s1, int *s2);
int ec_isbf_Init(void *state,void *hObj, IppPCMFrequency freq, int taptime_ms);
int ec_isbf_InitBuff(void *state, char *buff);
int ec_isbf_ModeOp(void *state, ECOpcode op);
int ec_isbf_ProcessFrame(void *state, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout);
int ec_isbf_GetFrameSize(IppPCMFrequency freq, int taptime_ms, int *s);
int ec_isbf_GetSendPathDelay(int *delay);
int ec_isbf_GetNLPGain(void *state);

int ec_isbf_GetInfo(void *state,ECOpcode *isNLP,ECOpcode *isTD,
                    ECOpcode *isAdapt,ECOpcode *isAdaptLite, AHOpcode *isAH);
int ec_isbf_GetSubbandNum(void *state);

int ec_isb_setAHmodeHD(void *stat, HDOpcode op);

int ec_isbf_setNR(void *stat, int n);
int ec_ifb_setNR(void *stat, int n);
int ec_isb_setNR(void *stat, int n);

int ec_isbf_setCNG(void *stat, int n);
int ec_ifb_setCNG(void *stat, int n);
int ec_isb_setCNG(void *stat, int n);

int ec_isb_setTDThreshold(void *stat,int thresh);// 1=1db
int ec_isbf_setTDThreshold(void *stat,int thresh);
int ec_ifb_setTDThreshold(void *stat,int thresh);

int ec_isb_setTDSilenceTime(void *stat,int thresh);// 1=10msec
int ec_isbf_setTDSilenceTime(void *stat,int thresh);
int ec_ifb_setTDSilenceTime(void *stat,int thresh);

#define FFT_ORDER11         8
#define FFT_ORDER10         10
#define FFT_ORDER6          6
#define AEC_FFT_ORDER11_LEN (((1<<FFT_ORDER11)+2)>>1)
#define AEC_FFT_ORDER10_LEN (((1<<FFT_ORDER10)+2)>>1)
#define AEC_FFT_ORDER6_LEN  (((1<<FFT_ORDER6)+2)>>1)
#define AEC_FFT_ORDER6_LEN2 ((1<<FFT_ORDER6)+2)
#define FFT_ORDER7          7
#define AEC_FFT_ORDER7_LEN  (((1<<FFT_ORDER7)+2)>>1)
#define FFT_ORDER7_SRC_LEN  (1<<FFT_ORDER7)
#define FFT_ORDER10_SRC_LEN (1<<FFT_ORDER10)
#define FFT_ORDER11_SRC_LEN (1<<FFT_ORDER11)
#define FRAME_SIZE_10ms     80
#define FRAME_SIZE_6ms      48
#define SBF_OFF     48
#ifdef AEC_SB_8ms_interface
#define SB_OFF      0
#else
#define SB_OFF      48
#endif
#define FB_OFF      0

//#define _AEC_DBG_PRT
#ifdef _AEC_DBG_PRT
 #define mprintf printf
#else
 #define mprintf //a
#endif

#define IPP_AEC_VER 86

#ifdef __cplusplus
}
#endif

#endif /* __EC_API_INT_H__ */
