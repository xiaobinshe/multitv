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
//  Purpose: Echo Canceller, fast subband algorithm
//
*/

#include <stdlib.h>
#include <ipps.h>
#include <ippsc.h>
#include "scratchmem.h"
#include "nlp_api.h"
#include "ec_api_int.h"
#include "ec_td_int.h"
#include "_ippnr_int.h"
#include "ah_api_int.h"
#include "ownah_int.h"
#include "ownec_int.h"

static void ownIIR_16s_I(Ipp16s* pSrcDst,
                  int len,
                  Ipp32s* pCoeff1,
                  Ipp32s* pDlyLine1)
{
  int i;
  Ipp32s *pCoeff = pCoeff1;
  Ipp32s *pDlyLine = pDlyLine1;

  for (i = 0; i < len; i++) {
    Ipp32s src = pSrcDst[i];

    pDlyLine[0] = src << 2;
    src = (Ipp32s)(((Ipp64s)pCoeff[0]*pDlyLine[0] +
      (Ipp64s)pCoeff[1]*pDlyLine[1] +
      (Ipp64s)pCoeff[2]*pDlyLine[2] -
      (Ipp64s)pCoeff[4]*pDlyLine[4] -
      (Ipp64s)pCoeff[5]*pDlyLine[5]) >> 32);

    pDlyLine[5] = pDlyLine[4]; pDlyLine[4] = (src << 2);
    pDlyLine[2] = pDlyLine[1]; pDlyLine[1] = pDlyLine[0];
//    pSrcDst[i] = Cnvrt_32s16s(src);
    if (src < IPP_MIN_16S || src > IPP_MAX_16S) {
      ippsZero_32s(pDlyLine, 6);
      src = 0;
    }
    pSrcDst[i] = (Ipp16s)src;
  }
}

static Ipp32s iirCfs[]= {
#if 1
  447392426, 0, -447392426,
  1073741824, -1972374047, 917062153,
#else
  1073741824,           0,   -1073741824, /* b00 b01 b02 */
  1073741824,   -1251274026,     280189149, /* a00 a01 a02 */
#endif
};

#undef PSD_SF
#undef COEF_SF
#define PSD_SF 14
#define COEF_SF 14
#define COEF_ONE (1 << COEF_SF)

#ifdef SBF_40
#define START_SUBBAND 3
#else
#define START_SUBBAND 2
#endif

static Ipp8u t_fp_norm_table[256] = {
  0, 1, 2, 2,
  3, 3, 3, 3,
  4, 4, 4, 4,
  4, 4, 4, 4,
  5, 5, 5, 5,
  5, 5, 5, 5,
  5, 5, 5, 5,
  5, 5, 5, 5,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  6, 6, 6, 6,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  7, 7, 7, 7,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8,
  8, 8, 8, 8
};

#define t_fp_norm(a)                        \
{                                           \
  int t, shift;                             \
  int m = a.m;                              \
                                            \
  if (m == 0) {                             \
    a.e = 0;                                \
  } else {                                  \
    if (m > 0xffff) {                       \
      t = m >> 16;                          \
    if (t > 0xff){                          \
      shift = 9 + t_fp_norm_table[t >> 8];  \
    } else {                                \
      shift = 1 + t_fp_norm_table[t];       \
    }                                       \
      a.m = m >> shift;                     \
      a.e += shift;                         \
    } else {                                \
      if (m > 0xff){                        \
        shift = 8 - t_fp_norm_table[m >> 8];\
      } else {                              \
        shift = 16 - t_fp_norm_table[m];    \
      }                                     \
      m <<= shift;                          \
      a.e -= (shift - 1);                   \
      a.m = m >> 1;                         \
    }                                       \
  }                                         \
}

__INLINE t_fp t_fp_ini_imm(int a, int b) {
  t_fp c;
  c.m = a;
  c.e = b;
  return c;
}

#define t_fp_ini32s(a, c) \
{                         \
  c.m = a;                \
  c.e = 15;               \
  t_fp_norm(c);           \
}

#define t_fp_align(a, b)                         \
{                                                \
  int shift;                                     \
  if (a.e < b.e) {                               \
    shift = b.e - a.e;                           \
    if (shift) {                                 \
      if (shift < 32) {                          \
        a.m >>= shift;                           \
      } else {                                   \
        a.m = 0;                                 \
      }                                          \
      a.e += shift;                              \
    }                                            \
  } else {                                       \
    shift = a.e - b.e;                           \
    if (shift) {                                 \
      if (shift < 32) {                          \
        b.m >>= shift;                           \
      } else {                                   \
        b.m = 0;                                 \
      }                                          \
      b.e += shift;                              \
    }                                            \
  }                                              \
}


#define t_fp_add(a, b, c) \
{                         \
  t_fp a0 = a;            \
  t_fp b0 = b;            \
                          \
  t_fp_align(a0, b0);     \
                          \
  c.m = a0.m + b0.m;      \
  c.e = a0.e;             \
                          \
  if (c.m >= 0x8000) {    \
    c.m >>= 1;            \
    c.e += 1;             \
  }                       \
}

#define t_fp_sub(a, b, c) \
{                         \
  t_fp a0 = a;            \
  t_fp b0 = b;            \
                          \
  t_fp_align(a0, b0);     \
  c.m = a0.m - b0.m;      \
  c.e = a0.e;             \
                          \
  if (c.m < 0x4000) {     \
    c.m <<= 1;            \
    c.e -= 1;             \
  }                       \
}

#define t_fp_eq(a, b, ret)       \
{                                \
  t_fp a0 = a;                   \
  t_fp b0 = b;                   \
                                 \
  t_fp_align(a0, b0);            \
                                 \
  if (a0.m < b0.m) ret = -1;     \
  else if (a0.m > b0.m) ret = 1; \
  else ret = 0;                  \
}

#define t_fp_mul(a, b, c) {    \
  c.m = a.m * b.m;             \
  if (c.m >= 0x20000000) {     \
    c.m >>= 15;                \
    c.e = a.e + b.e;           \
  } else {                     \
    c.m >>= 14;                \
    c.e = a.e + b.e - 1;       \
  }                            \
}

#define t_fp_div(a, b, c)    \
{                            \
  if (b.m == 0) {            \
    c.m = c.e = 0;           \
  } else {                   \
    c.m = (a.m << 15) / b.m; \
    if (c.m >= 0x8000) {     \
      c.m >>= 1;             \
      c.e = a.e - b.e + 1;   \
    } else {                 \
      c.e = a.e - b.e;       \
    }                        \
  }                          \
}

