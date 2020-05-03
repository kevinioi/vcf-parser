/**
 * @file CardHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the helper function used to initialize a vCard struct
 */

#include "VCardParser.h"
#include "ParseHelper.h"
#include "LinkedListAPI.h"

Card *initializeCard(char* (*printProp)(void* toBePrinted),void (*deleteProp)(void* toBeDeleted),int (*compareProp)(const void* first,const void* second))
{
  Card *newCard = malloc(sizeof(Card));

  if (newCard == NULL)//malloc failed
    return NULL;

  newCard->fn = NULL;
  newCard->birthday = NULL;
  newCard->anniversary = NULL;
  newCard->optionalProperties = initializeList(printProp, deleteProp, compareProp);

  if(newCard->optionalProperties == NULL)
  {
    free(newCard);
    return NULL;
  }

  return newCard;
}
