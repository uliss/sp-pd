// mill - granular synthesizer for Pure data
//
// Copyright (c) 2011 Olli Erik Keskinen
// All rights reserved.
//
// This code is released under The BSD 2-Clause License.
// See the file LICENSE.txt for information.


#include "m_pd.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h> 
#include <math.h>
#include <pthread.h>
#include "envtab.h"

#define ENVRES 4096
#define MAXGRAINS 10000
#define BLOCKSIZE 1024
#define JN 4

static t_class *mill_tilde_class;

typedef struct t_grain {
	t_sample	cur;
	t_int		offset;
	t_sample	targ;
	t_sample	vel;
	t_sample	step;
	t_sample	pos;
	t_sample	envtarg1;
	t_sample	envtarg2;
	//t_sample	envstep1;
	//t_sample	envstep2;
	t_sample	vol1;
	t_sample	vol2;
} t_grain;

typedef struct t_grainlist {
	t_grain **i;
	bool full;
	t_grain	*list[MAXGRAINS];
} t_grainlist;

typedef struct t_remlist {
	t_grain ***i;
	t_grain	**list[MAXGRAINS];
} t_remlist;

typedef struct t_threadholder {
	void *root;
	t_sample	bus1[BLOCKSIZE];
	t_sample	bus2[BLOCKSIZE];
	t_grain		**grain;
	t_grain		**execto;
	t_remlist rem;
} t_threadholder;

typedef struct t_randlist {
	int	i;
	t_float list[4096];
} t_randlist;

typedef struct _mill_tilde {

    t_object  x_obj;
	t_outlet *sigout1, *sigout2, *auxout;
    
	t_grain **lastgrain; // last core n

	t_grainlist run;

    t_sample f_addtime, f_dens, f_dur, f_vel, f_pos;
	t_sample f_envq, f_envb, f_pan;
    t_sample f_densr, f_durr, f_velr, f_posr, f_panr;
	t_grain grain[MAXGRAINS];

	t_int timer, addtime, nsamples;
	
    t_word *x_vec;
	int arraysize;
    t_symbol *arrayname;

	t_threadholder tdata[JN];

	pthread_t thread[JN];
	t_grain **threadlimits[JN+1];

	t_randlist random;
    
} t_mill_tilde;

void mill_tilde_addgrain(t_mill_tilde *c, int);
void mill_tilde_remgrain(t_mill_tilde *c, t_grain **grain);
void *mill_tilde_threadop(void *ptr);
void mill_tilde_refreshthalloc(t_mill_tilde *c);
void mill_tilde_initrand(t_mill_tilde *c);
t_float mill_tilde_getrand(t_mill_tilde *c);

//////////////////
///// CREATE /////
//////////////////

void *mill_tilde_new(t_symbol *sname)
{
    t_mill_tilde *x = (t_mill_tilde *)pd_new(mill_tilde_class);

    x->arrayname = sname;
    x->x_vec = 0;

    // init core lists
    int i;
    for (i=0; i < MAXGRAINS; i++)
    {
		x->run.list[i] = &x->grain[i];
//		post("i: %i \t run.list[i] p: %p", i, x->run.list[i]);
	}
	
	x->run.i = &x->run.list[0];
	x->run.full = false;
    x->lastgrain = &x->run.list[MAXGRAINS - 1];

    srand((unsigned)time(NULL));
	mill_tilde_initrand(x);

    x->f_dens	= 0;
	x->f_dur	= 0;
	x->f_vel	= 0;
	x->f_pos	= 0;
	x->f_pan	= 0;

	x->f_densr	= 0;
	x->f_durr	= 0;
	x->f_velr	= 0;
	x->f_posr	= 0;
	x->f_panr	= 0;

	x->f_envq	= 0;
	x->f_envb	= 0;
    x->timer	= 0;
    x->addtime	= 0;
		
	x->threadlimits[0]= &x->run.list[0];
	x->threadlimits[JN]= x->run.i;
	
	int core;
	for (core=0;core<JN;core++)
	{
		x->tdata[core].root = (void*)x;
		x->tdata[core].rem.i = &x->tdata[core].rem.list[0];
	}

   	x->sigout1 = outlet_new(&x->x_obj, &s_signal);
   	x->sigout2 = outlet_new(&x->x_obj, &s_signal);
   	x->auxout = outlet_new(&x->x_obj, &s_float);

    return (void *)x;
}

///////////////////////////
///// PERFORM ROUTINE /////
///////////////////////////

