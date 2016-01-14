#!/bin/sh
grep -Po "MAJOR +\d+|MINOR +\d+|RELEASE +\d+" src/config.h | awk '{print $2}' | sed '$!N;$!N;$!N;s/\n/./g'