static Ipp16u isbf_sqrt(Ipp32u x)
{
  Ipp16u r=0;
  Ipp16u i;
  for(i=IPP_MAX_16U; i>0; i>>=1)
  {
    r = r + i;
    if( (Ipp32u)(r*r) > x)
      r = r - i;
  }
  return (Ipp16u)r;
}
__INLINE t_fp t_fp_sqrt(t_fp a) {
  Ipp16u tt;
  t_fp sqrt2 = t_fp_ini_imm(23170, 1);

  a.e--;
  a.m <<= 1;
  tt = (Ipp16u)a.m;
  //ippsSqrt_16u_ISfs(&tt, 1, -7);
  tt = isbf_sqrt(tt<<14);
  a.m = tt;
  if (a.e & 1) {
    a.e = (a.e >> 1) + 1;
  } else {
    a.e = (a.e >> 1);
    t_fp_mul(a, sqrt2, a);
  }

  t_fp_norm(a);
  return a;
}
/*
__INLINE void t_fp_print(t_fp a) {
  int aaa = a.e;
  double f = a.m;
  while(aaa > 1) {aaa--; f *= 2;}
  while(aaa < 1) {aaa++; f *= 0.5;}
  printf("%f", (f)/16384.0);
}
*/


#ifdef SBF_40
#define COEF_SMOOTH ((COEF_ONE * 995 / 1000))
#define COEF_SMOOTH2 ((COEF_ONE * 953 / 1000))
#else
#define COEF_SMOOTH ((COEF_ONE * 998 / 1000))
#define COEF_SMOOTH2 ((COEF_ONE * 981 / 1000))
#endif

int ec_isbf_setNR(void *stat, int n){
    int status=0;
    _isbfECState *state=(_isbfECState *)stat;
    state->nlp = (Ipp16s)n;
    //mprintf("NR set to %d\n", state->nlp);
//    if (state->nlp == 2) {
//#ifdef ADAPT_NOISE
//      mprintf("NR 2, adaptive level noise\n");
//#else
//      mprintf("NR 2, fixed level noise\n");
//#endif
//    }
    return status;
}
int ec_isbf_setCNG(void *stat, int n){
    int status=0;
    _isbfECState *state=(_isbfECState *)stat;
    state->cng = (Ipp16s)n;
    return status;
}
int ec_isbf_GetSubbandNum(void *stat)
{
    _isbfECState *state=(_isbfECState *)stat;
    return state->numSubbands;
}
 /* init scratch memory buffer */
int ec_isbf_InitBuff(void *stat, char *buff) {
    _isbfECState *state=(_isbfECState *)stat;
    if (state==NULL) {
        return 1;
    }
    if(NULL==state)
        return 1;
    if(NULL != buff)
       state->Mem.base = buff;
    else
       if (NULL == state->Mem.base)
          return 1;
    state->Mem.CurPtr = state->Mem.base;
    state->Mem.VecPtr = (int *)(state->Mem.base+state->scratch_mem_size);
    return 0;
}

/* Returns size of frame */
int ec_isbf_GetFrameSize(IppPCMFrequency freq, int taptime_ms, int *s)
{
    *s = FRAME_SIZE_10ms+(int)freq-(int)freq+taptime_ms-taptime_ms;
    return 0;
}

/* Returns size of buffer for state structure */
int ec_isbf_GetSize(IppPCMFrequency freq, int taptime_ms, int *s, int *s1, int *scratch_size)
{
    int numSegments;
    int size = 0, csize, csize1;
    int sbsize, sbinitBufSize, sbbufSize;

    size += ALIGN(sizeof(_isbfECState));

    ippsSubbandProcessGetSize_16s(SBF_FFT_ORDER, SBF_WIN_LEN, &sbsize, &sbinitBufSize, &sbbufSize);

    size += ALIGN(sbsize);
    size += ALIGN(sbsize);
    size += ALIGN(sbsize);
    size += ALIGN(sbinitBufSize);
    size += ALIGN(sbbufSize);

    if (freq == IPP_PCM_FREQ_8000) {
        numSegments = (taptime_ms * 8 - 1) / SBF_FRAME_SIZE + 1;
    } else if (freq == IPP_PCM_FREQ_16000) {
        numSegments = (taptime_ms * 16 - 1) / SBF_FRAME_SIZE + 1;
    } else {
        return 1;
    }

    size += ALIGN(3 * numSegments * sizeof(Ipp32sc *));
    size += ALIGN(3 * numSegments * SBF_SUBBANDS * sizeof(Ipp32sc));

    ippsSubbandControllerDTGetSize_EC_16s(SBF_SUBBANDS, SBF_FRAME_SIZE, numSegments, freq, &sbsize);
    size += ALIGN(sbsize);

//    ippsSubbandControllerDTGetSize_EC_16s(&sbsize);
//    size += sbsize;

    ippsToneDetectGetStateSize_EC_16s(freq, &csize);
    size += ALIGN(csize * 2);
    *s = ALIGN(size+31);

    ah_GetSize_int(SBF_FRAME_SIZE, &csize,freq,scratch_size);
    shiftupCos_GetSize(FRAME_SIZE_10ms, &csize1);
    csize += ALIGN(csize1);
    *s1 = ALIGN(csize+31);

    return 0;
}

/* Returns delay introduced to send-path */
int ec_isbf_GetSendPathDelay(int *delay)
{
    *delay = SBF_WIN_LEN - SBF_FRAME_SIZE;
    return 0;
}
int ec_isbf_setTDThreshold(void *stat,int val)
{
    _isbfECState *state=(_isbfECState *)stat;
    if (val>-30) {
        return 1;
    }
    if (val<-89) {
        val=-89;
    }
    state->ah->td.td_thres=(Ipp64s)(IPP_AEC_dBtbl[-val-10]*IPP_AEC_dBtbl[-val-10]);
    return 0;
}
int ec_isbf_setTDSilenceTime(void *stat,int val)
{
    _isbfECState *state=(_isbfECState *)stat;
    if (val<100) {
        return 1;
    }
    state->ah->td.td_sil_thresh=(Ipp16s)val;
    return 0;
}
/* acquire NLP gain coeff */
int ec_isbf_GetNLPGain(void *stat)
{
   _isbfECState *state=(_isbfECState *)stat;
   return state->sgain;
}
#ifndef SBF_40
static Ipp16s ec_isbf_hann64[] = {
  0,
  20,
  81,
  181,
  321,
  498,
  711,
  958,
  1236,
  1542,
  1873,
  2227,
  2599,
  2986,
  3384,
  3789,
  4198,
  4605,
  5007,
  5400,
  5780,
  6144,
  6487,
  6806,
  7098,
  7361,
  7591,
  7786,
  7944,
  8065,
  8146,
  8186,
  8186,
  8146,
  8065,
  7944,
  7786,
  7591,
  7361,
  7098,
  6806,
  6487,
  6144,
  5780,
  5400,
  5007,
  4605,
  4198,
  3789,
  3384,
  2986,
  2599,
  2227,
  1873,
  1542,
  1236,
  958,
  711,
  498,
  321,
  181,
  81,
  20,
  0,
};
#else
static Ipp16s ec_isbf_hann128[] = {
  0,
  5,
  20,
  45,
  79,
  124,
  179,
  243,
  316,
  399,
  491,
  591,
  700,
  818,
  943,
  1077,
  1217,
  1365,
  1519,
  1680,
  1846,
  2018,
  2195,
  2377,
  2563,
  2753,
  2946,
  3142,
  3340,
  3540,
  3741,
  3944,
  4146,
  4349,
  4551,
  4751,
  4950,
  5147,
  5342,
  5533,
  5721,
  5905,
  6085,
  6259,
  6429,
  6592,
  6750,
  6901,
  7045,
  7182,
  7311,
  7433,
  7546,
  7651,
  7747,
  7835,
  7913,
  7982,
  8041,
  8090,
  8130,
  8160,
  8180,
  8190,
  8190,
  8180,
  8160,
  8130,
  8090,
  8041,
  7982,
  7913,
  7835,
  7747,
  7651,
  7546,
  7433,
  7311,
  7182,
  7045,
  6901,
  6750,
  6592,
  6429,
  6259,
  6085,
  5905,
  5721,
  5533,
  5342,
  5147,
  4950,
  4751,
  4551,
  4349,
  4146,
  3944,
  3741,
  3540,
  3340,
  3142,
  2946,
  2753,
  2563,
  2377,
  2195,
  2018,
  1846,
  1680,
  1519,
  1365,
  1217,
  1077,
  943,
  818,
  700,
  591,
  491,
  399,
  316,
  243,
  179,
  124,
  79,
  45,
  20,
  5,
  0,
};
#endif

