#====================================================================================
#			The MIT License (MIT)
#
#			Copyright (c) 2011 Kapparock LLC
#
#			Permission is hereby granted, free of charge, to any person obtaining a copy
#			of this software and associated documentation files (the "Software"), to deal
#			in the Software without restriction, including without limitation the rights
#			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#			copies of the Software, and to permit persons to whom the Software is
#			furnished to do so, subject to the following conditions:
#
#			The above copyright notice and this permission notice shall be included in
#			all copies or substantial portions of the Software.
#
#			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#			THE SOFTWARE.
#====================================================================================
SRC:=${CURDIR}/
INCLUDE_DIR+=-I$(SRC) -I$(STAGING_DIR)
INCLUDE_DIR+=-I$(STAGING_DIR)/../toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/mipsel-openwrt-linux-uclibc/include/c++/4.8.3
LIBNAME:=kappaio-zll
SONAME:=$(LIBNAME).so.0
EXEC:=$(LIBNAME).so.0.0.0
LINKERNAME:=$(LIBNAME).so

UCLIBSPECIFIC:=-fno-builtin -nostdinc++ -DGCC_HASCLASSVISIBILITY -nodefaultlibs 
CXXFLAGS+=$(INCLUDE_DIR) -std=c++11 -D_TIMT_ -g 
CFLAGS+=$(INCLUDE_DIR) -D_TIMT_ -g -x c++ 
CPPFLAGS:= -fpermissive # --cflags` #-Wall  
LDFLAGS+= -L$(LIBDIR) -ljansson -lkapi -lstdc++ -lc

OBJ= hue_control.o zcl.o
	 	
CPPFLAGS:=-fPIC -pedantic 
LDFLAGS+=-shared

VPATH=$(SRC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -Wl,-soname,$(SONAME) -o $(EXEC)
	ln -s $(EXEC) $(LINKERNAME)
	ln -s $(EXEC) $(SONAME)
	
${CURDIR}/%.o: %.c

${CURDIR}/%.o: %.cpp
.PHONY : clean
clean:
	rm *.o *.so*
