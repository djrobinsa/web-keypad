#!/usr/bin/perl

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


$keypad_model = &LCD5500Z;
#$keypad_model = &PK5500;

use IO::Socket;

$query = $ENV{'QUERY_STRING'};
$path = $ENV{'PATH_INFO'};
($0 =~ /([^\/]*)$/) && ($script=$1);

if ($query =~ /^new/)
{
  $keypad_model = &PK5500;
}
elsif ($query =~ /^old/)
{
  $keypad_model = &LCD5500Z;
}

if ($keypad_model == &PK5500)
{
  $normal = 'PK5500 Default.png';
  $active = 'PK5500 Active.png';
}
else
{
  $normal = 'DSC Keypad.png';
  $active = 'DSC Keypad Active.png';
}

if ($path =~ /^\/([^\/]*png)$/)
{
  print "Content-Type: image/png\n\n";
  exec('cat',$1);
}

if ($path eq '/status')
{
  &returnKeypadStatus;
  exit;
}

if ($path =~ /\/kp(up|down)\/(.*)/)
{
  print "Content-Type: text/plain\n\n$2 $1\n";
  &processKey($1,$2);
  exit;
}

if ($path eq '')
{
  &drawKeypad;
}

sub returnKeypadStatus
{
  print "Content-Type: text/plain\n\n";

  if ($query =~ /etag=([0-9]+)/)
  {
    $etag = $1;
  }

  my $sock = new IO::Socket::INET(PeerAddr => 'localhost',
                                  PeerPort => 53280,
                                  Proto => 'tcp');
  $cmd = "?$etag\n";
  print $sock $cmd;
  sysread($sock, $buffer, length($cmd));
  sysread($sock, $buffer, 256);
  close ($sock);
  $buffer =~ s/[\r\n]//g;
  print $buffer;
}

sub processKey
{
  my ($state,$key) = @_;
  my $sock = new IO::Socket::INET(PeerAddr => 'localhost',
                                  PeerPort => 53280,
                                  Proto => 'tcp');
  if ($state =~ /up/i) { $cmd = "^\n"; }
  elsif ($key =~ /KP([0-9])/) { $cmd = "$1\n"; }
  elsif ($key eq 'KPStar') {$cmd = "*\n"; }
  elsif ($key eq 'KPHash') {$cmd = "#\n"; }
  elsif ($key eq 'KPStay') {$cmd = "a\n"; }
  elsif ($key eq 'KPAway') {$cmd = "b\n"; }
  elsif ($key eq 'KPChime') {$cmd = "c\n"; }
  elsif ($key eq 'KPReset') {$cmd = "d\n"; }
  elsif ($key eq 'KPExit') {$cmd = "e\n"; }
  elsif ($key eq 'KPLeft') {$cmd = "<\n"; }
  elsif ($key eq 'KPRight') {$cmd = ">\n"; }
  elsif ($key eq 'KPFire') {$cmd = "F\n"; }
  elsif ($key eq 'KPAmbulance') {$cmd = "A\n"; }
  elsif ($key eq 'KPPanic') {$cmd = "P\n"; }

  print $sock $cmd;
  sysread($sock, $buffer, length($cmd));
  close ($sock);
}

###########################################################################

