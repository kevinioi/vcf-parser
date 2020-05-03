/**
 * @file ParseHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the helper functions used to read-in, construct and parse content
 *        lines of a vcf file (plus print, delete and compare string)
 */

#include "VCardParser.h"
#include "ParseHelper.h"
#include "LinkedListAPI.h"
#include "PropertyHelper.h"

/*
* printString
*/
char *printString(void *string)
{
  char *newString = calloc(sizeof(char), strlen(((char*)string))+1);
  memcpy(newString, string, sizeof(char)*strlen((char*)string));
  newString[strlen((char*)string)] = '\0';

  return newString;
}

/*
* deleteString
*/
void deleteString(void *string)
{
  if (string != NULL)
  {
    free((char*)string);
  }
}

/*
* compareString
*/
int compareString(const void *string1, const void *string2)
{
  return strcmp(string1, string2);
}

/**
* strcmpIC()
*
* string compare ignore case
*
**/
int strcmpIC(const char *string1,const char *string2)
{
  char *lowerS1, *lowerS2;
  int strlen1, strlen2;
  int i = 0;
  int result;

  strlen1 = strlen(string1);
  strlen2 = strlen(string2);

  lowerS1 = malloc(sizeof(char)*(strlen1 + 1));
  lowerS2 = malloc(sizeof(char)*(strlen2 + 1));


  for (i = 0; i < strlen1; i++)
  {
    lowerS1[i] = tolower(string1[i]);
  }
  lowerS1[i] = '\0';


  for (i = 0; i < strlen2; i++)
  {
    lowerS2[i] = tolower(string2[i]);
  }
  lowerS2[i] = '\0';

  result = strcmp(lowerS1, lowerS2);
  free(lowerS1);
  free(lowerS2);

  return result;
}

/**
* extractProp()
*
**/
VCardErrorCode extractProp(char **contentLine, char **prop)
{
  if (contentLine == NULL)
    return INV_PROP;
  else if(*contentLine == NULL)
    return INV_PROP;
  else if (strlen(*contentLine) == 0)
    return INV_PROP;

  int length = strlen(*contentLine);//length of the content line
  int i = 0;//copying index
  char *reducedContentLine;
  char *newProperty;

  if((newProperty = calloc(sizeof(char), 100))==NULL)//allocate room for property
    return OTHER_ERROR;

  while ((isalpha((*contentLine)[i]) != 0 || (*contentLine)[i] == '-') && i <length)//copy property name until hit punctuation
  {
    newProperty[i] = (*contentLine)[i];
    i++;
  }


  if (i < length)//valid read
  {
    newProperty[i] = '\0';

    if (strcmpIC(newProperty, "END") != 0) {
      if ((*contentLine)[length-1] != '\n' && (*contentLine)[length-2] != '\r')//invalid line end
        return INV_PROP;
    }

    if((reducedContentLine = calloc(sizeof(char), strlen(*contentLine) + 1))==NULL)//allocate room for smaller contentLine
    {
      free(newProperty);
      return OTHER_ERROR;
    }

    strcpy(reducedContentLine, &((*contentLine)[i]));//copy relevent portion of contentLine in to reduced
    free(*contentLine);
    *contentLine = reducedContentLine;


    *prop = newProperty;
    return OK;
  }
  else//reached end of line, invalid contentLine
  {
    free(newProperty);
    return INV_PROP;
  }
}

/**
* extractGroup()
*
**/
VCardErrorCode extractGroup(char **contentLine, char **group)
{
  if (contentLine == NULL){
    *group = NULL;
    return OTHER_ERROR;//No pointer to contentLine provided
  }
  else if(*contentLine == NULL){
    *group = NULL;
    return OTHER_ERROR;//No contentLine provided
  }
  else if (strlen(*contentLine) == 0){
    *group = NULL;
    return OTHER_ERROR;//Empty content line provided
  }

  int i = 0;//counter
  int length = strlen(*contentLine);
  char *newGroup;//holds the group string
  char *temp;//temp string to handle realloc failure
  char *reducedContentLine;//temp string to help make the content line smaller

  if((newGroup = malloc(200))==NULL)
  {
    *group = NULL;
    return OTHER_ERROR;
  }

  while (i < length && (*contentLine)[i] != '.' && (*contentLine)[i] != ':'&& (*contentLine)[i] != ';')//read group name into the string, char by char
  {
    newGroup[i] = (*contentLine)[i];
    i++;
  }

  if ((*contentLine)[i] == '.')//proper group format
  {
    newGroup[i] = '\0';
    if((reducedContentLine = calloc(sizeof(char), strlen(&(*contentLine)[i])+3))==NULL)
    {
      free(newGroup);
      return OTHER_ERROR;
    }
    if ((temp = realloc(newGroup, i+1))==NULL)
    {
      free(newGroup);
      free(reducedContentLine);
      return OTHER_ERROR;
    }
    else
      newGroup = temp;
    strcpy(reducedContentLine, &((*contentLine)[i+1]));
    free(*contentLine);
    *contentLine = reducedContentLine;
    *group = newGroup;
  }
  else//improper group format or no group found
  {
    free(newGroup);
    *group = NULL;
  }

  return OK;
}

