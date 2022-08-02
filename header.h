#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MAIN
#define LINK
#else
#define LINK extern
#endif

typedef unsigned char byte;
typedef unsigned short word;

LINK int     buildMonth;
LINK int     buildDay;
LINK int     buildYear;
LINK int     buildHour;
LINK int     buildMinute;
LINK int     buildSecond;
LINK int     buildNumber;
LINK int    *externals;
LINK word    fixups[10000];
LINK char    fixupTypes[10000];
LINK byte    fixupLowOffset[10000];
LINK int     inProc;
LINK char  **labelNames;
LINK char  **labelProcs;
LINK word   *labelValues;
LINK int    *labelLine;
LINK int     loaded6309;
LINK char    module[256];
LINK int     numExternals;
LINK int     numFixups;
LINK int     numLabels;
LINK int     use6309;
LINK int     usedExternal;
LINK int     usedLocal;

LINK char    opcodes[768][64];
LINK char    translations[768][64];
LINK int     numOpcodes;
LINK word    address;
LINK char  **sourceNames;
LINK int     numSourceFiles;
LINK char    baseName[1024];
LINK char    listName[1024];
LINK char    outName[1024];
LINK FILE   *sourceFile[100];
LINK int     lineNumber[100];
LINK int     fileNumber;
LINK FILE   *listFile;
LINK FILE   *outFile;
LINK char    listLine[1024];
LINK int     pass;
LINK int     lineCount;
LINK int     codeGenerated;
LINK char    label[256];
LINK char    opcode[256];
LINK word    args[1024];
LINK word    arg2;
LINK word    argCount;
LINK word    argNumber;
LINK word    indexNumber;
LINK word    amode;
LINK int     aext;
LINK char    aetype;
LINK char    extType;
LINK byte    opcodeOffset;
LINK int     numArgs;
LINK int     errors;
LINK char    sourceLine[1024];
LINK int     instructionBytes;
LINK byte    a1,a2;
LINK word    i1,i2;
LINK byte    c1,c2;
LINK int     evalErrors;
LINK byte    outBytes[16];
LINK int     outCount;
LINK word    outAddress;
LINK int     showList;
LINK int     showSymbols;
LINK int     createListFile;
LINK word    startAddress;
LINK char  **defines;
LINK char  **defineValues;
LINK int     numDefines;
LINK char    nests[100];
LINK int     numNests;
LINK int    *references;
LINK word   *referenceAddress;
LINK int     numReferences;
LINK int    *publics;
LINK int     numPublics;

#endif

