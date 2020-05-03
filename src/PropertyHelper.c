/**
 * @file PropertyHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the helper function used to create a new property struct
 */

#include "VCardParser.h"
#include "ParseHelper.h"
#include "LinkedListAPI.h"
#include "PropertyHelper.h"
#include "DateHelper.h"

/*
* newProperty
*
*/
VCardErrorCode newProperty(char *name, char *group, Property **newProp, char **contentLine)
{
  VCardErrorCode parseStatus = OK;

  if (name == NULL || (*contentLine) == NULL)
    return OTHER_ERROR;

  if((*newProp = malloc(sizeof(Property)))== NULL)
  {
    *newProp = NULL;
    free(name);
    if (group != NULL)
      free(group);
    return OTHER_ERROR;
  }

  if(((*newProp)->name = malloc(sizeof(char)*strlen(name)+1))==NULL)//allocate space for name
  {
    free(newProp);
    free(name);
    if (group != NULL)
      free(group);
    return OTHER_ERROR;
  }

  strcpy((*newProp)->name, name);//copy the property name over
  free(name);

  //initialize elements
  if(((*newProp)->parameters = initializeList(printParameter, deleteParameter, compareParameters))==NULL)
  {
    free((*newProp)->name);
    free(*newProp);
    if (group != NULL)
      free(group);
    *newProp = NULL;
    return OTHER_ERROR;
  }
  if(((*newProp)->values = initializeList(printString, deleteString, compareString))==NULL)
  {
    freeList((*newProp)->parameters);
    free((*newProp)->name);
    free(*newProp);
    *newProp = NULL;
    return OTHER_ERROR;
  }

  if (group != NULL)//there is a group
  {
    if(((*newProp)->group = malloc(strlen(group)+1))==NULL)
    {
      freeList((*newProp)->values);
      freeList((*newProp)->parameters);
      free((*newProp)->name);
      free(*newProp);
      *newProp = NULL;
      return OTHER_ERROR;
    }
    strcpy((*newProp)->group, group);
    (*newProp)->group[strlen(group)] = '\0';
    free(group);
  }
  else//no group provided
  {
    if(((*newProp)->group = malloc(1))==NULL)
    {
      freeList((*newProp)->values);
      freeList((*newProp)->parameters);
      free((*newProp)->name);
      free(*newProp);
      *newProp = NULL;
      return OTHER_ERROR;
    }
    strcpy((*newProp)->group, "\0");
  }

  if((parseStatus =  parseParameters(&contentLine, (*newProp)->parameters ))!=OK)//check for and add parameters to property
  {
    deleteProperty(*newProp);
    return parseStatus;
  }

  if((parseStatus =  parsePropertyValues(&contentLine, (*newProp)->values ))!=OK)//check for and add values to property
  {
    deleteProperty(*newProp);
    return parseStatus;
  }

  return parseStatus;
}


Property *createProperty()
{
  Property *newProp;

  newProp = malloc(sizeof(Property));
  newProp->parameters = initializeList(printParameter, deleteParameter, compareParameters);
  newProp->values = initializeList(printString, deleteString, compareString);

  return newProp;
}