/**
* openFile()
*
**/
VCardErrorCode openFileRead(FILE **fp, char *fileName)
{
  if (fileName == NULL)//no file address provided
  {
    return INV_FILE;
  }

  if (strcmp(&(fileName[strlen(fileName) - 3]), "vcf") != 0 && strcmp(&(fileName[strlen(fileName) - 5]), "vcard") != 0)//check if valid file extension
  {
    return INV_FILE;
  }

  //open file & make sure it opens
  *fp = fopen(fileName, "r+");
  if (*fp == NULL)
  {
    return INV_FILE;
  }

  return OK;
}

/**
* nextContentLine()
*
**/
VCardErrorCode nextContentLine(char *source, char **nextLine, int *fileIndex)
{
  int lineStart, lineEnd;//keep track of start and end indexes of the current contentline
  int lineRead = 0;//dummy variable used to mark when done reading a line
  int sourceMax = strlen(source);
  char *tempLine;//used to guard from realloc failure
  char *tempNxtLine;
  int memSize = 200;

	if (sourceMax < 20)
		return INV_FILE;
  if (*fileIndex >= sourceMax)
  {
    *nextLine = NULL;
    return OK;
  }

  if((*nextLine = malloc(sizeof(char)*200)) == NULL)
  {
    return OTHER_ERROR;
  }
  if((tempNxtLine = malloc(sizeof(char)*200)) == NULL)
  {
    free(*nextLine);
    return OTHER_ERROR;
  }

  strcpy(*nextLine, "");

  lineStart = *fileIndex;
  lineEnd = lineStart;

  if(*fileIndex == 0)//at
  {
    lineEnd++;
  }


  while (lineRead != 1 && lineEnd != sourceMax)
  {
		if(source[lineEnd] == '\n' && source[lineEnd-1] != '\r')//found new line without carriage return, error
		{
			free(*nextLine);
			return INV_PROP;
		}
		if(source[lineEnd] == '\r' && source[lineEnd+1] != '\n')//found carriage return without new line, error
		{
			free(*nextLine);
			return INV_PROP;
		}
    else if(source[lineEnd] == '\n' && source[lineEnd-1] == '\r')//found a break
    {
      if (source[lineEnd+1] == ' ' || source[lineEnd+1] == '\t')//folded line found
      {
        strncpy(tempNxtLine, &(source[lineStart]), lineEnd-lineStart-1);
        (tempNxtLine)[lineEnd-lineStart-1] = '\0';

        if((strlen(tempNxtLine)+strlen(*nextLine))>memSize)
        {
          if ((tempLine = realloc(*nextLine, memSize+200))==NULL)//realloc space if needed
          {
            free(*nextLine);
            return OTHER_ERROR;
          }
          *nextLine = tempLine;
          memSize += 200;
        }

        strcat(*nextLine, tempNxtLine);
        free(tempNxtLine);
        tempNxtLine = NULL;
        lineStart = lineEnd+2;
        if((tempNxtLine = malloc(sizeof(char)*200)) == NULL)
        {
          free(*nextLine);
          return OTHER_ERROR;
        }
      }
      else//line end found
      {
        strncpy(tempNxtLine, &(source[lineStart]), lineEnd-lineStart+1);
        (tempNxtLine)[lineEnd-lineStart+1] = '\0';
        *fileIndex = lineEnd+1;
        lineRead = 1;
        strcat(*nextLine, tempNxtLine);
        free(tempNxtLine);
        tempNxtLine = NULL;
      }
    }


    lineEnd++;
  }

  if (lineEnd == sourceMax && tempNxtLine != NULL)
  {
    strncpy(tempNxtLine, &(source[lineStart]), lineEnd-lineStart+1);
    (tempNxtLine)[lineEnd-lineStart+1] = '\0';
    *fileIndex = lineEnd+1;
    lineRead = 1;
    strcat(*nextLine, tempNxtLine);
    free(tempNxtLine);
  }

  return OK;
}

