/**
 * @file VCardParser.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the main API functions needed to parse a vcf file in to a vCard struct
 */

#include "VCardParser.h"
#include "WriteCardHelper.h"
#include "ParseHelper.h"
#include "CardHelper.h"
#include "PropertyHelper.h"
#include "DateHelper.h"
#include "ValidationHelper.h"


VCardErrorCode validateCard(const Card* obj)
{
  VCardErrorCode validationStatus;

  validationStatus = OK;

  if (obj == NULL)
    return INV_CARD;

  if (obj->fn == NULL)
    return INV_CARD;
  else
  {
    if(obj->fn->name == NULL)
      return INV_CARD;
    if (obj->fn->group == NULL)
      return INV_CARD;
    if (obj->fn->values == NULL || obj->fn->parameters == NULL)
      return INV_CARD;
    if (getLength(obj->fn->values)>1)
      return INV_CARD;
    if(strcmpIC(obj->fn->name, "FN") != 0)
      return INV_CARD;
  }


  if (obj->optionalProperties == NULL)
    return INV_CARD;

  //initialize the list used to track the number of specific properties in vCard
  List *propertyCounts =  initializeList(printPropNode,deletePropNode,cmpPropNode);
  loadPropertyCounter(propertyCounts);

  ListIterator itr = createIterator(obj->optionalProperties);
  Property *tempProp;
  while ((tempProp = (Property*)nextElement(&itr)) != NULL)//validate all property objects in optionalProperties
  {
    if((validationStatus = validateProp(tempProp, propertyCounts))!=OK)
    {
      freeList(propertyCounts);
      return validationStatus;
    }
  }

  if (countProperty("MEMBER", propertyCounts))//MEMBER 3 exists, KIND property MUST be group
  {
    itr = createIterator(obj->optionalProperties);
    while ((tempProp = (Property*)nextElement(&itr)) != NULL)//validate all property objects in optionalProperties
    {
      validationStatus = INV_CARD;
      if (strcmpIC(tempProp->name, "KIND")==0)
      {
        if (strcmpIC((char*)(tempProp->values->head->data), "group") == 0)
        {
          validationStatus = OK;
          break;
        }
      }
    }

    if(validationStatus != OK)
    {
      freeList(propertyCounts);
      return validationStatus;
    }
  }

  freeList(propertyCounts);

  if (obj->birthday != NULL)//validate birthday
  {
    if((validationStatus = validateDT(obj->birthday))!=OK)
      return validationStatus;
  }

  if (obj->anniversary != NULL)//validate birthday
  {
    if((validationStatus = validateDT(obj->anniversary))!=OK)
      return validationStatus;
  }

  return validationStatus;
}

VCardErrorCode writeCard(const char* fileName, const Card* obj)
{
  VCardErrorCode parseStatus;
  char *tempContentLine;
  FILE *fp;

  if (obj == NULL)//no card provided
  {
    parseStatus = WRITE_ERROR;
    return parseStatus;
  }

  //check validity of provided file address
  if((parseStatus = openFileWrite(&fp, fileName)) != OK)
    return parseStatus;

  fprintf(fp, "BEGIN:VCARD\r\nVERSION:4.0\r\n");//hardcoded header for vCard

  if((parseStatus = writeProperty(&fp, obj->fn))!= OK)
  {
    fclose(fp);
    return parseStatus;
  }

  writeOptionalProps(&fp, obj->optionalProperties);

  if (obj->anniversary != NULL) {
    tempContentLine = malloc(200);
    strcpy(tempContentLine, "ANNIVERSARY");
    parseStatus = writeDateTime(&tempContentLine, obj->anniversary);
    fprintf(fp, "%s\r\n", tempContentLine);
    free(tempContentLine);
  }

  if (obj->birthday != NULL) {
  tempContentLine = malloc(200);
  strcpy(tempContentLine, "BDAY");
  parseStatus = writeDateTime(&tempContentLine, obj->birthday);
  fprintf(fp, "%s\r\n", tempContentLine);
  free(tempContentLine);
  }

  fprintf(fp, "END:VCARD\r\n");

  fclose(fp);

  return OK;
}

