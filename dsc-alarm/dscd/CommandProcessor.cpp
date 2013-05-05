/* ---------------------------------------------------------------------------

  Copyright (c) 2009-2010, Adam Roach
  All rights reserved.
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
  
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--------------------------------------------------------------------------- */

#include "CommandProcessor.h"
#include "It100.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h> 
#include <iostream>

CommandProcessor::CommandProcessor(int descriptor, It100 &it100)
  : mDescriptor(descriptor), mIt100(it100), mBufferSize(0), mDone(false),
    mWaitingEtag(0), mWaitForStateChange(0)
{
  char one=1;
  ioctl(mDescriptor, FIONBIO, (char *)&one);
}

CommandProcessor::~CommandProcessor()
{
}

void
CommandProcessor::process()
{
  if (mDone) { return; }

  if (mWaitForStateChange && mWaitingEtag != mIt100.getKeypadEtag())
  {
    mWaitForStateChange = false;
    sendKeypadStatus();
  }

  char c;
  int bytesRead;
  while ((bytesRead = read(mDescriptor, &c, 1)) == 1)
  {
    if (c == '\n')
    {
      mBuffer[mBufferSize] = 0;
      processBuffer();
    }
    else if (c != '\r')
    {
      mBuffer[mBufferSize++] = c;
    }

    if (mBufferSize >= sizeof(mBuffer))
    {
      // TODO -- we should syslog this condition
      mDone = true;
      return;
    }
  }

  if (bytesRead == 0)
  {
    // remote end closed socket
    // TODO -- should syslog this condition
    mDone = true;
    return;
  }

  if (errno != EWOULDBLOCK)
  {
    // TODO -- we should syslog this condition
    perror("read()");
    mDone = true;
    return;
  }
}

void
CommandProcessor::processBuffer()
{
  if (mBufferSize == 1)
  {
    switch(mBuffer[0])
    {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
      case '*': case '#': case 'F': case 'A': case 'P':
      case 'a': case 'b': case 'c': case 'd': case 'e':
      case '<': case '>': case '=': case '^':
        mIt100.keyPressed(mBuffer[0]);
        write(mDescriptor, mBuffer, 1);
        write(mDescriptor, "\n", 1);
        break;

      case '?':
        sendKeypadStatus();
        break;

      case 'q':
        mDone = true;
        break;
    }
  }
  else if (mBuffer[0] == '?')
  {
    write(mDescriptor, mBuffer, mBufferSize);
    mWaitingEtag = strtol(mBuffer+1,0,10);
    write(mDescriptor, "\n", 1);
    mWaitForStateChange = true;
  }
  mBufferSize = 0;
}

void
CommandProcessor::sendKeypadStatus()
{
  char buffer[256];
  int buflen =
    snprintf(buffer, sizeof(buffer)-1, "[%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
                                       "'%s','%s',%d,%d,%d,"
                                       "%d,%d,%d,%d,%d,%d]\n",
            mIt100.getKeypadEtag(),
            mIt100.getLedState(It100::READY),
            mIt100.getLedState(It100::ARMED),
            mIt100.getLedState(It100::MEMORY),
            mIt100.getLedState(It100::BYPASS),
            mIt100.getLedState(It100::TROUBLE),
            mIt100.getLedState(It100::PROGRAM),
            mIt100.getLedState(It100::FIRE),
            mIt100.getLedState(It100::BACKLIGHT),
            mIt100.getLedState(It100::AC),
            std::string(mIt100.getLcd(),16).c_str(),
            mIt100.getLcd()+16,
            mIt100.getCursorType(),
            mIt100.getCursorLine(),
            mIt100.getCursorColumn(),
            // TODO -- Add the audio stuff in here
            0,0,0,0,0,0
            );
  write(mDescriptor, buffer, buflen);
}
