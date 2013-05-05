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

#include "Config.h"
#include "It100.h"
#include "CommandProcessor.h"

#include <iostream>
#include <termios.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <list>
#include <libgen.h>

int
main(int argc, char **argv)
{
   std::string configFile;
   if ( argc == 2 )
   {
      configFile = argv[1];
   }
   
  Config::Config &config = Config::getConfig( configFile );
  // TODO -- validate configuration

  //==================
  // Initialize syslog logging

  char processName[FILENAME_MAX];
  strncpy(processName, argv[0], FILENAME_MAX);
  char *temp = basename(processName);
  memmove(processName, temp, strlen(temp)+1);
  openlog(processName, 0, config.getSyslogFacility());

  //==================
  // Initialize the command socket
  // TODO -- Send errors to syslog

  int listenSocket;
  struct sockaddr_in localAddr;
  uint32_t s_addr;
  inet_pton(AF_INET, "127.0.0.1", &s_addr);
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons(config.getPort());
  localAddr.sin_addr.s_addr = s_addr;
  memset(localAddr.sin_zero, 0, sizeof(localAddr.sin_zero));

  listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int one = 1;
  setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
  if (listenSocket < 0) {perror("socket()"); return -1;}
  if (bind(listenSocket, (struct sockaddr*)&localAddr, sizeof(localAddr)))
    {perror("bind()"); return -1;}
  if (listen(listenSocket, 10)) {perror("listen()"); return -1;}

  //==================
  // Initialize the IT-100 board
  It100 it;
  it.timeStampControl(false);
  if (config.syncTime())
  {
    it.setTimeAndDate(time(0));
  }
  it.labelsRequest();
  it.virtualKeypadControl(true);
  it.timeDateBroadcastControl(true);
  
  bool statusRequested = false;

  //==================
  // Process incoming information
  std::list<CommandProcessor> cp;
  std::list<CommandProcessor>::iterator i;

  while(1)
  {
    fd_set set;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    int maxFd = listenSocket;
    if (it.getDescriptor() > maxFd) { maxFd = it.getDescriptor(); }
    FD_ZERO(&set);
    FD_SET(listenSocket, &set);
    FD_SET(it.getDescriptor(), &set);

    for (i = cp.begin(); i != cp.end(); i++)
    {
      FD_SET(i->getDescriptor(), &set);
      if (i->getDescriptor() > maxFd) { maxFd = i->getDescriptor(); }
    }

    select(maxFd + 1, &set, 0, 0, &timeout);

    // Inbound message from IT-100 board -- process it.
    if (FD_ISSET(it.getDescriptor(), &set))
    {
      it.processMessage();
    }

    // Check command channels for new commands
    for (i = cp.begin(); i != cp.end(); i++)
    {
      // These are non-blocking, and might be waiting on
      // something from the IT-100, so we always call
      // process.
      i->process();
    }

    // Remove "done" comand channels
    for (i = cp.begin(); i != cp.end(); i++)
    {
      if (i->isDone())
      {
        close (i->getDescriptor());
        cp.erase(i);
        i = cp.begin();
      }
    }

    // New connection to command socket -- accept it.
    if (FD_ISSET(listenSocket, &set))
    {
      int newSock;
      struct sockaddr_in remoteAddr;
      socklen_t addrSize = sizeof(remoteAddr);
      newSock = accept(listenSocket, (struct sockaddr*)&remoteAddr, &addrSize);
      cp.push_back(CommandProcessor(newSock,it));
    }

    // After the labels have been transferred, we ask for the
    // overall alarm panel status
    if (it.hasLabels() && !statusRequested)
    {
      it.statusRequest();
      statusRequested = true;
    }
  }

  return 0;
}