VCardErrorCode createCard(char* fileName, Card** newCardObject)
{
  FILE *fp;//file pointer to be read in
  VCardErrorCode parseStatus;//the current status of the parsing
  int lineIndex = 0;//the current index of the vCardString being process
  int maxIndex;
  int endFound = 0;
  Card *newCard;//the card object that is to be returned
  char *vCardString;//all of the vcard contentLines read in
  char *contentLine;//used to perform comparision checks against strings
  char *group;//string to hold a property's group strings, if found
  char *propName;//string to hold a property name as it is taken from content line
  Property *prop;//property object handler
  DateTime *newDT;//handler for new datetime objects

  //initialize the VCard
  if((newCard = initializeCard(printProperty, deleteProperty, compareProperties)) == NULL)
  {
    *newCardObject = NULL;
    parseStatus = OTHER_ERROR;
    return parseStatus;
  }

  //check validity of provided file address
  if((parseStatus = openFileRead(&fp, fileName)) != OK)
  {
    deleteCard(newCard);
    *newCardObject = NULL;
    return parseStatus;
  }

  //read all of the data in the file into vCardString
  if((parseStatus = readVCard(fp, &vCardString)) != OK)
  {
    deleteCard(newCard);
    *newCardObject = NULL;
    return parseStatus;
  }

  maxIndex = strlen(vCardString);

  if (maxIndex < 40)//vcard is too short to be valid
  {
    deleteCard(newCard);
    free(vCardString);
    *newCardObject = NULL;
    return INV_CARD;
  }

  //make sure the begin tag starts the file
  if((parseStatus = nextContentLine(vCardString, &contentLine, &lineIndex))!=OK)
  {
    deleteCard(newCard);
    free(vCardString);
    *newCardObject = NULL;
    return parseStatus;
  }
  if (strcmpIC(contentLine, "BEGIN:VCARD\r\n") != 0)
  {
    deleteCard(newCard);
    free(contentLine);
    free(vCardString);
    *newCardObject = NULL;
    parseStatus = INV_CARD;
    return parseStatus;
  }
  free(contentLine);

  //make sure the vcard is version 4.0
  if((parseStatus = nextContentLine(vCardString, &contentLine, &lineIndex)) != OK)
  {
    deleteCard(newCard);
    free(vCardString);
    *newCardObject = NULL;
    return parseStatus;
  }

  if (strcmpIC(contentLine, "VERSION:4.0\r\n") != 0)
  {
    free(contentLine);
    deleteCard(newCard);
    free(vCardString);
    *newCardObject = NULL;
    parseStatus = INV_CARD;
    return parseStatus;
  }
  free(contentLine);


  parseStatus = nextContentLine(vCardString, &contentLine, &lineIndex);//get first regular content line

  while (parseStatus == OK && contentLine!=NULL)//parse file unless error or hits eof
  {
    group = NULL;
    propName = NULL;

    if(endFound == 1)//parser already found end flag, shouldn't have been more to read
    {
      parseStatus = INV_CARD;
    }

    if((parseStatus = extractGroup(&contentLine, &group))== OK)
    {
      if((parseStatus = extractProp(&contentLine, &propName))==OK)
      {
        if(strcmpIC(propName, "bday") == 0)//found bday property
        {
          if (newCard->birthday != NULL)//found multiple birthday properties
          {
            free(propName);
            if(group!= NULL)
              free(group);
            group=NULL;
            parseStatus = INV_CARD;
          }
          else
          {
            free(propName);

            if (group != NULL)
              free(group);
            group = NULL;
            parseStatus = newDate(&newDT, &contentLine);

            if (parseStatus == OK)
              newCard->birthday = newDT;
          }
        }
        else if(strcmpIC(propName, "ANNIVERSARY") == 0)//found Anniversary property
        {
          if (newCard->anniversary != NULL)//found multiple anniversary properties
          {
            free(propName);
            if(group!= NULL)
              free(group);
            group=NULL;
            parseStatus = INV_CARD;
          }
          else
          {
            free(propName);
            if (group != NULL)
              free(group);
            group = NULL;
            parseStatus = newDate(&newDT, &contentLine);

            if (parseStatus == OK)
              newCard->anniversary = newDT;
          }
        }
        else if (strcmpIC(propName, "END")==0)//found end flag
        {

          if (strcmpIC(contentLine, ":VCARD\r\n")==0 || strcmpIC(contentLine, ":VCARD")==0)
            endFound = 1;
          else
            parseStatus = INV_CARD;

          free(propName);
          propName = NULL;
          if(group != NULL)
            free(group);
          group = NULL;
          free(contentLine);
          break;
        }
        else if (strcmpIC(propName, "BEGIN")==0)//found another begin flag
        {
          free(propName);
          if (group != NULL)
          {
            free(group);
          }
          parseStatus = INV_CARD;
        }
        else//have standard property type
        {
          if (strlen(propName) < 1 )
          {
            free(propName);
            parseStatus = INV_PROP;
          }
          else if((parseStatus = newProperty(propName, group, &prop, &contentLine))==OK)//build property
          {
            if(getLength(prop->values)==0)//make sure the property had a value
            {
              deleteProperty(prop);
              parseStatus = INV_PROP;
            }
            else if (strcmpIC(prop->name, "n")==0 && getLength(prop->values) != 5) {//check if name property had enough values
              deleteProperty(prop);
              parseStatus = INV_PROP;
            }
            else if (strcmpIC(prop->name, "ADR")==0 && getLength(prop->values) != 7) {//check if address property had enough values
              deleteProperty(prop);
              parseStatus = INV_PROP;
            }
            else if (strcmpIC(prop->name, "fn") == 0 && newCard->fn ==NULL)//new FN property
            {
              newCard->fn = prop;
            }
            else//optional property
            {
              insertBack(newCard->optionalProperties, prop);
            }
          }
        }
      }
    }


    free(contentLine);
    if (parseStatus == OK)
    {
      parseStatus = nextContentLine(vCardString, &contentLine, &lineIndex);//get next content line
    }
  }

  free(vCardString);

  if (parseStatus == OK)
   {
     if (endFound != 1)// didn't find end flag, error
      parseStatus = INV_CARD;

     if(newCard->fn  == NULL)
      parseStatus = INV_CARD;
  }

  if(parseStatus!=OK)
  {
    deleteCard(newCard);
    *newCardObject = NULL;
  }
  else
    *newCardObject = newCard;

  return parseStatus;
}