#ifdef SBF_40
#define WND ec_isbf_hann128
#else
#define WND ec_isbf_hann64
#endif

#ifdef OWNTEST
t_fp t_ini(float a) {
  t_fp c;
  c.m = a * (1 << 20);
  c.e = -5;
  t_fp_norm(c);
  return c;
}
static float t_get(t_fp a) {
  return (float)a.m * pow(2,a.e - 15);
}
#endif
/* Initializes state structure */
int ec_isbf_Init(void *stat,void *ahObj, IppPCMFrequency freq, int taptime_ms)
{
    int i, numSegments, csize,ret;
    int sbsize, sbinitBufSize, sbbufSize;
    Ipp8u *ptr, *buf;
    _isbfECState *state=(_isbfECState *)stat;

    if (state==0 || ahObj==0) {
        return 1;
    }

    ptr = (Ipp8u *)state;
    ptr += ALIGN(sizeof(_isbfECState));
/* Initialize SubbandProcess structures */
    ippsSubbandProcessGetSize_16s(SBF_FFT_ORDER, SBF_WIN_LEN, &sbsize, &sbinitBufSize, &sbbufSize);

    state->state_syn = (IppsSubbandProcessState_16s *)ptr;
    ptr += ALIGN(sbsize);
    state->state_ana_rin = (IppsSubbandProcessState_16s *)ptr;
    ptr += ALIGN(sbsize);
    state->state_ana_sin = (IppsSubbandProcessState_16s *)ptr;
    ptr += ALIGN(sbsize);

    if (ippsSubbandProcessInit_16s(state->state_syn,     SBF_FFT_ORDER, SBF_FRAME_SIZE, SBF_WIN_LEN, WND, ptr) != ippStsOk)
        return 1;// zero state->pSigBuf[256]
    if (ippsSubbandProcessInit_16s(state->state_ana_rin, SBF_FFT_ORDER, SBF_FRAME_SIZE, SBF_WIN_LEN, WND, ptr) != ippStsOk)
        return 1;// zero state->pSigBuf[256]
    if (ippsSubbandProcessInit_16s(state->state_ana_sin, SBF_FFT_ORDER, SBF_FRAME_SIZE, SBF_WIN_LEN, WND, ptr) != ippStsOk)
        return 1;// zero state->pSigBuf[256]

    ptr += ALIGN(sbinitBufSize);

    state->FFTSize = SBF_FFT_LEN;
    state->numSubbands = SBF_SUBBANDS;
    state->frameSize = SBF_FRAME_SIZE;
    state->windowLen = SBF_WIN_LEN;
    state->pBuf = ptr;
    ptr += ALIGN(sbbufSize);

    if (freq == IPP_PCM_FREQ_8000) {
        numSegments = (taptime_ms * 8 - 1) / state->frameSize + 1;
    } else if (freq == IPP_PCM_FREQ_16000) {
        numSegments = (taptime_ms * 16 - 1) / state->frameSize + 1;
    } else {
        return 1;
    }
    state->numSegments = numSegments;

    /* receive-in subband history */
    state->ppRinSubbandHistory = (Ipp32sc **)ptr;
    ptr += ALIGN(3 * numSegments * sizeof(Ipp32sc *));
    /* adaptive coeffs */
    state->ppAdaptiveCoefs = state->ppRinSubbandHistory + numSegments;
    /* fixed coeffs */
    state->ppFixedCoefs = state->ppRinSubbandHistory + numSegments * 2;

    buf = ptr;
    ptr += ALIGN(3 * numSegments * SBF_SUBBANDS * sizeof(Ipp32sc));

    /* Zero coeffs buffer and history buffer */
    ippsZero_32sc((Ipp32sc *)buf, 3 * numSegments * SBF_SUBBANDS);

    /* Initialize receive-in subband history array */
    for (i = 0; i < numSegments; i++) {
        (state->ppRinSubbandHistory)[i] = (Ipp32sc *)buf + i * (state->numSubbands);
    }
    buf += numSegments * (state->numSubbands) * sizeof(Ipp32sc);

    /* Initialize adaptive coeffs array */
    for (i = 0; i < numSegments; i++) {
        (state->ppAdaptiveCoefs)[i] = (Ipp32sc *)buf + i * (state->numSubbands);
    }
    buf += numSegments * (state->numSubbands) * sizeof(Ipp32sc);

    /* Initialize fixed coeffs array */
    for (i = 0; i < numSegments; i++) {
        (state->ppFixedCoefs)[i] = (Ipp32sc *)buf + i * (state->numSubbands);
    }

    /* Initialize subband controller */
    state->controller = (IppsSubbandControllerDTState_EC_16s *)ptr;
    if (ippsSubbandControllerDTInit_EC_16s(state->controller, state->numSubbands, state->frameSize, numSegments, freq) != ippStsOk)
        return 1;

    ippsSubbandControllerDTGetSize_EC_16s(state->numSubbands, state->frameSize, numSegments, freq, &csize);

    ptr += ALIGN(csize);

//    state->controller2 = (IppsSubbandControllerDTState_EC_16s *)ptr;
//    ippsSubbandControllerDTInit_EC_16s(state->controller2);
//    ippsSubbandControllerDTGetSize_EC_16s(&csize);
//    ptr += csize;

    state->ah = (ahState_int *)IPP_ALIGNED_PTR(ahObj, 16);
    ret=ah_Init_int(state->ah, freq, SBF_FRAME_SIZE,SBF_OFF);
    if (ret) {
        return ret;
    }

    ah_GetSize_int(SBF_FRAME_SIZE, &csize, freq,&state->scratch_mem_size);

    state->shiftupCos = (shiftupCosState_int *)IPP_ALIGNED_PTR((Ipp8u*)ahObj + ALIGN(csize), 16);
    shiftupCos_Init(state->shiftupCos, FRAME_SIZE_10ms);
    //ah_GetSize_int(SBF_FRAME_SIZE, &csize);
    //ptr += ALIGN(csize);

    ippsZero_32s((Ipp32s*)state->ns_lambda_d, SBF_SUBBANDS * 2);
/*    for (i = 0; i < SBF_SUBBANDS; i++) {
      state->ns_lambda_d[i] = t_fp_ini_imm(0x4000, 24);
    }*/
    ippsZero_32s((Ipp32s*)state->ns_x_pwr, SBF_SUBBANDS * 2);
    ippsZero_16s(state->saved_sin, 48);
    state->saved_sin_idx = 0;

    state->ns_one = t_fp_ini_imm(0x4000, 1);
    //state->ns_beta = t_fp_ini_imm(0x7fbe, 0);
    state->ns_beta = t_fp_ini_imm(0x7f7c, 0);
    //state->ns_beta1m = t_fp_ini_imm(0x4188, -8);
    state->ns_beta1m = t_fp_ini_imm(0x4188, -7);
    state->ns_alpha = t_fp_ini_imm(0x7d70, 0);
    state->ns_alpha1m = t_fp_ini_imm(0x51eb, -5);
    state->ns_gamma = t_fp_ini_imm(0x6666, 0);
    state->ns_gamma1m = t_fp_ini_imm(0x6666, -2);
    //    state->ns_al = state->ns_alpha;
    //    state->ns_al1m = state->ns_alpha1m;
//    state->ns_al = t_fp_ini_imm(0x7fce, 0); // 0.9985
//    state->ns_al1m = t_fp_ini_imm(0x6240, -9); // 0.0015
    state->ns_al = t_fp_ini_imm(0x7fbe, 0); //0.998
    state->ns_al1m = t_fp_ini_imm(0x4188, -8); //0.002
    //      ns_al = t_fp_ini_imm(0x7f9d, 0); // 0.997
    //      ns_al1m = t_fp_ini_imm(0x6248, -8); // 0.003
    //state->ns_al = t_fp_ini_imm(0x7eb8, 0); // 0.99
    //state->ns_al1m = t_fp_ini_imm(0x51ea, -6); // 0.01
    //        state->ns_al = t_ini(0.980);
//              printf("%x %d %f\n",state->ns_al.m, state->ns_al.e, t_get(state->ns_al));
//              printf("%x %d %f\n",state->ns_al1m.m, state->ns_al1m.e, t_get(state->ns_al1m));exit(1);
#ifdef ADAPT_NOISE
    state->noise_pow = t_fp_ini_imm(0, 0);
#endif

    /* enable adaptation */
    state->mode = 1;
    state->dt1 = 0;

    state->nlp_mult = COEF_ONE;
    state->smoothSin = state->smoothSout = 0;
    state->shortTermSin = state->shortTermSout = state->longTermSout = 0;

    ippsZero_32s(state->iirDly,6);
#ifdef ADAPT_NOISE
    ippsZero_32s((Ipp32s*)state->saved_noise_pow, SAFETY_BUFFER_SIZE*2);
    state->noise_idx = state->save_idx = 0;
#endif

    state->noise_seed = 12345;
    state->nlp = 1;
    state->cng = 1;
    state->r_in_pwr = 0;
    state->s_in_pwr = 0;
    state->rinPow = 0;
    state->sinPow = 0;
    state->soutPow = 0;
    state->coupling = 0;
    state->es_off = 0;
    state->conv = 0;
    state->start_time = 0;
    state->dt = 0;
    state->frame = 0;
    state->upCounter = 0;
    ippsZero_16s(state->rinBuf, SBF_FRAME_SIZE);
    ippsZero_16s(state->sinBuf, SBF_FRAME_SIZE);
    //mprintf("Intel(R) IPP AEC. FAST SUB-BAND algorithm. Version %d\n",IPP_AEC_VER );
    state->acpwr = 0;
    state->cdlt = 0;
    state->fixCfgInit = 0;
    ippsZero_64s(state->rinSubbandPow, SBF_SUBBANDS);

    return 0;
}

