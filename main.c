#define MAIN

#include <time.h>
#include "header.h"

#define OP_STOP     0xff
#define OP_DIV      0x71
#define OP_MUL      0x70
#define OP_SUB      0x61
#define OP_ADD      0x60
#define OP_SHR      0x51
#define OP_SHL      0x50
#define OP_LT       0x45
#define OP_LE       0x44
#define OP_GT       0x43
#define OP_GE       0x42
#define OP_NE       0x41
#define OP_EQ       0x40
#define OP_XOR      0x32
#define OP_OR       0x31
#define OP_AND      0x30
#define OP_LAND     0x20
#define OP_LOR      0x10
#define OP_OP       0x09
#define OP_CP       0x08
#define OP_END      0x01

typedef union {
  float f;
  unsigned int i;
  } FTOI;

void addOpcode(char* op, char* trans) {
  strcpy(opcodes[numOpcodes], op);
  strcpy(translations[numOpcodes], trans);
  numOpcodes++;
  if (numOpcodes > 767) {
    printf("Opcode table full\n");
    exit(1);
    }
  }

char* get32(char* line) {
  evalErrors = 0;
  args[argCount++] = 0x1234;
  args[argCount++] = 0x5678;
  while (*line != 0) line++;
  return line;
  }

void initOpcodes() {
  numOpcodes = 0;
  addOpcode("NOP",               "00");
  addOpcode("JCN Z,{W}",         "14 [S]");
  addOpcode("JCN C,{W}",         "12 [S]");
  addOpcode("JCN T,{W}",         "11 [S]");
  addOpcode("JCN NZ,{W}",        "1C [S]");
  addOpcode("JCN NC,{W}",        "1A [S]");
  addOpcode("JCN NT,{W}",        "19 [S]");
  addOpcode("JCN ZC,{W}",        "16 [S]");
  addOpcode("JCN CZ,{W}",        "16 [S]");
  addOpcode("JCN NZC,{W}",       "1e [S]");
  addOpcode("JCN NCZ,{W}",       "1e [S]");
  addOpcode("FIM {3},{W}",       "20<N + [L]");
  addOpcode("SRC {3}",           "21<N");
  addOpcode("FIN {3}",           "30<N");
  addOpcode("JIN {3}",           "31<N");
  addOpcode("JUN {12}",          "40|H [L]");
  addOpcode("JMS {12}",          "50|H [L]");
  addOpcode("INC {N}",           "60|N");
  addOpcode("ISZ {N},{W}",       "70|N + [S]");
  addOpcode("ADD {N}",           "80|N");
  addOpcode("SUB {N}",           "90|N");
  addOpcode("LD {N}",            "A0|N");
  addOpcode("XCH {N}",           "B0|N");
  addOpcode("BBL {N}",           "C0|N");
  addOpcode("LDM {N}",           "D0|N");

  addOpcode("CLB",               "F0");
  addOpcode("CLC",               "F1");
  addOpcode("IAC",               "F2");
  addOpcode("CMC",               "F3");
  addOpcode("CMA",               "F4");
  addOpcode("RAL",               "F5");
  addOpcode("RAR",               "F6");
  addOpcode("TCC",               "F7");
  addOpcode("DAC",               "F8");
  addOpcode("TCS",               "F9");
  addOpcode("STC",               "FA");
  addOpcode("DAA",               "FB");
  addOpcode("KBP",               "FC");
  addOpcode("DCL",               "FD");

  addOpcode("WRM",               "E0");
  addOpcode("WMP",               "E1");
  addOpcode("WRR",               "E2");
  addOpcode("WR0",               "E4");
  addOpcode("WR1",               "E5");
  addOpcode("WR2",               "E6");
  addOpcode("WR3",               "E7");
  addOpcode("SBM",               "E8");
  addOpcode("RDM",               "E9");
  addOpcode("RDR",               "EA");
  addOpcode("ADM",               "EB");
  addOpcode("RD0",               "EC");
  addOpcode("RD1",               "ED");
  addOpcode("RD2",               "EE");
  addOpcode("RD3",               "EF");
  }

char* trim(char* buffer) {
  while (*buffer == ' ' || *buffer == '\t') buffer++;
  return buffer;
  }

char* nextWord(char*buffer) {
  while (*buffer != 0 && *buffer != ' ' && *buffer != '\t') buffer++;
  return trim(buffer);
  }

void writeOutput() {
  int  i;
  char buffer[256];
  char tmp[4];
  sprintf(buffer,":%04x",outAddress);
  for (i=0; i<outCount; i++) {
    sprintf(tmp," %02x",outBytes[i]);
    strcat(buffer,tmp);
    }
  fprintf(outFile,"%s\n",buffer);
  outCount = 0;
  outAddress = address;
  }

void output(byte value) {
  int  i;
  char tmp[4];
  codeGenerated++;
  address++;
  instructionBytes++;
  sprintf(tmp," %02x",value);
  strcat(listLine,tmp);
  if (instructionBytes == 4) {
    strcat(listLine,"  ");
    strcat(listLine, sourceLine);
    }
  if ((instructionBytes % 4) == 0) {
    if (pass == 2) {
      if (showList != 0) printf("%s\n",listLine);
      if (createListFile != 0) fprintf(listFile,"%s\n",listLine);
      }
    strcpy(listLine,"             ");
    }
  if (pass == 2) {
    outBytes[outCount++] = value;
    if (outCount == 16) {
      writeOutput();
      }
    }
  }