char* strListToJSON(const List* strList)
{
  if (strList == NULL)
    return NULL;

  ListIterator iter = createIterator((List*)strList);
  int jsonIndex = 0;
  int strIndex = 0;
  int memAllocd = 400;
  int strSize;
  char *json = calloc(400, sizeof(char));
  char *tempStr;

  json[jsonIndex] = '[';//start json
  jsonIndex++;
  while ((tempStr = (char*)nextElement(&iter)) != NULL)
  {
    strIndex = 0;
    strSize = (int)strlen(tempStr);

    //if new string is too big to hold in allocd mem
    if (strlen(json) + strSize >= memAllocd*sizeof(char))
    {
      json = realloc(json, (memAllocd+strSize+200)*sizeof(char));
      memAllocd += 200+strSize;
    }

    json[jsonIndex] = '\"';
    jsonIndex++;

    for (strIndex = 0; strIndex < strSize; strIndex++)//copy str in char by char
    {
      if (tempStr[strIndex] == '\"')//catch escaped quotes
      {
        json[jsonIndex] = '\\';
        jsonIndex++;
      }
      json[jsonIndex] = tempStr[strIndex];
      jsonIndex++;
    }

    json[jsonIndex] = '\"';
    jsonIndex++;
    json[jsonIndex] = ',';
    jsonIndex++;
  }

  if (json[jsonIndex - 1] == ',')
    jsonIndex--;

  json[jsonIndex] = ']';
  json[jsonIndex+1] = '\0';


  return json;
}

List* JSONtoStrList(const char* str)
{
  if (str == NULL)
    return NULL;

  List *strList = initializeList(printString ,deleteString,compareString);

  char *tempStr = NULL;
  int jsonIndex = 0;

  if (str[jsonIndex] != '[' || str[jsonIndex+1]!= '\"')//make sure proper opening
  {
    freeList(strList);
    return NULL;
  }
  jsonIndex++;


  bool inValue = false;
  int allocdMem = 100;
  int copyIndex = 0;

  while(jsonIndex < strlen(str) && str[jsonIndex] != ']')//parse contentLine until end or close bracket
  {
    if(inValue == false)
    {
      if(str[jsonIndex] == '\"')//start value string)
      {
        copyIndex = 0;
        tempStr = calloc(sizeof(char),100);
        allocdMem = 100;
        tempStr[copyIndex] = '\0';
        inValue = true;
      }
      else if(str[jsonIndex] != ',')//has to be a comma
      {
        free(tempStr);
        freeList(strList);
        return NULL;
      }
      else if(str[jsonIndex-1] != '\"' && str[jsonIndex+1] != '\"')
      {
        free(tempStr);
        freeList(strList);
        return NULL;
      }
      else{}//here to remind me that this is complete, should just step over comma this loop
    }
    else //(inValue == true )
    {
      if(str[jsonIndex] == '\"')//end of value string
      {
        tempStr[copyIndex] = '\0';
        insertBack(strList, tempStr);
        inValue = false;
      }
      else //read in char of value
      {
        if((int)(copyIndex * sizeof(char)) > (allocdMem -3)*sizeof(char))
        {
          tempStr = realloc(tempStr, sizeof(char)*(allocdMem + 50));
          allocdMem += 50;
        }

        if (str[jsonIndex] == '\\')//escaped char
        {
          jsonIndex++;
          if (str[jsonIndex] == 'n' || str[jsonIndex] == 'N')
            tempStr[copyIndex] = '\n';
          else if(str[jsonIndex] == '\"')
            tempStr[copyIndex] = '\"';
          else if(str[jsonIndex] == '\\')
            tempStr[copyIndex] = '\\';
          else//i don't know what other chars could be escaped, just put them in raw
            tempStr[copyIndex] = str[jsonIndex];
        }
        else
          tempStr[copyIndex] = str[jsonIndex];
        copyIndex++;
      }
    }
    jsonIndex++;
  }

  return strList;
}