int ec_isbf_GetInfo(void *stat,ECOpcode *isNLP,ECOpcode *isTD,
                   ECOpcode *isAdapt,ECOpcode *isAdaptLite, AHOpcode *isAH)
{
    _isbfECState *state=(_isbfECState *)stat;
    if(state->mode & 2)
        *isNLP = EC_NLP_ENABLE;
    else
        *isNLP = EC_NLP_DISABLE;
    if(state->mode & 16)
        *isAH = AH_ENABLE;
    else
        *isAH = AH_DISABLE;

    if(state->mode & 4)
        *isTD = EC_TD_ENABLE;
    else
        *isTD = EC_TD_DISABLE;

    *isAdaptLite = EC_ADAPTATION_DISABLE_LITE;
    if(state->mode & 1){
        *isAdapt = EC_ADAPTATION_ENABLE;
        if(state->mode & 8)
            *isAdaptLite = EC_ADAPTATION_ENABLE_LITE;
    } else
        *isAdapt = EC_ADAPTATION_DISABLE;

    return 0;
}

/* Do operation or set mode */
int ec_isbf_ModeOp(void *stat, ECOpcode op)
{
    int i, j;
    _isbfECState *state=(_isbfECState *)stat;
    if (state == NULL)
        return 1;
    switch (op) {
    case (EC_COEFFS_ZERO):      /* Zero coeffs of filters */
        {
            for (i = 0; i < state->numSegments; i++)
                for (j = 0; j < state->numSubbands; j++)
                    state->ppAdaptiveCoefs[i][j].re = state->ppAdaptiveCoefs[i][j].im =
                        state->ppFixedCoefs[i][j].re = state->ppFixedCoefs[i][j].im = 0;
        }
        break;
    case(EC_ADAPTATION_ENABLE): /* Enable adaptation */
    case(EC_ADAPTATION_ENABLE_LITE): /* Enable adaptation */
        if (!(state->mode & 1))
            ippsSubbandControllerDTReset_EC_16s(state->controller);
        state->mode |= 1;
        break;
    case(EC_ADAPTATION_DISABLE): /* Disable adaptation */
        state->mode &= ~1;
        break;
    case(EC_NLP_ENABLE):    /* Enable NLP */
        state->mode |= 2;
        break;
    case(EC_NLP_DISABLE):   /* Disable NLP */
        state->mode &= ~2;
        break;
    case(EC_TD_ENABLE):     /* Enable ToneDisabler */
        state->mode |= 4;
        break;
    case(EC_TD_DISABLE):    /* Disable ToneDisabler */
        state->mode &= ~4;
        break;
    case(EC_AH_ENABLE):     /* Enable howling control */
    //case(EC_AH_ENABLE1):     /* Enable howling control */
        ah_SetMode_int(state->ah,AH_ENABLE1);// set anti-howling on
        state->mode |= 16;
        break;
    //case(EC_AH_ENABLE2):     /* Enable howling control */
    //    ah_SetMode_int(state->ah,AH_ENABLE2);// set anti-howling on
    //    state->mode |= 32;
    //    break;
    case(EC_AH_ENABLE3):     /* Enable howling control */
          ah_SetMode_int(state->ah,AH_ENABLE);// set anti-howling on
          state->mode |= 64;
          break;
    case(EC_AH_DISABLE):    /* Disable howling control */
        ah_SetMode_int(state->ah,AH_DISABLE);// set anti-howling off
        state->mode &= ~16;
        state->mode &= ~64;
        break;
    default:
        break;
    }

    return 0;
}

