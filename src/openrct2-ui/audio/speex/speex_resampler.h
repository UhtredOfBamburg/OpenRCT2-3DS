#pragma once
#include <stdint.h>
#include <stdlib.h>

typedef int16_t spx_int16_t;
typedef unsigned int spx_uint32_t;

struct SpeexResamplerState_ {
    spx_uint32_t channels;
    spx_uint32_t in_rate;
    spx_uint32_t out_rate;
};

typedef struct SpeexResamplerState_ SpeexResamplerState;

static inline SpeexResamplerState * speex_resampler_init(
    spx_uint32_t nb_channels,
    spx_uint32_t in_rate,
    spx_uint32_t out_rate,
    int quality,
    int *err)
{
    (void)quality;
    if (err) *err = 0;
    SpeexResamplerState *st = (SpeexResamplerState *)malloc(sizeof(SpeexResamplerState));
    if (st) {
        st->channels = nb_channels ? nb_channels : 2;
        st->in_rate = in_rate ? in_rate : 22050;
        st->out_rate = out_rate ? out_rate : 22050;
    }
    return st;
}

static inline void speex_resampler_destroy(SpeexResamplerState *st)
{
    if (st) free(st);
}

static inline int speex_resampler_set_rate(
    SpeexResamplerState *st,
    spx_uint32_t in_rate,
    spx_uint32_t out_rate)
{
    if (st) {
        if (in_rate) st->in_rate = in_rate;
        if (out_rate) st->out_rate = out_rate;
    }
    return 0;
}

static inline int speex_resampler_process_interleaved_int(
    SpeexResamplerState *st,
    const spx_int16_t *in,
    spx_uint32_t *in_len,
    spx_int16_t *out,
    spx_uint32_t *out_len)
{
    if (!st || !in || !out || !in_len || !out_len) return -1;
    spx_uint32_t ch = st->channels;
    spx_uint32_t in_frames = *in_len;
    spx_uint32_t out_frames = *out_len;
    if (in_frames == 0 || out_frames == 0) {
        *in_len = 0;
        *out_len = 0;
        return 0;
    }

    // 16.16 fixed-point step
    uint32_t step_fp = (uint32_t)(((uint64_t)st->in_rate << 16) / (st->out_rate ? st->out_rate : 1));
    uint32_t pos_fp = 0;
    spx_uint32_t produced = 0;

    while (produced < out_frames) {
        spx_uint32_t idx = pos_fp >> 16;
        if (idx >= in_frames) break;
        spx_uint32_t next_idx = (idx + 1 < in_frames) ? (idx + 1) : idx;
        uint32_t frac = pos_fp & 0xFFFFu;

        for (spx_uint32_t c = 0; c < ch; c++) {
            int32_t s0 = in[idx * ch + c];
            int32_t s1 = in[next_idx * ch + c];
            out[produced * ch + c] = (spx_int16_t)(s0 + (((s1 - s0) * (int32_t)frac) >> 16));
        }
        produced++;
        pos_fp += step_fp;
    }

    spx_uint32_t consumed = pos_fp >> 16;
    if (consumed > in_frames) consumed = in_frames;
    *in_len = consumed;
    *out_len = produced;
    return 0;
}