t_int *mill_tilde_perform(t_int *w)
{
    t_mill_tilde   *x = (t_mill_tilde *)(w[1]);
    t_sample        *out2 = (t_sample *)(w[2]);
    t_sample        *out1 = (t_sample *)(w[3]);
    int nsamples =      (int)(w[4]);
	x->nsamples = nsamples;

    t_word *buf = x->x_vec;

    t_sample bus = 0;

    int thissample;	// general sample counter
	int core;		// general core counter	

    if (!buf) goto zero;

// timer operation //

	if (x->f_dens > 0)
	{
		for (thissample=0; thissample<nsamples;thissample++)
		{
			if (++x->timer >= x->addtime)
			{
				mill_tilde_addgrain(x, thissample);
				mill_tilde_refreshthalloc(x);
				x->timer = 0;
			}
		}
	} else {
		x->timer = 0;
	}
	mill_tilde_refreshthalloc(x);


// init bus //
    for (thissample=0; thissample < x->nsamples; thissample++)
    {
		for (core=0;core<JN;core++)
		{
			x->tdata[core].bus1[thissample] = 0;
			x->tdata[core].bus2[thissample] = 0;
		}
    }

	if (x->run.i > &x->run.list[0])
	{

		for (core=0;core<JN;core++)
		{
			x->tdata[core].grain = x->threadlimits[core];
			x->tdata[core].execto = x->threadlimits[core+1];
			pthread_create(&x->thread[core], NULL, mill_tilde_threadop, (void*)&x->tdata[core]);
		}
		for (core=0;core<JN;core++)
		{
			pthread_join( x->thread[core], NULL);
		}
		for (core=0;core<JN;core++)
		{
			while (x->tdata[core].rem.i > &x->tdata[core].rem.list[0])
			{
				x->tdata[core].rem.i--;
				mill_tilde_remgrain(x, *x->tdata[core].rem.i); 
				mill_tilde_refreshthalloc(x);
			}
		}
	}

// dump bus //
    for (thissample=0; thissample < nsamples; thissample++)
    {
		t_sample temp1 =0;
		t_sample temp2 =0;
		for (core=0;core<JN;core++)
		{
			temp1 += x->tdata[core].bus1[thissample];
			temp2 += x->tdata[core].bus2[thissample];
		}
		
        *(out1 + thissample) = temp1;
        *(out2 + thissample) = temp2;
    }
	
    return (w+5);

        zero:
    for (thissample=0; thissample <= nsamples; thissample++) {
        *(out1 + thissample) = 0;
        *(out2 + thissample) = 0;
    }
    return (w+5);
}



void *mill_tilde_threadop(void *ptr)
{
	
    t_threadholder *thisthread;
    thisthread = (t_threadholder *) ptr;
    t_mill_tilde *x; 
    x = (t_mill_tilde *) thisthread->root;

	t_grain **g;
	g =thisthread->grain;

	int thissample;

	while (g < thisthread->execto)
	{
		if ((**g).offset != 0)
		{
			thissample = (**g).offset;
			(**g).offset = 0;
		} else {
			thissample = 0;
		}

		while (thissample < x->nsamples)
		{
			(**g).cur += (**g).vel;

 			int envindex; t_sample envsig;

			if ((**g).cur < (**g).envtarg1)
			{
				if ((**g).envtarg1 > 0)
					envindex = (ENVRES * (**g).cur) / (**g).envtarg1;
				else envindex = ENVRES;
			}
			else if ((**g).cur > (**g).envtarg2) 
			{
				if ((**g).envtarg2 < (**g).targ)
					envindex = ENVRES * ( ((**g).targ - (**g).cur) / ((**g).targ - (**g).envtarg2 ));
				else envindex = ENVRES;
			}
			else {
				envindex = ENVRES;
			}

			if (envindex >= ENVRES) {
				envsig = 1;
			} else if (envindex < 0) {
				envsig = 0;
			} else {
    			envsig = hannwindow[envindex];
			}

			int index = (**g).cur + (**g).pos;
			if  (index < x->arraysize) {
				t_sample unpanned = (x->x_vec[index].w_float)*envsig;
				*(thisthread->bus1 + thissample) += (**g).vol1 * unpanned;
				*(thisthread->bus2 + thissample) += (**g).vol2 * unpanned;
			}
			
			if ((( (**g).vel>0) && ( (**g).cur>=(**g).targ ))
				|| (( (**g).vel<0) && ( (**g).cur<=0) ))
			{
				*thisthread->rem.i = g;
				thisthread->rem.i++;
				thissample = x->nsamples;
			}
			thissample ++;
		}
		g++;
	}
	return (NULL);
}


