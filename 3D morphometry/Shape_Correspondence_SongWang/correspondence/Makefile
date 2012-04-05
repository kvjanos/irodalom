CC = gcc
CPP = g++
CDF_INC = -I cdf/include
OOQP_INC = -I ooqp/include

CFLAGS = -c -g -O3 $(CDF_INC) $(OOQP_INC)

BLAS = ooqp/extras/blas/libblas.a
LIBG2C = /usr/lib/gcc-lib/i386-redhat-linux/3.2.3/libg2c.a

LIBS = -L ooqp/lib/ -L cdf/lib/ -lcdf -looqpgendense -looqpdense -looqpgondzio -looqpbase $(BLAS) $(LIBG2C)

SRC = src/2d_corr.c src/curve_length.c src/locate_point.c src/slide_a_step.c src/thin_plate_spline.c src/initialize_landmarks.c src/shape_utils.c src/xfig_utils.c src/shape_statistic.c src/ascii_to_cdf.c src/cdf_to_ascii.c

CORR_OBJ = 2d_corr.o curve_length.o locate_point.o slide_a_step.o thin_plate_spline.o initialize_landmarks.o shape_utils.o xfig_utils.o
SS_OBJ = shape_statistic.o xfig_utils.o shape_utils.o
ASCII2CDF_OBJ = ascii_to_cdf.o 
CDF2ASCII_OBJ = cdf_to_ascii.o

OS=linux
ENV=gnu

ifeq ($(OS),cygwin)
	CORR_EXE = 2dcor.exe
	SS_EXE = ss2d.exe
	ASCII2CDF_EXE = a2cdf.exe
	CDF2ASCII_EXE = cdf2a.exe

else
	CORR_EXE = 2dcor
	SS_EXE = ss2d
	ASCII2CDF_EXE = a2cdf
	CDF2ASCII_EXE = cdf2a

endif


.PHONY: all clean demo

build:
	cd ooqp/extras/blas && ./configure && make
	cd ooqp/extras/ampl/solvers && make
	cd ooqp/extras/MA27 && make
#cd ooqp && ./configure && make all
	cd ooqp && make all
	cd cdf && make OS=$(OS) ENV=$(ENV) all && make OS=$(OS) ENV=$(ENV) install
	$(MAKE) make
	$(MAKE) cleano

# this runs a demo of the correspondance program, and stores the
# results in the directory which the shapes are located
demo:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(PWD)/cdf/lib && cd shapes/closed && ./correspond -v callosum1-10.cdf

make: clean comp
	$(CPP) -o $(CORR_EXE) $(CORR_OBJ) $(LIBS)
	$(CC) -o $(SS_EXE) $(SS_OBJ) -L cdf/lib -lcdf
	$(CC) -o $(ASCII2CDF_EXE) $(ASCII2CDF_OBJ) -L cdf/lib -lcdf
	$(CC) -o $(CDF2ASCII_EXE) $(CDF2ASCII_OBJ) -L cdf/lib -lcdf
	mkdir ./bin -p
	mv $(CORR_EXE) ./bin/$(CORR_EXE)
	mv $(SS_EXE) ./bin/$(SS_EXE)
	mv $(ASCII2CDF_EXE) ./bin/$(ASCII2CDF_EXE)
	mv $(CDF2ASCII_EXE) ./bin/$(CDF2ASCII_EXE)

comp: src/shape.h src/xfig_object.h
	$(CC) $(CFLAGS) $(SRC)

cleano:
	find . -name "*.o" -exec rm \{\} \;

clean:
	clear
	rm -fr /src/*.o
	rm -fr $(CORR_EXE) $(SS_EXE) $(ASCII2CDF_EXE) $(CDF2ASCII_EXE)
	rm -rf bin

uninstall: clean
	cd ooqp/extras/ampl/solvers && make clean
	cd ooqp/extras/MA27 && make clean
	cd ooqp/extras/blas && rm -fr *.a
	cd ooqp && make veryclean
	cd cdf && make clean
