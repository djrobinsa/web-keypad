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

#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


Config *Config::theConfig = 0;

Config &Config::getConfig(std::string filename)
{
   if ( filename.empty() )
   {
      filename = DEFAULT_CONFIG_FILE;
   }
   
  if (!theConfig)
  {
    theConfig = new Config(filename);
  }
  return *theConfig;
}

Config::Config (std::string filename)
{
   if ( filename.empty() )
   {
      filename = DEFAULT_CONFIG_FILE;
   }
  readConfig(filename);
}

class ConfigParser
{
  public:
    typedef enum
    {
      INIT,
      IN_COMMENT,
      IN_SECTION,
      IN_TAG,
      PRE_VALUE,
      IN_VALUE
    } state_t;

    typedef enum { NONE, SECTION, TAG, VALUE, ERROR } ready_t;

    ConfigParser() : mState(INIT), mLength(0), mLine(1), mLinePending(false) {;}

    ready_t parse(char c)
    {
      ready_t status = NONE;

      if (mLength >= sizeof(mBuffer))
      {
        setError("String too long");
        return ERROR;
      }

      if (mLinePending) { mLine++; mLinePending = false; }
      if (c == '\n') { mLinePending = true; }

      switch (mState)
      {
        case INIT:
          mLength = 0;
          switch(c)
          {
            case '[':
              setState( IN_SECTION );
              break;
            case '#':
              setState( IN_COMMENT );
              break;
            case ' ': case '\t': case '\r': case '\n':
              break;
            default:
              mBuffer[mLength++] = c;
              setState( IN_TAG );
          }
          break;

        case IN_COMMENT:
          if (c == '\r' || c == '\n') {setState( INIT );}
          break;

        case IN_SECTION:
          switch(c)
          {
            case '[': case '\r': case '\n':
              setError("Premature end of section tag");
              status = ERROR;
              break;
            case ']':
              setState( INIT );
              status =  SECTION;
              break;
            default:
              mBuffer[mLength++] = c;
          }
          break;

        case IN_TAG:
          switch(c)
          {
            case '\r': case '\n':
              setError("No value specified");
              status = ERROR;
            case ' ':
              break;
            case '=':
              setState( PRE_VALUE );
              status = TAG;
              break;
            default:
              mBuffer[mLength++] = c;
          }
          break;

        case PRE_VALUE:
          mLength = 0;
          switch(c)
          {
            case '\r': case '\n':
              setState( INIT );
              status = VALUE;
              break;
            case ' ':
              break;
            default:
              setState( IN_VALUE );
              mBuffer[mLength++] = c;
          }
          break;

        case IN_VALUE:
          switch(c)
          {
            case '\r': case '\n':
              setState( INIT );
              status = VALUE;
              break;
            default:
              mBuffer[mLength++] = c;
          }
          break;
      }

      return status;
    }

    const char *getValue()
    {
      mBuffer[mLength]=0;
      return mBuffer;
    }

    int getLine () { return mLine; }

  protected:
    void setError(const char *error)
    {
      mLength = sprintf(mBuffer, "%s on line %d", error, mLine);
    }

    void setState (state_t s)
    {
      /*
      std::cout << "State = ";
      switch(s)
      {
        case INIT : std::cout << "INIT"; break;
        case IN_COMMENT : std::cout << "IN_COMMENT"; break;
        case IN_SECTION : std::cout << "IN_SECTION"; break;
        case IN_TAG : std::cout << "IN_TAG"; break;
        case PRE_VALUE : std::cout << "PRE_VALUE"; break;
        case IN_VALUE : std::cout << "IN_VALUE"; break;
      }
      std::cout << std::endl;
      */
      mState = s;
    }

  private:
    state_t mState;
    char mBuffer[256];
    size_t mLength;
    int mLine;
    bool mLinePending;
};

bool
Config::syncTime()
{
  std::string flag = mDictionary["main"]["sync_time"];
  return ((flag=="true")?true:false);
}

