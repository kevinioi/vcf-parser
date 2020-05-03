/**
 * @file ParseHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the functions needed to
 */

#ifndef _PARSEHELPER_H_
#define  _PARSEHELPER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "LinkedListAPI.h"

char *printString(void *string);

void deleteString(void *string);

int compareString(const void *string1, const void *string2);

int strcmpIC(const char *string1,const char *string2);

void parseContentLine(Property *newProp, char *contentLine);

VCardErrorCode extractGroup(char **contentLine, char **group);

VCardErrorCode extractProp(char **contentLine, char **prop);

VCardErrorCode openFileRead(FILE **fp, char *fileName);

VCardErrorCode readVCard(FILE *fp, char **fileContents);

VCardErrorCode nextContentLine(char *source, char **nextLine, int *fileIndex);

VCardErrorCode parseParameters(char ***contentLine, List *paramList);

VCardErrorCode parsePropertyValues(char ***contentLine, List *propertyList);

char* toStringNoBreak(List * list);

#endif