char* propToJSON(const Property* prop)
{
  char *json = calloc(sizeof(char), 100);
  char *propValues;

  if (prop == NULL)
  {
    strcpy(json, "\0");
    return json;
  }

  if(prop->group == NULL)
  {
    strcpy(json, "\0");
    return json;
  }
  strcat(json, "{\"group\":\"\0");
  strcat(json, prop->group);

  strcat(json, "\",\"name\":\"\0");
  strcat(json, prop->name);

  strcat(json, "\",\"values\":\0");

  propValues = strListToJSON(prop->values);

  if (sizeof(char)*(strlen(propValues)+strlen(json)) > 98)
    json = realloc(json, sizeof(char)*(strlen(propValues)+strlen(json) + 10));

  strcat(json, propValues);
  free(propValues);
  strcat(json, "}\0");

  return json;
}

Property* JSONtoProp(const char* str)
{
  if(str == NULL)
    return NULL;
  if (strlen(str)<5)
    return NULL;

  Property *newProp = malloc(sizeof(Property));
  newProp->name = NULL;
  newProp->group = NULL;
  newProp->parameters = initializeList(printParameter, deleteParameter, compareParameters);
  newProp->values = initializeList(printString, deleteString, compareString);

  int strIndex = 0;
  int copyIndex = 0;
  int memAllocd = 0;
  int length = strlen(str) - 1;

  while (str[strIndex] != ':' && strIndex != length - 1)
    strIndex++;

  strIndex++;
  if (strIndex == length -1 || str[strIndex] != '\"')
  {
    deleteProperty(newProp);
    return NULL;
  }

  strIndex++;
  //group name
  if (str[strIndex] == '\"')//empty group name
  {
    newProp->group = calloc(sizeof(char), 2);//lets be safe give it two;
    strcpy(newProp->group, "\0");
  }
  else//copy in group value
  {
    newProp->group = calloc(sizeof(char),50);//allocate memory for group name
    memAllocd = 50;
    while (strIndex != length -1 && str[strIndex] != '\"')
    {
      if (copyIndex*sizeof(char)>= (memAllocd - 2)*sizeof(char))//realloc memory if needed
      {
        memAllocd += 50;
        newProp->group = realloc(newProp->group, memAllocd*sizeof(char));
      }
      newProp->group[copyIndex] = str[strIndex];
      strIndex++;
      copyIndex++;
    }
    newProp->group[copyIndex] = '\0';
    copyIndex = 0;
    memAllocd = 0;

    if (strIndex != length -1)
    {
      deleteProperty(newProp);
      return NULL;
    }
  }

  while (str[strIndex] != ':')//jump to property name
  {
    if(strIndex == length - 1)
    {
      deleteProperty(newProp);
      return NULL;
    }
    strIndex++;
  }
  strIndex++;//jump over colon
  if (strIndex == length -1 || str[strIndex] != '\"')
  {
    deleteProperty(newProp);
    return NULL;
  }

  strIndex++;
  if (str[strIndex] == '\"')//name can't be empty
  {
    deleteProperty(newProp);
    return NULL;
  }
  else//copy in name value
  {
    newProp->name = calloc(sizeof(char),40);//allocate memory for property name
    memAllocd = 40;

    while (strIndex != length -1 && str[strIndex] != '\"')
    {
      if (copyIndex*sizeof(char) >= (memAllocd - 2)*sizeof(char))//realloc memory if needed
      {
        memAllocd += 50;
        newProp->name = realloc(newProp->name, memAllocd*sizeof(char));
      }
      newProp->name[copyIndex] = str[strIndex];
      strIndex++;
      copyIndex++;
    }
    newProp->name[copyIndex] = '\0';
    copyIndex = 0;
    memAllocd = 0;

    if (strIndex == length -1)
    {
      deleteProperty(newProp);
      return NULL;
    }
  }


  while (str[strIndex] != ':')//jump to property values
  {
    if(strIndex == length - 1)
    {
      deleteProperty(newProp);
      return NULL;
    }
    strIndex++;
  }
  strIndex++;//jump over colon

  newProp->values = JSONtoStrList(&str[strIndex]);

  if(newProp->values == NULL)
  {
    deleteProperty(newProp);
    return NULL;
  }

  return newProp;
}

