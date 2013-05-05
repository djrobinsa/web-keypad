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

#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <syslog.h>
#include <string>
//#include <tr1/unordered_map>
#include <map>

// #define DEFAULT_CONFIG_FILE "/etc/dscd.conf"
#define DEFAULT_CONFIG_FILE "./dscd.conf"

class Config
{
  public:
    static Config &getConfig(std::string filename=DEFAULT_CONFIG_FILE);

    bool syncTime();
    int getBaud();
    std::string getDevice();
    short getPort();
    std::string getShell();

    std::string getZoneName(int zone);
    std::string getAccessCode(int partition);
    std::string getPartitionName(int partition);
    std::string getUserName(int user);
    std::string getKeyName(int key);
    std::string getKeypadName(int keypad);

    int getSyslogFacility();
    int getSyslogPriority(int command);

    std::string getEventAction(int command);

    bool readConfig(std::string fileame);

  protected:
    Config(std::string filename=DEFAULT_CONFIG_FILE);
    ~Config() {;}

    int commandNameToInt(std::string);
    std::string commandIntToName(int);

    std::string getValueByInt(std::string section, int tag);

  private:
    static Config *theConfig;
    std::string mConfigFile;
    /*
    std::tr1::unordered_map<std::string,
                            std::tr1::unordered_map<std::string,std::string> >
                            mDictionary;
    */
    std::map<std::string,
                            std::map<std::string,std::string> >
                            mDictionary;
};

#endif
