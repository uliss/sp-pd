set(SRC
	PenroseOscil.c
	PenroseRand.c
	bloscbank.c
	convert.c
	fft.c
	fft4.c
	fftease_setup.c
	fold.c
	leanconvert.c
	leanunconvert.c
	limit_fftsize.c
	makewindows.c
	overlapadd.c
	power_of_two.c
	qsortE.c
	unconvert.c
)

add_library(fftease STATIC ${SRC})