char* dtToJSON(const DateTime* prop)
{
  char *json;

  if (prop == NULL)
  {
    json = calloc(sizeof(char), 2);
    strcpy(json, "");
    return json;
  }
  else
  {
    json = calloc(sizeof(char), 150);
  }
  int memAllocd = 150;

  strcpy(json, "{\"isText\":\0");

  if (prop->isText == true)
  {
    strcat(json, "true,\"date\":\"\",\"time\":\"\",\"text\":\"\0");
    if( sizeof(char)*(strlen(json) + strlen(prop->text)) > memAllocd*sizeof(char))
    {
      memAllocd = 20 + strlen(json) + strlen(prop->text);
      json = realloc(json, sizeof(char)*(20 + strlen(json) + strlen(prop->text)));
    }
    strcat(json, prop->text);
    strcat(json, "\",\"isUTC\":false}");

  }
  else
  {
    strcat(json, "false,\"date\":\"");

    if(sizeof(char)*(strlen(json) + strlen(prop->date)) >= memAllocd*sizeof(char))//make sure there's mem for date
    {
      memAllocd = (int)(40 + strlen(json) + strlen(prop->date));
      json = realloc(json, sizeof(char)*(40 + strlen(json) + strlen(prop->date)));
    }
    strcat(json, prop->date);
    strcat(json, "\",\"time\":\"");

    if(sizeof(char)*(strlen(json) + strlen(prop->time)) >= memAllocd*sizeof(char))//make sure there's mem for time
    {
      memAllocd = (int)(40 + strlen(json) + strlen(prop->time));
      json = realloc(json, sizeof(char)*(40 + strlen(json) + strlen(prop->time)));
    }

    strcat(json, prop->time);
    strcat(json, "\",\"text\":\"\",\"isUTC\":");

    if(sizeof(char)*(strlen(json) + 20) >= memAllocd*sizeof(char))//make sure theres mem for UTC
    {
      memAllocd = (int)(40 + strlen(json) + strlen(prop->date));
      json = realloc(json, sizeof(char)*(40 + memAllocd));
    }

    if (prop->UTC == true)
      strcat(json, "true}");
    else
      strcat(json, "false}");
  }


  return json;
}