/*
static float t_fp_get(t_fp a) {
  return (float)a.m * pow(2,a.e - 15);
}
*/

#ifdef ADAPT_NOISE
static int noise_supp(_isbfECState *state, Ipp32sc *sout, Ipp32sc *sin) {
#else
static int noise_supp(_isbfECState *state, Ipp32sc *sout) {
#endif
  int i;
  int res;
#ifdef ADAPT_NOISE
  t_fp sum, sum_sin;

  sum.m = sum.e = 0;
  sum_sin.m = sum_sin.e = 0;
#endif

  for (i = 0; i < SBF_SUBBANDS; i++) {
    t_fp y_pwr_t_fp, gamma_t_fp, ksi_t_fp, fau_t_fp, h_f;
    t_fp tmp;
    Ipp32s tt;
#ifdef ADAPT_NOISE
    t_fp sin_pwr_t_fp;
#endif

    tt = sout[i].re;
    if (tt < 0) tt = -tt;
    t_fp_ini32s(tt, tmp);
    t_fp_mul(tmp, tmp, y_pwr_t_fp);
    tt = sout[i].im;
    if (tt < 0) tt = -tt;
    t_fp_ini32s(tt, tmp);
    t_fp_mul(tmp, tmp, tmp);
    t_fp_add(y_pwr_t_fp, tmp, y_pwr_t_fp);

#ifdef ADAPT_NOISE
    tt = sin[i].re;
    if (tt < 0) tt = -tt;
    t_fp_ini32s(tt, tmp);
    t_fp_mul(tmp, tmp, sin_pwr_t_fp);
    tt = sin[i].im;
    if (tt < 0) tt = -tt;
    t_fp_ini32s(tt, tmp);
    t_fp_mul(tmp, tmp, tmp);
    t_fp_add(sin_pwr_t_fp, tmp, sin_pwr_t_fp);
#endif

    if (!state->dt) {
      t_fp_mul(state->ns_beta, state->ns_lambda_d[i], state->ns_lambda_d[i]);
      t_fp_mul(state->ns_beta1m, y_pwr_t_fp, tmp);
      t_fp_add(state->ns_lambda_d[i], tmp, state->ns_lambda_d[i]);
    }

    ksi_t_fp.m = ksi_t_fp.e = 0;
    h_f = state->ns_one;

    if (state->ns_lambda_d[i].m > 0) {
      t_fp_mul(state->ns_alpha, state->ns_x_pwr[i], ksi_t_fp);

      t_fp_eq(y_pwr_t_fp, state->ns_lambda_d[i], res);
      if (res < 0){
        gamma_t_fp = state->ns_one;
      } else {
        t_fp_sub(y_pwr_t_fp, state->ns_lambda_d[i], tmp);
        t_fp_mul(state->ns_alpha1m, tmp, tmp);
        t_fp_add(ksi_t_fp, tmp, ksi_t_fp);
        t_fp_div(state->ns_lambda_d[i], y_pwr_t_fp, gamma_t_fp);
      }

      t_fp_add(ksi_t_fp, state->ns_lambda_d[i], tmp);
      t_fp_div(ksi_t_fp, tmp, fau_t_fp);

      t_fp_add(fau_t_fp, gamma_t_fp, tmp);
      t_fp_mul(fau_t_fp, tmp, h_f);

      t_fp_eq(state->ns_one, h_f, res);
      if (res < 0)
        h_f = state->ns_one;

      t_fp_mul(state->ns_gamma, state->ns_x_pwr[i], state->ns_x_pwr[i]);
      t_fp_mul(y_pwr_t_fp, h_f, tmp);
      t_fp_mul(state->ns_gamma1m, tmp, tmp);
      t_fp_add(state->ns_x_pwr[i], tmp, state->ns_x_pwr[i]);

#ifdef ADAPT_NOISE
      t_fp_sub(state->ns_one, h_f, tmp);
      t_fp_mul(y_pwr_t_fp, tmp, tmp);
      t_fp_add(sum, tmp, sum);
      t_fp_add(sum_sin, sin_pwr_t_fp, sum_sin);
      //sum = t_fp_add(sum, y_pwr_t_fp);
#endif

      h_f = t_fp_sqrt(h_f);
    }


    while(h_f.e > 1) {h_f.e--; h_f.m <<= 1;}
    while(h_f.e < 1) {h_f.e++; h_f.m >>= 1;}

    if (h_f.m < (1 << (COEF_SF - 3)))
      h_f.m = (1 << (COEF_SF - 3));

    if ((state->nlp == 2) && (state->es_off == 1)) {
      sout[i].re = (Ipp32s)(sout[i].re * (Ipp64s)h_f.m >> COEF_SF);
      sout[i].im = (Ipp32s)(sout[i].im * (Ipp64s)h_f.m >> COEF_SF);
    }
  }

#ifdef ADAPT_NOISE
  sum_sin.e -= 2;

  state->save_idx--;
  if (state->save_idx < 0)
    state->save_idx = 0;

  t_fp_eq(sum, sum_sin, res);

  if ((!state->dt) && (res > 0)) {
    t_fp tmp;
    if (state->save_idx == 0) {
      t_fp_mul(state->ns_al, state->noise_pow, state->noise_pow);
      t_fp_mul(state->ns_al1m, state->saved_noise_pow[state->noise_idx], tmp);
      t_fp_add(state->noise_pow, tmp, state->noise_pow);
      tmp = t_fp_sqrt(state->noise_pow);
      while(tmp.e > 15) {tmp.e--; tmp.m <<= 1;}
      while(tmp.e < 15) {tmp.e++; tmp.m >>= 1;}
      state->noise_lev = tmp.m / 50;
      if (state->noise_lev < 3)
        state->noise_lev = 3;
      if (state->noise_lev > 100)
        state->noise_lev = 100;
    }
    if (state->save_idx <= SAFETY_BUFFER_SIZE) {
      state->saved_noise_pow[state->noise_idx].m = sum.m;
      state->saved_noise_pow[state->noise_idx].e = sum.e;
      state->noise_idx = (state->noise_idx + 1) & (SAFETY_BUFFER_SIZE - 1);
    }
  } else {
    state->save_idx = SAFETY_BUFFER_SIZE * 2;
  }
#endif
  return 0;
}

#if !defined(SBF_20) && !defined(SBF_40)
#define SBF_COEF_NORM 11114
#endif

#ifdef SBF_20
#define SBF_COEF_NORM 13880
#endif

#ifdef SBF_40
#define SBF_COEF_NORM 13771
#endif
/*
static Ipp32s spectra_gain_arr[SBF_SUBBANDS] =
{32768, 32768, 32768, 32768, 32768, 32768, 25000, 22000,
19104, 18424, 17744, 17064, 16384, 15704, 15024, 14344,
13664, 12984, 12304, 11624, 10944, 10264, 9584, 8904,
8224, 7544, 6864, 6184, 5504, 4824, 4144, 3464, 2784};
*/

static Ipp32s spectra_gain_arr[SBF_SUBBANDS] = {
  32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
  32000, 31000, 30000, 29000, 28000, 27000, 26000, 25000,
  24000, 23500, 23000, 22500, 22000, 21500, 21000, 20500,
  20000, 19000, 18000, 17000, 16000, 16000, 16000, 14000,
  10400};

#define INITIAL_TIME 15 * FRAME_SIZE_10ms

static void suppressor(_isbfECState *state, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout)
{
  Ipp64s   tmp64;

  ippsMove_16s(rin, state->rinBuf+SBF_WIN_LEN-SBF_FRAME_SIZE, SBF_FRAME_SIZE);
  ippsMove_16s(sin, state->sinBuf+SBF_WIN_LEN-SBF_FRAME_SIZE, SBF_FRAME_SIZE);

  ippsDotProd_16s64s(state->rinBuf, state->rinBuf, SBF_FRAME_SIZE, &tmp64);
  state->rinPow = ((state->rinPow * 63 + tmp64) >> 6);

  ippsDotProd_16s64s(state->sinBuf, state->sinBuf, SBF_FRAME_SIZE, &tmp64);
  state->sinPow = ((state->sinPow * 63 + tmp64) >> 6);

  ippsDotProd_16s64s(sout, sout, SBF_FRAME_SIZE, &tmp64);
  state->soutPow = ((state->soutPow * 63 + tmp64) >> 6);

  ippsMove_16s(state->rinBuf+SBF_FRAME_SIZE, state->rinBuf, SBF_WIN_LEN-SBF_FRAME_SIZE);
  ippsMove_16s(state->sinBuf+SBF_FRAME_SIZE, state->sinBuf, SBF_WIN_LEN-SBF_FRAME_SIZE);

  //if ((state->frame > INITIAL_TIME - SBF_FRAME_SIZE) &&
  //    (state->frame <= INITIAL_TIME)) {
  //  if (state->rinPow < 1) state->rinPow = 1;
  //  state->coupling = (state->sinPow * 32768) / state->rinPow;
  //  mprintf("coupling = %i\n", state->coupling);
  //}

  if (state->frame < INITIAL_TIME) {
    //mprintf("init: conv = %i, es = %i, sin = %i, rin = %i, sout = %i\n",
    //  state->conv, state->es_off,
    //  (int)(state->sinPow >> 4), (int)(state->rinPow >> 4), (int)(state->soutPow >> 4));
    ippsZero_16s(sout, SBF_FRAME_SIZE);
    return;
  }

  if (state->conv == 0) {
    //if ((state->es_off == 0) || ((state->frame - state->start_time) < 16384)) {
    //  mprintf("conv = %i, es = %i, sin = %i, rin = %i, sout = %i\n",
    //    state->conv, state->es_off,
    //    (int)(state->sinPow >> 4), (int)(state->rinPow >> 4), (int)(state->soutPow >> 4));
    //}
    if ((state->soutPow * 40 < state->sinPow) &&
        (state->frame > INITIAL_TIME) && (state->es_off == 0)) {
      int j;
      state->conv = 1;
      state->es_off = 1;

      for (j = 0; j < state->numSegments; j++) {
        ippsCopy_32sc(state->ppAdaptiveCoefs[j], state->ppFixedCoefs[j], state->numSubbands);
      }
      state->fixCfgInit = 1;

      //mprintf("CONV ON!!!!!!!!!!!!!\n");
    } else {
      if ((state->sinPow > 3*state->rinPow) && (state->sinPow > 60000)) {
         /*then gradually stop suppression*/
        if (state->es_off == 0) {
          state->es_off = 1;
          state->start_time = state->frame;
          //mprintf("ES START OFF!!!!!!!!!!!\n");
        }
      } else {
        if (state->es_off == 0) {
          ippsZero_16s(sout, SBF_FRAME_SIZE);
        } else {
          int coeff = state->frame - state->start_time;
          if (coeff < 16384) {
            Ipp16s scoeff = (Ipp16s)coeff;
            ippsMulC_16s_ISfs(scoeff, sout, SBF_FRAME_SIZE, 14);
          } else {
            state->conv = 1;
            //mprintf("ES OFF!!!!!!!!!!!\n");
          }
        }
      }
    }
  }
}

static void spectra_gain(Ipp32sc *fira_err)
{
  int ii;
#if 0
  Ipp32s *ptmp = (Ipp32s*)fira_err;
  int coeff;

  if (AH_ENABLED3) {
    if (state->frame < INITIAL_TIME) {
      ippsZero_32sc(fira_err+4, 5);
      ippsZero_32sc(fira_err+14, 5);
      ippsZero_32sc(fira_err+24, 5);
    } else {
#if 0
      coeff = state->frame - INITIAL_TIME;

      if (coeff < 8192) {
        for (ii = 8; ii < 18; ii++) {
          ptmp[ii] = (Ipp32s)(((Ipp64s)ptmp[ii] * coeff) >> 13);
        }

        for (ii = 28; ii < 38; ii++) {
          ptmp[ii] = (Ipp32s)(((Ipp64s)ptmp[ii] * coeff) >> 13);
        }

        for (ii = 48; ii < 58; ii++) {
          ptmp[ii] = (Ipp32s)(((Ipp64s)ptmp[ii] * coeff) >> 13);
        }
      }
#endif
      coeff = state->frame - INITIAL_TIME;
      if (coeff < 32768) {
        if ((state->sinPow * 32768) < (10*state->rinPow * state->coupling)) {
          //mprintf("1");
          for (ii = 0; ii < 2*SBF_SUBBANDS; ii++) {
            ptmp[ii] = (Ipp32s)(((Ipp64s)ptmp[ii] * coeff) >> 15);
          }
        }
      }
    }
  }

#endif

  for (ii = 8; ii <= 32; ii++) {
    fira_err[ii].re = (Ipp32s)(((Ipp64s)(fira_err[ii].re) * spectra_gain_arr[ii]) >> 15);
    fira_err[ii].im = (Ipp32s)(((Ipp64s)(fira_err[ii].im) * spectra_gain_arr[ii]) >> 15);
  }
}

/* Process one frame */
static int processFrame(_isbfECState *state, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout,
                        int *dtFlag)
{
  int numSegments, tmp;
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,sin_sub,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,fira_err,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,firf_err,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,fira_out,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,firf_out,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32sc,prodStepErrQ,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY(32,Ipp32s_EC_Sfs,pStepSize,SBF_SUBBANDS,state);
  Ipp32sc* pSegment; /* Pointer to segment of input history */
  Ipp32sc* perr;
  Ipp32sc* pout;
  int      iSegment, i;
  int      compareFiltersEnable, isRestored = 0;
  int doubleTalk;
  if ((state == NULL) || (rin==NULL) || (sin==NULL) || (sout==NULL))
      return 1;

  compareFiltersEnable = ADAPTATION_ENABLED;

  if (state->cdlt > 0)
    compareFiltersEnable = 1;

  if (!((state->dt) && (state->upCounter)))
    compareFiltersEnable = 0;

  if (!ADAPTATION_ENABLED && (state->cdlt > 0) && (state->dt))
    compareFiltersEnable = 1; /* if beginning of the ADAPTATION_DISABLE and DT -> do comparison */

  numSegments = state->numSegments;

  /* update receive-in subband history buffer */
  pSegment = state->ppRinSubbandHistory[state->numSegments-1];

  for (i = START_SUBBAND; i < state->numSubbands; i++) {
    state->rinSubbandPow[i] -= SCALE(pSegment[i].re * (Ipp64s)pSegment[i].re +
                                            pSegment[i].im * (Ipp64s)pSegment[i].im, PSD_SF);
  }

  for (iSegment = state->numSegments-1; iSegment > 0; iSegment--) {
    state->ppRinSubbandHistory[iSegment] = state->ppRinSubbandHistory[iSegment-1];
  }
  state->ppRinSubbandHistory[0] = pSegment;

  /* do subband analysis */
  ippsSubbandAnalysis_16s32sc_Sfs(rin, state->ppRinSubbandHistory[0], state->state_ana_rin, 0, state->pBuf);
  ippsSubbandAnalysis_16s32sc_Sfs(sin, sin_sub, state->state_ana_sin, 0, state->pBuf);

  for (i = START_SUBBAND; i < state->numSubbands; i++) {
    state->rinSubbandPow[i] += SCALE(pSegment[i].re * (Ipp64s)pSegment[i].re +
                                                  pSegment[i].im * (Ipp64s)pSegment[i].im, PSD_SF);
  }

  /* do filtering with adaptive coeffs */

  ippsFIRSubbandLow_EC_32sc_Sfs((const Ipp32sc **)state->ppRinSubbandHistory,
                                (const Ipp32sc **)state->ppAdaptiveCoefs,
                                numSegments,
                                fira_out,
                                START_SUBBAND,
                                state->numSubbands,
                                F_SF);

  /* Get adaptive filter error */
  ippsSub_32sc_Sfs(fira_out, sin_sub, fira_err, SBF_SUBBANDS, 0);

  if (compareFiltersEnable) {
    /* do filtering with fixed coeffs */
    ippsFIRSubbandLow_EC_32sc_Sfs((const Ipp32sc **)state->ppRinSubbandHistory,
        (const Ipp32sc **)state->ppFixedCoefs, numSegments, firf_out,
                                  START_SUBBAND, state->numSubbands, F_SF);
    /* Get fixed filter error */
    ippsSub_32sc_Sfs(firf_out, sin_sub, firf_err, SBF_SUBBANDS, 0);
  }

  ippsSubbandControllerDT_EC_16s(fira_err, firf_err, state->ppAdaptiveCoefs, state->ppFixedCoefs,
      &state->acpwr, &isRestored, state->cdlt, compareFiltersEnable, ADAPTATION_ENABLED,
      START_SUBBAND, state->controller);
  if (isRestored == -1)
      state->fixCfgInit = 1;

  if (isRestored == 1) {
    perr = firf_err;
    pout = firf_out;
  } else {
    perr = fira_err;
    pout = fira_out;
  }

  doubleTalk = state->dt;
  ippsSubbandControllerDTUpdate_EC_16s((const Ipp32sc **)(state->ppRinSubbandHistory),
      sin_sub, perr, pStepSize, &doubleTalk, &(state->dt1), START_SUBBAND, state->controller);


  if (ADAPTATION_ENABLED) {
//    if (1) {
    {
      Ipp64s com_cpwr = 0;
      Ipp64s com_cdlt = 0;
      int ii;

      /* Update adaptive coeffs */
      ippsFIRSubbandLowCoeffUpdate_EC_32sc_I(
                        (const Ipp32sc **)(state->ppRinSubbandHistory),
                        perr,
                        state->ppAdaptiveCoefs,
                        numSegments,
                        prodStepErrQ,
                        pStepSize,
                        START_SUBBAND,
                        state->numSubbands,
                        F_SF);

      /* Update power of adapt. filter coeff. */
      for (ii = START_SUBBAND; ii < state->numSubbands; ii++) {
        Ipp32s shift;
        Ipp64s cdlt64, cdlt164;

        cdlt64 = (Ipp64s)prodStepErrQ[ii].re * pout[ii].re +
                 (Ipp64s)prodStepErrQ[ii].im * pout[ii].im;

        shift = pStepSize[ii].sf - 32 - (2*F_SF - PSD_SF);

        if (shift >= 0) cdlt64 >>=  shift;
        else            cdlt64 <<=  shift;

        cdlt164 = ((Ipp64s)prodStepErrQ[ii].re * prodStepErrQ[ii].re +
                  (Ipp64s)prodStepErrQ[ii].im * prodStepErrQ[ii].im) *
                    state->rinSubbandPow[ii];

        shift = 2*pStepSize[ii].sf - 62 - 2*F_SF;

        if (shift >= 0) cdlt164 >>= shift;
        else            cdlt164 <<= shift;

        cdlt64 += cdlt164;
        com_cpwr += cdlt64;

        if (cdlt64 < 0) cdlt64 = -cdlt64;
        com_cdlt += cdlt64;
      }

      state->acpwr += com_cpwr;
      state->cdlt = com_cdlt;
    }
  //else {
  //    //state->cdlt = 0;
  //  }
  } else {
    state->cdlt = 0;
  }

  if (state->nlp != 1)
#ifdef ADAPT_NOISE
  noise_supp(state, perr, sin_sub);
#else
  noise_supp(state, perr);
#endif

  ippsZero_32sc(perr, START_SUBBAND);
  spectra_gain(perr);

  /* do subband synthesis */
  ippsSubbandSynthesis_32sc16s_Sfs(perr, sout, state->state_syn, 0, state->pBuf);

  ippsMulC_16s_ISfs(SBF_COEF_NORM, sout, SBF_FRAME_SIZE, 14);

  if (state->nlp == 2)
    suppressor(state, rin,sin, sout);

  tmp = state->dt;
  state->dt = doubleTalk;

  if ((tmp == 0) && (state->dt == 1)) {
    int j;
//    state->controller2->fixedPwr = state->controller2->adaptivePwr = 0;
    state->upCounter = 0;
    if (state->fixCfgInit == 0) {
      //mprintf("INIT FIX_COFF!!!!!! frame = %i\n", state->frame);
      for (j = 0; j < numSegments; j++) {
        ippsCopy_32sc(state->ppAdaptiveCoefs[j], state->ppFixedCoefs[j], state->numSubbands);
      }
      state->fixCfgInit = 1;
    }
  } else if ((tmp == 1) && (state->dt == 0)) {
    /*int ii, jj;
    for (ii = 0; ii < state->numSubbands; ii++) {
    for (jj = 0; jj < state->numSegments; jj++) {
    state->ppAdaptiveCoefs[jj][ii] = state->ppFixedCoefs[jj][ii];
    }
    state->controller2->acpwr[ii] = state->controller2->fcpwr[ii];
    }*/
    }


  state->upCounter ^= 1;

  *dtFlag=state->dt1;
  state->frame += SBF_FRAME_SIZE;

  if (state->frame > 10*3600*8000) state->frame = 10*3600*8000;


  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32s_EC_Sfs,pStepSize,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,prodStepErrQ,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,firf_out,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,fira_out,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,firf_err,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,fira_err,SBF_SUBBANDS,state);
  LOCAL_ALIGN_ARRAY_FREE(32,Ipp32sc,sin_sub,SBF_SUBBANDS,state);

  return 0;
}

