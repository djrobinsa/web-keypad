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

#ifndef _COMMAND_H
#define _COMMAND_H 1

#include <string>
#include <ostream>
#include <stddef.h>
#include <map>

#include "It100.h"

class Command
{
  public:
    static Command *makeCommand(It100 &it100, std::string command);

    virtual ~Command(){;}

    virtual int getNumParams() const = 0;
    virtual int getIntParam(int number) const = 0;
    virtual std::string getStringParam(int number) const = 0;
    virtual std::string getParamName(int number) const = 0;
    virtual std::string getName() const = 0;
    virtual int getCommandNumber() const = 0;

    virtual void processStateChange() const {;}
    std::string getCommandWithChecksum() const;
    int getSyslogPriority() const;
    std::string getShellAction() const;

    virtual void dump(std::ostream &os) const;

  protected:
    Command(It100 &it100, std::string command)
      : mIt100(it100), mCommand(command) {;}

    int getInt(std::string::size_type index = 3,
               std::string::size_type length = std::string::npos) const;
    virtual bool displayParamName(int number) const { return false; }

    // tr1::unordered_map would be a better choice, but this is more
    // portable.
    typedef Command *(*creator_t)(It100&,std::string);
    static std::map<int,creator_t> &creators();

  protected:
    It100 &mIt100;
    std::string mCommand;

  public:
    /// THIS IS NOT FOR YOU.
    static bool addCreator(int cmdNum, creator_t f)
      { creators()[cmdNum] = f; return true; }

    typedef enum
    {
      POLL                                        = 0,
      STATUS_REQUEST                              = 1,
      LABELS_REQUEST                              = 2,
      SET_TIME_AND_DATE                           = 10,
      COMMAND_OUTPUT_CONTROL                      = 20,
      PARTITION_ARM_CONTROL_AWAY                  = 30,
      PARTITION_ARM_CONTROL_STAY                  = 31,
      PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY  = 32,
      PARTITION_ARM_CONTROL_WITH_CODE             = 33,
      PARTITION_DISARM_CONTROL_WITH_CODE          = 40,
      TIME_STAMP_CONTROL                          = 55,
      TIME_DATE_BROADCAST_CONTROL                 = 56,
      TEMPERATURE_BROADCAST_CONTROL               = 57,
      VIRTUAL_KEYPAD_CONTROL                      = 58,
      TRIGGER_PANIC_ALARM                         = 60,
      KEY_PRESSED                                 = 70,
      BAUD_RATE_CHANGE                            = 80,
      GET_TEMPERATURE_SET_POINT                   = 95,
      TEMPERATURE_CHANGE                          = 96,
      SAVE_TEMPERATURE_SETTING                    = 97,
      CODE_SEND                                   = 200,
      COMMAND_ACKNOWLEDGE                         = 500,
      COMMAND_ERROR                               = 501,
      SYSTEM_ERROR                                = 502,
      TIME_DATE_BROADCAST                         = 550,
      RING_DETECTED                               = 560,
      INDOOR_TEMPERATURE_BROADCAST                = 561,
      OUTDOOR_TEMPERATRURE_BROADCAST              = 562,
      THERMOSTAT_SET_POINTS                       = 563,
      BROADCAST_LABELS                            = 570,
      BAUD_RATE_SET                               = 580,
      ZONE_ALARM                                  = 601,
      ZONE_ALARM_RESTORE                          = 602,
      ZONE_TAMPER                                 = 603,
      ZONE_TAMPER_RESTORE                         = 604,
      ZONE_FAULT                                  = 605,
      ZONE_FAULT_RESTORE                          = 606,
      ZONE_OPEN                                   = 609,
      ZONE_RESTORED                               = 610,
      DURESS_ALARM                                = 620,
      F_KEY_ALARM                                 = 621,
      F_KEY_RESTORAL                              = 622,
      A_KEY_ALARM                                 = 623,
      A_KEY_RESTORAL                              = 624,
      P_KEY_ALARM                                 = 625,
      P_KEY_RESTORAL                              = 626,
      AUXILIARY_INPUT_ALARM                       = 631,
      AUXILIARY_INPUT_ALARM_RESTORED              = 632,
      PARTITION_READY                             = 650,
      PARTITION_NOT_READY                         = 651,
      PARTITION_ARMED_DESCRIPTIVE_MODE            = 652,
      PARTITION_IN_READY_TO_FORCE_ARM             = 653,
      PARTITION_IN_ALARM                          = 654,
      PARTITION_DISARMED                          = 655,
      EXIT_DELAY_IN_PROGRESS                      = 656,
      ENTRY_DELAY_IN_PROGRESS                     = 657,
      KEYPAD_LOCKOUT                              = 658,
      KEYPAD_BLANKING                             = 659,
      COMMAND_OUTPUT_IN_PROGRESS                  = 660,
      INVALID_ACCESS_CODE                         = 670,
      FUNCTION_NOT_AVAILABLE                      = 671,
      FAIL_TO_ARM                                 = 672,
      PARTITION_BUSY                              = 673,
      USER_CLOSING                                = 700,
      SPECIAL_CLOSING                             = 701,
      PARTIAL_CLOSING                             = 702,
      USER_OPENING                                = 750,
      SPECIAL_OPENING                             = 751,
      PANEL_BATTERY_TROUBLE                       = 800,
      PANEL_BATTERY_TROUBLE_RESTORE               = 801,
      PANEL_AC_TROUBLE                            = 802,
      PANEL_AC_RESTORE                            = 803,
      SYSTEM_BELL_TROUBLE                         = 806,
      SYSTEM_BELL_TROUBLE_RESTORAL                = 807,
      TLM_LINE_1_TROUBLE                          = 810,
      TLM_LINE_1_TROUBLE_RESTORED                 = 811,
      TLM_LINE_2_TROUBLE                          = 812,
      TLM_LINE_2_TROUBLE_RESTORED                 = 813,
      FTC_TROUBLE                                 = 814,
      BUFFER_NEAR_FULL                            = 816,
      GENERAL_DEVICE_LOW_BATTERY                  = 821,
      GENERAL_DEVICE_LOW_BATTERY_RESTORE          = 822,
      WIRELESS_KEY_LOW_BATTERY_TROUBLE            = 825,
      WIRELESS_KEY_LOW_BATTERY_TROUBLE_RESTORE    = 826,
      HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE         = 827,
      HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE_RESTORE = 828,
      GENERAL_SYSTEM_TAMPER                       = 829,
      GENERAL_SYSTEM_TAMPER_RESTORE               = 830,
      HOME_AUTOMATION_TROUBLE                     = 831,
      HOME_AUTOMATION_TROUBLE_RESTORE             = 832,
      TROUBLE_STATUS_LED_ON                       = 840,
      TROUBLE_STATUS_RESTORE_LED_OFF              = 841,
      FIRE_TROUBLE_ALARM                          = 842,
      FIRE_TROUBLE_ALARM_RESTORED                 = 843,
      CODE_REQUIRED                               = 900,
      LCD_UPDATE                                  = 901,
      LCD_CURSOR                                  = 902,
      LED_STATUS                                  = 903,
      BEEP_STATUS                                 = 904,
      TONE_STATUS                                 = 905,
      BUZZER_STATUS                               = 906,
      DOOR_CHIME_STATUS                           = 907,
      SOFTWARE_VERSION                            = 908
    } command_t;
};