int findLabelNumber(char* name) {
  int i;
  int j;
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0 &&
        strcasecmp(labelProcs[i], module) == 0)
      return i;
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0 &&
        strcasecmp(labelProcs[i], "*") == 0)
      return i;
  return -1;
  }

word findLabel(char* name, char* err) {
  int i;
  int j;
  *err = 0;
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0 &&
        strcasecmp(labelProcs[i], module) == 0) {
      usedLocal = -1;
      for (j=0; j<numExternals; j++)
        if (externals[j] == i) {
          usedExternal = j;
          }
      return labelValues[i];
      }
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0 &&
        strcasecmp(labelProcs[i], "*") == 0) {
      for (j=0; j<numExternals; j++)
        if (externals[j] == i) {
          usedExternal = j;
          }
      return labelValues[i];
      }
  if (pass == 1) return 0x0000;
  *err = 0xff;
  printf("****ERROR: Label not found: %s\n",name);
  errors++;
  return 0;
  }

int addLabel(char* name, word value) {
  int i;
  if (pass == 2) return 0;
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0 &&
        strcasecmp(labelProcs[i], module) == 0) {
      printf("****ERROR: Duplicate label: %s\n",name);
      errors++;
      return -1;
      }
  numLabels++;
  if (numLabels == 1) {
    labelNames = (char**)malloc(sizeof(char*));
    labelValues = (word*)malloc(sizeof(word));
    labelProcs = (char**)malloc(sizeof(char*));
    labelLine = (int*)malloc(sizeof(int));
    }
  else {
    labelNames = (char**)realloc(labelNames,sizeof(char*)*numLabels);
    labelValues = (word*)realloc(labelValues,sizeof(word)*numLabels);
    labelProcs = (char**)realloc(labelProcs,sizeof(char*)*numLabels);
    labelLine = (int*)realloc(labelLine,sizeof(int)*numLabels);
    }
  labelNames[numLabels-1] = (char*)malloc(strlen(name)+1);
  labelProcs[numLabels-1] = (char*)malloc(strlen(module)+1);
  strcpy(labelNames[numLabels-1], name);
  strcpy(labelProcs[numLabels-1], module);
  labelValues[numLabels-1] = value;
  labelLine[numLabels-1] = lineCount;
  return 0;
  }

void setLabel(char* name, word value) {
  int i;
  for (i=0; i<numLabels; i++)
    if (strcasecmp(labelNames[i], name) == 0) {
      labelValues[i] = value;
      return;
      }
  numLabels++;
  if (numLabels == 1) {
    labelNames = (char**)malloc(sizeof(char*));
    labelValues = (word*)malloc(sizeof(word));
    labelLine = (int*)malloc(sizeof(int));
    }
  else {
    labelNames = (char**)realloc(labelNames,sizeof(char*)*numLabels);
    labelValues = (word*)realloc(labelValues,sizeof(word)*numLabels);
    labelLine = (int*)realloc(labelLine,sizeof(int)*numLabels);
    }
  labelNames[numLabels-1] = (char*)malloc(strlen(name)+1);
  strcpy(labelNames[numLabels-1], name);
  labelValues[numLabels-1] = value;
  labelLine[numLabels-1] = lineCount;
  }

void addExternal(char* name) {
  int i;
  if (pass == 2) return;
  addLabel(name, 0);
  numExternals++;
  if (numExternals == 1)
    externals = (int*)malloc(sizeof(int));
  else
    externals = (int*)realloc(externals,sizeof(int)*numExternals);
  externals[numExternals-1] = findLabelNumber(name);
  }

void addDefine(char *def, char* value) {
  int i;
  for (i=0; i<numDefines; i++)
    if (strcmp(defines[i], def) == 0) {
      printf("****ERROR: %s is defined more than once\n");
      errors++;
      return;
      }
  numDefines++;
  if (numDefines == 1) {
    defines = (char**)malloc(sizeof(char*));
    defineValues = (char**)malloc(sizeof(char*));
    }
  else {
    defines = (char**)realloc(defines,sizeof(char*)*numDefines);
    defineValues = (char**)realloc(defineValues,sizeof(char*)*numDefines);
    }
  defines[numDefines-1] = (char*)malloc(strlen(def) + 1);
  defineValues[numDefines-1] = (char*)malloc(strlen(value) + 1);
  strcpy(defines[numDefines-1], def);
  strcpy(defineValues[numDefines-1], value);
  }

char* findDefine(char*def) {
  int i;
  for (i=0; i<numDefines; i++)
    if (strcmp(defines[i], def) == 0) {
      return defineValues[i];
      }
  return NULL;
  }

