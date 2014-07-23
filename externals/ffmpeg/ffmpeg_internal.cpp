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
#include <iostream>
#include <stdint.h>

#include "ffmpeg_internal.h"
#include "ffmpeg_common.h"


class OutputBuffer
{
private:
    t_float * data_[MAXSFCHANS];
    size_t channel_size_;
    size_t num_channels_;
    bool is_alloc_;

    void alloc()
    {
        for(size_t i = 0; i < num_channels_; i++) {
            data_[i] = (t_float*) calloc(channel_size_, sizeof(t_float));
            if(data_[i] == NULL) {
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

        for(size_t i = 0; i < num_channels_; i++)
            free(data_[i]);

        is_alloc_ = false;
    }
public:
    OutputBuffer(size_t ch_size, size_t n_ch) :
        channel_size_(ch_size),
        num_channels_(n_ch),
        is_alloc_(false)
    {
        alloc();
    }

    ~OutputBuffer()
    {
        dealloc();
    }

    bool isAllocated() const { return is_alloc_; }

    /**
     * Returns number of samples in each channel
     */
    inline size_t channelSize() const { return channel_size_; }

    /**
     * Returns number of audio channels in buffer
     */
    inline size_t channelCount() const { return num_channels_; }

    inline void setSample(int channel, int pos, t_float value)
    {
        assert(channel < num_channels_);
        assert(pos < channel_size_);
        data_[channel][pos] = value;
    }

    inline t_float sample(int channel, int pos) const {
        assert(channel < num_channels_);
        assert(pos < channel_size_);
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

static int maxChannelCount(const AVFrame * frame, OutputBuffer * buffer)
{
    const int frame_channels = av_frame_get_channels(frame);
    int res = std::min(frame_channels, (int) buffer->channelCount());
    return std::min((int) MAXSFCHANS, res);
}

static void readFrameData(const AVCodecContext * codecContext,
                          const AVFrame * frame,
                          OutputBuffer * buffer,
                          int samples_read)
{
    int is_planar = av_sample_fmt_is_planar(codecContext->sample_fmt);
    int sample_size = av_get_bytes_per_sample(codecContext->sample_fmt);

    for(int i = 0; i < maxChannelCount(frame, buffer); i++) {
        if(is_planar) {
            for(int j = 0; j < frame->nb_samples && j < buffer->channelSize(); j++) {
                uint8_t * v = frame->data[i] + (j * sample_size);

                switch(codecContext->sample_fmt) {
                case AV_SAMPLE_FMT_FLTP: {
                    float vf = *((float*)v);
                    buffer->setSample(i, j + samples_read, vf);
                }
                    break;
                case AV_SAMPLE_FMT_DBLP: {
                    double vd = *((double*)v);
                    buffer->setSample(i, j + samples_read, vd);
                }
                    break;
                case AV_SAMPLE_FMT_S16P: {
                    int16_t v16 = *((int16_t*) v);
                    buffer->setSample(i, j + samples_read, v16 / (t_float) 0x7FFF);
                }
                default:
                    break;
                }
            }
        }
        else {
            for(int j = 0; j < frame->linesize[0] && j < buffer->channelSize(); j++) {
                uint8_t * v = &(frame->data[0][j * sample_size + i]);

                switch(codecContext->sample_fmt) {
                case AV_SAMPLE_FMT_S16: {
                    int16_t vi = *((int16_t*)v);
                    buffer->setSample(i, j + samples_read, static_cast<t_float>(vi / (t_float) 0x7FFF));
                }
                    break;
                case AV_SAMPLE_FMT_S32: {
                    int32_t vi = *((int32_t*)v);
                    buffer->setSample(i, j + samples_read, static_cast<t_float>(vi / (t_float) INT32_MAX));
                }
                    break;
                default:
                    break;
                }
            }
        }
    }
}

/**
 * Resizes graphcal arrays to new size
 * @param garrays - pointer to graphical arrays
 * @param garray_count - number of destination arrays
 * @param size - desired size
 */
static void resize_garray(t_garray ** garrays, const int garray_count, long size)
{
    post(PREFIX "resize garrays to %ld samples", size);

    t_word * vecs[MAXSFCHANS];

    for (int i = 0; i < garray_count; i++) {
        garray_resize_long(garrays[i], size);
        /* for sanity's sake let's clear the save-in-patch flag here */
        garray_setsaveit(garrays[i], 0);

        int vecsize = 0;
        garray_getfloatwords(garrays[i], &vecsize, &vecs[i]);
        /* if the resize failed, garray_resize reported the error */
        if (vecsize != size) {
            error(PREFIX "garray resize failed");
            return;
        }
    }
}

static int copy_samples(OutputBuffer * buffer, t_garray ** garrays, int garray_count)
{
    post(PREFIX "copy samples");

    assert(buffer);
    assert(buffer->isAllocated());

    t_word * vecs[MAXSFCHANS];
    int vecsize = 0;

    for (int i = 0; i < garray_count && i < buffer->channelCount(); i++) {
        garray_getfloatwords(garrays[i], &vecsize, &vecs[i]);

        for(int j = 0; j < vecsize && j < buffer->channelSize(); j++) {
            vecs[i][j].w_float = buffer->sample(i, j);
        }
    }

    return vecsize;
}

int audio_decode2(const char * filename, t_garray ** garrays, int garray_count, int resize)
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
    OutputBuffer buffer(aprox_sample_count, garray_count);
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