/**
* readVCard()
*
**/
VCardErrorCode readVCard(FILE *fp, char **fileContents)
{
  char *fileLine;//to hold file lines as they're read in
  char *tempMem;//holds memory after realloc to check if funtion failed
  int memSize = 400, fileLength = 0;//keep track of memory allocated and memory used
  int lineLength;//keep track of how much we're adding to the file

  if ((*fileContents = malloc(400))==NULL)
  {
    fclose(fp);
    return OTHER_ERROR;
  }
  strcpy(*fileContents,"");//initialize the string to be ready for strcat

  if ((fileLine = malloc(200))==NULL)
  {
    free(*fileContents);
    fclose(fp);
    return OTHER_ERROR;
  }

  while(fgets(fileLine, 200, fp) != NULL)//loop until entire file has been read
  {

    lineLength = strlen(fileLine);

    if ((lineLength+fileLength)>= memSize)//realloc if need more memory for line
    {
      if((tempMem = realloc(*fileContents, memSize + 400))==NULL)
      {
        free(*fileContents);
        free(fileLine);
        fclose(fp);
        return OTHER_ERROR;
      }
      *fileContents = tempMem;
      memSize += 400;
    }

    strcat(*fileContents, fileLine);
    fileLength+= lineLength;

    free(fileLine);
    if ((fileLine = malloc(300))==NULL)
    {
      free(*fileContents);
      fclose(fp);
      return OTHER_ERROR;
    }
  }
  free(fileLine);

  if((tempMem = realloc(*fileContents, strlen(*fileContents)+1))==NULL)
  {
    free(*fileContents);
    fclose(fp);
    return OTHER_ERROR;
  }
  *fileContents = tempMem;

  fclose(fp);


  return OK;
}

/**
* parseParameters()
*
**/
VCardErrorCode parseParameters(char ***contentLine, List *paramList)
{
  int contentIndex = 0;//counter for current index of contentLine string
  int paramIndex = 0;////counter for current index of parametern string
  int length = strlen(*(*contentLine));//keep track of the length of the content line
  char *paramName;//holds parmeter name while being built
  char *paramValue;//char to hold param values
  char *temp;//used to swap string pointers during realloc, malloc fail guard
  Parameter *newParam;//parameter object being created

  while (contentIndex < length && (*(*contentLine))[contentIndex] !=':')//read all parameters
  {
    if((*(*contentLine))[contentIndex] == ';')//found parameter entry
    {
      contentIndex++;//skip over semi-colon

      if((paramName = malloc(sizeof(char)*200))==NULL)
        return OTHER_ERROR;

      paramIndex = 0;
      while ((*(*contentLine))[contentIndex] != '=' && (*(*contentLine))[contentIndex] != '\0')//copy param name char by char
      {
        paramName[paramIndex] = (*(*contentLine))[contentIndex];
        paramIndex++;
        contentIndex++;
      }

      if ((*(*contentLine))[contentIndex] == '\0' || contentIndex >= length)
      {
        free(paramName);
        return INV_PROP;
      }

      paramName[paramIndex] = '\0';
      contentIndex++;//skip over equals sign
      if((temp = realloc(paramName, sizeof(char)*(strlen(paramName) + 1)))==NULL)
      {
        free(paramName);
        return OTHER_ERROR;
      }
      paramName = temp;

      if((paramValue = malloc(200))==NULL)//allocate param value
      {
        free(paramName);
        return OTHER_ERROR;
      }
      paramIndex = 0;//reset param index counter to use for param value

        while ((*(*contentLine))[contentIndex] != ':' && (*(*contentLine))[contentIndex] != ';' && (*(*contentLine))[contentIndex] != '\0' && contentIndex < length)//copy param values until hit end quote
        {
          paramValue[paramIndex] = (*(*contentLine))[contentIndex];
          paramIndex++;
          contentIndex++;
        }
        paramValue[paramIndex] = '\0';

        if ((*(*contentLine))[contentIndex] == '\0' || contentIndex >= length)//param values shouldn't go this far...
        {
          free(paramValue);
          free(paramName);
          return INV_PROP;
        }

      if((newParam = malloc(sizeof(Parameter)+ paramIndex + 1))==NULL)//allocate param object
      {
        free(paramName);
        free(paramValue);
        return OTHER_ERROR;
      }

      if (strlen(paramName) < 1 || strlen(paramValue) < 1)//invalud param name or value
      {
        free(paramName);
        free(paramValue);
        return INV_PROP;
      }

      strcpy(newParam->name, paramName);
      free(paramName);
      strcpy(newParam->value, paramValue);
      free(paramValue);

      insertBack(paramList, newParam);

      if ((*(*contentLine))[contentIndex] == '"')//jump over second quote if there
      {
        contentIndex++;
        if ((*(*contentLine))[contentIndex] == '\r' || contentIndex >= length)//param values shouldn't go this far...
          return INV_PROP;
      }
    }
    else//a colon or semi-colon should be here, right?
    {
      return INV_PROP;
    }
  }

  if((temp = malloc(strlen(&((*(*contentLine))[contentIndex]))+1))==NULL)
    return OTHER_ERROR;

  //take off the parameter part of the content line
  strcpy(temp, &((*(*contentLine))[contentIndex]));
  free((*(*contentLine)));
  (*(*contentLine)) = temp;

  return OK;
}

