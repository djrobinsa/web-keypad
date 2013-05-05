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

#ifndef _IT100_H
#define _IT100_H 1

#include <time.h>
#include <stdio.h>
#include <queue>
#include <string>

class It100
{
  public:

    typedef enum { FIRE_PANIC=1, AMBULANCE_PANIC=2, POLICE_PANIC=3 } alarm_t;
    typedef enum { ITB9600=0, ITB19200=1, ITB38400=2, ITB57600=3, ITB115200=4 }
      baud_t;

    typedef enum {READY = 1, ARMED = 2, MEMORY = 3, BYPASS=4, TROUBLE = 5,
                  PROGRAM = 6, FIRE = 7, BACKLIGHT = 8, AC = 9} led_t;
    typedef enum {OFF = 0, ON = 1, FLASHING = 2} ledState_t;
    typedef enum {NONE = 0, UNDERLINE = 1, BLOCK = 2} cursor_t;

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
      RESTORE                                     = 826,
      HANDHELD_KEYPAD_LOW_BATTERY_TROUBLE         = 827,
      RESTORED                                    = 828,
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

    It100();

    int getDescriptor() { return mDescriptor; }

    void setFdBaud(int baud);
    void processMessage();

    static const char *commandToName(int command);

    // Here are the commands we can send
    void poll() {sendCommand(POLL); }
    void statusRequest() { sendCommand(STATUS_REQUEST); }
    void labelsRequest() { sendCommand(LABELS_REQUEST); }
    void setTimeAndDate(time_t time); 
    void commandOutputControl(int part, int pgm)
      { sendCommand(COMMAND_OUTPUT_CONTROL, "%1.1d%1.1d",part,pgm); }
    void partitionArmAway(int partition)
      { sendCommand(PARTITION_ARM_CONTROL_AWAY, "%1.1d", partition); }
    void partitionArmStay(int partition)
      { sendCommand(PARTITION_ARM_CONTROL_STAY, "%1.1d", partition); }
    void partitionArmNoDelay(int partition)
      { sendCommand(PARTITION_ARM_CONTROL_ARMED_NO_ENTRY_DELAY, 
                    "%1.1d", partition); }
    void partitionArmWithCode(int partition, const char* code)
      { sendCommand(PARTITION_ARM_CONTROL_WITH_CODE, 
                    "%1.1d%s", partition, code); }
    void partitionDisarmWithCode(int partition, const char* code)
      { sendCommand(PARTITION_DISARM_CONTROL_WITH_CODE, 
                    "%1.1d%s", partition, code); }
    void timeStampControl(bool flag)
      { sendCommand(TIME_STAMP_CONTROL, "%1.1d", flag?1:0); }
    void timeDateBroadcastControl(bool flag)
      { sendCommand(TIME_DATE_BROADCAST_CONTROL, "%1.1d", flag?1:0); }
    void temperatureBroadcastControl(bool flag)
      { sendCommand(TEMPERATURE_BROADCAST_CONTROL, "%1.1d", flag?1:0); }
    void virtualKeypadControl(bool flag)
      { sendCommand(VIRTUAL_KEYPAD_CONTROL, "%1.1d", flag?1:0); }
    void triggerPanicAlarm(alarm_t type)
      { sendCommand(TRIGGER_PANIC_ALARM, "%1.1d", type); }
    void keyPressed(char key)
      { sendCommand(KEY_PRESSED, "%c", key); }
    void baudRateChange(baud_t rate)
      { sendCommand(BAUD_RATE_CHANGE, "%1.1d", rate); }
    void getTemperatureSetPoint(int thermostat)
      { sendCommand(GET_TEMPERATURE_SET_POINT, "%1.1d", thermostat); }
    void temperatureChange(int thermostat,
                           char type,
                           char mode,
                           int temperature = 0)
      {sendCommand(TEMPERATURE_CHANGE,
                   "%1.1d%c%c%03.3d",thermostat, type, mode, temperature);}
    void saveTemperatureSetting(int thermostat)
      { sendCommand(SAVE_TEMPERATURE_SETTING, "%1.1d", thermostat); }
    void codeSend (int partition, const char *code)
      { sendCommand(CODE_SEND, "%1.1d%s", partition, code); }

    // Here are the things that can happen due to commands we receive
    void sendPendingCommand();
    void setZoneOpen(int zone, bool open);
    void sendAccessCode(int parition, int codeLength);
    void setLcdScreen(int line, int column, std::string);
    void setLcdCursor(int type, int line, int column);
    void setLedState(int led, int state);
    void setLabel(int num, std::string label);

    bool hasLabels() { return mHasLabels; }

    std::string getZoneName(int zone);
    std::string getPartitionName(int partition);
    std::string getUserName(int zone);

    ledState_t getLedState(led_t led) const { return mLedState[led]; }
    const char *getLcd() const { return mLCD; }
    cursor_t getCursorType() const { return mCursorType; }
    int getCursorLine() const { return mCursorLine; }
    int getCursorColumn() const { return mCursorColumn; }
    unsigned int getKeypadEtag() const { return mKeypadEtag; }
    uint64_t getZoneStatus() const;

  protected:
    void sendCommand(command_t cmd, const char *format, ...);
    void sendCommand(command_t cmd) { sendCommand(cmd, ""); }
    void updateState(command_t cmd, const char *parameters);

  private:
    char mDevice[FILENAME_MAX];
    int mDescriptor;

    bool mHasLabels;
    time_t mCommandPending;
    std::queue<std::string> mPendingCommands;

    /* Labels */
    std::string mLabel[152];

    /* Internal Status */
    bool mZoneOpen[65];

    /* Keypad Status */
    unsigned int mKeypadEtag;
    ledState_t mLedState[10];
    char mLCD[33];
    cursor_t mCursorType;
    int mCursorLine;
    int mCursorColumn;

    /* Noises -- these may need some refactoring, as they are all one-shot */
    int mBeepDuration;
    bool mToneConstant;
    int mToneCount;
    int mToneInterval;
    int mBuzzDuration;
    bool mDoorChimeStatus;
};

#endif
