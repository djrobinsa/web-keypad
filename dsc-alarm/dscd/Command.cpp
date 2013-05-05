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

#include "Command.h"
#include "Config.h"
#include "It100.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include <stdexcept>

std::map<int,Command::creator_t> &
Command::creators()
{
  static std::map<int,creator_t> creatorMap;
  return creatorMap;
}

/**
  Factory method for commands
*/

Command *
Command::makeCommand (It100 &it100, std::string command)
{
  int code = strtol(command.substr(0,3).c_str(), 0, 10);
  int remoteChecksum = strtol(command.substr(command.length()-2).c_str(),0,16);

  // Verify checksum
  unsigned char localChecksum = 0;
  for (int i = 0; i < command.length()-2; i++)
  {
    localChecksum += command[i];
  }

  if (localChecksum != remoteChecksum)
  {
    std::cout << "Bad checksum: remote = " << remoteChecksum 
              << "local = " << localChecksum << std::endl;
    return 0;
  }

  // Find creator in map, and call it
  std::map<int,Command::creator_t>::iterator c = creators().find(code);

  if (c != creators().end())
  {
    Command *cmd = (*(c->second))(it100, command.substr(0,command.length()-2));
    assert (cmd->getCommandNumber() == code);
    return cmd;
  }

  return 0;
}

/**
  Format a command for printing
*/
void
Command::dump(std::ostream &os) const
{
  os << getName() << '(';
  for (int i = 0; i < getNumParams(); i++)
  {
    if (displayParamName(i))
    {
      os << getParamName(i) << " = ";
    }

    std::string param = getStringParam(i);
    if (param.length() > 0 && 
        param.find_first_not_of("0123456789") == std::string::npos)
    {
      os << getIntParam(i);
    }
    else
    {
      os << '"' << param << '"';
    }

    if (i+1 < getNumParams())
    {
      os << ", ";
    }
  }
  os << ')';
}

std::string
Command::getCommandWithChecksum() const
{
  unsigned char checksum = 0;
  char buffer[5];
  for (int i = 0; i < mCommand.length(); i++)
  {
    checksum += mCommand[i];
  }
  snprintf(buffer, sizeof(buffer), "%2.2X\r\n",checksum);
  buffer[4] = 0;
  return mCommand + buffer;
}

int
Command::getSyslogPriority() const
{
  return Config::getConfig().getSyslogPriority(getCommandNumber());
}

int 
Command::getInt(std::string::size_type index, std::string::size_type length)
  const
{
  return strtol(mCommand.substr(index, length).c_str(),0,10);
}

// ===========================================================================
// This is optimized for ease of implementation. It could be much faster
// if I hand-rolled my own search mechanism.

std::string
Command::getShellAction() const
{
  std::string action = Config::getConfig().getEventAction(getCommandNumber());
  int pos;

  //   %c              - The command name
  while ((pos = action.find("%c")) != std::string::npos)
  {
    action.replace(pos,2,getName());
  }

  //   %n              - The command code as a number
  while ((pos = action.find("%n")) != std::string::npos)
  {
    std::stringstream s;
    s << getCommandNumber();
    action.replace(pos,2,s.str());
  }

  //   %d              - Current contents of LCD display
  while ((pos = action.find("%d")) != std::string::npos)
  {
    action.replace(pos,2,mIt100.getLcd());
  }

  //   %z              - 64-bit hex represntation of open zones
  while ((pos = action.find("%z")) != std::string::npos)
  {
    std::stringstream s;
    s << std::hex << std::setfill('0') << std::setw(16) 
      << mIt100.getZoneStatus();
    action.replace(pos,2,s.str());
  }

  int i;

  //   %1i through %9i - Integer representation of parameters 1 through 9
  for (i = 1; i <= 9; i++)
  {
    std::stringstream search;
    search << "%" << i << "i";
    while ((pos = action.find(search.str())) != std::string::npos)
    {
      std::stringstream s;
      s << getIntParam(i-1);
      action.replace(pos,3,s.str());
    }
  }

  //   %1s through %9s - String representation of parameters 1 through 9
  for (i = 1; i <= 9; i++)
  {
    std::stringstream search;
    search << "%" << i << "s";
    while ((pos = action.find(search.str())) != std::string::npos)
    {
      action.replace(pos,3,getStringParam(i-1));
    }
  }

  //   %1l through %9l - Keypad status for LEDs 1 through 9
  for (i = 1; i <= 9; i++)
  {
    std::stringstream search;
    search << "%" << i << "l";
    while ((pos = action.find(search.str())) != std::string::npos)
    {
      std::stringstream s;
      s << mIt100.getLedState(static_cast<It100::led_t>(i));
      action.replace(pos,3,s.str());
    }
  }

  return action;
}
// ===========================================================================

std::ostream &
operator<<(std::ostream &os, const Command &c)
{
  c.dump(os);
  return os;
}

/** *******************************************************************
 * Command 0 (Poll) - Sent to alarm system
 *********************************************************************/

bool registeredPoll = 
  Command::addCreator(Command::POLL,  Poll::create);

/** *******************************************************************
 * Command 1 (Status Request) - Sent to alarm system
 *********************************************************************/

bool registeredStatusRequest = 
  Command::addCreator(Command::STATUS_REQUEST,  StatusRequest::create);

/** *******************************************************************
 * Command 2 (Labels Request) - Sent to alarm system
 *********************************************************************/

bool registeredLabelsRequest = 
  Command::addCreator(Command::LABELS_REQUEST,  LabelsRequest::create);

/** *******************************************************************
 * Command 10 (Set Time and Date) - Sent to alarm system
 * Parameters: 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

int
SetTimeAndDate::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 2); // hh
    case 1: return getInt(5, 2); // mm
    case 2: return getInt(7, 2); // MM
    case 3: return getInt(9, 2); // DD
    case 4: return getInt(11); // YY
  }
  return 0;
}

std::string
SetTimeAndDate::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 2); // hh
    case 1: return mCommand.substr(5, 2); // mm
    case 2: return mCommand.substr(7, 2); // MM
    case 3: return mCommand.substr(9, 2); // DD
    case 4: return mCommand.substr(11); // YY
  }
  return "";
}

std::string
SetTimeAndDate::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "h";
    case 1: return "m";
    case 2: return "M";
    case 3: return "D";
    case 4: return "Y";
  }
  return "";
}

bool registeredSetTimeAndDate = 
  Command::addCreator(Command::SET_TIME_AND_DATE,  SetTimeAndDate::create);

/** *******************************************************************
 * Command 20 (Command Output Control) - Sent to alarm system
 * Parameters: 2 bytes (Part 1-8 , Pgm 1-4)
 *********************************************************************/

