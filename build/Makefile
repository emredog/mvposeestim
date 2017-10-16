
CC := g++

EXEC := detect_fast_C

INC := $(shell pkg-config --cflags opencv)

LDFLAGS := $(shell pkg-config --libs opencv) -lpthread
NVCCLDFLAGS := $(LDFLAGS)

# parameters default values
FPTYPE := double
DEBUG := no
USE_CUDA := no
USE_NMS := no
MV := no
MV_PTC := no
SAVE_TEXT := yes
SAVE_IMAGES := no
AVS := no
HUMANEVA := yes
UMPM := no
VERBOSE := no

# check if MV and MV_PTC is given together (it should not)
ifeq ($(MV),yes)
	ifeq ($(MV_PTC),yes)
        $(error MV and MV_PTC flags cannot be used in conjunction)
	endif
endif

# check if HUMANEVA and UMPM is given together (it should not)
ifeq ($(HUMANEVA),yes)
	ifeq ($(UMPM),yes)
        $(error HUMANEVA and UMPM flags cannot be used in conjunction)
	endif
endif



# check if it tries to use WFV without multi-view
ifeq ($(MV),no)
	ifeq ($(MV_PTC),no)
        ifeq ($(AVS),yes)
        	$(error AVS is only allowed with either MV or MV_PTC)
        endif     
	endif
endif

# check if USE_NMS and WFV is given together (currently it is not supported)
ifeq ($(USE_NMS),yes)
	ifeq ($(AVS),yes)
        $(error USE_NMS and AVS flags cannot be used in conjunction, it is not supported yet.)
	endif
endif


ifeq ($(DEBUG),yes)
	# debug
	CFLAGS := -g -pg -Wall -DDEBUG -DFPTYPE=$(FPTYPE)
	NVCCFLAGS := -g -G -DUSE_CUDA -DDEBUG -DFPTYPE=$(FPTYPE) --compiler-options -pg --compiler-options -Wall
else
	# release
	CFLAGS := -O3 -Wall -DNDEBUG -DFPTYPE=$(FPTYPE)
	#CFLAGS := -O2 -Wall -DNDEBUG -DFPTYPE=$(FPTYPE)
	NVCCFLAGS := -O2 -DUSE_CUDA -DNDEBUG -DFPTYPE=$(FPTYPE) --compiler-options -Wall
endif

ifeq ($(USE_NMS),yes)
	# use non-maxima suppression
	CFLAGS += -DUSE_NMS
	NVCCFLAGS += -DUSE_NMS
endif

ifeq ($(MV),yes)
	# mV implementation
	CFLAGS += -DMV
	NVCCFLAGS += -DMV

	ifeq ($(SAVE_IMAGES),yes)
		# outputs as images
		CFLAGS += -DSAVE_IMAGES
		NVCCFLAGS += -DSAVE_IMAGES
	endif
endif

ifeq ($(MV_PTC),yes)
	# mV + part type compatibility implementation
	CFLAGS += -DMV_PTC
	NVCCFLAGS += -DMV_PTC

	ifeq ($(SAVE_IMAGES),yes)
		# outputs as images
		CFLAGS += -DSAVE_IMAGES
		NVCCFLAGS += -DSAVE_IMAGES
	endif
endif

ifeq ($(SAVE_TEXT),yes)
	# outputs as text
	CFLAGS += -DSAVE_TEXT
	NVCCFLAGS += -DSAVE_TEXT
endif

ifeq ($(AVS),yes)
	# uses Weights for Viewpoints
	CFLAGS += -DAVS
	NVCCFLAGS += -DAVS
endif

ifeq ($(UMPM),yes)
	# uses UMPM configured code
	CFLAGS += -DUMPM
	NVCCFLAGS += -DUMPM
endif

ifeq ($(HUMANEVA),yes)
	# uses HUMANEVA configured code
	CFLAGS += -DHUMANEVA
	NVCCFLAGS += -DHUMANEVA
endif

ifeq ($(VERBOSE),yes)
	CFLAGS += -DVERBOSE
	NVCCFLAGS += -DVERBOSE
endif


all: $(EXEC)

ifeq ($(USE_CUDA),yes)
%: ../src/%.cpp ../src/*.h ../src/*.hpp ../src/*.cpp ../src/*.cu
	@echo
	@echo "compiling with CUDA"
	@echo "compiling with FPTYPE=$(FPTYPE)"
	PATH=/usr/local/cuda/bin:$(PATH) LD_LIBRARY_PATH=/usr/local/cuda/lib64:$(LD_LIBRARY_PATH) nvcc -o $@ $(NVCCFLAGS) $(INC) ../src/*.cpp ../src/*.cu $(NVCCLDFLAGS)
else
%: ../src/%.cpp ../src/*.h ../src/*.hpp ../src/*.cpp
	@echo
	@echo "compiling with FPTYPE=$(FPTYPE)"
	$(CC) -o $@ $(CFLAGS) $(INC) ../src/*.cpp $(LDFLAGS)
endif

.PHONY: clean mrproper tags

clean:
	rm $(EXEC)

help:
	@echo
	@echo "make  -> build target in release mode"
	@echo "make DEBUG=yes  -> build target in debug mode"
	@echo "make test  -> to debug Makefile"
	@echo "make FPTYPE=float  -> use floats instead of doubles"
	@echo "make MV=yes  -> perform mV pose estimation"
	@echo "make MV=yes AVS=yes -> perform mV pose estimation with Adaptive Viewpoint Selection"
	@echo "make MV_PTC=yes  -> perform MultiView pose estimation with Part Type Compatibility"	
	@echo "make MV_PTC=yes AVS=yes -> perform MultiView pose estimation with Part Type Compatibility and Adaptive Viewpoint Selection"
	@echo "make MV=yes SAVE_TEXT=no  -> do not save results as text files"
	@echo "make MV=yes SAVE_IMAGES=yes  -> save results as text files AND images"
	@echo "make HUMANEVA=no UMPM=yes  -> code configured for UMPM dataset, instead of HUMANEVA"
	@echo

test:
	@echo CC=$(CC) 
	@echo EXEC=$(EXEC)
	@echo INC=$(INC) 
	@echo LDFLAGS=$(LDFLAGS) 
	@echo MV=$(MV)
	@echo MV_PTC=$(MV_PTC)	
	@echo AVS=$(AVS)
	@echo USE_NMS=$(USE_NMS)
	@echo USE_CUDA=$(USE_CUDA)
	@echo SAVE_TEXT=$(SAVE_TEXT)
	@echo SAVE_IMAGES=$(SAVE_IMAGES)
	@echo HUMANEVA=$(HUMANEVA)
	@echo UMPM=$(UMPM)