std::ostream &operator<<(std::ostream &, const Command &);

class CommandWithNoParameters : public Command
{
  public:
    virtual int getNumParams() const { return 0; }
    virtual int getIntParam(int number) const { return 0; }
    virtual std::string getStringParam(int number) const { return ""; }
    virtual std::string getParamName(int number) const { return ""; }

  protected:
    CommandWithNoParameters(It100 &it100, std::string command)
            : Command(it100, command){;}
};

/** *******************************************************************
 * Command 0 (Poll) - Sent to alarm system
 *********************************************************************/

class Poll : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new Poll(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Poll"; }
    virtual int getCommandNumber() const { return POLL; }

  protected:
    Poll(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 1 (Status Request) - Sent to alarm system
 *********************************************************************/

class StatusRequest : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new StatusRequest(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Status Request"; }
    virtual int getCommandNumber() const { return STATUS_REQUEST; }

  protected:
    StatusRequest(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 2 (Labels Request) - Sent to alarm system
 *********************************************************************/

class LabelsRequest : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new LabelsRequest(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Labels Request"; }
    virtual int getCommandNumber() const { return LABELS_REQUEST; }

  protected:
    LabelsRequest(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 10 (Set Time and Date) - Sent to alarm system
 * Parameters: 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

class SetTimeAndDate : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SetTimeAndDate(it100, command); }

    /* 5 parameters, 10 bytes -- (hh,mm,MM,DD,YY) */
    virtual int getNumParams() const { return 5; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Set Time and Date"; }
    virtual int getCommandNumber() const { return SET_TIME_AND_DATE; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    SetTimeAndDate(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 20 (Command Output Control) - Sent to alarm system
 * Parameters: 2 bytes (Part 1-8 , Pgm 1-4)
 *********************************************************************/

class CommandOutputControl : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CommandOutputControl(it100, command); }

    /* 2 parameters, 2 bytes -- (Part 1-8 , Pgm 1-4) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Command Output Control"; }
    virtual int getCommandNumber() const { return COMMAND_OUTPUT_CONTROL; }

  protected:
    CommandOutputControl(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 30 (Partition Arm Control - Away) - Sent to alarm system
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionArmControlAway : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionArmControlAway(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Arm Control - Away"; }
    virtual int getCommandNumber() const { return PARTITION_ARM_CONTROL_AWAY; }

  protected:
    PartitionArmControlAway(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 31 (Partition Arm Control - Stay) - Sent to alarm system
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionArmControlStay : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionArmControlStay(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Arm Control - Stay"; }
    virtual int getCommandNumber() const { return PARTITION_ARM_CONTROL_STAY; }

  protected:
    PartitionArmControlStay(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 32 (Partition Arm Control - Armed, No Entry Delay) - Sent to alarm system
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionArmControlArmedNoEntryDelay : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionArmControlArmedNoEntryDelay(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Arm Control - Armed, No Entry Delay"; }
    virtual int getCommandNumber() const { return PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY; }

  protected:
    PartitionArmControlArmedNoEntryDelay(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 33 (Partition Arm Control - With Code) - Sent to alarm system
 * Parameters: 7 bytes (Part 1-8 , Code 6 bytes h)
 *********************************************************************/

class PartitionArmControlWithCode : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionArmControlWithCode(it100, command); }

    /* 2 parameters, 7 bytes -- (Part 1-8 , Code 6 bytes h) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Arm Control - With Code"; }
    virtual int getCommandNumber() const { return PARTITION_ARM_CONTROL_WITH_CODE; }

  protected:
    PartitionArmControlWithCode(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 40 (Partition Disarm Control - With Code) - Sent to alarm system
 * Parameters: 7 bytes (Part 1-8 , Code 6 bytes h)
 *********************************************************************/

class PartitionDisarmControlWithCode : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionDisarmControlWithCode(it100, command); }

    /* 2 parameters, 7 bytes -- (Part 1-8 , Code 6 bytes h) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Disarm Control - With Code"; }
    virtual int getCommandNumber() const { return PARTITION_DISARM_CONTROL_WITH_CODE; }

  protected:
    PartitionDisarmControlWithCode(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 55 (Time Stamp Control) - Sent to alarm system
 * Parameters: 1 bytes (On/Off )
 *********************************************************************/

class TimeStampControl : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TimeStampControl(it100, command); }

    /* 1 parameter, 1 bytes -- (On/Off ) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Time Stamp Control"; }
    virtual int getCommandNumber() const { return TIME_STAMP_CONTROL; }

  protected:
    TimeStampControl(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 56 (Time/Date Broadcast Control) - Sent to alarm system
 * Parameters: 1 bytes (On/Off  )
 *********************************************************************/

class TimeDateBroadcastControl : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TimeDateBroadcastControl(it100, command); }

    /* 1 parameter, 1 bytes -- (On/Off  ) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Time/Date Broadcast Control"; }
    virtual int getCommandNumber() const { return TIME_DATE_BROADCAST_CONTROL; }

  protected:
    TimeDateBroadcastControl(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 57 (Temperature Broadcast Control) - Sent to alarm system
 * Parameters: 1 bytes (On/Off  )
 *********************************************************************/

class TemperatureBroadcastControl : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TemperatureBroadcastControl(it100, command); }

    /* 1 parameter, 1 bytes -- (On/Off  ) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Temperature Broadcast Control"; }
    virtual int getCommandNumber() const { return TEMPERATURE_BROADCAST_CONTROL; }

  protected:
    TemperatureBroadcastControl(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 58 (Virtual Keypad Control) - Sent to alarm system
 * Parameters: 1 bytes (On/Off  )
 *********************************************************************/

class VirtualKeypadControl : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new VirtualKeypadControl(it100, command); }

    /* 1 parameter, 1 bytes -- (On/Off  ) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Virtual Keypad Control"; }
    virtual int getCommandNumber() const { return VIRTUAL_KEYPAD_CONTROL; }

  protected:
    VirtualKeypadControl(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 60 (Trigger Panic Alarm) - Sent to alarm system
 * Parameters: 1 bytes (1 = F; 2 = A; 3 = P)
 *********************************************************************/

class TriggerPanicAlarm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TriggerPanicAlarm(it100, command); }

    /* 1 parameter, 1 bytes -- (1 = F; 2 = A; 3 = P) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Trigger Panic Alarm"; }
    virtual int getCommandNumber() const { return TRIGGER_PANIC_ALARM; }

  protected:
    TriggerPanicAlarm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 70 (Key Pressed) - Sent to alarm system
 * Parameters: 1 bytes (Key)
 *********************************************************************/