int
Config::getBaud()
{
  std::string baud = mDictionary["main"]["baud"];
  return strtol(baud.c_str(),0,10);
}

std::string
Config::getDevice()
{
  return mDictionary["main"]["device"];
}

short
Config::getPort()
{
  std::string port = mDictionary["main"]["port"];
  return strtol(port.c_str(),0,10);
}

int
Config::getSyslogFacility()
{
  std::string facility = mDictionary["syslog"]["facility"];
  if (facility == "LOCAL0") {return LOG_LOCAL0;}
  if (facility == "LOCAL1") {return LOG_LOCAL1;}
  if (facility == "LOCAL2") {return LOG_LOCAL2;}
  if (facility == "LOCAL3") {return LOG_LOCAL3;}
  if (facility == "LOCAL4") {return LOG_LOCAL4;}
  if (facility == "LOCAL5") {return LOG_LOCAL5;}
  if (facility == "LOCAL6") {return LOG_LOCAL6;}
  if (facility == "LOCAL7") {return LOG_LOCAL7;}
  if (facility == "AUTHPRIV") {return LOG_AUTHPRIV;}
  if (facility == "CRON") {return LOG_CRON;}
  if (facility == "DAEMON") {return LOG_DAEMON;}
  if (facility == "FTP") {return LOG_FTP;}
  if (facility == "KERN") {return LOG_KERN;}
  if (facility == "LPR") {return LOG_LPR;}
  if (facility == "MAIL") {return LOG_MAIL;}
  if (facility == "NEWS") {return LOG_NEWS;}
  if (facility == "SYSLOG") {return LOG_SYSLOG;}
  if (facility == "USER") {return LOG_USER;}
  if (facility == "UUCP") {return LOG_UUCP;}
  return LOG_LOCAL0;
}

int
Config::getSyslogPriority(int command)
{
  std::string level = mDictionary["syslog"][commandIntToName(command)];
  if (level == "EMERG") { return LOG_EMERG; }
  if (level == "ALERT") { return LOG_ALERT; }
  if (level == "CRIT") { return LOG_CRIT; }
  if (level == "ERR") { return LOG_ERR; }
  if (level == "WARNING") { return LOG_WARNING; }
  if (level == "NOTICE") { return LOG_NOTICE; }
  if (level == "INFO") { return LOG_INFO; }
  if (level == "DEBUG") { return LOG_DEBUG; }
  if (level == "NONE") { return -1; }
  return -1;
}

std::string
Config::getShell()
{
  return mDictionary["main"]["shell"];
}

std::string
Config::getEventAction(int command)
{
  return mDictionary["actions"][commandIntToName(command)];
}

bool
Config::readConfig(std::string filename)
{
  int descriptor = open (filename.c_str(), O_RDONLY);
  if (descriptor < 0)
  {
    std::cerr << "Could not open config file: " << filename << std::endl;
    return false;
  }

  mConfigFile = filename;
 
  mDictionary.clear();

  char buffer[256];
  int buflen;
  int i;
  ConfigParser p;

  std::string section, tag;

  while ((buflen = read(descriptor, buffer, sizeof(buffer))) > 0)
  {
    for (i = 0; i < buflen; i++)
    {
      switch (p.parse(buffer[i]))
      {
        case ConfigParser::SECTION:
          section = p.getValue();
          break;

        case ConfigParser::TAG:
          tag = p.getValue();
          break;

        case ConfigParser::VALUE:
          mDictionary[section][tag] = p.getValue();
          break;

        case ConfigParser::ERROR:
          std::cerr << p.getValue() << std::endl;
          return false;
      }
    }
  }

  return true;
}

std::string
Config::getZoneName(int zone)
{
  return getValueByInt("zones",zone);
}

std::string
Config::getAccessCode(int partition)
{
  std::string retval = getValueByInt("access",partition);
  if (retval.length()) { return retval; }
  return mDictionary["access"]["default"];
}

std::string
Config::getPartitionName(int partition)
{
  return getValueByInt("partitions",partition);
}