sub drawKeypad
{
  my (@items);
  if ($keypad_model == &PK5500)
  {
    @items =
    (
      ["Backlight",112,54,343,83],

      ["Memory" ,50,62,60,17],
      ["Bypass" ,50,62+17,60,17],
      ["Fire"   ,50,62+17+17,60,17],
      ["Program",50,62+17+17+17,60,17],

      ["Ready",   468, 59, 16, 16],
      ["Armed",   468, 62+15, 16, 16],
      ["Trouble", 468, 62+16+16, 16, 16],
      ["AC",      468, 62+16+16+16+4, 17, 18],

      ["KP1", 208,     191, 55, 24],
      ["KP2", 208+63,  191, 56, 24],
      ["KP3", 208+126, 191, 55, 24],

      ["KP4", 208, 191+45, 55, 24],
      ["KP5", 271, 191+45, 56, 24],
      ["KP6", 334, 191+45, 55, 24],

      ["KP7", 208, 191+45+46, 55, 24],
      ["KP8", 271, 191+45+46, 56, 24],
      ["KP9", 334, 191+45+46, 55, 24],

      ["KPStar", 208, 191+45+46+44, 55, 24],
      ["KP0",    271, 191+45+46+44, 56, 24],
      ["KPHash", 334, 191+45+46+44, 55, 24],

      ["KPStay",  428,  191, 48, 21],
      ["KPAway",  428,  191+34, 48, 21],
      ["KPChime", 428,  191+34+34, 48, 21],
      ["KPReset", 428,  191+34+34+34, 48, 21],
      ["KPExit",  428,  191+34+34+34+32, 48, 21],

      ["KPLeft",  87, 192, 37, 19],
      ["KPRight", 132, 192, 37, 19],

      ["KPFire",      87, 246, 82, 19],
      ["KPAmbulance", 87, 246+41, 82, 19],
      ["KPPanic",     87, 246+82, 82, 19],
    );
    $lcdpos = 'width:310px;height:75px;left:127;top:58px;';
  }
  else
  {
    @items =
    (
      ["Backlight",61,48,310,75],

      ["Memory",388,40,70,20],
      ["Bypass",388,62,70,20],
      ["Fire",388,64+22,70,20],
      ["Program",388,64+22+22,70,20],

      ["Ready", 230, 155, 12, 20],
      ["Armed", 230+36, 155, 12, 20],
      ["Trouble", 230+36+35, 155, 12, 20],

      ["KP1", 38, 182 , 29, 29],
      ["KP2", 38+40, 182, 29, 29],
      ["KP3", 38+80, 182, 29, 29],

      ["KP4", 38,    182+45, 29, 29],
      ["KP5", 38+40, 182+45, 29, 29],
      ["KP6", 38+80, 182+45, 29, 29],

      ["KP7", 38,    182+45+46, 29, 29],
      ["KP8", 38+40, 182+45+46, 29, 29],
      ["KP9", 38+80, 182+45+46, 29, 29],

      ["KPStar", 38,    182+45+46+45, 29, 29],
      ["KP0", 38+40,    182+45+46+45, 29, 29],
      ["KPHash", 38+80, 182+45+46+45, 29, 29],

      ["KPStay", 164,  188, 16, 16],
      ["KPAway", 164,  188+29, 16, 16],
      ["KPChime", 164, 188+29+29, 16, 16],
      ["KPReset", 164, 188+29+29+28, 16, 16],
      ["KPExit", 164,  188+29+29+28+29, 16, 16],

      ["KPLeft",  235, 190, 29, 31],
      ["KPRight", 272, 190, 29, 31],

      ["KPFire", 235, 240, 66, 31],
      ["KPAmbulance", 235, 240+35, 66, 31],
      ["KPPanic", 235, 240+35+34, 66, 31],
    );
    $lcdpos = 'width:310px;height:75px;left:61px;top:48px;';
  }

print <<EOT
Content-Type: text/html

<html><head><title>Virtual Alarm Keypad</title></head>
<body>
<script>
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

var flashState = false;
var kpHttp = null;
var statusHttp = null;
var guiElementState = new Array;
var kpPending = new Array;
var lightNames = new Array( "Ready", "Armed", "Memory", "Bypass",
                            "Trouble", "Program", "Fire", "Backlight", "AC");
var loadTimeout = setTimeout('loadFailed()',10000);
var keypadEtag = 0;

function loadFailed()
{
  setLcdText('Timeout Waiting','for Server');
  statusHttp = null;
  loadTimeout = null;
  pollState();
}

function startTasks()
{
  //setLcdText('Keypad Loaded','Awaiting Status');
  setInterval('flashLights()',1000);
  document.onkeydown = keyDown;
  document.onkeyup = keyUp;
  pollState();
}

function eventToKey(event)
{
  var key = (String.fromCharCode(event.which));
  switch (key)
  {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return('KP'+key);
      break;
    case 'S': return('KPStay'); break;
    case 'A': return('KPAway'); break;
    case 'C': return('KPChime'); break;
    case 'R': return('KPReset'); break;
    case 'E': return('KPExit'); break;
    case 'F': return('KPFire'); break;
    case 'M': return('KPAmbulance'); break;
    case 'P': return('KPPanic'); break;
    case '.': case '>': return('KPRight'); break;
    case ',': case '<': return('KPLeft'); break;
    default:
    {
      switch (event.which)
      {
        case 110: case 32: return('KPHash'); break;
        case 13: return('KPStar'); break;
        case 37: case 188: return('KPLeft'); break;
        case 39: case 190: return('KPRight'); break;
        case 96: case 97: case 98: case 99:
        case 100: case 101: case 102: case 103:
        case 104: case 105:
          return('KP'+(event.which-96));
          break;
      }
    }
  }
  // setLcdText(''+event.which,key);
  return('');
}

function keyDown(event)
{
  var key = eventToKey(event);
  if (key.length)
  {
    keyPress(key);
  }
}

function keyUp(event)
{
  var key = eventToKey(event);
  if (key.length)
  {
    keyRelease(key);
  }
}

function displayState()
{
  try
  {
    var ready = statusHttp.readyState;
    var status = statusHttp.status;
    var contents = statusHttp.responseText;
  }
  catch (ex)
  {
    return;
  }

  if (ready != 4) { return; }

  statusHttp = null;

  var state = null;
  var i;

  eval ("state = " + contents);

  keypadEtag = state.shift();

  // Array order is:
  // 0=Ready, 1=Armed, 2=Memory, 3=Bypass, 4=Trouble, 5=Program,
  // 6=Fire, 7=Backlight, 8=AC,
  // 9=LCD1, 10=LCD2, 
  // 11=Cursor Type, 12=Cursor Line, 13=Cursor Column
  // 14 = Beep (duration in seconds)
  // 15,16,17 = Tone (constant flag, number, interval)
  // 18 = Buzz (duration in seconds)
  // 19 = Door Chime

  for (i = 0; i < 8; i++)
  {
    guiElementState[lightNames[i]] = state[i];
    if (state[i]  == 0)
    {
      document.images[lightNames[i]].src='$script/$normal';
    }
    else if (state[i] == 1)
    {
      document.images[lightNames[i]].src='$script/$active';
    }
  }

  setLcdText(state[9],state[10],state[11],state[12],state[13]);

  if (loadTimeout)
  {
    clearTimeout(loadTimeout);
    loadTimeout = null;
  }

  setTimeout('pollState()', 10);
}

function pollState()
{
  statusHttp = new XMLHttpRequest();
  statusHttp.onreadystatechange = displayState;
  statusHttp.open("GET", "$script/status?etag=" + keypadEtag, true);
  statusHttp.send(null);
  if (loadTimeout == null)
  {
    loadTimeout = setTimeout('loadFailed()',70000);
  }
}

function flashLights()
{
  var i;
  flashState = !flashState;
  for (i = 0; i < 8; i++)
  {
    if (guiElementState[lightNames[i]] == 2)
    {
      if (flashState)
      {
        document.images[lightNames[i]].src='$script/$active';
      }
      else
      {
        document.images[lightNames[i]].src='$script/$normal';
      }
    }
  }
}

function setLcdText(line1,line2,cursorType,cursorLine,cursorColumn)
{
  line1 = (''+line1).replace(/ /g, '\\u00A0');
  line1 = (line1 + "\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0").substring(0,16);
  line2 = (''+line2).replace(/ /g, '\\u00A0');
  line2 = (line2 + "\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0\\u00A0").substring(0,16);

  var cursorText = cursorLine?line2:line1;
  var cursorLeft = cursorText.substring(0,cursorColumn);
  var cursorMid = cursorText.substring(cursorColumn, cursorColumn+1);
  var cursorRight = cursorText.substring(cursorColumn+1,16);
  var cursorElement = null;

  switch (cursorType)
  {
    case 1:
      cursorElement = document.createElement("u");
      break;
    case 2:
      var color = guiElementState['Backlight']?'#CBFC68':'#C9C9C9';
      cursorElement = document.createElement("span");
      cursorElement.setAttribute('style',
                                 'color:'+color+';background-color:#000000');
      break;
    default:
      cursorElement = document.createElement("span");
  }
  cursorElement.appendChild(document.createTextNode(cursorMid));

  var newSpan1 = document.createElement("span");
  if (cursorType == 0 || cursorLine != 0)
  {
    newSpan1.appendChild(document.createTextNode(line1));
  }
  else
  {
    newSpan1.appendChild(document.createTextNode(cursorLeft));
    newSpan1.appendChild(cursorElement);
    newSpan1.appendChild(document.createTextNode(cursorRight));
  }

  var newSpan2 = document.createElement("span");
  if (cursorType == 0 || cursorLine != 1)
  {
    newSpan2.appendChild(document.createTextNode(line2));
  }
  else
  {
    newSpan2.appendChild(document.createTextNode(cursorLeft));
    newSpan2.appendChild(cursorElement);
    newSpan2.appendChild(document.createTextNode(cursorRight));
  }

  var lcd = document.getElementById("LCD");
  lcd.replaceChild(newSpan1, lcd.childNodes[0]);
  lcd.replaceChild(newSpan2, lcd.childNodes[2]);
}

function kpStatus()
{
  try
  {
    var ready = kpHttp.readyState;
    var status = kpHttp.status;
    var contents = kpHttp.responseText;
  }
  catch (ex)
  {
    return;
  }

  if (ready != 4) { return; }


  if (kpPending.length > 0)
  {
    kpHttp = new XMLHttpRequest();
    kpHttp.onreadystatechange = kpStatus;
    kpHttp.open("GET", kpPending.shift(), true);
    kpHttp.send(null);
  }
  else
  {
    kpHttp = null;
  }

  //setLcdText(status,contents);
}

function keyPress(name)
{
  document.images[name].src="$script/$active"
  guiElementState[name] = true;
  if (kpHttp == null)
  {
    kpHttp = new XMLHttpRequest();
    kpHttp.onreadystatechange = kpStatus;
    kpHttp.open("GET", "$script/kpdown/"+name, true);
    kpHttp.send(null);
  }
  else
  {
    kpPending.push("$script/kpdown/"+name);
  }
}

function keyRelease(name)
{
  if (guiElementState[name])
  {
    document.images[name].src="$script/$normal"
    guiElementState[name] = false;

    if (kpHttp == null)
    {
      kpHttp = new XMLHttpRequest();
      kpHttp.onreadystatechange = kpStatus;
      kpHttp.open("GET", "$script/kpup/"+name, true);
      kpHttp.send(null);
    }
    else
    {
      kpPending.push("$script/kpup/"+name);
    }
  }
}
</script>

<style>
#keypad { position:absolute; left:0px;top:0px }
#keypad img { border:none;position:absolute;left:0px;top:0px }
#LCD {display:block;overflow:hidden;position:absolute;
      $lcdpos
      margin:0px 0px; font-size:31px;
      font-family:"Andale Mono",Courier,"Courier New",Monospace}