char* evaluate(char* expr, word* ret) {
  int  i;
  word nstack[256];
  int  nsp;
  byte ostack[256];
  int  osp;
  int  flag;
  char err;
  char token[128];
  int  pos;
  char isHex;
  char hexChar;
  word dec;
  word hex;
  word value;
  byte op;
  char ntype;
  *ret = 0;
  evalErrors = 0;
  usedExternal = -1;
  usedLocal = -1;
  extType = 'W';
  osp = 0;
  nsp = 0;
  flag = 0;
  if (*expr == '-') {
    nstack[nsp++] = 0;
    ostack[osp++] = OP_SUB;
    expr++;
    }
  while (flag == 0) {
    hexChar = 'N';
    isHex = 'N';
    ntype = 'W';
    dec = 0;
    hex = 0;
    while (*expr == ' ') expr++;
    while (*expr == '(') {
      ostack[osp++] = OP_OP;
      expr++;
      }
    while (*expr == ' ') expr++;
    if (*expr == '<') {
      ntype = 'L';
      expr++;
      extType = 'L';
      }
    if (*expr == '>') {
      ntype = 'H';
      expr++;
      extType = 'H';
      }
    if (*expr == '$' && 
        ((*(expr+1) >= '0' && *(expr+1) <= '9') ||
         (*(expr+1) >= 'a' && *(expr+1) <= 'f') ||
         (*(expr+1) >= 'A' && *(expr+1) <= 'F')
        )) {
      isHex = 'Y';
      expr++;
      }
    if (strncmp(expr, "[month]", 7) == 0) {
      value = buildMonth;
      expr += 7;
      }
    else if (strncmp(expr, "[day]", 5) == 0) {
      value = buildDay;
      expr += 5;
      }
    else if (strncmp(expr, "[year]", 6) == 0) {
      value = buildYear;
      expr += 6;
      }
    else if (strncmp(expr, "[hour]", 6) == 0) {
      value = buildHour;
      expr += 6;
      }
    else if (strncmp(expr, "[minute]", 8) == 0) {
      value = buildMinute;
      expr += 8;
      }
    else if (strncmp(expr, "[second]", 8) == 0) {
      value = buildSecond;
      expr += 8;
      }
    else if (strncmp(expr, "[build]", 7) == 0) {
      value = buildNumber;
      expr += 7;
      }
    else if (*expr == '$') {
      value = address;
      expr++;
      }
    else if (*expr == '%') {
      expr++;
      isHex = 'N';
      value = 0;
      while (*expr == '0' || *expr == '1' || *expr == '_') {
        if (*expr == '0') value <<= 1;
        if (*expr == '1') value = (value << 1) | 1;
        expr++;
        }
      }
    else if ((*expr >= '0' && *expr <= '9') || isHex == 'Y') {
      while ((*expr >= '0' && *expr <= '9') ||
             (*expr >= 'a' && *expr <= 'f') ||
             (*expr >= 'A' && *expr <= 'F')) {
        if (*expr >= '0' && *expr <= '9') {
          dec = (dec * 10) + (*expr - '0');
          hex = (hex * 16) + (*expr - '0');
          }
        if (*expr >= 'a' && *expr <= 'f') {
          hexChar = 'Y';
          hex = (hex * 16) + (*expr - 87);
          }
        if (*expr >= 'A' && *expr <= 'F') {
          hexChar = 'Y';
          hex = (hex * 16) + (*expr - 55);
          }
        expr++;
        }
      if (*expr == 'h' || *expr == 'H') {
        isHex = 'Y';
        expr++;
        }
      if (isHex == 'Y') {
        value = hex;
        }
      else if (hexChar == 'N') {
        value = dec;
        }
      else {
        printf("****ERROR: Expression error\n");
        *ret = 0;
        evalErrors = 1;
        return expr;
        }
      }
    else if ((*expr >= 'a' && *expr <= 'z') ||
             (*expr >= 'A' && *expr <= 'Z') ||
             *expr == '_') {
      pos = 0;
      while ((*expr >= 'a' && *expr <= 'z') ||
             (*expr >= 'A' && *expr <= 'Z') ||
             (*expr >= '0' && *expr <= '9') ||
             *expr == '_') token[pos++] = *expr++;
      token[pos] = 0;
      value = findLabel(token, &err);
      if (err != 0) evalErrors = 1;
      }
    else if (*expr == '\'') {
      expr++;
      value = *expr++;
      while (*expr != 0 && *expr != '\'') expr++;
      if (*expr == '\'') expr++;
      }
    else {
      evalErrors = 1;
      return expr;
      }
    if (ntype == 'W') nstack[nsp++] = value;
    if (ntype == 'H') nstack[nsp++] = (value >> 8) & 0xff;
    if (ntype == 'L') nstack[nsp++] = value & 0xff;
    while (*expr == ' ' || *expr == '\t') expr++;
    op = OP_CP;
    while (op == OP_CP) {
      if (*expr == 0) op = OP_END;
      else if (*expr == '+') op = OP_ADD;
      else if (*expr == '-') op = OP_SUB;
      else if (*expr == '*') op = OP_MUL;
      else if (*expr == '/') op = OP_DIV;
      else if (*expr == ')') op = OP_CP;
      else if (*expr == '<' && *(expr+1) == '<') { op = OP_SHL; expr++; }
      else if (*expr == '>' && *(expr+1) == '>') { op = OP_SHR; expr++; }
      else if (*expr == '<' && *(expr+1) == '>') { op = OP_NE; expr++; }
      else if (*expr == '<' && *(expr+1) == '=') { op = OP_LE; expr++; }
      else if (*expr == '>' && *(expr+1) == '=') { op = OP_GE; expr++; }
      else if (*expr == '&' && *(expr+1) == '&') { op = OP_LAND; expr++; }
      else if (*expr == '|' && *(expr+1) == '|') { op = OP_LOR; expr++; }
      else if (*expr == '>') op = OP_GT;
      else if (*expr == '<') op = OP_LT;
      else if (*expr == '=') op = OP_EQ;
      else if (*expr == '&') op = OP_AND;
      else if (*expr == '|') op = OP_OR;
      else if (*expr == '^') op = OP_XOR;
      else {
        op = OP_END;
        flag = 0xff;
        }
      if (*expr != 0 && op != OP_END) expr++;
      while (osp > 0 && (ostack[osp-1] & 0xf0) >= (op & 0xf0)) {
        if (ostack[osp-1] == OP_ADD) { nstack[nsp-2] += nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_SUB) { nstack[nsp-2] -= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_MUL) { nstack[nsp-2] *= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_DIV) { nstack[nsp-2] /= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_SHL) { nstack[nsp-2] <<= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_SHR) { nstack[nsp-2] >>= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_AND) { nstack[nsp-2] &= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_OR) { nstack[nsp-2] |= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_XOR) { nstack[nsp-2] ^= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_NE) { nstack[nsp-2] = nstack[nsp-2] != nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_EQ) { nstack[nsp-2] = nstack[nsp-2] == nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_GE) { nstack[nsp-2] = nstack[nsp-2] >= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_LE) { nstack[nsp-2] = nstack[nsp-2] <= nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_GT) { nstack[nsp-2] = nstack[nsp-2] > nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_LT) { nstack[nsp-2] = nstack[nsp-2] < nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_LAND) { nstack[nsp-2] = nstack[nsp-2] && nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_LOR) { nstack[nsp-2] = nstack[nsp-2] || nstack[nsp-1]; nsp--; osp--; }
        else if (ostack[osp-1] == OP_OP) {
          osp--;
          op = OP_STOP;
          }
        }
      if (op == OP_STOP) op = OP_CP;
      while (*expr == ' ' || *expr == '\t') expr++;
      }
    ostack[osp++] = op;
    if (*expr == 0) flag = 0xff;
    }
  if (nsp != 1) {
    printf("****ERROR: Expression did not reduce to single term\n");
    evalErrors = 1;
    }
  *ret = nstack[0];
  return expr;
  }