std::string
Config::getUserName(int user)
{
  return getValueByInt("users",user);
}

std::string
Config::getKeyName(int user)
{
  return getValueByInt("wireless_key",user);
}

std::string
Config::getKeypadName(int user)
{
  return getValueByInt("wireless_keypad",user);
}

std::string
Config::getValueByInt(std::string section, int tag)
{
  char tagname[16];
  sprintf(tagname, "%d", tag);
  return mDictionary[section][tagname];
}

int
Config::commandNameToInt(std::string cmd)
{
  if (cmd == "POLL")
    { return It100::POLL; }
  if (cmd == "STATUS_REQUEST")
    { return It100::STATUS_REQUEST; }
  if (cmd == "LABELS_REQUEST")
    { return It100::LABELS_REQUEST; }
  if (cmd == "SET_TIME_AND_DATE")
    { return It100::SET_TIME_AND_DATE; }
  if (cmd == "COMMAND_OUTPUT_CONTROL")
    { return It100::COMMAND_OUTPUT_CONTROL; }
  if (cmd == "PARTITION_ARM_CONTROL_AWAY")
    { return It100::PARTITION_ARM_CONTROL_AWAY; }
  if (cmd == "PARTITION_ARM_CONTROL_STAY")
    { return It100::PARTITION_ARM_CONTROL_STAY; }
  if (cmd == "PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY")
    { return It100::PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY; }
  if (cmd == "PARTITION_ARM_CONTROL_WITH_CODE")
    { return It100::PARTITION_ARM_CONTROL_WITH_CODE; }
  if (cmd == "PARTITION_DISARM_CONTROL_WITH_CODE")
    { return It100::PARTITION_DISARM_CONTROL_WITH_CODE; }
  if (cmd == "TIME_STAMP_CONTROL")
    { return It100::TIME_STAMP_CONTROL; }
  if (cmd == "TIME_DATE_BROADCAST_CONTROL")
    { return It100::TIME_DATE_BROADCAST_CONTROL; }
  if (cmd == "TEMPERATURE_BROADCAST_CONTROL")
    { return It100::TEMPERATURE_BROADCAST_CONTROL; }
  if (cmd == "VIRTUAL_KEYPAD_CONTROL")
    { return It100::VIRTUAL_KEYPAD_CONTROL; }
  if (cmd == "TRIGGER_PANIC_ALARM")
    { return It100::TRIGGER_PANIC_ALARM; }
  if (cmd == "KEY_PRESSED")
    { return It100::KEY_PRESSED; }
  if (cmd == "BAUD_RATE_CHANGE")
    { return It100::BAUD_RATE_CHANGE; }
  if (cmd == "GET_TEMPERATURE_SET_POINT")
    { return It100::GET_TEMPERATURE_SET_POINT; }
  if (cmd == "TEMPERATURE_CHANGE")
    { return It100::TEMPERATURE_CHANGE; }
  if (cmd == "SAVE_TEMPERATURE_SETTING")
    { return It100::SAVE_TEMPERATURE_SETTING; }
  if (cmd == "CODE_SEND")
    { return It100::CODE_SEND; }
  if (cmd == "COMMAND_ACKNOWLEDGE")
    { return It100::COMMAND_ACKNOWLEDGE; }
  if (cmd == "COMMAND_ERROR")
    { return It100::COMMAND_ERROR; }
  if (cmd == "SYSTEM_ERROR")
    { return It100::SYSTEM_ERROR; }
  if (cmd == "TIME_DATE_BROADCAST")
    { return It100::TIME_DATE_BROADCAST; }
  if (cmd == "RING_DETECTED")
    { return It100::RING_DETECTED; }
  if (cmd == "INDOOR_TEMPERATURE_BROADCAST")
    { return It100::INDOOR_TEMPERATURE_BROADCAST; }
  if (cmd == "OUTDOOR_TEMPERATRURE_BROADCAST")
    { return It100::OUTDOOR_TEMPERATRURE_BROADCAST; }
  if (cmd == "THERMOSTAT_SET_POINTS")
    { return It100::THERMOSTAT_SET_POINTS; }
  if (cmd == "BROADCAST_LABELS")
    { return It100::BROADCAST_LABELS; }
  if (cmd == "BAUD_RATE_SET")
    { return It100::BAUD_RATE_SET; }
  if (cmd == "ZONE_ALARM")
    { return It100::ZONE_ALARM; }
  if (cmd == "ZONE_ALARM_RESTORE")
    { return It100::ZONE_ALARM_RESTORE; }
  if (cmd == "ZONE_TAMPER")
    { return It100::ZONE_TAMPER; }
  if (cmd == "ZONE_TAMPER_RESTORE")
    { return It100::ZONE_TAMPER_RESTORE; }
  if (cmd == "ZONE_FAULT")
    { return It100::ZONE_FAULT; }
  if (cmd == "ZONE_FAULT_RESTORE")
    { return It100::ZONE_FAULT_RESTORE; }
  if (cmd == "ZONE_OPEN")
    { return It100::ZONE_OPEN; }
  if (cmd == "ZONE_RESTORED")
    { return It100::ZONE_RESTORED; }
  if (cmd == "DURESS_ALARM")
    { return It100::DURESS_ALARM; }
  if (cmd == "F_KEY_ALARM")
    { return It100::F_KEY_ALARM; }
  if (cmd == "F_KEY_RESTORAL")
    { return It100::F_KEY_RESTORAL; }
  if (cmd == "A_KEY_ALARM")
    { return It100::A_KEY_ALARM; }
  if (cmd == "A_KEY_RESTORAL")
    { return It100::A_KEY_RESTORAL; }
  if (cmd == "P_KEY_ALARM")
    { return It100::P_KEY_ALARM; }
  if (cmd == "P_KEY_RESTORAL")
    { return It100::P_KEY_RESTORAL; }
  if (cmd == "AUXILIARY_INPUT_ALARM")
    { return It100::AUXILIARY_INPUT_ALARM; }
  if (cmd == "AUXILIARY_INPUT_ALARM_RESTORED")
    { return It100::AUXILIARY_INPUT_ALARM_RESTORED; }
  if (cmd == "PARTITION_READY")
    { return It100::PARTITION_READY; }
  if (cmd == "PARTITION_NOT_READY")
    { return It100::PARTITION_NOT_READY; }
  if (cmd == "PARTITION_ARMED_DESCRIPTIVE_MODE")
    { return It100::PARTITION_ARMED_DESCRIPTIVE_MODE; }
  if (cmd == "PARTITION_IN_READY_TO_FORCE_ARM")
    { return It100::PARTITION_IN_READY_TO_FORCE_ARM; }
  if (cmd == "PARTITION_IN_ALARM")
    { return It100::PARTITION_IN_ALARM; }
  if (cmd == "PARTITION_DISARMED")
    { return It100::PARTITION_DISARMED; }
  if (cmd == "EXIT_DELAY_IN_PROGRESS")
    { return It100::EXIT_DELAY_IN_PROGRESS; }
  if (cmd == "ENTRY_DELAY_IN_PROGRESS")
    { return It100::ENTRY_DELAY_IN_PROGRESS; }
  if (cmd == "KEYPAD_LOCKOUT")
    { return It100::KEYPAD_LOCKOUT; }
  if (cmd == "KEYPAD_BLANKING")
    { return It100::KEYPAD_BLANKING; }
  if (cmd == "COMMAND_OUTPUT_IN_PROGRESS")
    { return It100::COMMAND_OUTPUT_IN_PROGRESS; }
  if (cmd == "INVALID_ACCESS_CODE")
    { return It100::INVALID_ACCESS_CODE; }
  if (cmd == "FUNCTION_NOT_AVAILABLE")
    { return It100::FUNCTION_NOT_AVAILABLE; }
  if (cmd == "FAIL_TO_ARM")
    { return It100::FAIL_TO_ARM; }
  if (cmd == "PARTITION_BUSY")
    { return It100::PARTITION_BUSY; }
  if (cmd == "USER_CLOSING")
    { return It100::USER_CLOSING; }
  if (cmd == "SPECIAL_CLOSING")
    { return It100::SPECIAL_CLOSING; }
  if (cmd == "PARTIAL_CLOSING")
    { return It100::PARTIAL_CLOSING; }
  if (cmd == "USER_OPENING")
    { return It100::USER_OPENING; }
  if (cmd == "SPECIAL_OPENING")
    { return It100::SPECIAL_OPENING; }
  if (cmd == "PANEL_BATTERY_TROUBLE")
    { return It100::PANEL_BATTERY_TROUBLE; }
  if (cmd == "PANEL_BATTERY_TROUBLE_RESTORE")
    { return It100::PANEL_BATTERY_TROUBLE_RESTORE; }
  if (cmd == "PANEL_AC_TROUBLE")
    { return It100::PANEL_AC_TROUBLE; }
  if (cmd == "PANEL_AC_RESTORE")
    { return It100::PANEL_AC_RESTORE; }
  if (cmd == "SYSTEM_BELL_TROUBLE")
    { return It100::SYSTEM_BELL_TROUBLE; }
  if (cmd == "SYSTEM_BELL_TROUBLE_RESTORAL")
    { return It100::SYSTEM_BELL_TROUBLE_RESTORAL; }
  if (cmd == "TLM_LINE_1_TROUBLE")
    { return It100::TLM_LINE_1_TROUBLE; }
  if (cmd == "TLM_LINE_1_TROUBLE_RESTORED")
    { return It100::TLM_LINE_1_TROUBLE_RESTORED; }
  if (cmd == "TLM_LINE_2_TROUBLE")
    { return It100::TLM_LINE_2_TROUBLE; }
  if (cmd == "TLM_LINE_2_TROUBLE_RESTORED")
    { return It100::TLM_LINE_2_TROUBLE_RESTORED; }
  if (cmd == "FTC_TROUBLE")
    { return It100::FTC_TROUBLE; }
  if (cmd == "BUFFER_NEAR_FULL")
    { return It100::BUFFER_NEAR_FULL; }
  if (cmd == "GENERAL_DEVICE_LOW_BATTERY")
    { return It100::GENERAL_DEVICE_LOW_BATTERY; }
  if (cmd == "GENERAL_DEVICE_LOW_BATTERY_RESTORE")
    { return It100::GENERAL_DEVICE_LOW_BATTERY_RESTORE; }
  if (cmd == "WIRELESS_KEY_LOW_BATTERY_TROUBLE")
    { return It100::WIRELESS_KEY_LOW_BATTERY_TROUBLE; }
  if (cmd == "RESTORE")
    { return It100::RESTORE; }
  if (cmd == "HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE")
    { return It100::HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE; }
  if (cmd == "RESTORED")
    { return It100::RESTORED; }
  if (cmd == "GENERAL_SYSTEM_TAMPER")
    { return It100::GENERAL_SYSTEM_TAMPER; }
  if (cmd == "GENERAL_SYSTEM_TAMPER_RESTORE")
    { return It100::GENERAL_SYSTEM_TAMPER_RESTORE; }
  if (cmd == "HOME_AUTOMATION_TROUBLE")
    { return It100::HOME_AUTOMATION_TROUBLE; }
  if (cmd == "HOME_AUTOMATION_TROUBLE_RESTORE")
    { return It100::HOME_AUTOMATION_TROUBLE_RESTORE; }
  if (cmd == "TROUBLE_STATUS_LED_ON")
    { return It100::TROUBLE_STATUS_LED_ON; }
  if (cmd == "TROUBLE_STATUS_RESTORE_LED_OFF")
    { return It100::TROUBLE_STATUS_RESTORE_LED_OFF; }
  if (cmd == "FIRE_TROUBLE_ALARM")
    { return It100::FIRE_TROUBLE_ALARM; }
  if (cmd == "FIRE_TROUBLE_ALARM_RESTORED")
    { return It100::FIRE_TROUBLE_ALARM_RESTORED; }
  if (cmd == "CODE_REQUIRED")
    { return It100::CODE_REQUIRED; }
  if (cmd == "LCD_UPDATE")
    { return It100::LCD_UPDATE; }
  if (cmd == "LCD_CURSOR")
    { return It100::LCD_CURSOR; }
  if (cmd == "LED_STATUS")
    { return It100::LED_STATUS; }
  if (cmd == "BEEP_STATUS")
    { return It100::BEEP_STATUS; }
  if (cmd == "TONE_STATUS")
    { return It100::TONE_STATUS; }
  if (cmd == "BUZZER_STATUS")
    { return It100::BUZZER_STATUS; }
  if (cmd == "DOOR_CHIME_STATUS")
    { return It100::DOOR_CHIME_STATUS; }
  if (cmd == "SOFTWARE_VERSION")
    { return It100::SOFTWARE_VERSION; }
}