int
CommandOutputControl::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part 1-8
    case 1: return getInt(4); // Pgm 1-4
  }
  return 0;
}

std::string
CommandOutputControl::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mCommand.substr(4); // Pgm 1-4
  }
  return "";
}

std::string
CommandOutputControl::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Output";
  }
  return "";
}

bool registeredCommandOutputControl = 
  Command::addCreator(Command::COMMAND_OUTPUT_CONTROL,  CommandOutputControl::create);

/** *******************************************************************
 * Command 30 (Partition Arm Control - Away) - Sent to alarm system
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionArmControlAway::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionArmControlAway::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionArmControlAway = 
  Command::addCreator(Command::PARTITION_ARM_CONTROL_AWAY,  PartitionArmControlAway::create);

/** *******************************************************************
 * Command 31 (Partition Arm Control - Stay) - Sent to alarm system
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionArmControlStay::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionArmControlStay::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionArmControlStay = 
  Command::addCreator(Command::PARTITION_ARM_CONTROL_STAY,  PartitionArmControlStay::create);

/** *******************************************************************
 * Command 32 (Partition Arm Control - Armed, No Entry Delay) - Sent to 
 * alarm system
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionArmControlArmedNoEntryDelay::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionArmControlArmedNoEntryDelay::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionArmControlArmedNoEntryDelay = 
  Command::addCreator(Command::PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY,  PartitionArmControlArmedNoEntryDelay::create);

/** *******************************************************************
 * Command 33 (Partition Arm Control - With Code) - Sent to alarm system
 * Parameters: 7 bytes (Part 1-8 , Code 6 bytes h)
 *********************************************************************/

int
PartitionArmControlWithCode::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part 1-8
    case 1: return getInt(4); // Code 6 bytes h
  }
  return 0;
}

std::string
PartitionArmControlWithCode::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mCommand.substr(4); // Code 6 bytes h
  }
  return "";
}

std::string
PartitionArmControlWithCode::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Code";
  }
  return "";
}

bool registeredPartitionArmControlWithCode = 
  Command::addCreator(Command::PARTITION_ARM_CONTROL_WITH_CODE,  PartitionArmControlWithCode::create);

/** *******************************************************************
 * Command 40 (Partition Disarm Control - With Code) - Sent to alarm system
 * Parameters: 7 bytes (Part 1-8 , Code 6 bytes h)
 *********************************************************************/

int
PartitionDisarmControlWithCode::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part 1-8
    case 1: return getInt(4); // Code 6 bytes h
  }
  return 0;
}

std::string
PartitionDisarmControlWithCode::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mCommand.substr(4); // Code 6 bytes h
  }
  return "";
}

std::string
PartitionDisarmControlWithCode::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Code";
  }
  return "";
}

bool registeredPartitionDisarmControlWithCode = 
  Command::addCreator(Command::PARTITION_DISARM_CONTROL_WITH_CODE,  PartitionDisarmControlWithCode::create);

/** *******************************************************************
 * Command 55 (Time Stamp Control) - Sent to alarm system
 * Parameters: 1 byte (On/Off)
 *********************************************************************/

std::string
TimeStampControl::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return getIntParam(0)?"On":"Off";
  }
  return "";
}

std::string
TimeStampControl::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "On/Off";
  }
  return "";
}

bool registeredTimeStampControl = 
  Command::addCreator(Command::TIME_STAMP_CONTROL,  TimeStampControl::create);

/** *******************************************************************
 * Command 56 (Time/Date Broadcast Control) - Sent to alarm system
 * Parameters: 1 byte (On/Off)
 *********************************************************************/

std::string
TimeDateBroadcastControl::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return getIntParam(0)?"On":"Off";
  }
  return "";
}

std::string
TimeDateBroadcastControl::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "On/Off";
  }
  return "";
}

bool registeredTimeDateBroadcastControl = 
  Command::addCreator(Command::TIME_DATE_BROADCAST_CONTROL,  TimeDateBroadcastControl::create);

/** *******************************************************************
 * Command 57 (Temperature Broadcast Control) - Sent to alarm system
 * Parameters: 1 byte (On/Off)
 *********************************************************************/

std::string
TemperatureBroadcastControl::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return getIntParam(0)?"On":"Off";
  }
  return "";
}

std::string
TemperatureBroadcastControl::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "On/Off";
  }
  return "";
}

bool registeredTemperatureBroadcastControl = 
  Command::addCreator(Command::TEMPERATURE_BROADCAST_CONTROL,  TemperatureBroadcastControl::create);

/** *******************************************************************
 * Command 58 (Virtual Keypad Control) - Sent to alarm system
 * Parameters: 1 byte (On/Off)
 *********************************************************************/

std::string
VirtualKeypadControl::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return getIntParam(0)?"On":"Off";
  }
  return "";
}

std::string
VirtualKeypadControl::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "On/Off";
  }
  return "";
}

bool registeredVirtualKeypadControl = 
  Command::addCreator(Command::VIRTUAL_KEYPAD_CONTROL,  VirtualKeypadControl::create);

/** *******************************************************************
 * Command 60 (Trigger Panic Alarm) - Sent to alarm system
 * Parameters: 1 byte (1 = F; 2 = A; 3 = P)
 *********************************************************************/

std::string
TriggerPanicAlarm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: 
      // 1 = F; 2 = A; 3 = P
      switch (getIntParam(0))
      {
        case 1: return "Fire";
        case 2: return "Ambulance";
        case 3: return "Panic";
        default: return mCommand.substr(3);
      }
  }
  return "";
}

std::string
TriggerPanicAlarm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Key";
  }
  return "";
}

bool registeredTriggerPanicAlarm = 
  Command::addCreator(Command::TRIGGER_PANIC_ALARM,  TriggerPanicAlarm::create);

/** *******************************************************************
 * Command 70 (Key Pressed) - Sent to alarm system
 * Parameters: 1 byte (Key)
 *********************************************************************/

std::string
KeyPressed::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // Key
  }
  return "";
}

std::string
KeyPressed::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Key";
  }
  return "";
}

