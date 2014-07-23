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

#include <m_pd.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ffmpeg_internal.h"
#include "ffmpeg_common.h"

enum {
    AVCODEC_MAX_AUDIO_FRAME_SIZE = 192000
};

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 204800
#define AUDIO_REFILL_THRESH 4096

static t_class * ffmpeg_class;

typedef struct ffmpeg {
    t_object         x_obj;
    t_outlet * x_out;
} t_ffmpeg;

void * ffmpeg_new(void)
{
    t_ffmpeg *x = (t_ffmpeg *)pd_new(ffmpeg_class);
    x->x_out = outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void ffmpeg_free(t_ffmpeg * x)
{
    outlet_free(x->x_out);
}

static void ffmpeg_read(t_ffmpeg *x, t_symbol *s, int argc, t_atom *argv)
{
    int resize = 0;
    long finalsize = 0, maxsize = DEFMAXSIZE;

    t_garray * garrays[MAXSFCHANS];

    while (argc > 0 && argv->a_type == A_SYMBOL && *argv->a_w.w_symbol->s_name == '-') {
        char * flag = argv->a_w.w_symbol->s_name + 1;

        if (!strcmp(flag, "resize")) {
            resize = 1;
            argc -= 1;
            argv += 1;
        }
        else if (!strcmp(flag, "maxsize")) {
            if (argc < 2 || argv[1].a_type != A_FLOAT || ((maxsize = argv[1].a_w.w_float) < 0))
                goto usage;
            resize = 1; /* maxsize implies resize. */
            argc -= 2;
            argv += 2;
        }
        else
            goto usage;
    }

    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL)
        goto usage;

    const char * filename = argv[0].a_w.w_symbol->s_name;
    argc--;
    argv++;

    const int table_count = argc;

    for (int i = 0; i < table_count; i++) {
        int vecsize;
        t_word * vecs[MAXSFCHANS];

        if (argv[i].a_type != A_SYMBOL)
            goto usage;

        if (!(garrays[i] = (t_garray *)pd_findbyclass(argv[i].a_w.w_symbol, garray_class))) {
            pd_error(x, "%s: no such table", argv[i].a_w.w_symbol->s_name);
            return;
        }
        else if (!garray_getfloatwords(garrays[i], &vecsize, &vecs[i]))
            error("%s: bad template for tabwrite", argv[i].a_w.w_symbol->s_name);


        if (finalsize && finalsize != vecsize && !resize) {
            post("soundfiler_read: arrays have different lengths; resizing...");
            resize = 1;
        }

        finalsize = vecsize;
    }

    /* zero out remaining elements of vectors */
    //    for (int i = 0; i < argc; i++) {
    //        int vecsize;
    //        garray_getfloatwords(garrays[i], &vecsize, &vecs[i]);
    //        for (int j = itemsread; j < vecsize; j++)
    //            vecs[i][j].w_float = 0;
    //    }

    /* zero out vectors in excess of number of channels */
    //    for (int i = channels; i < argc; i++) {
    //        int vecsize;
    //        t_word *foo;
    //        garray_getfloatwords(garrays[i], &vecsize, &foo);
    //        for (j = 0; j < vecsize; j++)
    //            foo[j].w_float = 0;
    //    }

    post(PREFIX "audio_decode");

    int samples = audio_decode2(filename, garrays, table_count, resize);
    if(samples < 0) {
        error(PREFIX "audio decode failed");
        return;
    }

    outlet_float(x->x_obj.ob_outlet, (t_float) samples);

    return;

usage:
    pd_error(x, "usage: read [flags] filename tablename...");
    post("flags: -resize -maxsize <n> ...");
}

static void ffmpeg_write(t_ffmpeg * x, t_symbol *s, int argc, t_atom *argv)
{
    post(PREFIX "TODO");
    //    long bozo = soundfiler_dowrite(x, x->x_canvas,
    //                                   argc, argv);
    //    outlet_float(x->x_obj.ob_outlet, (t_float)bozo);
}

void ffmpegfiler_setup(void)
{
    ffmpeg_class = class_new(gensym("ffmpegfiler"),
                             (t_newmethod)ffmpeg_new,
                             (t_method)ffmpeg_free,
                             sizeof(t_ffmpeg),
                             CLASS_DEFAULT,
                             A_NULL, 0);

    class_addmethod(ffmpeg_class,
                    (t_method)ffmpeg_read, gensym("read"), A_GIMME, 0);
    class_addmethod(ffmpeg_class,
                    (t_method)ffmpeg_write, gensym("write"), A_GIMME, 0);

    // Initialize FFmpeg
    av_register_all();
}


