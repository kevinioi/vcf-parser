/**
 * @file CardHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the functions needed to
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "LinkedListAPI.h"
#include "PropertyHelper.h"

Card *initializeCard(char* (*printProp)(void* toBePrinted),void (*deleteProp)(void* toBeDeleted),int (*compareProp)(const void* first,const void* second));