bool registeredKeyPressed = 
  Command::addCreator(Command::KEY_PRESSED,  KeyPressed::create);

/** *******************************************************************
 * Command 80 (Baud Rate Change) - Sent to alarm system
 * Parameters: 1 byte (Val 0 - 4)
 *********************************************************************/

std::string
BaudRateChange::getStringParam(int number) const
{
  switch (number)
  {
    case 0:
      // 0 = 9600,  1 = 19200,  2 = 38400,  3 = 57600,  4 = 115200
      switch (getIntParam(0))
      {
        case 0: return "9600";
        case 1: return "19200";
        case 2: return "38400";
        case 3: return "57600";
        case 4: return "115200";
        default: return mCommand.substr(3);
      }
  }
  return "";
}

std::string
BaudRateChange::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Baud Rate";
  }
  return "";
}

bool registeredBaudRateChange = 
  Command::addCreator(Command::BAUD_RATE_CHANGE,  BaudRateChange::create);

/** *******************************************************************
 * Command 95 (Get Temperature Set Point) - Sent to alarm system
 * Parameters: 1 byte (Val 1 - 4)
 *********************************************************************/

std::string
GetTemperatureSetPoint::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // Val 1 - 4
  }
  return "";
}

std::string
GetTemperatureSetPoint::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Val";
  }
  return "";
}

bool registeredGetTemperatureSetPoint = 
  Command::addCreator(Command::GET_TEMPERATURE_SET_POINT,  GetTemperatureSetPoint::create);

/** *******************************************************************
 * Command 96 (Temperature Change) - Sent to alarm system
 * Parameters: 8 bytes? (T,S,M,A1-A3)
 *********************************************************************/

int
TemperatureChange::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // T
    case 1: return getInt(4, 1); // S
    case 2: return getInt(5, 1); // M
    case 3: return getInt(6); // A1-A3
  }
  return 0;
}

std::string
TemperatureChange::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 1); // T
    case 1: return mCommand.substr(4, 1); // S
    case 2: return mCommand.substr(5, 1); // M
    case 3: return mCommand.substr(6); // A1-A3
  }
  return "";
}

std::string
TemperatureChange::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Thermostat";
    case 1: return "Type of set point";
    case 2: return "Mode";
    case 3: return "Temperature";
  }
  return "";
}

bool registeredTemperatureChange = 
  Command::addCreator(Command::TEMPERATURE_CHANGE,  TemperatureChange::create);

/** *******************************************************************
 * Command 97 (Save Temperature Setting) - Sent to alarm system
 * Parameters: 1 byte (Val 1 - 4)
 *********************************************************************/

std::string
SaveTemperatureSetting::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // Val 1 - 4
  }
  return "";
}

std::string
SaveTemperatureSetting::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Thermostat";
  }
  return "";
}

bool registeredSaveTemperatureSetting = 
  Command::addCreator(Command::SAVE_TEMPERATURE_SETTING,  SaveTemperatureSetting::create);

/** *******************************************************************
 * Command 200 (Code Send) - Sent to alarm system
 * Parameters: 6 bytes (Access Code in hex ASCII)
 *********************************************************************/

std::string
CodeSend::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // Access Code in hex ASCII
  }
  return "";
}

std::string
CodeSend::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Access Code";
  }
  return "";
}

bool registeredCodeSend = 
  Command::addCreator(Command::CODE_SEND,  CodeSend::create);

/** *******************************************************************
 * Command 500 (Command Acknowledge)
 * Parameters: 3 bytes (CMD received in Hex ASCII)
 *********************************************************************/

std::string
CommandAcknowledge::getStringParam(int number) const
{
  switch (number)
  {
    case 0: 
    {
      int code = getIntParam(0);
      std::map<int,Command::creator_t>::iterator c = creators().find(code);

      if (c != creators().end())
      {
        Command *cmd = (*(c->second))(mIt100, "");
        if (cmd)
        {
          std::string name = cmd->getName();
          delete cmd;
          return name;
        }
      }
      return mCommand.substr(3);
    }
  }
  return "";
}

std::string
CommandAcknowledge::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "CMD received in Hex ASCII";
  }
  return "";
}

bool registeredCommandAcknowledge = 
  Command::addCreator(Command::COMMAND_ACKNOWLEDGE,  CommandAcknowledge::create);

/** *******************************************************************
 * Command 501 (Command Error)
 *********************************************************************/

bool registeredCommandError = 
  Command::addCreator(Command::COMMAND_ERROR,  CommandError::create);

/** *******************************************************************
 * Command 502 (System Error)
 * Parameters: 3 bytes (Error Code in Hex ASCII)
 *********************************************************************/

std::string
SystemError::getStringParam(int number) const
{
  switch (number)
  {
    case 0:
      switch (getIntParam(0))
      {
        case 17: return "Keybus Busy - Installer Mode";
        case 21: return "Requested Partition is out of Range";
        case 23: return "Partition is Not Armed";
        case 24: return "Partition is Not Ready to Arm";
        case 26: return "User Code Not Required";
        case 28: return "Virtual Keypad is Disabled";
        case 29: return "Not Valid Parameter";
        case 30: return "Keypad Does Not Come Out of Blank Mode";
        case 31: return "IT-100 is already in Thermostat menu";
        case 32: return "IT-100 is Not in Thermostat menu";
        case 33: return "No Response from Thermostat or Escort Module";
        default: return mCommand.substr(3);
      }
  }
  return "";
}

std::string
SystemError::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Error Code in Hex ASCII";
  }
  return "";
}

bool registeredSystemError = 
  Command::addCreator(Command::SYSTEM_ERROR,  SystemError::create);

