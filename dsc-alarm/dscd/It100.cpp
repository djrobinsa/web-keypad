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

#include "It100.h"
#include "Config.h"
#include "Command.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>
#include <iostream>
#include <ostream>
#include <queue>
#include <syslog.h>
#include <sstream>
#include <libgen.h>
#include <vector>


It100::It100() : mHasLabels(false), mKeypadEtag(1)
{
  strncpy(mDevice, Config::getConfig().getDevice().c_str(), sizeof(mDevice));
  mDescriptor = open(mDevice, O_RDWR);

  if (mDescriptor < 0)
  {
    perror(mDevice);
    exit(-1);
  }

  int baud;
  int baudInt = Config::getConfig().getBaud();
  switch (baudInt)
  {
    case 9600: baud = B9600; break;
    case 19200: baud = B19200; break;
    case 38400: baud = B38400; break;
    case 57600: baud = B57600; break;
    case 115200: baud = B115200; break;
    default: baud = B9600;
  }

  setFdBaud(baud);

  mCommandPending = 0;

  // Set up the keypad state
  memset(mLCD, ' ', 32);
  mLCD[32] = 0;
  memset(mLedState, sizeof(mLedState), OFF);
  for (int i = 0; i <= 64; i++) { mZoneOpen[i] = false; }
}

uint64_t
It100::getZoneStatus() const
{
  uint64_t status = 0;
  for (int i = 64; i > 0; i--)
  {
    status <<= 1;
    status |= (mZoneOpen[i]?1:0);
  }
  return status;
}

// TODO -- This is butt ugly, and should be made to cache stuff into
// a class attribute buffer so we can return immediately,
// even if a fully command has not arrived yet.
void
It100::processMessage()
{
  char buffer[80];
  char commandStr[4] = {0,0,0,0};
  char checksumStr[3] = {0,0,0};
  char *parameters = buffer+3;
  int length = 0;
  bool lineDone = false;

  // Read an entire line of output from the IT-100
  while (length < sizeof(buffer) && !lineDone)
  {
    int s = read(mDescriptor,buffer + length,1);
    if (s < 1)
    {
      return;
    }
    if (buffer[length] == '\n') 
    {
      buffer[length-1] = '\0';
      lineDone = true;
      length--;
    }
    else
    {
      length += s;
    }
  }

  // Parse the line into a command object; log it,
  // update our internal state, and execute any
  // applicable shell commands.
  {
    Command *c = Command::makeCommand(*this, buffer);
    if (c)
    {
      std::stringstream logMessage;
      logMessage << (*c);

      std::cout << ">>> " << logMessage.str() << std::endl;
      int priority = c->getSyslogPriority();
      if (priority != -1)
      {
        syslog(priority, "%s", logMessage.str().c_str());
      }

      c->processStateChange();

      std::string action = c->getShellAction();

      // We use a double-fork approach to avoid zombies
      if (action.length() > 0)
      {
        std::cout << "Executing command: " << action << std::endl;

        pid_t child = fork();
        if (!child)
        {
          pid_t grandchild = fork();
          if (grandchild) 
          { 
            if (grandchild == -1)
            {
              //syslog();
            }
            exit(0); 
          }
          std::string shell = Config::getConfig().getShell();

          char flag[] = {'-','c',0};

          char *const av[] = {(char *)(shell.c_str()),
                              flag,
                              (char *)(action.c_str()),
                              (char *)0};

          char *const ev[] = {(char *)0};
          execve("/bin/sh",av,ev);
          exit(-1);
        }
        else
        {
          if (child == -1)
          {
            //syslog();
          }
          waitpid(child,0,0);
        }

      }
      delete c;
    }
  }

}

void
It100::setFdBaud(int baud)
{
  // TODO -- make sure the IT-100 is set to the selected speed
  struct termios desc;
#if 0 
  // Cullen commented out this code and replaced it with the code in the #else
  // side. This code won't compile on OSX and the code I replaced with looks
  // right but I have not tested it on linux so I left this code here so people
  // could see how to fix it.
  ioctl(mDescriptor, TCGETA, &desc);
  desc.c_cflag &= ~CBAUD;
  desc.c_cflag |= baud;
  ioctl(mDescriptor, TCSETA, &desc);
#else
  tcgetattr( mDescriptor, &desc );
  cfsetspeed( &desc, baud );
  tcsetattr( mDescriptor, TCSANOW, &desc );
#endif 
}