std::string
Config::commandIntToName(int cmd)
{
  switch (cmd)
  {
    case It100::POLL: 
      return "POLL";
    case It100::STATUS_REQUEST: 
      return "STATUS_REQUEST";
    case It100::LABELS_REQUEST: 
      return "LABELS_REQUEST";
    case It100::SET_TIME_AND_DATE: 
      return "SET_TIME_AND_DATE";
    case It100::COMMAND_OUTPUT_CONTROL: 
      return "COMMAND_OUTPUT_CONTROL";
    case It100::PARTITION_ARM_CONTROL_AWAY: 
      return "PARTITION_ARM_CONTROL_AWAY";
    case It100::PARTITION_ARM_CONTROL_STAY: 
      return "PARTITION_ARM_CONTROL_STAY";
    case It100::PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY: 
      return "PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY";
    case It100::PARTITION_ARM_CONTROL_WITH_CODE: 
      return "PARTITION_ARM_CONTROL_WITH_CODE";
    case It100::PARTITION_DISARM_CONTROL_WITH_CODE: 
      return "PARTITION_DISARM_CONTROL_WITH_CODE";
    case It100::TIME_STAMP_CONTROL: 
      return "TIME_STAMP_CONTROL";
    case It100::TIME_DATE_BROADCAST_CONTROL: 
      return "TIME_DATE_BROADCAST_CONTROL";
    case It100::TEMPERATURE_BROADCAST_CONTROL: 
      return "TEMPERATURE_BROADCAST_CONTROL";
    case It100::VIRTUAL_KEYPAD_CONTROL: 
      return "VIRTUAL_KEYPAD_CONTROL";
    case It100::TRIGGER_PANIC_ALARM: 
      return "TRIGGER_PANIC_ALARM";
    case It100::KEY_PRESSED: 
      return "KEY_PRESSED";
    case It100::BAUD_RATE_CHANGE: 
      return "BAUD_RATE_CHANGE";
    case It100::GET_TEMPERATURE_SET_POINT: 
      return "GET_TEMPERATURE_SET_POINT";
    case It100::TEMPERATURE_CHANGE: 
      return "TEMPERATURE_CHANGE";
    case It100::SAVE_TEMPERATURE_SETTING: 
      return "SAVE_TEMPERATURE_SETTING";
    case It100::CODE_SEND: 
      return "CODE_SEND";
    case It100::COMMAND_ACKNOWLEDGE: 
      return "COMMAND_ACKNOWLEDGE";
    case It100::COMMAND_ERROR: 
      return "COMMAND_ERROR";
    case It100::SYSTEM_ERROR: 
      return "SYSTEM_ERROR";
    case It100::TIME_DATE_BROADCAST: 
      return "TIME_DATE_BROADCAST";
    case It100::RING_DETECTED: 
      return "RING_DETECTED";
    case It100::INDOOR_TEMPERATURE_BROADCAST: 
      return "INDOOR_TEMPERATURE_BROADCAST";
    case It100::OUTDOOR_TEMPERATRURE_BROADCAST: 
      return "OUTDOOR_TEMPERATRURE_BROADCAST";
    case It100::THERMOSTAT_SET_POINTS: 
      return "THERMOSTAT_SET_POINTS";
    case It100::BROADCAST_LABELS: 
      return "BROADCAST_LABELS";
    case It100::BAUD_RATE_SET: 
      return "BAUD_RATE_SET";
    case It100::ZONE_ALARM: 
      return "ZONE_ALARM";
    case It100::ZONE_ALARM_RESTORE: 
      return "ZONE_ALARM_RESTORE";
    case It100::ZONE_TAMPER: 
      return "ZONE_TAMPER";
    case It100::ZONE_TAMPER_RESTORE: 
      return "ZONE_TAMPER_RESTORE";
    case It100::ZONE_FAULT: 
      return "ZONE_FAULT";
    case It100::ZONE_FAULT_RESTORE: 
      return "ZONE_FAULT_RESTORE";
    case It100::ZONE_OPEN: 
      return "ZONE_OPEN";
    case It100::ZONE_RESTORED: 
      return "ZONE_RESTORED";
    case It100::DURESS_ALARM: 
      return "DURESS_ALARM";
    case It100::F_KEY_ALARM: 
      return "F_KEY_ALARM";
    case It100::F_KEY_RESTORAL: 
      return "F_KEY_RESTORAL";
    case It100::A_KEY_ALARM: 
      return "A_KEY_ALARM";
    case It100::A_KEY_RESTORAL: 
      return "A_KEY_RESTORAL";
    case It100::P_KEY_ALARM: 
      return "P_KEY_ALARM";
    case It100::P_KEY_RESTORAL: 
      return "P_KEY_RESTORAL";
    case It100::AUXILIARY_INPUT_ALARM: 
      return "AUXILIARY_INPUT_ALARM";
    case It100::AUXILIARY_INPUT_ALARM_RESTORED: 
      return "AUXILIARY_INPUT_ALARM_RESTORED";
    case It100::PARTITION_READY: 
      return "PARTITION_READY";
    case It100::PARTITION_NOT_READY: 
      return "PARTITION_NOT_READY";
    case It100::PARTITION_ARMED_DESCRIPTIVE_MODE: 
      return "PARTITION_ARMED_DESCRIPTIVE_MODE";
    case It100::PARTITION_IN_READY_TO_FORCE_ARM: 
      return "PARTITION_IN_READY_TO_FORCE_ARM";
    case It100::PARTITION_IN_ALARM: 
      return "PARTITION_IN_ALARM";
    case It100::PARTITION_DISARMED: 
      return "PARTITION_DISARMED";
    case It100::EXIT_DELAY_IN_PROGRESS: 
      return "EXIT_DELAY_IN_PROGRESS";
    case It100::ENTRY_DELAY_IN_PROGRESS: 
      return "ENTRY_DELAY_IN_PROGRESS";
    case It100::KEYPAD_LOCKOUT: 
      return "KEYPAD_LOCKOUT";
    case It100::KEYPAD_BLANKING: 
      return "KEYPAD_BLANKING";
    case It100::COMMAND_OUTPUT_IN_PROGRESS: 
      return "COMMAND_OUTPUT_IN_PROGRESS";
    case It100::INVALID_ACCESS_CODE: 
      return "INVALID_ACCESS_CODE";
    case It100::FUNCTION_NOT_AVAILABLE: 
      return "FUNCTION_NOT_AVAILABLE";
    case It100::FAIL_TO_ARM: 
      return "FAIL_TO_ARM";
    case It100::PARTITION_BUSY: 
      return "PARTITION_BUSY";
    case It100::USER_CLOSING: 
      return "USER_CLOSING";
    case It100::SPECIAL_CLOSING: 
      return "SPECIAL_CLOSING";
    case It100::PARTIAL_CLOSING: 
      return "PARTIAL_CLOSING";
    case It100::USER_OPENING: 
      return "USER_OPENING";
    case It100::SPECIAL_OPENING: 
      return "SPECIAL_OPENING";
    case It100::PANEL_BATTERY_TROUBLE: 
      return "PANEL_BATTERY_TROUBLE";
    case It100::PANEL_BATTERY_TROUBLE_RESTORE: 
      return "PANEL_BATTERY_TROUBLE_RESTORE";
    case It100::PANEL_AC_TROUBLE: 
      return "PANEL_AC_TROUBLE";
    case It100::PANEL_AC_RESTORE: 
      return "PANEL_AC_RESTORE";
    case It100::SYSTEM_BELL_TROUBLE: 
      return "SYSTEM_BELL_TROUBLE";
    case It100::SYSTEM_BELL_TROUBLE_RESTORAL: 
      return "SYSTEM_BELL_TROUBLE_RESTORAL";
    case It100::TLM_LINE_1_TROUBLE: 
      return "TLM_LINE_1_TROUBLE";
    case It100::TLM_LINE_1_TROUBLE_RESTORED: 
      return "TLM_LINE_1_TROUBLE_RESTORED";
    case It100::TLM_LINE_2_TROUBLE: 
      return "TLM_LINE_2_TROUBLE";
    case It100::TLM_LINE_2_TROUBLE_RESTORED: 
      return "TLM_LINE_2_TROUBLE_RESTORED";
    case It100::FTC_TROUBLE: 
      return "FTC_TROUBLE";
    case It100::BUFFER_NEAR_FULL: 
      return "BUFFER_NEAR_FULL";
    case It100::GENERAL_DEVICE_LOW_BATTERY: 
      return "GENERAL_DEVICE_LOW_BATTERY";
    case It100::GENERAL_DEVICE_LOW_BATTERY_RESTORE: 
      return "GENERAL_DEVICE_LOW_BATTERY_RESTORE";
    case It100::WIRELESS_KEY_LOW_BATTERY_TROUBLE: 
      return "WIRELESS_KEY_LOW_BATTERY_TROUBLE";
    case It100::RESTORE: 
      return "RESTORE";
    case It100::HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE: 
      return "HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE";
    case It100::RESTORED: 
      return "RESTORED";
    case It100::GENERAL_SYSTEM_TAMPER: 
      return "GENERAL_SYSTEM_TAMPER";
    case It100::GENERAL_SYSTEM_TAMPER_RESTORE: 
      return "GENERAL_SYSTEM_TAMPER_RESTORE";
    case It100::HOME_AUTOMATION_TROUBLE: 
      return "HOME_AUTOMATION_TROUBLE";
    case It100::HOME_AUTOMATION_TROUBLE_RESTORE: 
      return "HOME_AUTOMATION_TROUBLE_RESTORE";
    case It100::TROUBLE_STATUS_LED_ON: 
      return "TROUBLE_STATUS_LED_ON";
    case It100::TROUBLE_STATUS_RESTORE_LED_OFF: 
      return "TROUBLE_STATUS_RESTORE_LED_OFF";
    case It100::FIRE_TROUBLE_ALARM: 
      return "FIRE_TROUBLE_ALARM";
    case It100::FIRE_TROUBLE_ALARM_RESTORED: 
      return "FIRE_TROUBLE_ALARM_RESTORED";
    case It100::CODE_REQUIRED: 
      return "CODE_REQUIRED";
    case It100::LCD_UPDATE: 
      return "LCD_UPDATE";
    case It100::LCD_CURSOR: 
      return "LCD_CURSOR";
    case It100::LED_STATUS: 
      return "LED_STATUS";
    case It100::BEEP_STATUS: 
      return "BEEP_STATUS";
    case It100::TONE_STATUS: 
      return "TONE_STATUS";
    case It100::BUZZER_STATUS: 
      return "BUZZER_STATUS";
    case It100::DOOR_CHIME_STATUS: 
      return "DOOR_CHIME_STATUS";
    case It100::SOFTWARE_VERSION: 
      return "SOFTWARE_VERSION";
  }
}