/** *******************************************************************
 * Command 550 (Time/Date Broadcast)
 * Parameters: 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

int
TimeDateBroadcast::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 2); // hh
    case 1: return getInt(5, 2); // mm
    case 2: return getInt(7, 2); // MM
    case 3: return getInt(9, 2); // DD
    case 4: return getInt(11); // YY
  }
  return 0;
}

std::string
TimeDateBroadcast::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 2); // hh
    case 1: return mCommand.substr(5, 2); // mm
    case 2: return mCommand.substr(7, 2); // MM
    case 3: return mCommand.substr(9, 2); // DD
    case 4: return mCommand.substr(11); // YY
  }
  return "";
}

std::string
TimeDateBroadcast::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "h";
    case 1: return "m";
    case 2: return "M";
    case 3: return "D";
    case 4: return "Y";
  }
  return "";
}

bool registeredTimeDateBroadcast = 
  Command::addCreator(Command::TIME_DATE_BROADCAST,  TimeDateBroadcast::create);

/** *******************************************************************
 * Command 560 (Ring Detected)
 * Parameters: None (Note the IT100 doc is wrong - it says 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

int
RingDetected::getIntParam(int number) const
{
  return 0;
}

std::string
RingDetected::getStringParam(int number) const
{
  return "";
}

std::string
RingDetected::getParamName(int number) const
{
  return "";
}

bool registeredRingDetected = 
  Command::addCreator(Command::RING_DETECTED,  RingDetected::create);

/** *******************************************************************
 * Command 561 (Indoor Temperature Broadcast)
 * Parameters: 4 bytes (Thermostat , Temp)
 *********************************************************************/

int
IndoorTemperatureBroadcast::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Thermostat
    case 1: return getInt(4); // Temp
  }
  return 0;
}

std::string
IndoorTemperatureBroadcast::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 1); // Thermostat
    case 1: return mCommand.substr(4); // Temp
  }
  return "";
}

std::string
IndoorTemperatureBroadcast::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Thermostat";
    case 1: return "Temp";
  }
  return "";
}

bool registeredIndoorTemperatureBroadcast = 
  Command::addCreator(Command::INDOOR_TEMPERATURE_BROADCAST,  IndoorTemperatureBroadcast::create);

/** *******************************************************************
 * Command 562 (Outdoor Temperatrure Broadcast)
 * Parameters: 4 bytes (Thermostat ,Temp)
 *********************************************************************/

int
OutdoorTemperatrureBroadcast::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Thermostat
    case 1: return getInt(4); // Temp
  }
  return 0;
}

std::string
OutdoorTemperatrureBroadcast::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 1); // Thermostat
    case 1: return mCommand.substr(4); // Temp
  }
  return "";
}

std::string
OutdoorTemperatrureBroadcast::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Thermostat";
    case 1: return "Temp";
  }
  return "";
}

bool registeredOutdoorTemperatrureBroadcast = 
  Command::addCreator(Command::OUTDOOR_TEMPERATRURE_BROADCAST,  OutdoorTemperatrureBroadcast::create);

/** *******************************************************************
 * Command 563 (Thermostat Set Points)
 * Parameters: 8 bytes (Thermostat,C1-C3, H1-H3)
 *********************************************************************/

int
ThermostatSetPoints::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 2); // Thermostat
    case 1: return getInt(5, 3); // C1-C3
    case 2: return getInt(8); // H1-H3
  }
  return 0;
}

std::string
ThermostatSetPoints::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 2); // Thermostat
    case 1: return mCommand.substr(5, 3); // C1-C3
    case 2: return mCommand.substr(8); // H1-H3
  }
  return "";
}

std::string
ThermostatSetPoints::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Thermostat";
    case 1: return "Cooling";
    case 2: return "Heating";
  }
  return "";
}

bool registeredThermostatSetPoints = 
  Command::addCreator(Command::THERMOSTAT_SET_POINTS,  ThermostatSetPoints::create);

/** *******************************************************************
 * Command 570 (Broadcast Labels)
 * Parameters: 35 bytes (Lbl# 3, Lbl 32 Bytes)
 *********************************************************************/

int
BroadcastLabels::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 3); // Lbl# 3
    case 1: return getInt(6); // Lbl 32 Bytes
  }
  return 0;
}

std::string
BroadcastLabels::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 3); // Lbl# 3
    case 1:
      {
        std::string label = mCommand.substr(6); // Lbl 32 Bytes
        return label.substr(0,label.find_last_not_of(" ") + 1);
      }
  }
  return "";
}

std::string
BroadcastLabels::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Lbl#";
    case 1: return "Lbl";
  }
  return "";
}

bool registeredBroadcastLabels = 
  Command::addCreator(Command::BROADCAST_LABELS,  BroadcastLabels::create);

/** *******************************************************************
 * Command 580 (Baud Rate Set)
 * Parameters: 1 byte (Val = 0- 4)
 *********************************************************************/

std::string
BaudRateSet::getStringParam(int number) const
{
  switch (number)
  {
    case 0:
      // 0 = 9600,  1 = 19200,  2 = 38400,  3 = 57600,  4 = 115200
      switch (getIntParam(0))
      {
        case 0: return "9600";
        case 1: return "19200";
        case 2: return "38400";
        case 3: return "57600";
        case 4: return "115200";
        default: return mCommand.substr(3);
      }
  }
  return "";
}

std::string
BaudRateSet::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Val";
  }
  return "";
}

bool registeredBaudRateSet = 
  Command::addCreator(Command::BAUD_RATE_SET,  BaudRateSet::create);

/** *******************************************************************
 * Command 601 (Zone Alarm)
 * Parameters: 4 bytes (Partition. 1-8, Zn 1-64)
 *********************************************************************/

int
ZoneAlarm::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Partition. 1-8
    case 1: return getInt(4); // Zn 1-64
  }
  return 0;
}

std::string
ZoneAlarm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getZoneName(getIntParam(1));
  }
  return "";
}

std::string
ZoneAlarm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Zone";
  }
  return "";
}

bool registeredZoneAlarm = 
  Command::addCreator(Command::ZONE_ALARM,  ZoneAlarm::create);