// TODO -- Really, we should add constructors to the appropriate
// Command classes and use them to do things like create checksums
// for us. This is ugly because it predates the object-orientation
// of the command handling.

void 
It100::sendCommand(command_t cmd, const char *format, ...)
{
  char buffer[40];
  va_list parameters;
  int length;
  unsigned char checksum = 0;
  va_start(parameters, format);
  snprintf(buffer, sizeof(buffer), "%3.3d", cmd);
  length = vsnprintf(buffer+3, sizeof(buffer)-3, format, parameters) + 3;

  for (int i = 0; i < length; i++)
  {
    checksum += buffer[i];
  }
  length += snprintf(buffer+length, sizeof(buffer)-length,
                     "%2.2X", checksum);
  {
    Command *c = Command::makeCommand(*this, buffer);
    if (c)
    {
      std::cout << "<<< " << (*c) << std::endl;
      delete c;
    }
    else
    {
      std::cout << "<<< " << buffer << std::endl;
    }
  }
  length += snprintf(buffer+length, sizeof(buffer)-length, "\r\n");

  if (!mCommandPending)
  {
    write(mDescriptor, buffer, length);
    mCommandPending = time(0);
  }
  else
  {
    mPendingCommands.push(std::string(buffer));
  }
}

void
It100::setTimeAndDate(time_t time)
{
  struct tm t;
  localtime_r (&time,&t);
  sendCommand(SET_TIME_AND_DATE, "%02.2d%02.2d%02.2d%02.2d%02.2d",
    t.tm_hour, t.tm_min, t.tm_mon+1, t.tm_mday, t.tm_year - 100);
}

/* ***************************************************************************
  State updates that can happen as a result of command execution
*************************************************************************** */

// TODO -- We need to add timeout handling to queued commands.
// Otherwise, a failure to acknowledge a command will kill
// our ability to send any further commands. (I've seen
// this happen). Also, we should probably trigger this on
// command error to recover from bad checksum handling.

void
It100::sendPendingCommand()
{
  if (mPendingCommands.size())
  {
    std::string cmd = mPendingCommands.front();
    mPendingCommands.pop();
    write(mDescriptor, cmd.c_str(), cmd.length());
    mCommandPending = time(0);
  }
  else
  {
    mCommandPending = 0;
  }
}

void
It100::setZoneOpen(int zone, bool open)
{
  mZoneOpen[zone] = open;
}

void
It100::sendAccessCode(int partition, int codeLength)
{
  char code[7] = "000000";
  std::string cc = Config::getConfig().getAccessCode(partition);
  if (cc.length() <= 6)
  {
    memmove(code, cc.c_str(), cc.length());
  } 
  if (codeLength <= 6)
  code[codeLength] = 0;
  codeSend(partition,code);
}

void
It100::setLcdScreen(int line, int column, std::string str)
{
  column += (line * 16);
  if (column + str.length() <= 32)
  {
    memmove(mLCD + column, str.c_str(), str.length());
  }
  mKeypadEtag++;
}

void
It100::setLcdCursor(int type, int line, int column)
{
  mCursorType = static_cast<cursor_t>(type);
  mCursorLine = line;
  mCursorColumn = column;
  mKeypadEtag++;
}

void
It100::setLedState(int led, int state)
{
  mLedState[led] = static_cast<ledState_t>(state);
  mKeypadEtag++;
}

void
It100::setLabel(int num, std::string label)
{
  if (num <= 151)
  {
    mLabel[num] = label;
  }

  if (num == 151) { mHasLabels = true; }
}

std::string
It100::getZoneName(int zone)
{
  if (zone > 151) { return ""; }
  std::string override = Config::getConfig().getZoneName(zone);
  if (override.length() != 0)
  {
    return override;
  }
  return mLabel[zone];
}

std::string
It100::getPartitionName(int partition)
{
  if (partition > 8) { return ""; }
  std::string override = Config::getConfig().getPartitionName(partition);
  if (override.length() != 0)
  {
    return override;
  }
  return mLabel[100 + partition];
}

std::string
It100::getUserName(int user)
{
  if (user > 42) { return ""; }
  return Config::getConfig().getUserName(user);
}