class KeyPressed : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new KeyPressed(it100, command); }

    /* 1 parameter, 1 bytes -- (Key) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Key Pressed"; }
    virtual int getCommandNumber() const { return KEY_PRESSED; }

  protected:
    KeyPressed(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 80 (Baud Rate Change) - Sent to alarm system
 * Parameters: 1 bytes (Val 0 - 4)
 *********************************************************************/

class BaudRateChange : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BaudRateChange(it100, command); }

    /* 1 parameter, 1 bytes -- (Val 0 - 4) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Baud Rate Change"; }
    virtual int getCommandNumber() const { return BAUD_RATE_CHANGE; }

  protected:
    BaudRateChange(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 95 (Get Temperature Set Point) - Sent to alarm system
 * Parameters: 1 bytes (Val 1 - 4)
 *********************************************************************/

class GetTemperatureSetPoint : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new GetTemperatureSetPoint(it100, command); }

    /* 1 parameter, 1 bytes -- (Val 1 - 4) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Get Temperature Set Point"; }
    virtual int getCommandNumber() const { return GET_TEMPERATURE_SET_POINT; }

  protected:
    GetTemperatureSetPoint(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 96 (Temperature Change) - Sent to alarm system
 * Parameters: 8 bytes (T,S,M,A1-A3)
 *********************************************************************/

class TemperatureChange : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TemperatureChange(it100, command); }

    /* 4 parameters, 8 bytes -- (T,S,M,A1-A3) */
    virtual int getNumParams() const { return 4; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Temperature Change"; }
    virtual int getCommandNumber() const { return TEMPERATURE_CHANGE; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    TemperatureChange(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 97 (Save Temperature Setting) - Sent to alarm system
 * Parameters: 1 bytes (Val 1 - 4)
 *********************************************************************/

class SaveTemperatureSetting : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SaveTemperatureSetting(it100, command); }

    /* 1 parameter, 1 bytes -- (Val 1 - 4) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Save Temperature Setting"; }
    virtual int getCommandNumber() const { return SAVE_TEMPERATURE_SETTING; }

  protected:
    SaveTemperatureSetting(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 200 (Code Send) - Sent to alarm system
 * Parameters: 6 bytes (Access Code in hex ASCII)
 *********************************************************************/

class CodeSend : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CodeSend(it100, command); }

    /* 1 parameter, 6 bytes -- (Access Code in hex ASCII) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Code Send"; }
    virtual int getCommandNumber() const { return CODE_SEND; }

  protected:
    CodeSend(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 500 (Command Acknowledge)
 * Parameters: 3 bytes (CMD received in Hex ASCII)
 *********************************************************************/

class CommandAcknowledge : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CommandAcknowledge(it100, command); }

    /* 1 parameter, 3 bytes -- (CMD received in Hex ASCII) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Command Acknowledge"; }
    virtual int getCommandNumber() const { return COMMAND_ACKNOWLEDGE; }

    virtual void processStateChange() const {mIt100.sendPendingCommand();}

  protected:
    CommandAcknowledge(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 501 (Command Error)
 *********************************************************************/

