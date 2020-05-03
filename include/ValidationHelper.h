/**
 * @file ValidationHelper.h
 * @author Kevin ioi
 * @date Oct 2018
 * @brief File containing the functions needed to
 */

#ifndef _VALIDATIONHELPER_H_
#define  _VALIDATIONHELPER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "LinkedListAPI.h"


typedef enum properties {SOURCE, KIND, XML, FN, N, NICKNAME, PHOTO, BDAY,
						ANNIVERSARY, GENDER, ADR, TEL, EMAIL, IMPP, LANG, TZ, GEO, TITLE,
						ROLE, LOGO, ORG, MEMBER, RELATED, CATEGORIES, NOTE, PRODID, REV,
						SOUND, UID, CLIENTPIDMAP, URL, VERSION, KEY, FBURL, CALADRURI, CALURI} pValue;

typedef struct propertyNode{
	int count;
	char *name;
	}pNode;


VCardErrorCode validateProp(const Property *prop, List *pList);

VCardErrorCode validateParam(const Parameter *param);

VCardErrorCode validateDT(const DateTime *date);

pNode *createPNode(char *name);

void loadPropertyCounter(List *pList);

void deletePropNode(void *toDelete);

char *printPropNode(void *toPrint);

int countProperty(char *name, List *pList);

int cmpPropNode(const void *one, const void *two);

bool nodeNameCmp(const void *node, const void *name);


#endif
