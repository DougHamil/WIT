# Makefile for util (utility library used by CINCH and DTIQuery)
#

include ../Makefile.definitions

CC = g++

AR = ar csr $@
RM = rm -f

OUTPUT_DIR = ../lib

NIFTI_INCLUDE_PATH = -I../nifti

CFLAGS = -O -Wno-deprecated -DUSE_RAPID -I../ $(NIFTI_INCLUDE_PATH) $(TNT_INCLUDE_PATH) \
	$(RAPID_INCLUDE_PATH) $(SYS_INCLUDE_PATHS) $(VTK_INCLUDE_PATHS)

SRCS = DTIFilter.cpp DTIFilterCube.cpp DTIFilterVOI.cpp \
	DTIFilterPathwayScalar.cpp DTIPathDistanceMatrix.cpp \
	DTIPathwayIO.cpp DTIPathway.cpp DTIPathwayDatabase.cpp \
	DTIVolume.cpp DTIVolumeIO.cpp ScalarVolume.cpp DTIFilterTree.cpp \
	FunctionParser.cpp FunctionNodes.cpp DTIFilterAlgorithm.cpp \
	DTIPathwayFactory.cpp DTIPathwayInterface.cpp \
	DTIFilterSphere.cpp DTIFilterComplex.cpp DTIQueryProcessor.cpp \
	DTIPathTracer.cpp DTITENDPathTracer.cpp \
	DTISTTPathTracer.cpp DTIUniformSeedGenerator.cpp \
	DTIDeterministicPathTracer.cpp PathwayIntersectionVolume.cpp \
	DTIStats.cpp DTIMath.cpp DTIPathwaySelection.cpp \
	DTIPathwayAssignment.cpp \



OBJS = $(SRCS:%.cpp=%.o) 

TARGET = $(OUTPUT_DIR)/libdtiUtil.a

default: $(TARGET)

$(TARGET) : ${OBJS}
	${RM} $@
	${AR} ${OBJS}

.cpp.o: $*.cpp Makefile
	$(CC) $(CFLAGS) -c $*.cpp -o $*.o

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)

depend:
	makedepend -f- -- $(CFLAGS) -- $(SRCS) > Makefile.dependencies

ifeq ($(wildcard Makefile.dependencies),Makefile.dependencies)
include Makefile.dependencies
  else
# do nothing
  endif