void mill_tilde_refreshthalloc(t_mill_tilde *x) 
{
	int dist, core;
	x->threadlimits[JN] = x->run.i;
	dist = (x->run.i - &x->run.list[0]);
	dist /= JN;
	for (core=1;core<JN;core++)
	{
		x->threadlimits[core] = &x->run.list[0] + (dist*core);
	}
}

void mill_tilde_addgrain(t_mill_tilde *x, int offset)
{
	t_sample targ, vel, pos;	

    if (x->run.full == false) 
	{

		(**x->run.i).targ = x->f_dur * (1 + mill_tilde_getrand(x) * x->f_durr);
		if ((**x->run.i).targ <= 0) return;

		(**x->run.i).vel = x->f_vel * (1 + mill_tilde_getrand(x) * x->f_velr);
		if ((x->f_vel >= 0) && ((**x->run.i).vel <= 0)) return;
		if ((x->f_vel <= 0) && ((**x->run.i).vel >= 0)) return;

		(**x->run.i).pos = x->f_pos * (1 + mill_tilde_getrand(x)*x->f_posr);
		if ((**x->run.i).pos <= 0) return;

		if ((**x->run.i).vel > 0)	(**x->run.i).cur = 0;			// start from 0
		else						(**x->run.i).cur = (**x->run.i).targ;	// start from targ



		(**x->run.i).offset = offset;
//		(**x->run.i).targ = targ;		// go there
//		(**x->run.i).vel = vel;			// velocity
//		(**x->run.i).step = vel;		// step size
//		(**x->run.i).pos = pos;		// playhead position
		(**x->run.i).envtarg1 = ((**x->run.i).targ * x->f_envq * x->f_envb) - 1;
		(**x->run.i).envtarg2 = (**x->run.i).targ * (1 - x->f_envq * ( 1 -x->f_envb)) + 1;
		//(**x->run.i).envstep1 = (4096 * vel) / (**x->run.i).envtarg1;			// step size
		//(**x->run.i).envstep2 = (4096 * vel) /(targ - (**x->run.i).envtarg2);			// step size

		t_sample pan = mill_tilde_getrand(x) * x->f_panr + x->f_pan;
		if (pan > 1) pan = 1;
		else if (pan < 0) pan = 0;
		//post("pan: %f", pan);
		(**x->run.i).vol1 = pan;
		(**x->run.i).vol2 = 1 - pan;		

		//post("ADDGRAIN et1 %f et2 %f es1 %f es2 %f", (**x->run.i).envtarg1, (**x->run.i).envtarg2, (**x->run.i).envstep1, (**x->run.i).envstep2); 

		if (x->run.i != x->lastgrain)	x->run.i++;
		else							x->run.full = true;
	}
	x->addtime = x->f_dens * (1 + mill_tilde_getrand(x)*x->f_densr);
}

void mill_tilde_remgrain(t_mill_tilde *x, t_grain **removed) 
{
	t_grain *temp;
	x->run.i--;
	temp = *x->run.i;
	*x->run.i = *removed;
	*removed = temp;
	x->run.full = false;
}

void mill_tilde_initrand(t_mill_tilde *x)
{
	int i;
	t_float entry;
	for (i=0; i<4096; i++)
	{
		do {
			entry = ((float) rand()/((float)RAND_MAX/2)) - 1;
		} while ( entry > 1 || entry < -1);

		x->random.list[i] = pow(entry, 3);
	}
	x->random.i = 0;
}

t_float mill_tilde_getrand(t_mill_tilde *x)
{
	x->random.i +=1;
	if (x->random.i >= 4096)
	{
		x->random.i = 0;
	}
	return (x->random.list[x->random.i]);
}

