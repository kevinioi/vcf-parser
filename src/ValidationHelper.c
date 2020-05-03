/**
 * @file ValidationHelper.c
 * @author Kevin ioi
 * @date Oct 2018
 * @brief
 */

#include "VCardParser.h"
#include "ValidationHelper.h"
#include "LinkedListAPI.h"
#include "ParseHelper.h"

VCardErrorCode validateProp(const Property *prop, List *pList)
{
  pNode *pCountNode;

  if (prop == NULL)
    return INV_PROP;

  if (prop->group == NULL || prop->parameters == NULL|| prop->values == NULL || prop->name == NULL)
    return INV_PROP;

  if ((pCountNode = (pNode*)findElement(pList, nodeNameCmp, prop->name)) == NULL)//check if valid prop name
    return INV_PROP;//invalid Property name

  pCountNode->count++;//increment count of property type

  if (strcmpIC(prop->name, "N")==0 && getLength(prop->values) != 5)//make sure N has 5 values
    return INV_PROP;

  if (strcmpIC(prop->name, "ADR")==0 && getLength(prop->values) != 7)//make sure ADR has 7 values
    return INV_PROP;

  if (strcmpIC(prop->name, "VERSION")==0)//should have a version in optionalProperties
  {
    return INV_CARD;
  }

  //check if there are multiple of a property, which should only exist once in a vCard
  if (strcmpIC(prop->name, "KIND")==0 || strcmpIC(prop->name, "N")==0|| strcmpIC(prop->name, "GENDER")==0|| strcmpIC(prop->name, "PRODID")==0|| strcmpIC(prop->name, "REV")==0|| strcmpIC(prop->name, "UID")==0)
  {
    if (pCountNode->count > 1)
      return INV_PROP;
  }

  if (getLength(prop->values)<1)//the property mut have a value
    return INV_PROP;

  //make sure that all properties with multiple values are supposed to have multiple values
  if (getLength(prop->values)>1)
  {
    if (!(strcmpIC(prop->name, "N")==0 || strcmpIC(prop->name, "NICKNAME")==0|| strcmpIC(prop->name, "PHOTO")==0|| strcmpIC(prop->name, "GENDER")==0|| strcmpIC(prop->name, "ADR")==0|| strcmpIC(prop->name, "TEL")==0|| strcmpIC(prop->name, "ORG")==0||
        strcmpIC(prop->name, "CATEGORIES")==0|| strcmpIC(prop->name, "CLIENTPIDMAP")==0))
    {
      return INV_PROP;
    }
  }

  return OK;
}

VCardErrorCode validateParam(const Parameter *param)
{
  if(param == NULL)
    return INV_PROP;

  if (strlen(param->name)<1)//name needs to exist
    return INV_PROP;

  if (strlen(param->value)<1)//value needs to exist
    return INV_PROP;

  return OK;
}

VCardErrorCode validateDT(const DateTime *date)
{
  if (date->isText == true)//text format
  {
    if (date->UTC == true)
      return INV_DT;

    if (strlen(date->text) == 0)
      return INV_DT;

    if (strlen(date->time) != 0 || strlen(date->date) != 0)
      return INV_DT;
  }
  else if (date->isText == false)//not text format
  {
    if (strlen(date->time) == 0 && strlen(date->date) == 0)
      return INV_DT;

    if (strlen(date->time) != 0)
    {
      if (strlen(date->time) <2)
        return INV_DT;

      if (date->time[strlen(date->time)-1] == '-')
        return INV_DT;

      for (int i = 0; i < strlen(date->time); i++)
      {
        if (!(isdigit(date->time[i])) && date->time[i] != '-')
          return INV_DT;
      }
    }

    if (strlen(date->date) != 0)
    {
      if (strlen(date->date) <2)
        return INV_DT;

      if (date->date[strlen(date->date)-1] == '-')
        return INV_DT;

      for (int i = 0; i < strlen(date->date); i++)
      {
        if (date->date[i] != '-' && !(isdigit(date->date[i])))
          return INV_DT;
      }
    }

    if (strlen(date->text) != 0)
      return INV_DT;
  }
  else
  {
    return INV_DT;//WHAAA
  }

  return OK;
}

void loadPropertyCounter(List *pList)
{
  insertBack(pList, createPNode("ORG"));
  insertBack(pList, createPNode("SOURCE"));
  insertBack(pList, createPNode("KIND"));
  insertBack(pList, createPNode("XML"));
  insertBack(pList, createPNode("FN"));
  insertBack(pList, createPNode("N"));
  insertBack(pList, createPNode("NICKNAME"));
  insertBack(pList, createPNode("PHOTO"));
  insertBack(pList, createPNode("BDAY"));
  insertBack(pList, createPNode("ANNIVERSARY"));
  insertBack(pList, createPNode("GENDER"));
  insertBack(pList, createPNode("ADR"));
  insertBack(pList, createPNode("TEL"));
  insertBack(pList, createPNode("EMAIL"));
  insertBack(pList, createPNode("IMPP"));
  insertBack(pList, createPNode("LANG"));
  insertBack(pList, createPNode("TZ"));
  insertBack(pList, createPNode("GEO"));
  insertBack(pList, createPNode("TITLE"));
  insertBack(pList, createPNode("ROLE"));
  insertBack(pList, createPNode("LOGO"));
  insertBack(pList, createPNode("MEMBER"));
  insertBack(pList, createPNode("RELATED"));
  insertBack(pList, createPNode("CATEGORIES"));
  insertBack(pList, createPNode("NOTE"));
  insertBack(pList, createPNode("PRODID"));
  insertBack(pList, createPNode("REV"));
  insertBack(pList, createPNode("SOUND"));
  insertBack(pList, createPNode("UID"));
  insertBack(pList, createPNode("CLIENTPIDMAP"));
  insertBack(pList, createPNode("URL"));
  insertBack(pList, createPNode("VERSION"));
  insertBack(pList, createPNode("KEY"));
  insertBack(pList, createPNode("FBURL"));
  insertBack(pList, createPNode("CALADRURI"));
  insertBack(pList, createPNode("CALURI"));
}

int countProperty(char *name, List *pList)
{
  ListIterator itr = createIterator(pList);
  pNode *tempNode;

  while ((tempNode = (pNode*)nextElement(&itr)) != NULL)//validate all property objects in optionalProperties
  {
    if (strcmpIC(tempNode->name, "KIND")==0)
    {
      return tempNode->count;
    }
  }

  return 0;
}

pNode *createPNode(char *name)
{
  pNode *newNode  = malloc(sizeof(pNode));
  newNode->name = malloc(strlen(name)+1);
  strcpy(newNode->name, name);
  newNode->count = 0;

  return newNode;
}

void deletePropNode(void *toDelete)
{
  free(((pNode*)toDelete)->name);
  free(toDelete);
}

char *printPropNode(void *toPrint)
{
  return "Kevin";
}

int cmpPropNode(const void *one, const void *two)
{
  return strcmpIC(((pNode*)one)->name, ((pNode*)two)->name);
}

bool nodeNameCmp(const void *node, const void *name)
{
  pNode *testNode = (pNode*)node;

  if (strcmpIC(testNode->name, (char*)name) == 0)
    return true;

  return false;
}