DateTime* JSONtoDT(const char* str)
{
  if (str == NULL)
    return NULL;

  int sourceIndex= 0;
  int copyIndex = 0;
  int length = strlen(str) - 1;
  char *booleanCheck;
  DateTime *newDt = calloc(sizeof(DateTime), 1);

  while (str[sourceIndex] != ':' && sourceIndex != length)
  {
    sourceIndex++;
  }

  if (sourceIndex > length - 3)//invalid format, should be more in string
  {
    free(newDt);
    return NULL;
  }
  sourceIndex++;//step over colon


  booleanCheck = calloc(sizeof(char), 10);

  while (str[sourceIndex] != ',')//copy isText Boolean
  {
    booleanCheck[copyIndex] = str[sourceIndex];
    sourceIndex++;
    copyIndex++;

    if (sourceIndex == length || sourceIndex == length -1)
    {
      free(newDt);
      return NULL;
    }
  }

  if (strcmpIC(booleanCheck, "true")==0)
  {
    newDt->isText = true;
  }
  else if (strcmpIC(booleanCheck, "false")==0)
  {
    newDt->isText = false;
  }
  else
  {
    free(newDt);
    free(booleanCheck);
    return NULL;
  }
  free(booleanCheck);

  while (str[sourceIndex] != ':')//go to date value
  {
    sourceIndex++;
    if(sourceIndex == length -2)
    {
      free(newDt);
      return NULL;
    }
  }
  sourceIndex++;//step over colon
  sourceIndex++;//step over parenthesis

  copyIndex = 0;
  while (str[sourceIndex] != '\"' && copyIndex < 8)//copy in date value
  {
    newDt->date[copyIndex] = str[sourceIndex];
    copyIndex++;
    sourceIndex++;

    if (sourceIndex == length || sourceIndex == length -1)
    {
      free(newDt);
      return NULL;
    }
  }
  newDt->date[copyIndex] = '\0';

  if (str[sourceIndex] != '\"')//date was too long
  {
    free(newDt);
    return NULL;
  }

  while (str[sourceIndex] != ':')//go to time value
  {
    sourceIndex++;
    if (sourceIndex == length || sourceIndex == length -1)//make sure not end of string
    {
      free(newDt);
      return NULL;
    }
  }
  sourceIndex++;//step over colon
  sourceIndex++;//step over parenthesis


  copyIndex = 0;
  while (str[sourceIndex] != '"' && copyIndex < 6)//copy time string
  {
    newDt->time[copyIndex] = str[sourceIndex];
    copyIndex++;
    sourceIndex++;

    if (sourceIndex == length -1)
    {
      free(newDt);
      return NULL;
    }
  }
  newDt->time[copyIndex] = '\0';//null terminate time


  while (str[sourceIndex] != ':')//go to text value
  {
    sourceIndex++;
    if (sourceIndex == length -3)//make sure not end of string
    {
      free(newDt);
      return NULL;
    }
  }
  sourceIndex++;//step over colon
  sourceIndex++;//step over parenthesis

  copyIndex = 0;
  char *tempDate = calloc(sizeof(char),200);
  int mem = 200;
  while (str[sourceIndex] != '\"')//copy entire text value
  {
    tempDate[copyIndex] = str[sourceIndex];
    sourceIndex++;
    copyIndex++;
    if (sourceIndex == length -2)//make sure not end of string
    {
      free(newDt);
      return NULL;
    }
    if (sourceIndex > mem - 10)
    {
      mem += 100;
      tempDate = realloc(tempDate, sizeof(char)*(mem+copyIndex));
    }
  }
  tempDate[copyIndex] = '\0';

  newDt = realloc(newDt, sizeof(DateTime) + sizeof(char)*(strlen(tempDate) + 2));
  strcpy(newDt->text, tempDate);
  free(tempDate);

  while (str[sourceIndex] != ':')//go to is text boolean
  {
    sourceIndex++;
    if (sourceIndex == length -1)//make sure not end of string
    {
      free(newDt);
      return NULL;
    }
  }
  sourceIndex++;//step over colon

  booleanCheck = calloc(sizeof(char),20);
  copyIndex = 0;

  while (str[sourceIndex] != '}' && sourceIndex != length)//copy isText Boolean
  {
    booleanCheck[copyIndex] = str[sourceIndex];
    sourceIndex++;
    copyIndex++;
  }

  if (strcmpIC(booleanCheck, "true")==0)
    newDt->UTC = true;
  else if (strcmp(booleanCheck, "false")==0)
    newDt->UTC = false;
  else
  {
    free(newDt);
    free(booleanCheck);
    return NULL;
  }
  free(booleanCheck);

  return newDt;
}

Card* JSONtoCard(const char* str)
{
  if (str == NULL)
    return NULL;

  int sourceIndex = 0;
  int copyIndex;
  int length = strlen(str) - 1;
  char *name = calloc(sizeof(char),20);
  char *value;
  char *group = malloc(sizeof(char));

  Card *newCard = initializeCard(printProperty, deleteProperty, compareProperties);

  while(str[sourceIndex] != '\"')
  {
    sourceIndex++;

    if (sourceIndex == length-1)
    {
      free(name);
      free(group);
      deleteCard(newCard);
      return NULL;
    }
  }
  sourceIndex++;//step over parenthesis


  copyIndex = 0;
  while(str[sourceIndex] != '\"')//copy in name
  {
    name[copyIndex] = str[sourceIndex];
    copyIndex++;
    sourceIndex++;

    if (sourceIndex == length)
    {
      free(name);
      free(group);
      deleteCard(newCard);
      return NULL;
    }
  }
  name[copyIndex] = '\0';
  sourceIndex++;
  sourceIndex++;
  sourceIndex++;


  value = calloc(sizeof(char), strlen(&str[sourceIndex])+1);
  copyIndex = 0;
  while (str[sourceIndex] != '\"')
  {
    value[copyIndex] = str[sourceIndex];
    sourceIndex++;
    copyIndex++;
    if (sourceIndex == length)
    {
      free(value);
      free(name);
      free(group);
      deleteCard(newCard);
      return NULL;
    }
  }
  value[copyIndex] = '\0';

  Property *fn = createProperty();

  group[0] = '\0';

  fn->group = group;
  fn->name = name;
  insertBack(fn->values, value);
  newCard->fn = fn;

  return newCard;
}