class CommandError : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CommandError(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Command Error"; }
    virtual int getCommandNumber() const { return COMMAND_ERROR; }

  protected:
    CommandError(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 502 (System Error)
 * Parameters: 3 bytes (Error Code in Hex ASCII)
 *********************************************************************/

class SystemError : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SystemError(it100, command); }

    /* 1 parameter, 3 bytes -- (Error Code in Hex ASCII) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "System Error"; }
    virtual int getCommandNumber() const { return SYSTEM_ERROR; }

  protected:
    SystemError(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 550 (Time/Date Broadcast)
 * Parameters: 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

class TimeDateBroadcast : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TimeDateBroadcast(it100, command); }

    /* 5 parameters, 10 bytes -- (hh,mm,MM,DD,YY) */
    virtual int getNumParams() const { return 5; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Time/Date Broadcast"; }
    virtual int getCommandNumber() const { return TIME_DATE_BROADCAST; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    TimeDateBroadcast(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 560 (Ring Detected)
 * Parameters: 10 bytes (hh,mm,MM,DD,YY)
 *********************************************************************/

class RingDetected : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new RingDetected(it100, command); }

    /* 0 parameters - (Note IT100 doc is wrong and says 10 bytes -- (hh,mm,MM,DD,YY)) */
    virtual int getNumParams() const { return 0; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Ring Detected"; }
    virtual int getCommandNumber() const { return RING_DETECTED; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    RingDetected(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 561 (Indoor Temperature Broadcast)
 * Parameters: 4 bytes (Thermostat , Temp)
 *********************************************************************/

class IndoorTemperatureBroadcast : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new IndoorTemperatureBroadcast(it100, command); }

    /* 2 parameters, 4 bytes -- (Thermostat , Temp) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Indoor Temperature Broadcast"; }
    virtual int getCommandNumber() const { return INDOOR_TEMPERATURE_BROADCAST; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    IndoorTemperatureBroadcast(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 562 (Outdoor Temperatrure Broadcast)
 * Parameters: 4 bytes (Thermostat ,Temp)
 *********************************************************************/

class OutdoorTemperatrureBroadcast : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new OutdoorTemperatrureBroadcast(it100, command); }

    /* 2 parameters, 4 bytes -- (Thermostat ,Temp) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Outdoor Temperatrure Broadcast"; }
    virtual int getCommandNumber() const { return OUTDOOR_TEMPERATRURE_BROADCAST; }

    virtual bool displayParamName(int number) const { return true; }

protected:
    OutdoorTemperatrureBroadcast(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 563 (Thermostat Set Points)
 * Parameters: 8 bytes (Thermostat,C1-C3, H1-H3)
 *********************************************************************/

class ThermostatSetPoints : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ThermostatSetPoints(it100, command); }

    /* 3 parameters, 8 bytes -- (Thermostat,C1-C3, H1-H3) */
    virtual int getNumParams() const { return 3; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Thermostat Set Points"; }
    virtual int getCommandNumber() const { return THERMOSTAT_SET_POINTS; }

    virtual bool displayParamName(int number) const { return true; }

  protected:
    ThermostatSetPoints(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 570 (Broadcast Labels)
 * Parameters: 35 bytes (Lbl# 3, Lbl 32 Bytes)
 *********************************************************************/

class BroadcastLabels : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BroadcastLabels(it100, command); }

    /* 2 parameters, 35 bytes -- (Lbl# 3, Lbl 32 Bytes) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Broadcast Labels"; }
    virtual int getCommandNumber() const { return BROADCAST_LABELS; }

    virtual void processStateChange() const
      {mIt100.setLabel(getIntParam(0), getStringParam(1));}

  protected:
    BroadcastLabels(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 580 (Baud Rate Set)
 * Parameters: 1 bytes (Val = 0- 4)
 *********************************************************************/

class BaudRateSet : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BaudRateSet(it100, command); }

    /* 1 parameter, 1 bytes -- (Val = 0- 4) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Baud Rate Set"; }
    virtual int getCommandNumber() const { return BAUD_RATE_SET; }

  protected:
    BaudRateSet(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 601 (Zone Alarm)
 * Parameters: 4 bytes (Partition. 1-8, Zn 1-64)
 *********************************************************************/

class ZoneAlarm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneAlarm(it100, command); }

    /* 2 parameters, 4 bytes -- (Partition. 1-8, Zn 1-64) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Alarm"; }
    virtual int getCommandNumber() const { return ZONE_ALARM; }

  protected:
    ZoneAlarm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 602 (Zone Alarm Restore)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

class ZoneAlarmRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneAlarmRestore(it100, command); }

    /* 2 parameters, 4 bytes -- (Part. 1-8, Zn 1-64) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Alarm Restore"; }
    virtual int getCommandNumber() const { return ZONE_ALARM_RESTORE; }

  protected:
    ZoneAlarmRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 603 (Zone Tamper)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

class ZoneTamper : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneTamper(it100, command); }

    /* 2 parameters, 4 bytes -- (Part. 1-8, Zn 1-64) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Tamper"; }
    virtual int getCommandNumber() const { return ZONE_TAMPER; }

  protected:
    ZoneTamper(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 604 (Zone Tamper Restore)
 * Parameters: 4 bytes (Part. 1-8, Zn 1-64)
 *********************************************************************/

class ZoneTamperRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneTamperRestore(it100, command); }

    /* 2 parameters, 4 bytes -- (Part. 1-8, Zn 1-64) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Tamper Restore"; }
    virtual int getCommandNumber() const { return ZONE_TAMPER_RESTORE; }

  protected:
    ZoneTamperRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 605 (Zone Fault)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

class ZoneFault : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneFault(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 1-64) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Fault"; }
    virtual int getCommandNumber() const { return ZONE_FAULT; }

  protected:
    ZoneFault(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 606 (Zone Fault Restore)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

class ZoneFaultRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneFaultRestore(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 1-64) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Fault Restore"; }
    virtual int getCommandNumber() const { return ZONE_FAULT_RESTORE; }

  protected:
    ZoneFaultRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 609 (Zone Open)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

class ZoneOpen : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneOpen(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 1-64) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Open"; }
    virtual int getCommandNumber() const { return ZONE_OPEN; }

    virtual void processStateChange() const
      {mIt100.setZoneOpen(getIntParam(0), true);}

  protected:
    ZoneOpen(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 610 (Zone Restored)
 * Parameters: 3 bytes (Zn 1-64)
 *********************************************************************/

class ZoneRestored : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ZoneRestored(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 1-64) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Zone Restored"; }
    virtual int getCommandNumber() const { return ZONE_RESTORED; }

    virtual void processStateChange() const
      {mIt100.setZoneOpen(getIntParam(0), false);}

  protected:
    ZoneRestored(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 620 (Duress Alarm)
 * Parameters: 4 bytes (0000)
 *********************************************************************/

class DuressAlarm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new DuressAlarm(it100, command); }

    /* 1 parameter, 4 bytes -- (0000) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Duress Alarm"; }
    virtual int getCommandNumber() const { return DURESS_ALARM; }

  protected:
    DuressAlarm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 621 ([F] Key Alarm)
 *********************************************************************/

class FKeyAlarm : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FKeyAlarm(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[F] Key Alarm"; }
    virtual int getCommandNumber() const { return F_KEY_ALARM; }

  protected:
    FKeyAlarm(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 622 ([F] Key Restoral)
 *********************************************************************/

class FKeyRestoral : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FKeyRestoral(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[F] Key Restoral"; }
    virtual int getCommandNumber() const { return F_KEY_RESTORAL; }

  protected:
    FKeyRestoral(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 623 ([A] Key Alarm)
 *********************************************************************/

class AKeyAlarm : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new AKeyAlarm(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[A] Key Alarm"; }
    virtual int getCommandNumber() const { return A_KEY_ALARM; }

  protected:
    AKeyAlarm(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 624 ([A] Key Restoral)
 *********************************************************************/

class AKeyRestoral : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new AKeyRestoral(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[A] Key Restoral"; }
    virtual int getCommandNumber() const { return A_KEY_RESTORAL; }

  protected:
    AKeyRestoral(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 625 ([P] Key Alarm)
 *********************************************************************/

class PKeyAlarm : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PKeyAlarm(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[P] Key Alarm"; }
    virtual int getCommandNumber() const { return P_KEY_ALARM; }

  protected:
    PKeyAlarm(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 626 ([P] Key Restoral)
 *********************************************************************/

class PKeyRestoral : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PKeyRestoral(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "[P] Key Restoral"; }
    virtual int getCommandNumber() const { return P_KEY_RESTORAL; }

  protected:
    PKeyRestoral(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 631 (Auxiliary Input Alarm)
 *********************************************************************/

class AuxiliaryInputAlarm : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new AuxiliaryInputAlarm(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Auxiliary Input Alarm"; }
    virtual int getCommandNumber() const { return AUXILIARY_INPUT_ALARM; }

  protected:
    AuxiliaryInputAlarm(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 632 (Auxiliary Input Alarm Restored)
 *********************************************************************/

class AuxiliaryInputAlarmRestored : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new AuxiliaryInputAlarmRestored(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Auxiliary Input Alarm Restored"; }
    virtual int getCommandNumber() const { return AUXILIARY_INPUT_ALARM_RESTORED; }

  protected:
    AuxiliaryInputAlarmRestored(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 650 (Partition Ready)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionReady : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionReady(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Ready"; }
    virtual int getCommandNumber() const { return PARTITION_READY; }

  protected:
    PartitionReady(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 651 (Partition Not Ready)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionNotReady : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionNotReady(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Not Ready"; }
    virtual int getCommandNumber() const { return PARTITION_NOT_READY; }

  protected:
    PartitionNotReady(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 652 (Partition Armed - Descriptive Mode)
 * Parameters: 2 bytes (Partition 1 - 8 , Mode)
 *********************************************************************/

class PartitionArmedDescriptiveMode : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionArmedDescriptiveMode(it100, command); }

    /* 2 parameters, 2 bytes -- (Partition 1 - 8 , Mode) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Armed - Descriptive Mode"; }
    virtual int getCommandNumber() const { return PARTITION_ARMED_DESCRIPTIVE_MODE; }

  protected:
    PartitionArmedDescriptiveMode(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 653 (Partition in Ready to Force Arm)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitioninReadytoForceArm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitioninReadytoForceArm(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition in Ready to Force Arm"; }
    virtual int getCommandNumber() const { return PARTITION_IN_READY_TO_FORCE_ARM; }

  protected:
    PartitioninReadytoForceArm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 654 (Partition In Alarm)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionInAlarm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionInAlarm(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition In Alarm"; }
    virtual int getCommandNumber() const { return PARTITION_IN_ALARM; }

  protected:
    PartitionInAlarm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 655 (Partition Disarmed)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionDisarmed : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionDisarmed(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Disarmed"; }
    virtual int getCommandNumber() const { return PARTITION_DISARMED; }

  protected:
    PartitionDisarmed(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 656 (Exit Delay in Progress)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class ExitDelayinProgress : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ExitDelayinProgress(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Exit Delay in Progress"; }
    virtual int getCommandNumber() const { return EXIT_DELAY_IN_PROGRESS; }

  protected:
    ExitDelayinProgress(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 657 (Entry Delay in Progress)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class EntryDelayinProgress : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new EntryDelayinProgress(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Entry Delay in Progress"; }
    virtual int getCommandNumber() const { return ENTRY_DELAY_IN_PROGRESS; }

  protected:
    EntryDelayinProgress(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 658 (Keypad Lock-out)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class KeypadLockout : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new KeypadLockout(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Keypad Lock-out"; }
    virtual int getCommandNumber() const { return KEYPAD_LOCKOUT; }

  protected:
    KeypadLockout(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 659 (Keypad Blanking)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class KeypadBlanking : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new KeypadBlanking(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Keypad Blanking"; }
    virtual int getCommandNumber() const { return KEYPAD_BLANKING; }

  protected:
    KeypadBlanking(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 660 (Command Output In Progress)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class CommandOutputInProgress : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CommandOutputInProgress(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Command Output In Progress"; }
    virtual int getCommandNumber() const { return COMMAND_OUTPUT_IN_PROGRESS; }

  protected:
    CommandOutputInProgress(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 670 (Invalid Access Code)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class InvalidAccessCode : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new InvalidAccessCode(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Invalid Access Code"; }
    virtual int getCommandNumber() const { return INVALID_ACCESS_CODE; }

  protected:
    InvalidAccessCode(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 671 (Function Not Available)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class FunctionNotAvailable : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FunctionNotAvailable(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Function Not Available"; }
    virtual int getCommandNumber() const { return FUNCTION_NOT_AVAILABLE; }

  protected:
    FunctionNotAvailable(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 672 (Fail to Arm)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class FailtoArm : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FailtoArm(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Fail to Arm"; }
    virtual int getCommandNumber() const { return FAIL_TO_ARM; }

  protected:
    FailtoArm(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 673 (Partition Busy)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartitionBusy : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartitionBusy(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partition Busy"; }
    virtual int getCommandNumber() const { return PARTITION_BUSY; }

  protected:
    PartitionBusy(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 700 (User Closing)
 * Parameters: 5 bytes (Part 1-8 , User Code)
 *********************************************************************/

class UserClosing : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new UserClosing(it100, command); }

    /* 2 parameters, 5 bytes -- (Part 1-8 , User Code) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "User Closing"; }
    virtual int getCommandNumber() const { return USER_CLOSING; }

  protected:
    UserClosing(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 701 (Special Closing)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class SpecialClosing : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SpecialClosing(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Special Closing"; }
    virtual int getCommandNumber() const { return SPECIAL_CLOSING; }

  protected:
    SpecialClosing(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 702 (Partial Closing)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class PartialClosing : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PartialClosing(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Partial Closing"; }
    virtual int getCommandNumber() const { return PARTIAL_CLOSING; }

  protected:
    PartialClosing(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 750 (User Opening)
 * Parameters: 5 bytes (Part 1-8 , User Code)
 *********************************************************************/

class UserOpening : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new UserOpening(it100, command); }

    /* 2 parameters, 5 bytes -- (Part 1-8 , User Code) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "User Opening"; }
    virtual int getCommandNumber() const { return USER_OPENING; }

  protected:
    UserOpening(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 751 (Special Opening)
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class SpecialOpening : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SpecialOpening(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Special Opening"; }
    virtual int getCommandNumber() const { return SPECIAL_OPENING; }

  protected:
    SpecialOpening(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 800 (Panel Battery Trouble)
 *********************************************************************/

class PanelBatteryTrouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PanelBatteryTrouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Panel Battery Trouble"; }
    virtual int getCommandNumber() const { return PANEL_BATTERY_TROUBLE; }

  protected:
    PanelBatteryTrouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 801 (Panel Battery Trouble Restore)
 *********************************************************************/

class PanelBatteryTroubleRestore : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PanelBatteryTroubleRestore(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Panel Battery Trouble Restore"; }
    virtual int getCommandNumber() const { return PANEL_BATTERY_TROUBLE_RESTORE; }

  protected:
    PanelBatteryTroubleRestore(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 802 (Panel AC Trouble)
 *********************************************************************/

class PanelACTrouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PanelACTrouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Panel AC Trouble"; }
    virtual int getCommandNumber() const { return PANEL_AC_TROUBLE; }

  protected:
    PanelACTrouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 803 (Panel AC Restore)
 *********************************************************************/

class PanelACRestore : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new PanelACRestore(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Panel AC Restore"; }
    virtual int getCommandNumber() const { return PANEL_AC_RESTORE; }

  protected:
    PanelACRestore(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 806 (System Bell Trouble)
 *********************************************************************/

class SystemBellTrouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SystemBellTrouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "System Bell Trouble"; }
    virtual int getCommandNumber() const { return SYSTEM_BELL_TROUBLE; }

  protected:
    SystemBellTrouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 807 (System Bell Trouble Restoral)
 *********************************************************************/

class SystemBellTroubleRestoral : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SystemBellTroubleRestoral(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "System Bell Trouble Restoral"; }
    virtual int getCommandNumber() const { return SYSTEM_BELL_TROUBLE_RESTORAL; }

  protected:
    SystemBellTroubleRestoral(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 810 (TLM Line 1 Trouble)
 *********************************************************************/

class TLMLine1Trouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TLMLine1Trouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "TLM Line 1 Trouble"; }
    virtual int getCommandNumber() const { return TLM_LINE_1_TROUBLE; }

  protected:
    TLMLine1Trouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 811 (TLM Line 1 Trouble Restored)
 *********************************************************************/

class TLMLine1TroubleRestored : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TLMLine1TroubleRestored(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "TLM Line 1 Trouble Restored"; }
    virtual int getCommandNumber() const { return TLM_LINE_1_TROUBLE_RESTORED; }

  protected:
    TLMLine1TroubleRestored(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 812 (TLM Line 2 Trouble)
 *********************************************************************/

class TLMLine2Trouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TLMLine2Trouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "TLM Line 2 Trouble"; }
    virtual int getCommandNumber() const { return TLM_LINE_2_TROUBLE; }

  protected:
    TLMLine2Trouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 813 (TLM Line 2 Trouble Restored)
 *********************************************************************/

class TLMLine2TroubleRestored : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TLMLine2TroubleRestored(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "TLM Line 2 Trouble Restored"; }
    virtual int getCommandNumber() const { return TLM_LINE_2_TROUBLE_RESTORED; }

  protected:
    TLMLine2TroubleRestored(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 814 (FTC Trouble)
 *********************************************************************/

class FTCTrouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FTCTrouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "FTC Trouble"; }
    virtual int getCommandNumber() const { return FTC_TROUBLE; }

  protected:
    FTCTrouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 816 (Buffer Near Full)
 *********************************************************************/

class BufferNearFull : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BufferNearFull(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Buffer Near Full"; }
    virtual int getCommandNumber() const { return BUFFER_NEAR_FULL; }

  protected:
    BufferNearFull(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 821 (General Device Low Battery)
 * Parameters: 3 bytes (Zn 001-032)
 *********************************************************************/

class GeneralDeviceLowBattery : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new GeneralDeviceLowBattery(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 001-032) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "General Device Low Battery"; }
    virtual int getCommandNumber() const { return GENERAL_DEVICE_LOW_BATTERY; }

  protected:
    GeneralDeviceLowBattery(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 822 (General Device Low Battery Restore)
 * Parameters: 3 bytes (Zn 001-032)
 *********************************************************************/

class GeneralDeviceLowBatteryRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new GeneralDeviceLowBatteryRestore(it100, command); }

    /* 1 parameter, 3 bytes -- (Zn 001-032) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "General Device Low Battery Restore"; }
    virtual int getCommandNumber() const { return GENERAL_DEVICE_LOW_BATTERY_RESTORE; }

  protected:
    GeneralDeviceLowBatteryRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 825 (Wireless Key Low Battery Trouble)
 * Parameters: 3 bytes (001-016)
 *********************************************************************/

class WirelessKeyLowBatteryTrouble : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new WirelessKeyLowBatteryTrouble(it100, command); }

    /* 1 parameter, 3 bytes -- (001-016) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Wireless Key Low Battery Trouble"; }
    virtual int getCommandNumber() const { return WIRELESS_KEY_LOW_BATTERY_TROUBLE; }

  protected:
    WirelessKeyLowBatteryTrouble(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 826 (Wireless Key Low Battery Trouble Restore)
 * Parameters: 3 bytes (001-016)
 *********************************************************************/

class WirelessKeyLowBatteryTroubleRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new WirelessKeyLowBatteryTroubleRestore(it100, command); }

    /* 1 parameter, 3 bytes -- (001-016) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Wireless Key Low Battery Trouble Restore"; }
    virtual int getCommandNumber() const { return WIRELESS_KEY_LOW_BATTERY_TROUBLE_RESTORE; }

  protected:
    WirelessKeyLowBatteryTroubleRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 827 (Handheld Keypad Low Battery Trouble)
 * Parameters: 3 bytes (001-004)
 *********************************************************************/

class HandheldKeypadLowBatteryTrouble : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new HandheldKeypadLowBatteryTrouble(it100, command); }

    /* 1 parameter, 3 bytes -- (001-004) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Handheld Keypad Low Battery Trouble"; }
    virtual int getCommandNumber() const { return HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE; }

  protected:
    HandheldKeypadLowBatteryTrouble(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 828 (Handheld Keypad Low Battery Trouble Restore)
 * Parameters: 3 bytes (001-004)
 *********************************************************************/

class HandheldKeypadLowBatteryTroubleRestore : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new HandheldKeypadLowBatteryTroubleRestore(it100, command); }

    /* 1 parameter, 3 bytes -- (001-004) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Handheld Keypad Low Battery Restore"; }
    virtual int getCommandNumber() const { return HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE_RESTORE; }

  protected:
    HandheldKeypadLowBatteryTroubleRestore(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 829 (General System Tamper)
 *********************************************************************/

class GeneralSystemTamper : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new GeneralSystemTamper(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "General System Tamper"; }
    virtual int getCommandNumber() const { return GENERAL_SYSTEM_TAMPER; }

  protected:
    GeneralSystemTamper(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 830 (General System Tamper Restore)
 *********************************************************************/

class GeneralSystemTamperRestore : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new GeneralSystemTamperRestore(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "General System Tamper Restore"; }
    virtual int getCommandNumber() const { return GENERAL_SYSTEM_TAMPER_RESTORE; }

  protected:
    GeneralSystemTamperRestore(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 831 (Home Automation Trouble)
 *********************************************************************/

class HomeAutomationTrouble : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new HomeAutomationTrouble(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Home Automation Trouble"; }
    virtual int getCommandNumber() const { return HOME_AUTOMATION_TROUBLE; }

  protected:
    HomeAutomationTrouble(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 832 (Home Automation Trouble Restore)
 *********************************************************************/

class HomeAutomationTroubleRestore : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new HomeAutomationTroubleRestore(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Home Automation Trouble Restore"; }
    virtual int getCommandNumber() const { return HOME_AUTOMATION_TROUBLE_RESTORE; }

  protected:
    HomeAutomationTroubleRestore(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 840 (Trouble Status (LED ON))
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class TroubleStatusLEDON : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TroubleStatusLEDON(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Trouble Status (LED ON)"; }
    virtual int getCommandNumber() const { return TROUBLE_STATUS_LED_ON; }

  protected:
    TroubleStatusLEDON(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 841 (Trouble Status Restore (LED OFF))
 * Parameters: 1 bytes (Partition 1-8)
 *********************************************************************/

class TroubleStatusRestoreLEDOFF : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new TroubleStatusRestoreLEDOFF(it100, command); }

    /* 1 parameter, 1 bytes -- (Partition 1-8) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Trouble Status Restore (LED OFF)"; }
    virtual int getCommandNumber() const { return TROUBLE_STATUS_RESTORE_LED_OFF; }

  protected:
    TroubleStatusRestoreLEDOFF(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 842 (Fire Trouble Alarm)
 *********************************************************************/

class FireTroubleAlarm : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FireTroubleAlarm(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Fire Trouble Alarm"; }
    virtual int getCommandNumber() const { return FIRE_TROUBLE_ALARM; }

  protected:
    FireTroubleAlarm(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 843 (Fire Trouble Alarm Restored)
 *********************************************************************/

class FireTroubleAlarmRestored : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new FireTroubleAlarmRestored(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Fire Trouble Alarm Restored"; }
    virtual int getCommandNumber() const { return FIRE_TROUBLE_ALARM_RESTORED; }

  protected:
    FireTroubleAlarmRestored(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 900 (Code Required)
 * Parameters: 2 bytes (Part , Code length 6)
 *********************************************************************/

class CodeRequired : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new CodeRequired(it100, command); }

    /* 2 parameters, 2 bytes -- (Part , Code length 6) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Code Required"; }
    virtual int getCommandNumber() const { return CODE_REQUIRED; }

    virtual void processStateChange() const
      {mIt100.sendAccessCode(getIntParam(0), getIntParam(1));}

  protected:
    CodeRequired(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 901 (LCD Update)
 * Parameters: 6-37 bytes (L,C1-C2, D1-D2, A1-An)
 *********************************************************************/

class LCDUpdate : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new LCDUpdate(it100, command); }

    /* 4 parameters, 6-37 bytes -- (L,C1-C2, D1-D2, A1-An) */
    virtual int getNumParams() const { return 4; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "LCD Update"; }
    virtual int getCommandNumber() const { return LCD_UPDATE; }

    virtual bool displayParamName(int number) const { return (number < 3); }

    virtual void processStateChange() const
      {mIt100.setLcdScreen(getIntParam(0), getIntParam(1), getStringParam(3));}

  protected:
    LCDUpdate(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 902 (LCD Cursor)
 * Parameters: 4 bytes (T,L,C1-C2)
 *********************************************************************/

class LCDCursor : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new LCDCursor(it100, command); }

    /* 3 parameters, 4 bytes -- (T,L,C1-C2) */
    virtual int getNumParams() const { return 3; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "LCD Cursor"; }
    virtual int getCommandNumber() const { return LCD_CURSOR; }

    virtual bool displayParamName(int number) const { return (number > 0); }

    virtual void processStateChange() const
      {mIt100.setLcdCursor(getIntParam(0), getIntParam(1), getIntParam(3));}

  protected:
    LCDCursor(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 903 (LED Status)
 * Parameters: 2 bytes (L,S)
 *********************************************************************/

class LEDStatus : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new LEDStatus(it100, command); }

    /* 2 parameters, 2 bytes -- (L,S) */
    virtual int getNumParams() const { return 2; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "LED Status"; }
    virtual int getCommandNumber() const { return LED_STATUS; }

    virtual void processStateChange() const
      {mIt100.setLedState(getIntParam(0), getIntParam(1));}

  protected:
    LEDStatus(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 904 (Beep Status)
 * Parameters: 3 bytes (0-255 Beeps)
 *********************************************************************/

class BeepStatus : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BeepStatus(it100, command); }

    /* 1 parameter, 3 bytes -- (0-255 Beeps) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Beep Status"; }
    virtual int getCommandNumber() const { return BEEP_STATUS; }

  protected:
    BeepStatus(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 905 (Tone Status)
 * Parameters: 4 bytes (C, B, I1-I2)
 *********************************************************************/

class ToneStatus : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new ToneStatus(it100, command); }

    /* 3 parameters, 4 bytes -- (C, B, I1-I2) */
    virtual int getNumParams() const { return 3; }
    virtual int getIntParam(int number) const;
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Tone Status"; }
    virtual int getCommandNumber() const { return TONE_STATUS; }

  protected:
    ToneStatus(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 906 (Buzzer Status)
 * Parameters: 3 bytes (000-255 secs)
 *********************************************************************/

class BuzzerStatus : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new BuzzerStatus(it100, command); }

    /* 1 parameter, 3 bytes -- (000-255 secs) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Buzzer Status"; }
    virtual int getCommandNumber() const { return BUZZER_STATUS; }

  protected:
    BuzzerStatus(It100 &it100, std::string command)
      : Command(it100, command){;}
};

/** *******************************************************************
 * Command 907 (Door Chime Status)
 *********************************************************************/

class DoorChimeStatus : public CommandWithNoParameters
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new DoorChimeStatus(it100, command); }

    /* No parameters */
    virtual std::string getName() const { return "Door Chime Status"; }
    virtual int getCommandNumber() const { return DOOR_CHIME_STATUS; }

  protected:
    DoorChimeStatus(It100 &it100, std::string command)
      : CommandWithNoParameters(it100, command){;}
};

/** *******************************************************************
 * Command 908 (Software Version)
 * Parameters: 6 bytes (VVSSXX)
 *********************************************************************/

class SoftwareVersion : public Command
{
  public:
    static Command *create(It100 &it100, std::string command)
      { return new SoftwareVersion(it100, command); }

    /* 1 parameter, 6 bytes -- (VVSSXX) */
    virtual int getNumParams() const { return 1; }
    virtual int getIntParam(int number) const { return number?0:getInt(); }
    virtual std::string getStringParam(int number) const;
    virtual std::string getParamName(int number) const;
    virtual std::string getName() const { return "Software Version"; }
    virtual int getCommandNumber() const { return SOFTWARE_VERSION; }

  protected:
    SoftwareVersion(It100 &it100, std::string command)
      : Command(it100, command){;}
};


#endif