/** *******************************************************************
 * Command 602 (Zone Alarm Restore)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

int
ZoneAlarmRestore::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part. 1-8
    case 1: return getInt(4); // Zn 1-64
  }
  return 0;
}

std::string
ZoneAlarmRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getZoneName(getIntParam(1));
  }
  return "";
}

std::string
ZoneAlarmRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Zone";
  }
  return "";
}

bool registeredZoneAlarmRestore = 
  Command::addCreator(Command::ZONE_ALARM_RESTORE,  ZoneAlarmRestore::create);

/** *******************************************************************
 * Command 603 (Zone Tamper)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

int
ZoneTamper::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part. 1-8
    case 1: return getInt(4); // Zn 1-64
  }
  return 0;
}

std::string
ZoneTamper::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getZoneName(getIntParam(1));
  }
  return "";
}

std::string
ZoneTamper::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Zone";
  }
  return "";
}

bool registeredZoneTamper = 
  Command::addCreator(Command::ZONE_TAMPER,  ZoneTamper::create);

/** *******************************************************************
 * Command 604 (Zone Tamper Restore)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

int
ZoneTamperRestore::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part. 1-8
    case 1: return getInt(4); // Zn 1-64
  }
  return 0;
}

std::string
ZoneTamperRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getZoneName(getIntParam(1));
  }
  return "";
}

std::string
ZoneTamperRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Zone";
  }
  return "";
}

bool registeredZoneTamperRestore = 
  Command::addCreator(Command::ZONE_TAMPER_RESTORE,  ZoneTamperRestore::create);

/** *******************************************************************
 * Command 605 (Zone Fault)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

std::string
ZoneFault::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
ZoneFault::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredZoneFault = 
  Command::addCreator(Command::ZONE_FAULT,  ZoneFault::create);

/** *******************************************************************
 * Command 606 (Zone Fault Restore)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

std::string
ZoneFaultRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
ZoneFaultRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredZoneFaultRestore = 
  Command::addCreator(Command::ZONE_FAULT_RESTORE,  ZoneFaultRestore::create);

/** *******************************************************************
 * Command 609 (Zone Open)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

std::string
ZoneOpen::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
ZoneOpen::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredZoneOpen = 
  Command::addCreator(Command::ZONE_OPEN,  ZoneOpen::create);

/** *******************************************************************
 * Command 610 (Zone Restored)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

std::string
ZoneRestored::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
ZoneRestored::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredZoneRestored = 
  Command::addCreator(Command::ZONE_RESTORED,  ZoneRestored::create);

/** *******************************************************************
 * Command 620 (Duress Alarm)
 * Parameters: 4 bytes (0000)
 *********************************************************************/

std::string
DuressAlarm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // 0000
  }
  return "";
}

std::string
DuressAlarm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "0000";
  }
  return "";
}

bool registeredDuressAlarm = 
  Command::addCreator(Command::DURESS_ALARM,  DuressAlarm::create);

/** *******************************************************************
 * Command 621 ([F] Key Alarm)
 *********************************************************************/

bool registeredFKeyAlarm = 
  Command::addCreator(Command::F_KEY_ALARM,  FKeyAlarm::create);

/** *******************************************************************
 * Command 622 ([F] Key Restoral)
 *********************************************************************/

bool registeredFKeyRestoral = 
  Command::addCreator(Command::F_KEY_RESTORAL,  FKeyRestoral::create);

/** *******************************************************************
 * Command 623 ([A] Key Alarm)
 *********************************************************************/

bool registeredAKeyAlarm = 
  Command::addCreator(Command::A_KEY_ALARM,  AKeyAlarm::create);

/** *******************************************************************
 * Command 624 ([A] Key Restoral)
 *********************************************************************/

bool registeredAKeyRestoral = 
  Command::addCreator(Command::A_KEY_RESTORAL,  AKeyRestoral::create);

/** *******************************************************************
 * Command 625 ([P] Key Alarm)
 *********************************************************************/

bool registeredPKeyAlarm = 
  Command::addCreator(Command::P_KEY_ALARM,  PKeyAlarm::create);

/** *******************************************************************
 * Command 626 ([P] Key Restoral)
 *********************************************************************/

bool registeredPKeyRestoral = 
  Command::addCreator(Command::P_KEY_RESTORAL,  PKeyRestoral::create);

/** *******************************************************************
 * Command 631 (Auxiliary Input Alarm)
 *********************************************************************/

bool registeredAuxiliaryInputAlarm = 
  Command::addCreator(Command::AUXILIARY_INPUT_ALARM,  AuxiliaryInputAlarm::create);

/** *******************************************************************
 * Command 632 (Auxiliary Input Alarm Restored)
 *********************************************************************/

bool registeredAuxiliaryInputAlarmRestored = 
  Command::addCreator(Command::AUXILIARY_INPUT_ALARM_RESTORED,  AuxiliaryInputAlarmRestored::create);

/** *******************************************************************
 * Command 650 (Partition Ready)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionReady::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionReady::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionReady = 
  Command::addCreator(Command::PARTITION_READY,  PartitionReady::create);

/** *******************************************************************
 * Command 651 (Partition Not Ready)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionNotReady::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionNotReady::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionNotReady = 
  Command::addCreator(Command::PARTITION_NOT_READY,  PartitionNotReady::create);

/** *******************************************************************
 * Command 652 (Partition Armed - Descriptive Mode)
 * Parameters: 2 bytes (Partition 1 - 8 , Mode)
 *********************************************************************/

int
PartitionArmedDescriptiveMode::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Partition 1 - 8
    case 1: return getInt(4); // Mode
  }
  return 0;
}

std::string
PartitionArmedDescriptiveMode::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1:
      switch (getIntParam(1))
      {
        case 0: return "Away";
        case 1: return "Stay";
        case 2: return "Away, No Delay";
        case 3: return "Stay, No Delay";
        default: return mCommand.substr(4);
      }
  }
  return "";
}

std::string
PartitionArmedDescriptiveMode::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Mode";
  }
  return "";
}

bool registeredPartitionArmedDescriptiveMode = 
  Command::addCreator(Command::PARTITION_ARMED_DESCRIPTIVE_MODE,  PartitionArmedDescriptiveMode::create);

