#ifndef REGISTRY_H_
#define REGISTRY_H_

extern void initRegistry();
extern int readReg(char*, int);
extern char * readRegStr(char*, char *);
extern void writeRegInt(char*, int);
extern void writeRegStr(char*, char*);

#endif