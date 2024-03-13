CC=g++

CCFLAGS=`root-config --cflags`
CFLAGS=`root-config --cflags`

INCLUDE= -Iinclude/

LIBS=-lSpectrum -lrt
IMPORT_LIBS=
LDFLAGS=-Wl,--no-as-needed `root-config --glibs`

binaries=MultChAna
obj=src/RunControl.o src/Analysis.o src/Channel.o src/Graphix.o src/Calibration.o

all: $(binaries)

MultChAna: $(obj) src/MultChAna.o
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(LIBS) -o bin/MultChAna

src/EventDict.cpp:
	@echo "Generating Dictionary $@..."
	cd include ; rootcint -f EventDict.cpp -c $(CFLAGS) -p EventType.h TSTiC2_Ana.h LinkDef.h ; mv EventDict.cpp ../src/
	@echo "Done generating, exit code: $!"


clear:
	rm -f src/*.o
#	rm -f include/EventDict.h
#	rm -f src/EventDict.cpp

clean:	clear
	rm -f $(binaries)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -include sstream -o $@ -g -c $<