/** *******************************************************************
 * Command 653 (Partition in Ready to Force Arm)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitioninReadytoForceArm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitioninReadytoForceArm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitioninReadytoForceArm = 
  Command::addCreator(Command::PARTITION_IN_READY_TO_FORCE_ARM,  PartitioninReadytoForceArm::create);

/** *******************************************************************
 * Command 654 (Partition In Alarm)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionInAlarm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionInAlarm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionInAlarm = 
  Command::addCreator(Command::PARTITION_IN_ALARM,  PartitionInAlarm::create);

/** *******************************************************************
 * Command 655 (Partition Disarmed)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionDisarmed::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionDisarmed::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionDisarmed = 
  Command::addCreator(Command::PARTITION_DISARMED,  PartitionDisarmed::create);

/** *******************************************************************
 * Command 656 (Exit Delay in Progress)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
ExitDelayinProgress::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
ExitDelayinProgress::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredExitDelayinProgress = 
  Command::addCreator(Command::EXIT_DELAY_IN_PROGRESS,  ExitDelayinProgress::create);

/** *******************************************************************
 * Command 657 (Entry Delay in Progress)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
EntryDelayinProgress::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
EntryDelayinProgress::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredEntryDelayinProgress = 
  Command::addCreator(Command::ENTRY_DELAY_IN_PROGRESS,  EntryDelayinProgress::create);

/** *******************************************************************
 * Command 658 (Keypad Lock-out)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
KeypadLockout::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
KeypadLockout::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredKeypadLockout = 
  Command::addCreator(Command::KEYPAD_LOCKOUT,  KeypadLockout::create);

/** *******************************************************************
 * Command 659 (Keypad Blanking)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
KeypadBlanking::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
KeypadBlanking::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredKeypadBlanking = 
  Command::addCreator(Command::KEYPAD_BLANKING,  KeypadBlanking::create);

/** *******************************************************************
 * Command 660 (Command Output In Progress)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
CommandOutputInProgress::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
CommandOutputInProgress::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredCommandOutputInProgress = 
  Command::addCreator(Command::COMMAND_OUTPUT_IN_PROGRESS,  CommandOutputInProgress::create);

/** *******************************************************************
 * Command 670 (Invalid Access Code)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
InvalidAccessCode::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
InvalidAccessCode::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredInvalidAccessCode = 
  Command::addCreator(Command::INVALID_ACCESS_CODE,  InvalidAccessCode::create);

/** *******************************************************************
 * Command 671 (Function Not Available)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
FunctionNotAvailable::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
FunctionNotAvailable::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredFunctionNotAvailable = 
  Command::addCreator(Command::FUNCTION_NOT_AVAILABLE,  FunctionNotAvailable::create);

/** *******************************************************************
 * Command 672 (Fail to Arm)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
FailtoArm::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
FailtoArm::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredFailtoArm = 
  Command::addCreator(Command::FAIL_TO_ARM,  FailtoArm::create);

/** *******************************************************************
 * Command 673 (Partition Busy)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartitionBusy::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartitionBusy::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartitionBusy = 
  Command::addCreator(Command::PARTITION_BUSY,  PartitionBusy::create);

/** *******************************************************************
 * Command 700 (User Closing)
 * Parameters: 5 bytes (Part 1-8 , User Code)
 *********************************************************************/

int
UserClosing::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part 1-8
    case 1: return getInt(4); // User Code
  }
  return 0;
}

std::string
UserClosing::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getUserName(getIntParam(1));
  }
  return "";
}

std::string
UserClosing::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "User Code";
  }
  return "";
}

bool registeredUserClosing = 
  Command::addCreator(Command::USER_CLOSING,  UserClosing::create);

/** *******************************************************************
 * Command 701 (Special Closing)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
SpecialClosing::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
SpecialClosing::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredSpecialClosing = 
  Command::addCreator(Command::SPECIAL_CLOSING,  SpecialClosing::create);

/** *******************************************************************
 * Command 702 (Partial Closing)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
PartialClosing::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
PartialClosing::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredPartialClosing = 
  Command::addCreator(Command::PARTIAL_CLOSING,  PartialClosing::create);

/** *******************************************************************
 * Command 750 (User Opening)
 * Parameters: 5 bytes (Part 1-8 , User Code)
 *********************************************************************/

int
UserOpening::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part 1-8
    case 1: return getInt(4); // User Code
  }
  return 0;
}

std::string
UserOpening::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mIt100.getUserName(getIntParam(1));
  }
  return "";
}

std::string
UserOpening::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "User Code";
  }
  return "";
}

bool registeredUserOpening = 
  Command::addCreator(Command::USER_OPENING,  UserOpening::create);

/** *******************************************************************
 * Command 751 (Special Opening)
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
SpecialOpening::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
SpecialOpening::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredSpecialOpening = 
  Command::addCreator(Command::SPECIAL_OPENING,  SpecialOpening::create);

/** *******************************************************************
 * Command 800 (Panel Battery Trouble)
 *********************************************************************/

bool registeredPanelBatteryTrouble = 
  Command::addCreator(Command::PANEL_BATTERY_TROUBLE,  PanelBatteryTrouble::create);

/** *******************************************************************
 * Command 801 (Panel Battery Trouble Restore)
 *********************************************************************/

bool registeredPanelBatteryTroubleRestore = 
  Command::addCreator(Command::PANEL_BATTERY_TROUBLE_RESTORE,  PanelBatteryTroubleRestore::create);

/** *******************************************************************
 * Command 802 (Panel AC Trouble)
 *********************************************************************/

bool registeredPanelACTrouble = 
  Command::addCreator(Command::PANEL_AC_TROUBLE,  PanelACTrouble::create);

/** *******************************************************************
 * Command 803 (Panel AC Restore)
 *********************************************************************/

bool registeredPanelACRestore = 
  Command::addCreator(Command::PANEL_AC_RESTORE,  PanelACRestore::create);

/** *******************************************************************
 * Command 806 (System Bell Trouble)
 *********************************************************************/

bool registeredSystemBellTrouble = 
  Command::addCreator(Command::SYSTEM_BELL_TROUBLE,  SystemBellTrouble::create);

/** *******************************************************************
 * Command 807 (System Bell Trouble Restoral)
 *********************************************************************/

bool registeredSystemBellTroubleRestoral = 
  Command::addCreator(Command::SYSTEM_BELL_TROUBLE_RESTORAL,  SystemBellTroubleRestoral::create);

/** *******************************************************************
 * Command 810 (TLM Line 1 Trouble)
 *********************************************************************/

bool registeredTLMLine1Trouble = 
  Command::addCreator(Command::TLM_LINE_1_TROUBLE,  TLMLine1Trouble::create);

/** *******************************************************************
 * Command 811 (TLM Line 1 Trouble Restored)
 *********************************************************************/

bool registeredTLMLine1TroubleRestored = 
  Command::addCreator(Command::TLM_LINE_1_TROUBLE_RESTORED,  TLMLine1TroubleRestored::create);

/** *******************************************************************
 * Command 812 (TLM Line 2 Trouble)
 *********************************************************************/

bool registeredTLMLine2Trouble = 
  Command::addCreator(Command::TLM_LINE_2_TROUBLE,  TLMLine2Trouble::create);

/** *******************************************************************
 * Command 813 (TLM Line 2 Trouble Restored)
 *********************************************************************/

bool registeredTLMLine2TroubleRestored = 
  Command::addCreator(Command::TLM_LINE_2_TROUBLE_RESTORED,  TLMLine2TroubleRestored::create);

