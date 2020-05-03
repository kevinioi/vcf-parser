/**
 * @file PropertyHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the functions needed to
 */

#ifndef _PROPERTYHELPER_H
#define  _PROPERTYHELPER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LinkedListAPI.h"
#include "VCardParser.h"


VCardErrorCode newProperty(char *name, char *group, Property **newProp, char **contentLine);

Property *createProperty();

#endif