char upcase(char in) {
  if (in >= 'a' && in <= 'z') in -= 32;
  return in;
  }

int match(char* pattern, char* input) {
  int i;
  word value;
  char *temp;
  while (*pattern == ' ' || *pattern == '\t') pattern++;
  while (*input == ' ' || *input == '\t') input++;
  while (*pattern != 0) {
    while (*pattern == ' ' || *pattern == '\t') pattern++;
    while (*input == ' ' || *input == '\t') input++;
    if (strncasecmp(pattern,"{3}",3) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      if (value >= 8) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 3;
      }
    if (strncasecmp(pattern,"{12}",4) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      if (value >= 4096) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 4;
      }
    else if (strncasecmp(pattern,"{B}",3) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      if (value >= 256) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 3;
      }
    else if (strncasecmp(pattern,"{N}",3) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      if (value >= 16) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 3;
      }
    else if (strncasecmp(pattern,"{W}",3) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 3;
      }
    else if (strncasecmp(pattern,"{5}",3) == 0) {
      temp = evaluate(input, &value);
      if (evalErrors != 0) return 0;
      if (value == 0x10) return 0;
      if ((value & 0xffe0) != 0xffe0 &&
          (value & 0xffe0) != 0x0000) return 0;
      args[argCount++] = value;
      input = temp;
      pattern += 3;
      }
    else {
      if (upcase(*pattern) != upcase(*input)) return 0;
      pattern++;
      input++;
      if (*pattern == ' ' && *input != ' ' && *input != '\t') return 0;
      }
    }
  while (*input == ' ' || *input == '\t') input++;
  if (*input == 0 || *input == ';') return -1;
  return 0;
  }

int lookupInstruction(char* input) {
  int i;
  usedExternal = -1;
  for (i=0; i<numOpcodes; i++) {
    argCount = 0;
    if (match(opcodes[i], input) != 0) return i;
    }
  return -1;
  }