#define COEF_SMOOTH3 ((COEF_ONE * 9999 / 10000))
#define NLP_TRH ((COEF_ONE / 5))
#define NLP_SMOOTH ((COEF_ONE / 1024))

static void nlp(_isbfECState *state, Ipp16s *sin, Ipp16s *sout)
{
  int i;
  Ipp32s tmp0, tmp1;

  for (i = 0; i < SBF_FRAME_SIZE; i++) {
    tmp0 = sout[i];
    if (tmp0 <= 0) tmp0 = -tmp0;

    tmp1 = (COEF_SMOOTH * state->smoothSout) >> COEF_SF;
    if (tmp1 >= tmp0) {
      tmp0 = tmp1;
    }
    state->smoothSout = tmp0;
//saved
//    tmp0 = sin[i];
    tmp0 = state->saved_sin[state->saved_sin_idx];
    state->saved_sin[state->saved_sin_idx] = sin[i];
    state->saved_sin_idx ++;
    if (state->saved_sin_idx == 48)
      state->saved_sin_idx = 0;

    if (tmp0 <= 0) tmp0 = -tmp0;

    tmp1 = (COEF_SMOOTH * state->smoothSin) >> COEF_SF;
    if (tmp1 >= tmp0) {
      tmp0 = tmp1;
    }
    state->smoothSin = tmp0;

    state->shortTermSin = WEIGHTED_INT(state->shortTermSin, state->smoothSin, COEF_SMOOTH2);
    state->shortTermSout = WEIGHTED_INT(state->shortTermSout, state->smoothSout, COEF_SMOOTH2);

    /*if (!state->dt) {
      Ipp32s powSout = sout[i]*sout[i];
      state->longTermSout = WEIGHTED_INT(state->longTermSout, powSout, (Ipp64s)COEF_SMOOTH3);
    }*/

    if (((state->smoothSin <= state->smoothSout * 16)/* &&
      (state->shortTermSout <= state->smoothSout)*/) || state->dt1) {
        state->nlp_mult = ((COEF_ONE + NLP_SMOOTH) * state->nlp_mult) >> COEF_SF;
        if (state->nlp_mult > COEF_ONE) state->nlp_mult = COEF_ONE;
    } else {
      state->nlp_mult = ((COEF_ONE - NLP_SMOOTH) * state->nlp_mult) >> COEF_SF;
      if (state->nlp_mult < NLP_TRH) state->nlp_mult = NLP_TRH;
    }

    sout[i] = (Ipp16s)((sout[i] * state->nlp_mult) >> COEF_SF);
  }
  /* SBF_FRAME_SIZE = 16 -> ScaleFactor = 4 */
  //ippsDotProd_16s32s_Sfs(tmpBuf, tmpBuf, SBF_FRAME_SIZE, &tmpEn, 4);

  //if (tmpEn > state->longTermSout) {
  //  ippsCopy_16s(tmpBuf, sout, SBF_FRAME_SIZE);
  //}
}