void addProperty(Card* card, const Property* toBeAdded)
{
  if (toBeAdded == NULL || card == NULL)
    return;

  if (card->optionalProperties == NULL)
    return;

  insertBack(card->optionalProperties, (void*)toBeAdded);
}

/*
* deleteCard
*/
void deleteCard(Card* obj)
{
  if (obj == NULL)
    return;

  if (obj->fn != NULL)
    deleteProperty(obj->fn);


  if (obj->optionalProperties != NULL)
    freeList(obj->optionalProperties);

  if (obj->birthday != NULL)
    deleteDate(obj->birthday);

  if (obj->anniversary != NULL)
    deleteDate(obj->anniversary);

  free(obj);
}

/*
* printCard
*/
char* printCard(const Card* obj)
{
  char *cardString;
  if(obj == NULL)
  {
      cardString = malloc(5);
      strcpy(cardString, "NULL");
  }
  else
  {
    cardString = malloc(17);
    strcpy(cardString, "This is the Card");
  }
  return cardString;
}

/*
* printError
*/
char* printError(VCardErrorCode err)
{
  char *errorCode;
  errorCode = NULL;

  if (err == OK)
  {
    errorCode = malloc(3);
    strcpy(errorCode, "OK");
  }
  else if (err == INV_FILE)
  {
    errorCode = malloc(9);
    strcpy(errorCode, "INV_FILE");
  }
  else if (err == INV_CARD)
  {
    errorCode = malloc(9);
    strcpy(errorCode, "INV_CARD");
  }
  else if (err == INV_PROP)
  {
    errorCode = malloc(9);
    strcpy(errorCode, "INV_PROP");
  }
  else if (err == WRITE_ERROR)
  {
    errorCode = malloc(12);
    strcpy(errorCode, "WRITE_ERROR");
  }
  else if (err == OTHER_ERROR)
  {
    errorCode = malloc(12);
    strcpy(errorCode, "OTHER_ERROR");
  }
  else if (err == INV_DT)
  {
    errorCode = malloc(7);
    strcpy(errorCode, "INV_DT");
  }
  else
  {
    errorCode = malloc(19);
    strcpy(errorCode, "Invalid error code");
  }

  return errorCode;
}

/*
* deleteProperty
*/
void deleteProperty(void* toBeDeleted)
{
  if (toBeDeleted == NULL)
    return;

  Property *deleteProp;
  deleteProp = (Property*)toBeDeleted;

  if (deleteProp != NULL)
  {
    if (deleteProp->name != NULL)
      free(deleteProp->name);

    if (deleteProp->group != NULL)
      free(deleteProp->group);

    if (deleteProp->parameters != NULL)
      freeList(deleteProp->parameters);

    if (deleteProp->values != NULL)
      freeList(deleteProp->values);

    free(deleteProp);
  }
}

/*
* compareProperties
*/
int compareProperties(const void* first,const void* second)
{
  if (first == NULL && second == NULL)
    return 0;
  else if (first == NULL)
    return -1;
  else if (second == NULL)
    return 1;

  int result;//int value to be returned
  char *oneStr, *twoStr;//holders for string property values
  Property *oneProp, *twoProp;//holders for property objects

  oneProp = (Property*)first;
  twoProp = (Property*)second;

  oneStr = printProperty(oneProp);
  twoStr = printProperty(twoProp);

  result = strcmp(oneStr, twoStr);

  free(oneStr);
  free(twoStr);

  return result;
}

/*
* printProperty
*/
char* printProperty(void* toBePrinted)
{
  char *propString = NULL;
  char *values = NULL;
  char *params = NULL;
  int vlen = 0, plen = 0, glen = 0, nlen = 0;

  if (toBePrinted == NULL)
  {
      propString = malloc(5);
      strcpy(propString, "NULL");
      return propString;
  }

  Property *prop = (Property*)toBePrinted;

	if(prop->values == NULL || prop->parameters == NULL || prop->name == NULL || prop->group == NULL)
  {
      propString = malloc(5);
      strcpy(propString, "NULL");
      return propString;
  }

  params = toStringNoBreak(prop->parameters);//grab Property params
  values = toStringNoBreak(prop->values);//grab Property values

  if (params == NULL || values == NULL)
  {
    return NULL;
  }
  else if (params == NULL)
  {
    free(values);
    return  NULL;
  }
  else if (values == NULL)
  {
    free(params);
    return  NULL;
  }

	glen = strlen(prop->group);
	vlen = strlen(values);
	nlen = strlen(prop->name);
	plen = strlen(params);

  propString = malloc( vlen + plen + nlen + glen + 1);

	if (glen>0)
		strcpy(propString, prop->group);
  else
    strcpy(propString, "");

  strcat(propString, prop->name);
  strcat(propString, params);
  strcat(propString, values);

  free(params);
  free(values);

  return propString;
}