void translateInstruction(char* trans) {
  int valid;
  byte b;
  word a;
  valid = 0;
  b = 0;
  while (*trans != 0) {
    if (strncasecmp(trans, "[L]", 3) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        if (pass == 2) fprintf(outFile,"\\%s %04x\n",labelNames[externals[usedExternal]],address);
        }
      output(args[argNumber] & 0xff);
      valid = 0;
      b = 0;
      trans += 3;
      }
    if (strncasecmp(trans, "[S]", 3) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        if (pass == 2) fprintf(outFile,"\\%s %04x\n",labelNames[externals[usedExternal]],address);
        }
      if ((args[argNumber] & 0xff00) != (address & 0xff00) && pass == 2) {
        printf("****ERROR: Branch out of page\n");
        errors++;
        }
      output(args[argNumber] & 0xff);
      valid = 0;
      b = 0;
      trans += 3;
      }
    else if (strncasecmp(trans, "[H]", 3) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        if (pass == 2) fprintf(outFile,"/%s %04x %02x\n",labelNames[externals[usedExternal]],address,args[argNumber] & 0xff);
        }
      output(args[argNumber] >> 8);
      valid = 0;
      b = 0;
      trans += 3;
      }
    else if (strncasecmp(trans, "<N", 2) == 0) {
      b |= (args[argNumber] << 1);
      valid = -1;
      trans += 2;
      }
    else if (strncasecmp(trans, "|H", 2) == 0) {
      b |= (args[argNumber] & 0xf00) >> 8;
      valid = -1;
      trans += 2;
      }
    else if (strncasecmp(trans, "|N", 2) == 0) {
      b |= (args[argNumber] & 0xf);
      valid = -1;
      trans += 2;
      }
    else if (strncasecmp(trans, "[Q]", 3) == 0) {
      if (valid) output(b);
      output(args[argNumber] >> 8);
      output(args[argNumber] & 0xff);
      output(args[argNumber+1] >> 8);
      output(args[argNumber+1] & 0xff);
      valid = 0;
      trans += 3;
      }
    else if (strncasecmp(trans, "[BT]", 4) == 0) {
      b |= ((args[argNumber] & 0x7) << 3) | (args[argNumber+1] & 0x7);
      valid = -1;
      trans += 4;
      }
    else if (strncasecmp(trans, "[TF]", 4) == 0) {
      if (valid) output(b);
      b |= (args[argNumber] << 4) | args[argNumber+1];
      valid = -1;
      trans += 4;
      }
    else if (strncasecmp(trans, "[DW]", 4) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        fprintf(outFile,"?%s %04x\n",labelNames[externals[usedExternal]],address);
        printf("???WARNING: Using relative addresses as EXTRN will likely not produce the expected result\n");
        }
      a = args[argNumber] - (address+2);
      output(a >> 8);
      output(a & 0xff);
      valid = 0;
      b = 0;
      trans += 4;
      }
    else if (strncasecmp(trans, "[D]", 3) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        fprintf(outFile,"\\%s %04x\n",labelNames[externals[usedExternal]],address);
        printf("???WARNING: Using relative addresses as EXTRN will likely not produce the expected result\n");
        }
      a = args[argNumber] - (address+1);
      output(a & 0xff);
      valid = 0;
      b = 0;
      trans += 3;
      }
    else if (strncasecmp(trans, "[W]", 3) == 0) {
      if (valid) output(b);
      if (usedExternal >= 0 && pass == 2) {
        if (pass == 2) fprintf(outFile,"?%s %04x\n",labelNames[externals[usedExternal]],address);
        }
      if (usedLocal >= 0 && pass == 2) {
        fixups[numFixups] = address;
        fixupTypes[numFixups] = 'W';
        fixupLowOffset[numFixups] = 0;
        numFixups++;
        }
      output(args[argNumber] >> 8);
      output(args[argNumber] & 0xff);
      valid = 0;
      b = 0;
      trans += 3;
      }
    else if (strncasecmp(trans, "[5]", 3) == 0) {
      if (valid) b <<= 4;
      b |= (args[argNumber] & 0x1f);
      output(b);
      valid = 0;
      b = 0;
      trans += 3;
      }
    else if (*trans >= '0' && *trans <= '9') {
      b = (b << 4) | (*trans - '0');
      valid = -1;
      trans++;
      }
    else if (*trans >= 'a' && *trans <= 'f') {
      b = (b << 4) | (*trans - 87);
      valid = -1;
      trans++;
      }
    else if (*trans >= 'A' && *trans <= 'F') {
      b = (b << 4) | (*trans - 55);
      valid = -1;
      trans++;
      }
    else if (*trans == '+') {
      if (valid) output(b);
      b = 0;
      valid = 0;
      argNumber++;
      trans++;
      }
    else if (*trans == ' ') {
      if (valid) output(b);
      b = 0;
      valid = 0;
      trans++;
      }
    }
  if (valid) output(b);
  }

void defReplace(char* line) {
  char  buffer[1024];
  char *pchar;
  int i;
  for (i=0; i<numDefines; i++) {
    while ((pchar = strstr(line, defines[i])) != NULL) {
      strncpy(buffer,line,pchar-line);
      buffer[pchar-line] = 0;
      strcat(buffer,defineValues[i]);
      strcat(buffer,pchar+strlen(defines[i]));
      strcpy(line,buffer);
      }
    }
  }

