/**
 * @file ParseHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the helper functions used to read-in, construct and parse content
 *        lines of a vcf file (plus print, delete and compare string)
 */

#include "VCardParser.h"
#include "LinkedListAPI.h"
#include "WriteCardHelper.h"

VCardErrorCode openFileWrite(FILE **fp, const char *fileName)
{
  if (fileName == NULL)//no file address provided
  {
    return WRITE_ERROR;
  }

  if (strcmp(&(fileName[strlen(fileName) - 3]), "vcf") != 0 && strcmp(&(fileName[strlen(fileName) - 5]), "vcard") != 0)//check if valid file extension
  {
    return WRITE_ERROR;
  }

  //open file & make sure it opens
  *fp = fopen(fileName, "w+");
  if (*fp == NULL)
  {
    return WRITE_ERROR;
  }

  return OK;

}

VCardErrorCode writeProperty(FILE **fp, Property *writeProp)
{
  //check for validity of paramter values
  if (writeProp == NULL || fp == NULL)
    return WRITE_ERROR;
  if (*fp == NULL)
    return WRITE_ERROR;

  char *contentLine;
  VCardErrorCode parseStatus = OK;

  if((contentLine = malloc(200)) == NULL)
    return WRITE_ERROR;

  strcpy(contentLine, "");

  if (strlen(writeProp->group) > 0)
  {
    strcat(contentLine, writeProp->group);
    strcat(contentLine, ".");
  }

  strcat(contentLine, writeProp->name);

  //add parameters to contentLine, if exist
  if (getLength(writeProp->parameters) > 0)
    parseStatus = paramListToString(&contentLine, writeProp->parameters);


  if (parseStatus == OK)
    parseStatus = propListToString(&contentLine, writeProp->values);

  if (parseStatus == OK)
    fprintf(*fp, "%s\r\n", contentLine);

  free(contentLine);

  return parseStatus;
}


VCardErrorCode propListToString(char **contentLine, List *values)
{
  ListIterator iter = createIterator(values);
  char *tempValue;

  strcat(*contentLine, ":");

  while ((tempValue = (char*)nextElement(&iter)) != NULL)
  {
    strcat(*contentLine, tempValue);
    strcat(*contentLine, ";");
  }

  (*contentLine)[strlen(*contentLine) - 1] = '\0';//remove last semicolon

  return OK;
}

VCardErrorCode paramListToString(char **contentLine, List *parameters)
{
  ListIterator iter = createIterator(parameters);
  Parameter *tempParam;

  while ((tempParam = (Parameter*)nextElement(&iter)) != NULL)
  {
    strcat(*contentLine, ";");
    strcat(*contentLine, tempParam->name);
    strcat(*contentLine, "=");
    strcat(*contentLine, tempParam->value);
  }

  return OK;
}

VCardErrorCode writeOptionalProps(FILE **fp, List *props)
{
  if (fp == NULL || props == NULL)
    return WRITE_ERROR;
  if (*fp == NULL)
    return WRITE_ERROR;

  VCardErrorCode parseStatus;
  Property *tempProp;
  ListIterator iter = createIterator(props);

  while ((tempProp = (Property*)nextElement(&iter)) != NULL)
  {
    if((parseStatus = writeProperty(fp, tempProp))!= OK)
      return parseStatus;
  }

  return OK;
}

VCardErrorCode writeDateTime(char **contentLine, DateTime *writeDate)
{
  if (writeDate == NULL)
  {
     return OK;
  }

  if (writeDate->isText == 1)
  {
    strcat(*contentLine, ";Value=Text:");
    strcat(*contentLine, writeDate->text);
  }
  else if(strlen(writeDate->date)>0)
  {
    strcat(*contentLine, ":");
    strcat(*contentLine, writeDate->date);

    if (strlen(writeDate->time)>0)
    {
      strcat(*contentLine, "T");
      strcat(*contentLine, writeDate->time);
    }

    if (writeDate->UTC == 1)
    {
      strcat(*contentLine, "Z");
    }
  }
  else if (strlen(writeDate->time)>0)
  {
    strcat(*contentLine, ":");
    strcat(*contentLine, "T");
    strcat(*contentLine, writeDate->time);
    if (writeDate->UTC == 1)
    {
      strcat(*contentLine, "Z");
    }
  }
  else
  {
    return WRITE_ERROR;
  }

  return OK;
}