void deleteParameter(void* toBeDeleted)
{
  if (toBeDeleted == NULL)
    return;

  Parameter *deleteParam;
  deleteParam = (Parameter*)toBeDeleted;

  free(deleteParam);
}
int compareParameters(const void* first,const void* second)
{
  if (first == NULL && second == NULL)
    return 0;
  else if (first == NULL)
    return -1;
  else if (second == NULL)
    return 1;

  int result;
  char *oneS, *twoS;
  Parameter *oneP, *twoP;

  oneP = (Parameter*)first;
  twoP = (Parameter*)second;

  oneS = malloc(strlen(oneP->name)+ strlen(oneP->value) + 1);
  strcpy(oneS, oneP->name);
  strcat(oneS, oneP->value);
  oneS[strlen(oneP->name)+ strlen(oneP->value)] = '\0';

  twoS = malloc(strlen(twoP->name)+ strlen(twoP->value) + 1);
  strcpy(twoS, twoP->name);
  strcat(twoS, twoP->value);
  twoS[strlen(twoP->name)+ strlen(twoP->value)] = '\0';

  result = strcmpIC(oneS, twoS);

  free(oneS);
  free(twoS);

  return result;
}
char* printParameter(void* toBePrinted)
{
  char *paramString;

  if (toBePrinted == NULL)
  {
      if((paramString = malloc(5))== NULL)
        return NULL;
      strcpy(paramString, "NULL");
      return paramString;
  }

  Parameter *param = (Parameter*)toBePrinted;

  if (param->name == NULL || param->value == NULL)
  {
      if((paramString = malloc(5))== NULL)
        return NULL;
      strcpy(paramString, "NULL");
      return paramString;
  }

  if((paramString = malloc((strlen(param->name)+strlen(param->value)+1))) == NULL)
    return NULL;

  strcpy(paramString, param->name);
  strcat(paramString, param->value);

  return paramString;
}

void deleteValue(void* toBeDeleted)
{
  if(toBeDeleted != NULL)
    free((char*)toBeDeleted);
}
int compareValues(const void* first,const void* second)
{
  char *one, *two;

  one = (char*)first;
  two = (char*)second;

  return strcmp(one, two);
}
char* printValue(void* toBePrinted)
{
  char *newStr;

  if (toBePrinted == NULL)
  {
    newStr = malloc(5);
    strcpy(newStr, "NULL");
    return newStr;
  }

  newStr = malloc(sizeof(char)*strlen((char*)toBePrinted));
  strcpy(newStr, (char*)toBePrinted);

  return newStr;
}

void deleteDate(void* toBeDeleted)
{
  if(toBeDeleted == NULL)
    return;

  DateTime *toDelete = (DateTime*)toBeDeleted;

  free(toDelete);
}
int compareDates(const void* first,const void* second)
{
  return 1;
}
char* printDate(void* toBePrinted)
{
  char *printedDate;

  if (toBePrinted == NULL)
  {
    printedDate = malloc(5);
    strcpy(printedDate, "NULL");
    return printedDate;
  }

  DateTime *toPrint = (DateTime*)toBePrinted;

  if (toPrint->isText != 0)
  {
    printedDate = malloc(strlen(toPrint->text)+ 1);
    strcpy(printedDate, toPrint->text);
    printedDate[strlen(toPrint->text)] = '\0';
  }
  else
  {
    printedDate = malloc(strlen(toPrint->date) + strlen(toPrint->time) + 2);
    strcpy(printedDate, toPrint->date);
    if (strlen(toPrint->time) > 0)
    {
      strcat(printedDate, "T");
      strcat(printedDate, toPrint->time);
      printedDate[strlen(toPrint->date) + strlen(toPrint->time) + 1] = '\0';
    }
    else
    {
      printedDate[strlen(toPrint->date)] = '\0';
    }
  }

  return printedDate;
}