char* nextLine(char* line) {
  int   i;
  char* ret;
  int   flag;
  char  buffer[1024];
  int   pos;
  char *pchar;
  word  value;
  flag = -1;
  while (flag) {
    ret = fgets(line, 1024, sourceFile[fileNumber]);
    lineCount++;
    lineNumber[fileNumber]++;
    if (ret != NULL) {
      while (strlen(ret) > 0 && line[strlen(ret)-1] <= ' ')
        line[strlen(ret)-1] = 0;
      flag = 0;
      if (*ret == '#') {
        if (fileNumber == 0)
          sprintf(listLine,"[%05d] ",lineNumber[fileNumber]);
        else
          sprintf(listLine,"<%05d> ",lineNumber[fileNumber]);
        while (strlen(listLine) < 25) strcat(listLine," ");
        strcat(listLine,"  ");
        strcat(listLine,ret);
        if (pass == 2) {
          if (showList != 0) printf("%s\n",listLine);
          if (createListFile != 0) fprintf(listFile,"%s\n",listLine);
          }
        flag = -1;
        if (strncmp(ret,"#include ",9) == 0) {
          ret += 9;
          while (*ret == ' ' || *ret == '\t') ret++;
          pos = 0;
          while (*ret != 0 && *ret > ' ')
            buffer[pos++] = *ret++;
          buffer[pos] = 0;
          fileNumber++;
          lineNumber[fileNumber] = 0;
          sourceFile[fileNumber] = fopen(buffer,"r");
          if (sourceFile[fileNumber] == NULL) {
            }
          }
        if (strncmp(ret,"#define ",8) == 0) {
          ret += 8;
          while (*ret == ' ' || *ret == '\t') ret++;
          pos = 0;
          while (*ret != 0 && *ret > ' ')
            buffer[pos++] = *ret++;
          buffer[pos] = 0;
          while (*ret == ' ' || *ret == '\t') ret++;
          if (*ret == 0) addDefine(buffer,"1");
            else addDefine(buffer, ret);
          }
        if (strncmp(ret,"#ifdef ",7) == 0) {
          ret += 7;
          while (*ret == ' ' || *ret == '\t') ret++;
          pos = 0;
          while (*ret != 0 && *ret > ' ')
            buffer[pos++] = *ret++;
          buffer[pos] = 0;
          pchar = findDefine(buffer);
          if (pchar != NULL) {
            numNests++;
            nests[numNests] = 'Y';
            }
          else {
            numNests++;
            nests[numNests] = 'N';
            }
          }
        if (strncmp(ret,"#ifndef ",8) == 0) {
          ret += 8;
          while (*ret == ' ' || *ret == '\t') ret++;
          pos = 0;
          while (*ret != 0 && *ret > ' ')
            buffer[pos++] = *ret++;
          buffer[pos] = 0;
          pchar = findDefine(buffer);
          if (pchar != NULL) {
            numNests++;
            nests[numNests] = 'N';
            }
          else {
            numNests++;
            nests[numNests] = 'Y';
            }
          }
        if (strncmp(ret,"#if ",4) == 0) {
          ret += 4;
          while (*ret == ' ' || *ret == '\t') ret++;
          defReplace(ret);
          evaluate(ret, &value);
          if (value != 0) {
            numNests++;
            nests[numNests] = 'Y';
            }
          else {
            numNests++;
            nests[numNests] = 'N';
            }
          }
        if (strncmp(ret,"#endif",6) == 0) {
          if (numNests > 0) numNests--;
          else {
            printf("Error: Unmatched #endif\n");
            errors++;
            }
          }
        if (strncmp(ret,"#else",5) == 0) {
          if (numNests > 0)
            nests[numNests] = (nests[numNests] == 'Y') ? 'N' : 'Y';
          else {
            printf("Error: Unmatched #else\n");
            errors++;
            }
          }
        }
      else if (nests[numNests] != 'Y') {
        flag = -1;
        }
      else {
        defReplace(ret);
        }
      }
    else {
      if (fileNumber == 0) flag = 0;
      else {
        fclose(sourceFile[fileNumber]);
        fileNumber--;
        flag = -1;
        }
      }
    }
  return ret;
  }