/** *******************************************************************
 * Command 814 (FTC Trouble)
 *********************************************************************/

bool registeredFTCTrouble = 
  Command::addCreator(Command::FTC_TROUBLE,  FTCTrouble::create);

/** *******************************************************************
 * Command 816 (Buffer Near Full)
 *********************************************************************/

bool registeredBufferNearFull = 
  Command::addCreator(Command::BUFFER_NEAR_FULL,  BufferNearFull::create);

/** *******************************************************************
 * Command 821 (General Device Low Battery)
 * Parameters: 3 bytes (Zn 001-032)
 *********************************************************************/

std::string
GeneralDeviceLowBattery::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
GeneralDeviceLowBattery::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredGeneralDeviceLowBattery = 
  Command::addCreator(Command::GENERAL_DEVICE_LOW_BATTERY,  GeneralDeviceLowBattery::create);

/** *******************************************************************
 * Command 822 (General Device Low Battery Restore)
 * Parameters: 3 bytes (Zn 001-032)
 *********************************************************************/

std::string
GeneralDeviceLowBatteryRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getZoneName(getIntParam(0));
  }
  return "";
}

std::string
GeneralDeviceLowBatteryRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Zone";
  }
  return "";
}

bool registeredGeneralDeviceLowBatteryRestore = 
  Command::addCreator(Command::GENERAL_DEVICE_LOW_BATTERY_RESTORE,  GeneralDeviceLowBatteryRestore::create);

/** *******************************************************************
 * Command 825 (Wireless Key Low Battery Trouble)
 * Parameters: 3 bytes (001-016)
 *********************************************************************/

std::string
WirelessKeyLowBatteryTrouble::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return Config::getConfig().getKeyName(getIntParam(0));
  }
  return "";
}

std::string
WirelessKeyLowBatteryTrouble::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Keyfob";
  }
  return "";
}

bool registeredWirelessKeyLowBatteryTrouble = 
  Command::addCreator(Command::WIRELESS_KEY_LOW_BATTERY_TROUBLE,  WirelessKeyLowBatteryTrouble::create);

/** *******************************************************************
 * Command 826 (Wireless Key Low Battery Trouble Restore)
 * Parameters: 3 bytes (001-016)
 *********************************************************************/

std::string
WirelessKeyLowBatteryTroubleRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return Config::getConfig().getKeyName(getIntParam(0));
  }
  return "";
}

std::string
WirelessKeyLowBatteryTroubleRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Keyfob";
  }
  return "";
}

bool registeredWirelessKeyLowBatteryTroubleRestore = 
  Command::addCreator(Command::WIRELESS_KEY_LOW_BATTERY_TROUBLE_RESTORE,  WirelessKeyLowBatteryTroubleRestore::create);

/** *******************************************************************
 * Command 827 (Handheld Keypad Low Battery Trouble)
 * Parameters: 3 bytes (001-004)
 *********************************************************************/

std::string
HandheldKeypadLowBatteryTrouble::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return Config::getConfig().getKeypadName(getIntParam(0));
  }
  return "";
}

std::string
HandheldKeypadLowBatteryTrouble::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Keypad";
  }
  return "";
}

bool registeredHandheldKeypadLowBatteryTrouble = 
  Command::addCreator(Command::HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE,  HandheldKeypadLowBatteryTrouble::create);

/** *******************************************************************
 * Command 828 (Handheld Keypad Low Battery Restore)
 * Parameters: 3 bytes (001-004)
 *********************************************************************/

std::string
HandheldKeypadLowBatteryTroubleRestore::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return Config::getConfig().getKeypadName(getIntParam(0));
  }
  return "";
}

std::string
HandheldKeypadLowBatteryTroubleRestore::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Keypad";
  }
  return "";
}

bool registeredHandheldKeypadLowBatteryTroubleRestore = 
  Command::addCreator(Command::HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE_RESTORE,  HandheldKeypadLowBatteryTroubleRestore::create);

/** *******************************************************************
 * Command 829 (General System Tamper)
 *********************************************************************/

bool registeredGeneralSystemTamper = 
  Command::addCreator(Command::GENERAL_SYSTEM_TAMPER,  GeneralSystemTamper::create);

/** *******************************************************************
 * Command 830 (General System Tamper Restore)
 *********************************************************************/

bool registeredGeneralSystemTamperRestore = 
  Command::addCreator(Command::GENERAL_SYSTEM_TAMPER_RESTORE,  GeneralSystemTamperRestore::create);

/** *******************************************************************
 * Command 831 (Home Automation Trouble)
 *********************************************************************/

bool registeredHomeAutomationTrouble = 
  Command::addCreator(Command::HOME_AUTOMATION_TROUBLE,  HomeAutomationTrouble::create);

/** *******************************************************************
 * Command 832 (Home Automation Trouble Restore)
 *********************************************************************/

bool registeredHomeAutomationTroubleRestore = 
  Command::addCreator(Command::HOME_AUTOMATION_TROUBLE_RESTORE,  HomeAutomationTroubleRestore::create);

/** *******************************************************************
 * Command 840 (Trouble Status (LED ON))
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
TroubleStatusLEDON::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
TroubleStatusLEDON::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredTroubleStatusLEDON = 
  Command::addCreator(Command::TROUBLE_STATUS_LED_ON,  TroubleStatusLEDON::create);

/** *******************************************************************
 * Command 841 (Trouble Status Restore (LED OFF))
 * Parameters: 1 byte (Partition 1-8)
 *********************************************************************/

std::string
TroubleStatusRestoreLEDOFF::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
  }
  return "";
}

std::string
TroubleStatusRestoreLEDOFF::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
  }
  return "";
}

bool registeredTroubleStatusRestoreLEDOFF = 
  Command::addCreator(Command::TROUBLE_STATUS_RESTORE_LED_OFF,  TroubleStatusRestoreLEDOFF::create);

/** *******************************************************************
 * Command 842 (Fire Trouble Alarm)
 *********************************************************************/

bool registeredFireTroubleAlarm = 
  Command::addCreator(Command::FIRE_TROUBLE_ALARM,  FireTroubleAlarm::create);

/** *******************************************************************
 * Command 843 (Fire Trouble Alarm Restored)
 *********************************************************************/

bool registeredFireTroubleAlarmRestored = 
  Command::addCreator(Command::FIRE_TROUBLE_ALARM_RESTORED,  FireTroubleAlarmRestored::create);

