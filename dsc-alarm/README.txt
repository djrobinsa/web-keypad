This is not ready. There will be instructions here when it is. :)


Installing the daemon on a OSX box:

1) go into the dscd directorry
2) compile by doing "make clean; make "
3) Copy dscd exectable to /usr/bin (if you change location, you need to edit the
plist file ) with "	sudo cp dscd /usr/bin/."


4) Copy the com.fluffyhome.dsc-alarm.plist file to /Library/LaunchDaemons with
"sudo cp com.fluffyhome.dsc-alarm.plist /Library/LaunchDaemons/."


5) copy dscd.conf.dist to /etc/dscd.conf (if you put it somwhere else, edit the
plist file ) with "	sudo cp dscd.conf.dist /etc/."

6) edit the dscd.conf file as described <uh, somwhere else> 
sudo emacs /etc/dscd.conf

You will need to figure out what serial device you are using. If you are using a
keyspace USA-19HS which I highly recomend, you can find it by unpluging it,
doing a 
ls -l /dev/cu.* > /tmp/old
then plugging it in and doing a 
ls -l /dev/cu.* > /tmp/new
diff /tmp/old /tmp/new 

And looking for a new device name that looks something like
/dec/cu.USA19 then some more stuff. Use that device name in the dscd.conf file 


7) start the deamon with 
"sudo launchctl load /Library/LaunchDaemons/com.fluffyhome.dsc-alarm.plist"


8) start the console application and have a look at what is going on 

You should see things like 
5/27/11 12:27:51 PM	com.fluffyhome.dsc-alarm[6271]	<<< Time Stamp Control("Off")

If something when wrong you can unload the plist with 
"sudo launchctl unload /Library/LaunchDaemons/com.fluffyhome.dsc-alarm.plist"
edit files and try again. 