EOT
;


  foreach $item (@items)
  {
    ($name, $x, $y, $width, $height) = @{$item};
      print <<EOT
#$name {display:block;overflow:hidden;position:absolute;
        width:${width}px;height:${height}px;left:${x}px;top:${y}px}
#$name img { border:none;position:absolute;left:-${x}px;top:-${y}px }
EOT
  ;
    $actions = '';
    if ($name =~ /^KP/)
    {
      $actions = "onMouseDown='keyPress(\"$name\")' onMouseUp='keyRelease(\"$name\")'";
      $actions .= "onMouseOut='keyRelease(\"$name\")'";
    }
    else
    {
      #$actions = "onMouseOver='document.images[\"$name\"].src=\"$script/$active\"' onMouseOut='document.images[\"$name\"].src=\"$script/$normal\"'";
    }
    $body .= "<div id=\"$name\"><img name='$name' $actions src='$script/$normal'/></div>\n";
  }

  print "</style>\n";
  print "<div id=\"keypad\"><img src='$script/$normal' onLoad='startTasks()'/></div>\n";
  print $body;

  print "<center><div id='LCD'><span>Loading Keypad&nbsp;</span><br><span>Please Wait...&nbsp</span></div></center>\n</body></html>\n"
}

sub LCD5500Z {return 1;}
sub PK5500 {return 2;}
