NAME=soundtouch~
CSYM=soundtouch_tilde

# for MSW, set path to pd.dll:
PDPATH = "c:/Program Files/pd/bin"

# set current target suffix: 
current: pd_linux

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -msse -DPD -DUNIX -DICECAST -O3 -funroll-loops -fomit-frame-pointer -fcheck-new \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch -fvisibility=hidden

LINUXINCLUDE =  -I ./include

$(NAME).pd_linux: 
	g++ $(LINUXCFLAGS) $(LINUXINCLUDE) -c *.cpp 
	g++ --export-dynamic -shared -o $*.pd_linux *.o -lc -lm -lstdc++ 
	strip --strip-unneeded $(NAME).pd_linux
	rm -f *.o ../$(NAME).pd_linux
	cp $(NAME).pd_linux ../$(NAME).pd_linux

# ----------------------- Mac OSX -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O3 -Wall -W -Wshadow \
    	-Wno-unused -Wno-parentheses -Wno-switch \
	-fcheck-new -fvisibility=hidden

DARWINLDLAGS = -bundle -undefined suppress -flat_namespace

DARWININCLUDE = -I ./include	

$(NAME).pd_darwin:
	
	g++ $(DARWINCFLAGS) $(DARWININCLUDE) -c *.cpp
	
	g++ $(DARWINLDLAGS) -o $(NAME).pd_darwin *.o 

	strip -x $(NAME).pd_darwin	
	rm -f *.o ../$(NAME).pd_darwin
	cp $(NAME).pd_darwin ../$(NAME).pd_darwin

# ----------------------- WIN i386 with MinGW ----------------------

pd_win: $(NAME).dll

.SUFFIXES: .dll

WINCFLAGS = -msse -DPD -DMSW -Werror -Wno-unused -mms-bitfields\
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer\
	-fno-strict-aliasing -fcheck-new

WINLDFLAGS = -static-libgcc -static-libstdc++ --export-dynamic -shared

WININCLUDE =  -I ./include

$(NAME).dll:
	
	g++ $(WINCFLAGS) $(WININCLUDE) -c *.cpp

	g++ $(WINLDFLAGS) -o $*.dll *.o  $(PDPATH)/pd.dll -lm	

	strip --strip-unneeded $*.dll
	rm -f *.o ../$*.dll
	cp $(NAME).dll ../$(NAME).dll

# ----------------------------------------------------------------------

clean:
	rm -f *.o $(NAME).pd_darwin $(NAME).pd_linux $(NAME).dll