/** *******************************************************************
 * Command 900 (Code Required)
 * Parameters: 2 bytes (Part , Code length 6)
 *********************************************************************/

int
CodeRequired::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // Part
    case 1: return getInt(4); // Code length 6
  }
  return 0;
}

std::string
CodeRequired::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mIt100.getPartitionName(getIntParam(0));
    case 1: return mCommand.substr(4); // Code length 6
  }
  return "";
}

std::string
CodeRequired::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Partition";
    case 1: return "Code length";
  }
  return "";
}

bool registeredCodeRequired = 
  Command::addCreator(Command::CODE_REQUIRED,  CodeRequired::create);

/** *******************************************************************
 * Command 901 (LCD Update)
 * Parameters: 6-37 bytes (L,C1-C2, D1-D2, A1-An)
 *********************************************************************/

int
LCDUpdate::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // L
    case 1: return getInt(4, 2); // C1-C2
    case 2: return getInt(6, 2); // D1-D2
    case 3: return getInt(8); // A1-An
  }
  return 0;
}

std::string
LCDUpdate::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3, 1); // L
    case 1: return mCommand.substr(4, 2); // C1-C2
    case 2: return mCommand.substr(6, 2); // D1-D2
    case 3: return mCommand.substr(8); // A1-An
  }
  return "";
}

std::string
LCDUpdate::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Line";
    case 1: return "Col";
    case 2: return "Len";
    case 3: return "Val";
  }
  return "";
}

bool registeredLCDUpdate = 
  Command::addCreator(Command::LCD_UPDATE,  LCDUpdate::create);

/** *******************************************************************
 * Command 902 (LCD Cursor)
 * Parameters: 4 bytes (T,L,C1-C2)
 *********************************************************************/

int
LCDCursor::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // T
    case 1: return getInt(4, 1); // L
    case 2: return getInt(5); // C1-C2
  }
  return 0;
}

std::string
LCDCursor::getStringParam(int number) const
{
  switch (number)
  {
    case 0:
      switch (getIntParam(0))
      {
        case 0: return "Off";
        case 1: return "Normal Underscore";
        case 2: return "Block";
        default: return mCommand.substr(3);
      }
    case 1: return mCommand.substr(4, 1); // L
    case 2: return mCommand.substr(5); // C1-C2
  }
  return "";
}

std::string
LCDCursor::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Type";
    case 1: return "Line";
    case 2: return "Column";
  }
  return "";
}

bool registeredLCDCursor = 
  Command::addCreator(Command::LCD_CURSOR,  LCDCursor::create);

/** *******************************************************************
 * Command 903 (LED Status)
 * Parameters: 2 bytes (L,S)
 *********************************************************************/

int
LEDStatus::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // L
    case 1: return getInt(4); // S
  }
  return 0;
}

std::string
LEDStatus::getStringParam(int number) const
{
  switch (number)
  {
    case 0:
      switch (getIntParam(0))
      {
        case 1: return "Ready";
        case 2: return "Armed";
        case 3: return "Memory";
        case 4: return "Bypass";
        case 5: return "Trouble";
        case 6: return "Program";
        case 7: return "Fire";
        case 8: return "Backlight";
        case 9: return "AC";
        default: return mCommand.substr(3,1);
      }
    case 1:
      switch (getIntParam(1))
      {
        case 0: return "Off";
        case 1: return "On";
        case 2: return "Flashing";
        default: return mCommand.substr(4);
      }
  }
  return "";
}

std::string
LEDStatus::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "LED";
    case 1: return "Status";
  }
  return "";
}

bool registeredLEDStatus = 
  Command::addCreator(Command::LED_STATUS,  LEDStatus::create);

/** *******************************************************************
 * Command 904 (Beep Status)
 * Parameters: 3 bytes (0-255 Beeps)
 *********************************************************************/

std::string
BeepStatus::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3); // 0-255 Beeps
  }
  return "";
}

std::string
BeepStatus::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Beeps";
  }
  return "";
}

bool registeredBeepStatus = 
  Command::addCreator(Command::BEEP_STATUS,  BeepStatus::create);

/** *******************************************************************
 * Command 905 (Tone Status)
 * Parameters: 4 bytes (C, B, I1-I2)
 *********************************************************************/

int
ToneStatus::getIntParam(int number) const
{
  switch (number)
  {
    case 0: return getInt(3, 1); // C
    case 1: return getInt(4, 1); // B
    case 2: return getInt(5); // I1-I2
  }
  return 0;
}

std::string
ToneStatus::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return getIntParam(0)?"On":"Off";
    case 1: return mCommand.substr(4, 1); // B
    case 2: return mCommand.substr(5); // I1-I2
  }
  return "";
}

std::string
ToneStatus::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Constant";
    case 1: return "Beeps";
    case 2: return "Interval";
  }
  return "";
}

bool registeredToneStatus = 
  Command::addCreator(Command::TONE_STATUS,  ToneStatus::create);

/** *******************************************************************
 * Command 906 (Buzzer Status)
 * Parameters: 3 bytes (000-255 secs)
 *********************************************************************/

std::string
BuzzerStatus::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3) + " secs"; // 000-255 secs
  }
  return "";
}

std::string
BuzzerStatus::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Duration";
  }
  return "";
}

bool registeredBuzzerStatus = 
  Command::addCreator(Command::BUZZER_STATUS,  BuzzerStatus::create);

/** *******************************************************************
 * Command 907 (Door Chime Status)
 *********************************************************************/

bool registeredDoorChimeStatus = 
  Command::addCreator(Command::DOOR_CHIME_STATUS,  DoorChimeStatus::create);

/** *******************************************************************
 * Command 908 (Software Version)
 * Parameters: 6 bytes (VVSSXX)
 *********************************************************************/

std::string
SoftwareVersion::getStringParam(int number) const
{
  switch (number)
  {
    case 0: return mCommand.substr(3,2) + "." +
                   mCommand.substr(5,2) + "." +
                   mCommand.substr(7,2); // VVSSXX
  }
  return "";
}

std::string
SoftwareVersion::getParamName(int number) const
{
  switch (number)
  {
    case 0: return "Version";
  }
  return "";
}

bool registeredSoftwareVersion = 
  Command::addCreator(Command::SOFTWARE_VERSION,  SoftwareVersion::create);

