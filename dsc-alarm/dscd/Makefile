##############################################################################
#
#  Copyright (c) 2009-2010, Adam Roach
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#  
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##############################################################################

all:dscd

SRC := $(wildcard *.cpp)
DEPS := $(patsubst %.cpp, .%.d, $(SRC))
OBJS := $(patsubst %.cpp, %.o, $(SRC))
ASMS := $(patsubst %.cpp, %.s, $(SRC))

CPPFLAGS += -g

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPS)
endif

all: dscd 


dscd: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

.%.d: %.cpp
	@echo Generating dependencies for $*.o
	@$(CPP) $(CPPFLAGS) -MM $< -MT $*.o -MT .$*.d > $@

%.s: %.cpp
	$(CXX) $(CPPFLAGS) -fverbose-asm -S $<

.PHONY: clean osx-reload all 

clean:
	$(RM) *.a *.o .*.d *.udo *.sym *.s \
		$(GENERATED_SOURCES) $(GENERATED_HEADERS)
	$(MAKE) -C test clean

show.%:
	@echo $*=$($*)


osx-reload:
	launchctl unload /Library/LaunchDaemons/com.fluffyhome.dsc-alarm.plist
	cp dscd /usr/bin/.
	launchctl load /Library/LaunchDaemons/com.fluffyhome.dsc-alarm.plist
