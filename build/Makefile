
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
MULTIVIEW := no
MV_PTC := no
SAVE_TEXT := yes
SAVE_IMAGES := no
WFV := no
WFV_CNN := no
HUMANEVA := yes
UMPM := no
VERBOSE := no

# check if MULTIVIEW and MV_PTC is given together (it should not)
ifeq ($(MULTIVIEW),yes)
	ifeq ($(MV_PTC),yes)
        $(error MULTIVIEW and MV_PTC flags cannot be used in conjunction)
	endif
endif

# check if HUMANEVA and UMPM is given together (it should not)
ifeq ($(HUMANEVA),yes)
	ifeq ($(UMPM),yes)
        $(error HUMANEVA and UMPM flags cannot be used in conjunction)
	endif
endif

# check if WFV and WFV_CNN is given together (it should not)
ifeq ($(WFV),yes)
	ifeq ($(WFV_CNN),yes)
        $(error WFV and WFV_CNN flags cannot be used in conjunction)
	endif
endif

# check if it tries to use WFV without multi-view
ifeq ($(MULTIVIEW),no)
	ifeq ($(MV_PTC),no)
		ifeq ($(WFV),yes)
        	$(error WFV is only allowed with either MULTIVIEW or MV_PTC)
        endif
        ifeq ($(WFV_CNN),yes)
        	$(error WFV_CNN is only allowed with either MULTIVIEW or MV_PTC)
        endif     
	endif
endif

# check if USE_NMS and WFV is given together (currently it is not supported)
ifeq ($(USE_NMS),yes)
	ifeq ($(WFV),yes)
        $(error USE_NMS and WFV flags cannot be used in conjunction, it is not supported yet.)
	endif
	ifeq ($(WFV_CNN),yes)
        $(error USE_NMS and WFV_CNN flags cannot be used in conjunction, it is not supported yet.)
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

ifeq ($(MULTIVIEW),yes)
	# multiview implementation
	CFLAGS += -DMULTIVIEW
	NVCCFLAGS += -DMULTIVIEW

	ifeq ($(SAVE_IMAGES),yes)
		# outputs as images
		CFLAGS += -DSAVE_IMAGES
		NVCCFLAGS += -DSAVE_IMAGES
	endif
endif

ifeq ($(MV_PTC),yes)
	# multiview + part type compatibility implementation
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

ifeq ($(WFV),yes)
	# uses Weights for Viewpoints
	CFLAGS += -DWFV
	NVCCFLAGS += -DWFV
endif

ifeq ($(WFV_CNN),yes)
	# uses Weights for Viewpoints
	CFLAGS += -DWFV_CNN
	NVCCFLAGS += -DWFV_CNN
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
	@echo "make MULTIVIEW=yes  -> perform multiview pose estimation"
	@echo "make MULTIVIEW=yes WFV=yes -> perform multiview pose estimation with Weights for Viewpoints"
	@echo "make MULTIVIEW=yes WFV_CNN=yes -> perform multiview pose estimation with Weights for Viewpoints on ConvNets"
	@echo "make MV_PTC=yes  -> perform multiview pose estimation with Part Type Compatibility"
	@echo "make MV_PTC=yes WFV=yes -> perform multiview pose estimation with Part Type Compatibility and Weights for Viewpoints"
	@echo "make MV_PTC=yes WFV_CNN=yes -> perform multiview pose estimation with Part Type Compatibility and Weights for Viewpoints on ConvNets"
	@echo "make MULTIVIEW=yes SAVE_TEXT=no  -> do not save results as text files"
	@echo "make MULTIVIEW=yes SAVE_IMAGES=yes  -> save results as text files AND images"
	@echo "make HUMANEVA=no UMPM=yes  -> code configured for UMPM dataset, instead of HUMANEVA"
	@echo

test:
	@echo CC=$(CC) 
	@echo EXEC=$(EXEC)
	@echo INC=$(INC) 
	@echo LDFLAGS=$(LDFLAGS) 
	@echo MULTIVIEW=$(MULTIVIEW)
	@echo MV_PTC=$(MV_PTC)
	@echo WFV=$(WFV)
	@echo WFV_CNN=$(WFV_CNN)
	@echo USE_NMS=$(USE_NMS)
	@echo USE_CUDA=$(USE_CUDA)
	@echo SAVE_TEXT=$(SAVE_TEXT)
	@echo SAVE_IMAGES=$(SAVE_IMAGES)
	@echo HUMANEVA=$(HUMANEVA)
	@echo UMPM=$(UMPM)