int assemblyPass(char* sourceName) {
  int  o;
  int  i;
  word value;
  char *pchar;
  char *pline;
  char  err;
  FTOI ftoi;
  char line[1024];
  sourceFile[0] = fopen(sourceName, "r");
  if (sourceFile[0] == NULL) {
    printf("Error: Could not open: %s\n",sourceName);
    return -1;
    }
  lineNumber[0] = 0;
  fileNumber = 0;
  lineCount = 0;
  address = 0;
  outCount = 0;
  outAddress = 0;
  codeGenerated = 0;
  numDefines = 0;
  nests[0] = 'Y';
  numNests = 0;
  strcpy(module,"*");
  inProc = 0;
  while (nextLine(line)) {
    strcpy(sourceLine, line);
    if (strncmp(line,".list",5) == 0) showList = -1;
    if (strncmp(line,".sym",4) == 0) showSymbols = -1;
    if (strncmp(line,".link ",6) == 0) {
      if (pass == 2) {
        fprintf(outFile,"%s\n",line+6);
        }
      }
    if (strncmp(line,".sym",4) == 0) showSymbols = -1;
    if (strncmp(line,".align word", 11) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 1) & 0xfffe;
      outAddress = address;
      }
    if (strncmp(line,".align dword", 12) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 3) & 0xfffc;
      outAddress = address;
      }
    if (strncmp(line,".align qword", 12) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 7) & 0xfff8;
      outAddress = address;
      }
    if (strncmp(line,".align para", 11) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 15) & 0xfff0;
      outAddress = address;
      }
    if (strncmp(line,".align 32", 9) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 31) & 0xffe0;
      outAddress = address;
      }
    if (strncmp(line,".align 64", 9) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 63) & 0xffc0;
      outAddress = address;
      }
    if (strncmp(line,".align 128", 10) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 127) & 0xff80;
      outAddress = address;
      }
    if (strncmp(line,".align page", 11) == 0) {
      if (outCount != 0) writeOutput();
      address = (address + 255) & 0xff00;
      outAddress = address;
      }
    if (line[0] == '.') strcpy(line,"");
    pchar = strchr(line, ';');
    if (pchar != NULL) {
      *pchar = 0;
      while (strlen(line) > 0 && line[strlen(line)-1] <= ' ')
        line[strlen(line)-1] = 0;
      }
    pline = line;
    if (*pline != 0 && *pline != ' ' && *pline != '\t') {
      i = 0;
      while (*pline != ' ' && *pline != '\t' && *pline != ':' && *pline != 0)
        label[i++] = *pline++;
      label[i] = 0;
      }
    else strcpy(label, "");
    if (*pline == ':') pline++;
    while (*pline != 0 && *pline <= ' ') pline++;
    if (strlen(label) > 0 || strlen(pline) > 0) {
      if (fileNumber == 0)
        sprintf(listLine,"[%05d] %04x:",lineNumber[fileNumber],address);
      else
        sprintf(listLine,"<%05d> %04x:",lineNumber[fileNumber],address);
      if (strlen(label) > 0) addLabel(label, address);

      instructionBytes = 0;
      if (strlen(pline) > 0) {
        if (strncasecmp(pline,"org",3) == 0) {
          if (outCount != 0) writeOutput();
          pline = nextWord(pline);
          evaluate(pline, &address);
          outAddress = address;
          }
        else if (strncasecmp(pline,"extrn",5) == 0) {
          pline = nextWord(pline);
          addExternal(pline);
          }
        else if (strncasecmp(pline,"proc",4) == 0) {
          if (inProc) {
            printf("***Error: PROC encountered inside of another PROC %s\n",pline);
            errors++;
            }
          else {
            pline = nextWord(pline);
            if (outCount != 0) writeOutput();
            address = 0;
            outAddress = 0;
            inProc = -1;
            strcpy(module, pline);
            if (pass == 1) addLabel(pline, 0);
            if (pass == 2) {
              fprintf(outFile,"{%s\n",pline);
              }
            numFixups = 0;
            }
          }
        else if (strcasecmp(pline,"endp") == 0) {
          if (inProc == 0) {
            printf("***ERROR: ENDP encountered outside PROC\n");
            errors++;
            }
          inProc = 0;
          if (outCount != 0) writeOutput();
          if (pass == 2) {
            for (i=0; i<numFixups; i++) {
              if (fixupTypes[i] == 'W')
                fprintf(outFile,"+%04x\n",fixups[i]);
              if (fixupTypes[i] == 'H') {
                if (fixupLowOffset[i] != 0)
                  fprintf(outFile,"^%04x %02x\n",fixups[i],fixupLowOffset[i]);
                else
                  fprintf(outFile,"^%04x\n",fixups[i]);
                }
              if (fixupTypes[i] == 'L')
                fprintf(outFile,"v%04x\n",fixups[i]);
              }
            fprintf(outFile,"}\n");
            }
          }
        else if (strncasecmp(pline,"public",6) == 0) {
          if (pass == 2) {
            pline = nextWord(pline);
            i = findLabelNumber(pline);
            if (i < 0) {
              printf("****ERROR: Symbol not found: %s\n",pline);
              errors++;
              }
            else {
              numPublics++;
              if (numPublics == 1)
                publics = (int*)malloc(sizeof(int));
              else
                publics = (int*)realloc(publics,sizeof(int)*numPublics);
              publics[numPublics-1] = i;
              }
            }
          }
        else if (strncasecmp(pline,"ds",2) == 0) {
          if (outCount != 0) writeOutput();
          pline = nextWord(pline);
          evaluate(pline, &value);
          address += value;
          outAddress = address;
          }
        else if (strncasecmp(pline,"db",2) == 0 ||
                 strncasecmp(pline,".byte",5) == 0 ||
                 strncasecmp(pline,"byte",4) == 0) {
          pline = nextWord(pline);
          pchar = pline;
          while (*pchar != 0) {
            if (*pchar == '"') {
              pchar++;
              while (*pchar != 0 && *pchar != '"') {
                output(*pchar);
                pchar++;
                }
              if (*pchar == '"') pchar++;
              }
            else {
              pchar = evaluate(pchar, &value);
              output(value & 0xff);
              }
            while (*pchar == ' ') pchar++;
            if (*pchar == ',') pchar++;
            while (*pchar == ' ') pchar++;
            }
          }
        else if (strncasecmp(pline,"dw",2) == 0 ||
                 strncasecmp(pline,"word",4) == 0 ||
                 strncasecmp(pline,".word",5) == 0) {
          pline = nextWord(pline);
          pchar = pline;
          while (*pchar != 0) {
            pchar = evaluate(pchar, &value);
            output((value >> 8) & 0xff);
            output(value & 0xff);
            while (*pchar == ' ') pchar++;
            if (*pchar == ',') pchar++;
            while (*pchar == ' ') pchar++;
            }
          }
        else if (strncasecmp(pline,"df",2) == 0) {
          pline = nextWord(pline);
          ftoi.f = atof(pline);
          output((ftoi.i >> 24) & 0xff);
          output((ftoi.i >> 16) & 0xff);
          output((ftoi.i >> 8) & 0xff);
          output(ftoi.i & 0xff);
          }
        else if (strncasecmp(pline,"end",3) == 0) {
          if (outCount != 0) writeOutput();
          pline = nextWord(pline);
          evaluate(pline, &startAddress);
          }
        else if (strncasecmp(pline,"equ",3) == 0 ||
                 strncasecmp(pline,".equ",4) == 0) {
          pline = nextWord(pline);
          evaluate(pline, &value);
          if (strlen(label) > 0) setLabel(label, value);
          }
        else {
          indexNumber = 0xffff;
          o = lookupInstruction(pline);
          if (o >= 0) {
            argNumber = 0;
            translateInstruction(translations[o]);
            }
          else {
            printf("****ERROR: Invalid instruction\n");
            printf("   [%05d] %s\n",lineCount,sourceLine);
            errors++;
            }
          }
        }
      if (instructionBytes < 4) {
        while (strlen(listLine) < 25) strcat(listLine," ");
        strcat(listLine,"  ");
        strcat(listLine,sourceLine);
        }
      if (instructionBytes < 4 || ((instructionBytes % 4) != 0) && instructionBytes != 4)
        if (pass == 2) {
          if (showList != 0) printf("%s\n",listLine);
          if (createListFile != 0) fprintf(listFile,"%s\n",listLine);
          }
      }
    else {
      if (fileNumber == 0)
        sprintf(listLine,"[%05d] ",lineNumber[fileNumber]);
      else
        sprintf(listLine,"<%05d> ",lineNumber[fileNumber]);
      while (strlen(listLine) < 27) strcat(listLine," ");
      strcat(listLine, sourceLine);
      if (pass == 2) {
        if (showList != 0) printf("%s\n",listLine);
        if (createListFile != 0) fprintf(listFile,"%s\n",listLine);
        }
      }
    }
  while (fileNumber > 0) {
    fclose(sourceFile[fileNumber--]);
    }
  fclose(sourceFile[0]);
  for (i=0; i<numDefines; i++) {
    free(defines[i]);
    free(defineValues[i]);
    }
  if (numDefines > 0) {
    free(defines);
    free(defineValues);
    }
  return errors;
  }

