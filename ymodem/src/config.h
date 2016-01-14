#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAJOR   0
#define MINOR   1
#define RELEASE 5 

#define _STR(x)  #x
#define STR(x)  _STR(x)
#define VERSION  STR(MAJOR)"."STR(MINOR)"."STR(RELEASE)

#endif
