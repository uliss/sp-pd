/***************************************************************************
 *   Copyright (C) 2014 by Serge Poltavski                                 *
 *   serge.poltavski@gmail.com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program. If not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <stdint.h>

#include "ffmpeg_internal.h"
#include "ffmpeg_common.h"


class AudioBuffer
{
private:
    t_float * data_[MAXSFCHANS];
    size_t n_samples_;
    size_t n_channels_;
    bool is_alloc_;

    void alloc()
    {
        for(size_t i = 0; i < n_channels_; i++) {
            data_[i] = (t_float*) calloc(n_samples_, sizeof(t_float));
            if(data_[i] == NULL) {
                error(PREFIX "AudioBuffer allocation failed.");
                is_alloc_ = false;
                break;
            }
        }

        is_alloc_ = true;
    }

    void dealloc()
    {
        if(!is_alloc_)
            return;

        for(size_t i = 0; i < n_channels_; i++)
            free(data_[i]);

        is_alloc_ = false;
    }
public:
    AudioBuffer(size_t n_samples, size_t n_channels) :
        n_samples_(n_samples),
        n_channels_(n_channels),
        is_alloc_(false)
    {
        alloc();
    }

    ~AudioBuffer()
    {
        dealloc();
    }

    bool isAllocated() const { return is_alloc_; }

    /**
     * Returns maximum number of samples in each channel
     */
    inline size_t sampleCount() const { return n_samples_; }

    /**
     * Returns maximum number of audio channels in buffer
     */
    inline size_t channelCount() const { return n_channels_; }

    inline void setSample(int channel, int pos, t_float value)
    {
        assert(channel < n_channels_);
        assert(pos < n_samples_);
        data_[channel][pos] = value;
    }

    inline t_float sample(int channel, int pos) const {
        assert(channel < n_channels_);
        assert(pos < n_samples_);
        return data_[channel][pos];
    }
};

size_t aproxStreamSampleCount(AVStream * stream)
{
    if(!stream)
        return 0;

    static const double INCREASE = 1.2;
    const double time_base = (double) stream->time_base.num / (double) stream->time_base.den;
    const double duration = stream->duration * time_base;
    return (size_t) floor(duration * stream->codec->sample_rate * INCREASE);
}

static int maxChannelCount(const AVFrame * frame, AudioBuffer * buffer)
{
    const int frame_channels = av_frame_get_channels(frame);
    int res = std::min(frame_channels, (int) buffer->channelCount());
    return std::min((int) MAXSFCHANS, res);
}

static void readPlanarFrameData(const AVFrame * frame,
                                AudioBuffer * buffer,
                                AVSampleFormat fmt,
                                size_t dest_offset)
{
    const size_t sample_size = av_get_bytes_per_sample(fmt);

    // read each channel
    const int max_channels = maxChannelCount(frame, buffer);
    for(int i = 0; i < max_channels; i++) {
        // read samples
        for(int j = 0; j < frame->nb_samples; j++) {
            const uint8_t * p = frame->data[i] + (j * sample_size);
            const size_t dest_pos = dest_offset + j;

            if(dest_pos >= buffer->sampleCount())
                return;

            switch(fmt) {
            case AV_SAMPLE_FMT_U8P :
                buffer->setSample(i, dest_pos, (*(const uint8_t*)p)/(127.0 - 1.0));
                break;
            case AV_SAMPLE_FMT_S16P:
                buffer->setSample(i, dest_pos, (*(const int16_t*)p)/32767.0);
                break;
            case AV_SAMPLE_FMT_S32P:
                buffer->setSample(i, dest_pos, (*(const int32_t*)p)/2147483647.0);
                break;
            case AV_SAMPLE_FMT_FLTP:
                buffer->setSample(i, dest_pos, (*(const float *)p));
                break;
            case AV_SAMPLE_FMT_DBLP:
                buffer->setSample(i, dest_pos, (*(const double *)p));
                break;
            default:
                error(PREFIX "invalid sample format");
                return;
            }
        }
    }
}

static void readMixedFrameData(const AVFrame * frame,
                               AudioBuffer * buffer,
                               AVSampleFormat fmt,
                               size_t dest_offset)
{
    const size_t sample_size = av_get_bytes_per_sample(fmt);
    const int max_channels = maxChannelCount(frame, buffer);
    const int num_channels = av_frame_get_channels(frame);

    for(int smp = 0; smp < frame->linesize[0]; smp++) {
        const size_t dest_pos = dest_offset + smp;
        if(dest_pos >= buffer->sampleCount())
            return;

        for(int ch = 0; ch < max_channels; ch++) {
            const uint8_t * p = frame->data[0] + ((smp * num_channels) + ch) * sample_size;

            switch(fmt) {
            case AV_SAMPLE_FMT_U8 :
                buffer->setSample(ch, dest_pos, ((*(const uint8_t*)p)/127.0) - 1.0);
                break;
            case AV_SAMPLE_FMT_S16:
                buffer->setSample(ch, dest_pos, (*(const int16_t*)p)/32767.0);
                break;
            case AV_SAMPLE_FMT_S32:
                buffer->setSample(ch, dest_pos, (*(const int32_t*)p)/2147483647.0);
                break;
            case AV_SAMPLE_FMT_FLT:
                buffer->setSample(ch, dest_pos, (*(const float *)p));
                break;
            case AV_SAMPLE_FMT_DBL:
                buffer->setSample(ch, dest_pos, (*(const double *)p));
                break;
            default:
                error(PREFIX "invalid sample format");
                return;
            }
        }
    }
}