void assembleFile(char* sourceName) {
  int   i;
  char *pchar;
  char buffer[1024];
  char tmp[1024];
  FILE *buildFile;
  strcpy(baseName, sourceName);
  pchar = strchr(baseName,'.');
  if (pchar != NULL) *pchar = 0;
  strcpy(listName, baseName);
  strcat(listName, ".lst");
  strcpy(outName, baseName);
  strcat(outName, ".prg");

  printf("Source file: %s\n",sourceName);
  errors = 0;
  numLabels = 0;
  numExternals = 0;
  numReferences = 0;
  numPublics = 0;
  startAddress = 0xffff;

  strcpy(tmp, baseName);
  strcat(tmp,".build");
  buildFile = fopen(tmp, "r");
  if (buildFile == NULL) {
    buildNumber = 1;
    }
  else {
    fgets(buffer, 32, buildFile);
    buildNumber = atoi(buffer) + 1;
    fclose(buildFile);
    }
  buildFile = fopen(tmp,"w");
  fprintf(buildFile,"%d\n",buildNumber);
  fclose(buildFile);

  pass = 1;
  if (assemblyPass(sourceName) != 0) {
    printf("Errors encountered. Assembly aborted\n");
    }
  else {
    pass = 2;
    outFile = fopen(outName, "w");
    fprintf(outFile,".big\n");
    if (createListFile != 0) listFile = fopen(listName, "w");
    assemblyPass(sourceName);
    if (outBytes > 0) writeOutput();
    if (startAddress != 0xffff)
      fprintf(outFile,"@%04x\n",startAddress);
    if (numReferences > 0) {
      for (i=0; i<numReferences; i++) {
        fprintf(outFile,"?%s %04x\n",externals[references[i]],referenceAddress[i]);
        }
      }
    if (numPublics > 0) {
      for (i=0; i<numPublics; i++) {
        fprintf(outFile,"=%s %04x\n",labelNames[publics[i]],labelValues[publics[i]]);
        }
      }
    fclose(outFile);
    if (showSymbols != 0) {
      printf("\n");
      if (createListFile) fprintf(listFile,"\n");
      for (i=0; i<numLabels; i++) {
        sprintf(buffer,"%-16s %-16s %04x <%05d> \n",labelNames[i],labelProcs[i],labelValues[i],labelLine[i]);
        printf("%s",buffer);
        if (createListFile) fprintf(listFile,"%s",buffer);
        }
      printf("\n");
      if (createListFile) fprintf(listFile,"\n");
      }
    }
  printf("Lines assembled: %d\n",lineCount);
  printf("Errors         : %d\n",errors);
  printf("Code generated : %d\n",codeGenerated);
  if (createListFile != 0 && pass == 2) {
    fprintf(listFile,"Lines assembled: %d\n",lineCount);
    fprintf(listFile,"Errors         : %d\n",errors);
    fprintf(listFile,"Code generated : %d\n",codeGenerated);
    }
  if (createListFile != 0 && pass == 2) fclose(listFile);
  printf("\n");
  for (i=0; i<numLabels; i++) {
    free(labelNames[i]);
    }
  if (numExternals > 0) free(externals);
  if (numPublics > 0) free(publics);
  if (numReferences > 0) {
    free(references);
    free(referenceAddress);
    }
  if (numLabels > 0) {
    free(labelNames);
    free(labelValues);
    free(labelLine);
    }
  }

int main(int argc, char **argv) {
  int i;
  time_t tv;
  struct tm dt;
  printf("Asm/04 1.0 by Michael H. Riley\n\n");
  initOpcodes();
  showList = 0;
  showSymbols = 0;
  createListFile = 0;
  numSourceFiles = 0;
  tv = time(NULL);
  localtime_r(&tv, &dt);
  buildMonth = dt.tm_mon + 1;
  buildDay = dt.tm_mday;
  buildYear = dt.tm_year + 1900;
  buildHour = dt.tm_hour;
  buildMinute = dt.tm_min;
  buildSecond = dt.tm_sec;
  for (i=1; i<argc; i++) {
    if (strcmp(argv[i],"-l") == 0) showList = -1;
    else if (strcmp(argv[i],"-L") == 0) createListFile = -1;
    else if (strcmp(argv[i],"-s") == 0) showSymbols = -1;
    else {
      numSourceFiles++;
      if (numSourceFiles == 1)
        sourceNames = (char**)malloc(sizeof(char*));
      else
        sourceNames = (char**)realloc(sourceNames,sizeof(char*) * numSourceFiles);
      sourceNames[numSourceFiles-1] = (char*)malloc(strlen(argv[i])+1);
      strcpy(sourceNames[numSourceFiles-1], argv[i]);
      }
    }
  if (numSourceFiles == 0) {
    printf("No source file specified\n");
    exit(1);
    }
  for (i=0; i<numSourceFiles; i++)
    assembleFile(sourceNames[i]);
  return 0;
  }