//DENS//
void mill_tilde_setdens(t_mill_tilde *x, t_floatarg f)
{
    x->f_dens = f*44100;
	x->f_addtime = 0;
#if 0    
    if (x->b_syncOn==0)
			{   
        if (x->f_dens < 0) {
            x->f_addTime = 0;
        } else {
            x->f_addTime = x->f_dens;
        }   
    }   
#endif
}
//DUR//
void mill_tilde_setdur(t_mill_tilde *x, t_floatarg f)
{
    x->f_dur = f*44100;
	post("dur: %f", x->f_dur);
}
//VEL//
void mill_tilde_setvel(t_mill_tilde *x, t_floatarg f)
{
    x->f_vel = f;
}
//POS//
void mill_tilde_setpos(t_mill_tilde *x, t_floatarg f)
{
    x->f_pos = f*44100;
}
#if 0
//DIR//
void mill_tilde_setdir(t_mill_tilde *x, t_floatarg f)
{
    x->f_dirAdd = f;

    if (x->f_dirAdd > 1)    {   
        x->f_dirAdd = 1;
    } else if (x->f_dirAdd < 0) {
        x->f_dirAdd = 0;
    } else if (x->f_dirAdd < 0.4) {
        x->f_dirAdd *= 1.25;
    } else if (x->f_dirAdd > 0.6) {
        x->f_dirAdd = 1.25*x->f_dirAdd - 0.25;
    } else {
        x->f_dirAdd = 0.5;
    }   
}
#endif
//DENS R//
void mill_tilde_setdensr(t_mill_tilde *x, t_floatarg f)
{
	x->f_densr = f;
}
//DUR R//
void mill_tilde_setdurr(t_mill_tilde *x, t_floatarg f)
{
    x->f_durr = f;
}
//VEL R//
void mill_tilde_setvelr(t_mill_tilde *x, t_floatarg f)
{
    x->f_velr = f;
}
//POS R//
void mill_tilde_setposr(t_mill_tilde *x, t_floatarg f)
{
    x->f_posr = f;
}

//PAN R//
void mill_tilde_setpanr(t_mill_tilde *x, t_floatarg f)
{
    x->f_panr = f;
}

//ENVQ//
void mill_tilde_setenvq(t_mill_tilde *x, t_floatarg f)
{
    x->f_envq = f;

    if (x->f_envq > 1)  {
        x->f_envq = 1;
    } else if (x->f_envq < 0) {
        x->f_envq = 0;
    }
}
//ENVB//
void mill_tilde_setenvb(t_mill_tilde *x, t_floatarg f)
{
    x->f_envb = f;

    if (x->f_envb > 1)  {
        x->f_envb = 1;
    } else if (x->f_envb < 0) {
        x->f_envb = 0;
    }
}
//PAN//
void mill_tilde_setpan(t_mill_tilde *x, t_floatarg f)
{
    x->f_pan = f;

    if (x->f_pan > 1)  {
        x->f_pan = 1;
    } else if (x->f_pan < 0) {
        x->f_pan = 0;
    }
}

//JUICE//
void mill_tilde_juice(t_mill_tilde *x)
{
	int dist;
	dist = (x->run.i - &x->run.list[0]);
	post("grains in use: %i", dist);
}


// set the array //

void mill_tilde_set(t_mill_tilde *x, t_symbol *s)
{
     t_garray *a;

     x->arrayname = s;
     if (!(a = (t_garray *)pd_findbyclass(x->arrayname, garray_class)))
     {
         if (*s->s_name)
             pd_error(x, "tabread~: %s: no such array", x->arrayname->s_name);
         x->x_vec = 0;
     }
     else if (!garray_getfloatwords(a, &x->arraysize, &x->x_vec))
     {
         pd_error(x, "%s: bad template for tabread~", x->arrayname->s_name);
         x->x_vec = 0;
     }
     else garray_usedindsp(a);
}


// dsp connect //

void mill_tilde_dsp(t_mill_tilde *x, t_signal **sp)
{

    mill_tilde_set(x, x->arrayname);

    dsp_add(mill_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
	post("DSP ADD");
}

// setup //

void mill_tilde_setup(void) {
  mill_tilde_class = class_new(gensym("mill~"),/* nimi */
        (t_newmethod)mill_tilde_new,       /* constructor*/
        0, sizeof(t_mill_tilde),       /* dstructor, size */
        CLASS_DEFAULT,              /* ulkoasu */
        A_DEFSYM, A_DEFSYM, 0);                 /* argumentit */
    //class_addlist(mill_tilde_class, mill_tilde_list);
    //class_addbang(mill_tilde_class, mill_tilde_bang);
    class_addmethod(mill_tilde_class,
            (t_method)mill_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setdens, gensym("dens"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setdur, gensym("dur"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setvel, gensym("vel"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setpos, gensym("pos"),  
            A_DEFFLOAT, 0); 
#if 0
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setdir, gensym("dir"),  
            A_DEFFLOAT, 0);
#endif
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setdensr, gensym("densr"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setdurr, gensym("durr"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setvelr, gensym("velr"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setposr, gensym("posr"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setenvq, gensym("envq"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setenvb, gensym("envb"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setpan, gensym("pan"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_setpanr, gensym("panr"),  
            A_DEFFLOAT, 0); 
    class_addmethod(mill_tilde_class, 
            (t_method)mill_tilde_juice, gensym("juice"),  
            A_DEFFLOAT, 0); 
}