VCardErrorCode parsePropertyValues(char ***contentLine, List *propertyList)
{
  int contentIndex = 0;//counter for current index of contentLine string
  int propertyIndex = 0;////counter for current index of parametern string
  int length = strlen(*(*contentLine));//keep track of the length of the content line
  char *propertyValue;//holds a property value
  char *temp;//used to swap string pointers during realloc, malloc fail guard

  if ((*(*contentLine))[contentIndex] == ':')
    contentIndex++;

  while (contentIndex < length && (*(*contentLine))[contentIndex] != '\0' && (*(*contentLine))[contentIndex] != '\r' && (*(*contentLine))[contentIndex] != '\n')//parse entire contentLine
  {
    if((propertyValue = malloc(sizeof(char)*200))==NULL)
      return OTHER_ERROR;

    propertyIndex = 0;
    while ((*(*contentLine))[contentIndex] != '\0' && (*(*contentLine))[contentIndex] != ';' && (*(*contentLine))[contentIndex] != '\r' && (*(*contentLine))[contentIndex] != '\n')
    {

      //                                                                                          ESCAPING
      // if ((*(*contentLine))[contentIndex] == '\\')//offset for the escaping of specific chars
      // {
      //   contentIndex++;
      //
      //   if ((*(*contentLine))[contentIndex] == 'n' || (*(*contentLine))[contentIndex] == 'N')
      //   {
      //     propertyValue[propertyIndex] = '\n';
      //   }
      //   else if((*(*contentLine))[contentIndex] == '"' || ((*(*contentLine))[contentIndex] == '\\'))
      //   {
      //     propertyValue[propertyIndex] =(*(*contentLine))[contentIndex];
      //   }
      //   else//whaaaaaa
      //   {
      //     propertyValue[propertyIndex] = (*(*contentLine))[contentIndex];//this should probably be an error
      //   }
      // }
      // else
      // {
      //   propertyValue[propertyIndex] = (*(*contentLine))[contentIndex];
      // }
      //                                                                                          ESCAPING

      propertyValue[propertyIndex] = (*(*contentLine))[contentIndex];

      propertyIndex++;
      contentIndex++;
    }

    propertyValue[propertyIndex] = '\0';

    if ((*(*contentLine))[contentIndex] == ',' || (*(*contentLine))[contentIndex] == ';')
      contentIndex++;

    if((temp = realloc(propertyValue, sizeof(char)*(propertyIndex + 1)))==NULL)
    {
      free(propertyValue);
      return OTHER_ERROR;
    }
    propertyValue = temp;


    insertBack(propertyList, propertyValue);
  }
  if ((*(*contentLine))[contentIndex-1] == ';')
  {
    if((propertyValue = malloc(1))==NULL)
      return OTHER_ERROR;
    strcpy(propertyValue, "\0");
    insertBack(propertyList, propertyValue);

  }
  return OK;
}

char* toStringNoBreak(List * list)
{
	ListIterator iter = createIterator(list);
	char* str;

	str = (char*)malloc(sizeof(char));
	strcpy(str, "");

	void* elem;
	while((elem = nextElement(&iter)) != NULL){
		char* currDescr = list->printData(elem);
    int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc( str, newLen);
		strcat(str, currDescr);
    strcat(str, ";");

		free(currDescr);
	}

	return str;
}