static void readFrameData(const AVCodecContext * codecContext,
                          const AVFrame * frame,
                          AudioBuffer * buffer,
                          int dest_offset)
{
    if(av_sample_fmt_is_planar(codecContext->sample_fmt))
        readPlanarFrameData(frame, buffer, codecContext->sample_fmt, dest_offset);
    else
        readMixedFrameData(frame, buffer, codecContext->sample_fmt, dest_offset);
}

/**
 * Resizes graphical arrays to new size
 * @param garrays - pointer to graphical arrays
 * @param garray_count - number of destination arrays
 * @param size - desired size
 */
static void resize_garray(t_garray ** garrays, const int garray_count, long new_size)
{
    t_word * vecs[MAXSFCHANS];

    for (int i = 0; i < garray_count; i++) {
        garray_resize_long(garrays[i], new_size);
        /* for sanity's sake let's clear the save-in-patch flag here */
        garray_setsaveit(garrays[i], 0);

        int vecsize = 0;
        garray_getfloatwords(garrays[i], &vecsize, &vecs[i]);
        /* if the resize failed, garray_resize reported the error */
        if (vecsize != new_size) {
            error(PREFIX "garray resize failed");
            return;
        }
    }
}

static int copy_samples(AudioBuffer * buffer, t_garray ** garrays, int garray_count)
{
    post(PREFIX "copy samples");

    assert(buffer);
    assert(buffer->isAllocated());

    t_word * vecs[MAXSFCHANS];
    int vecsize = 0;

    for (int i = 0; i < garray_count && i < buffer->channelCount(); i++) {
        garray_getfloatwords(garrays[i], &vecsize, &vecs[i]);

        for(int j = 0; j < vecsize && j < buffer->sampleCount(); j++) {
            vecs[i][j].w_float = buffer->sample(i, j);
        }
    }

    return vecsize;
}

int audio_decode(const char * filename, t_garray ** garrays, int garray_count, int resize)
{
    AVFrame * frame = av_frame_alloc();
    if (!frame) {
        error(PREFIX "Error allocating the frame");
        return -1;
    }

    AVFormatContext * formatContext = NULL;
    if (avformat_open_input(&formatContext, filename, NULL, NULL) != 0) {
        av_free(frame);
        error(PREFIX "Error opening file: %s", filename);
        return -1;
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        av_free(frame);
        avformat_close_input(&formatContext);
        error(PREFIX "Error finding the stream info");
        return -1;
    }

    // Find the audio stream
    AVCodec * cdc = NULL;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
    if (streamIndex < 0) {
        av_free(frame);
        avformat_close_input(&formatContext);
        error(PREFIX "Could not find any audio stream in the file");
        return -1;
    }

    AVStream * audioStream = formatContext->streams[streamIndex];

    av_dump_format(formatContext, streamIndex, NULL, 0);

    AVCodecContext * codecContext = audioStream->codec;
    codecContext->codec = cdc;

    if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0) {
        av_free(frame);
        avformat_close_input(&formatContext);
        error(PREFIX "Couldn't open the context with the decoder");
        return -1;
    }

    const size_t aprox_sample_count = aproxStreamSampleCount(audioStream);
    if(!aprox_sample_count) {
        av_free(frame);
        avformat_close_input(&formatContext);
        error(PREFIX "Can't calculate sample count.");
        return -1;
    }

    // allocate memory for decoding
    AudioBuffer buffer(aprox_sample_count, garray_count);
    if(!buffer.isAllocated()) {
        av_free(frame);
        avformat_close_input(&formatContext);
        error(PREFIX "Can't allocate memory for decoded data");
        return -1;
    }

    AVPacket readingPacket;
    av_init_packet(&readingPacket);

    int total_samples = 0;

    // Read the packets in a loop
    while (av_read_frame(formatContext, &readingPacket) == 0) {
        if (readingPacket.stream_index == audioStream->index) {
            AVPacket decodingPacket = readingPacket;

            // Audio packets can have multiple audio frames in a single packet
            while (decodingPacket.size > 0) {
                // Try to decode the packet into a frame
                // Some frames rely on multiple packets, so we have to make sure the frame is finished before
                // we can use it
                int gotFrame = 0;
                int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

                if (result >= 0 && gotFrame) {
                    decodingPacket.size -= result;
                    decodingPacket.data += result;

                    // We now have a fully decoded audio frame
                    readFrameData(codecContext, frame, &buffer, total_samples);

                    total_samples += frame->nb_samples;
                }
                else {
                    decodingPacket.size = 0;
                    decodingPacket.data = NULL;
                }
            }
        }

        // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
        av_free_packet(&readingPacket);
    }

    // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
    // is set, there can be buffered up frames that need to be flushed, so we'll do that
    if (codecContext->codec->capabilities & CODEC_CAP_DELAY) {
        av_init_packet(&readingPacket);
        // Decode all the remaining frames in the buffer, until the end is reached
        int gotFrame = 0;
        while (avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket) >= 0 && gotFrame) {
            // We now have a fully decoded audio frame
            readFrameData(codecContext, frame, &buffer, total_samples);

            total_samples += frame->nb_samples;
        }
    }

    // Clean up!
    av_free(frame);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    // resize output garray
    if(resize) {
        resize_garray(garrays, garray_count, total_samples);
    }

    // copy samples
    int res = copy_samples(&buffer, garrays, garray_count);

    // redraw arrays
    for (int i = 0; i < garray_count; i++)
        garray_redraw(garrays[i]);

    return res;
}
