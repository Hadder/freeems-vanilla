#!/bin/bash

#       dumpAllFlash3forms.bash
#
#       Copyright 2008 Fred Cooke
#
#       This file is part of the FreeEMS project.
#
#       FreeEMS software is free software: you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation, either version 3 of the License, or
#       (at your option) any later version.
#
#       FreeEMS software is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with any FreeEMS software.  If not, see http://www.gnu.org/licenses/
#
#       We ask that if you make any changes to this file you send them upstream to us at admin@diyefi.org
#
#       Thank you for choosing FreeEMS to run your engine!

# These are all we can use now as we are paging flash finally :-)

# This pulls down the entire devices flash contents including all erased regions
#hcs12mem -b 115200 -i sm -p /dev/ttyUSB0 -t mc9s12xdp512 -a banked-ppage -e --flash-read freeems.full.dump.ppage.`date +%s`.s19

# This only pulls down what has been written.
hcs12mem -b 115200 -i sm -p /dev/ttyUSB0 -t mc9s12xdp512 -a banked-ppage --flash-read freeems.full.dump.ppage.`date +%s`.s19

# non banked only outputs 48k which is too small.
#hcs12mem -b 115200 -i sm -p /dev/ttyUSB0 -t mc9s12xdp512 -a non-banked -e --flash-read zoutput/freeems.full.dump.48k.s19
# banked linear outputs incorrect numbers on our system.
#hcs12mem -b 115200 -i sm -p /dev/ttyUSB0 -t mc9s12xdp512 -a banked-linear -e --flash-read zoutput/freeems.full.dump.linear.s19