static void nlp_cng(_isbfECState *state, Ipp16s *sout, int nr)
{
  Ipp16s tmpBuf[32];

  if (nr==2) {
#ifdef ADAPT_NOISE
    ippsRandUniform_Direct_16s(tmpBuf, SBF_FRAME_SIZE, -noise_lev<<3, noise_lev<<3, (unsigned int*)&(state->noise_seed));
#else
    ippsRandUniform_Direct_16s(tmpBuf, SBF_FRAME_SIZE, -NOISE_GEN_LEVEL<<3, NOISE_GEN_LEVEL<<3, (unsigned int*)&state->noise_seed);
#endif

    ownIIR_16s_I(tmpBuf, SBF_FRAME_SIZE, iirCfs, state->iirDly);
    ippsRShiftC_16s_I(3, tmpBuf, SBF_FRAME_SIZE);
//    ippsAddC_16s_I(30, tmpBuf, SBF_FRAME_SIZE);
    ippsAdd_16s_ISfs(tmpBuf, sout, SBF_FRAME_SIZE, 0);
//    ippsCopy_16s(tmpBuf, sout, SBF_FRAME_SIZE);
  }
}

int ec_isbf_ProcessFrame(void *stat, Ipp16s *rin, Ipp16s *sin, Ipp16s *sout)
{
    _isbfECState *state=(_isbfECState *)stat;
    int     ret=0,i,nlpOff=0;
    Ipp32s  dtFlag;

    //state->ah->Mem=state->Mem;
    state->ah->Mem.base=state->Mem.base;
    state->ah->Mem.CurPtr=state->Mem.CurPtr;
    state->ah->Mem.VecPtr=state->Mem.VecPtr;
    state->ah->Mem.offset=state->Mem.offset;

    if (TD_ENABLED){
        nlpOff=toneDisabler(rin, sin, state->ah->samplingRate, FRAME_SIZE_10ms,
            &state->ah->td);
    }
    if(nlpOff){
        ippsCopy_16s(sin,sout,FRAME_SIZE_10ms);
    } else {
        for (i=0;i<FRAME_SIZE_10ms;i+=SBF_FRAME_SIZE) {
            ret=processFrame(state, &rin[i], &sin[i], &sout[i],&dtFlag);
            if (ret)
                return ret;
            if (state->nlp == 2) {
                nlp(state, &sin[i], &sout[i]);
                state->save_noise_lev[i >> 4] = state->noise_lev;
            }
        }
        if (!(state->nlp == 2 && state->cng==2)) {
            cnr(state->ah,rin,sin,sout,FRAME_SIZE_10ms,state->nlp,state->cng,205,SBF_OFF);
        }
        if (AH_ENABLED || AH_ENABLED3) {
            //            formFFTin(state->ah,sout,FRAME_SIZE_10ms);
            //          shiftupFft(state->ah,sout,FRAME_SIZE_10ms);
            //    } else if (AH_ENABLED3) {
            shiftupCos(state->shiftupCos, sout, sout);
        }
        if (state->nlp == 2 && state->cng==2)
          for (i = 0;i < FRAME_SIZE_10ms; i+= SBF_FRAME_SIZE) {
            nlp_cng(state, &sout[i],state->nlp);
          }
    }

    return ret;
}

