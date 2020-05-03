/**
 * @file DateHelper.c
 * @author Kevin ioi
 * @date Sept 2018
 * @brief File containing the helper functions needed to create a date struct
 */

#include "VCardParser.h"
#include "ParseHelper.h"
#include "LinkedListAPI.h"
#include "PropertyHelper.h"
#include "DateHelper.h"

VCardErrorCode newDate(DateTime **date, char **contentLine)
{
  int contentIndex = 0;//keep track of current index in content line
  int copyStringIndex = 0;//keep track of current index in string we're making
  int length;//keep track of contentLine length
  int textCheckLength;
  char *copyString;
  DateTime *temp;//used to handle memory duing reallocs, incase malloc fails
  DateTime *newDT;
  List *params;
  VCardErrorCode status;

  if((params = initializeList(printParameter, deleteParameter, compareParameters))==NULL)
  {
    *date = NULL;
    return OTHER_ERROR;
  }

  if((status = parseParameters(&contentLine, params ))!=OK)
  {
    freeList(params);
    *date = NULL;
    return status;
  }
  length = strlen(*contentLine);

  if ((*contentLine)[contentIndex] != ':' && (*contentLine)[contentIndex] != ';')//unrecognized formatting
  {
    freeList(params);
    *date = NULL;
    return INV_PROP;
  }

  if((newDT= malloc(sizeof(DateTime)+ 1))==NULL)
  {
    freeList(params);
    *date = NULL;
    return OTHER_ERROR;
  }

  if((copyString = malloc(100))==NULL)
  {
    freeList(params);
    free(newDT);
    *date = NULL;
    return OTHER_ERROR;
  }

 //initialize elements
 newDT->isText = 0;
 newDT->UTC = 0;
 strcpy(newDT->text, "\0");
 strcpy(newDT->time, "\0");
 strcpy(newDT->date,"\0");

 if (getLength(params) > 0)
 {
   if (strcmpIC(((Parameter*)(params->head->data))->value, "text")==0)
      newDT->isText = 1;
 }

  freeList(params);

 if((*contentLine)[contentIndex] == ':')//found date values
 {
   contentIndex++;//skip over colon

  textCheckLength = contentIndex;
  while (textCheckLength < length)
  {
    if (isalpha((*contentLine)[contentIndex]) && (*contentLine)[contentIndex] != 't' && (*contentLine)[contentIndex] != 'T' && (*contentLine)[contentIndex] != 'z'  && (*contentLine)[contentIndex] != 'Z' )
      newDT->isText = 1;
    textCheckLength++;
  }

   if(newDT->isText == 1 || ((length - contentIndex))>20)//date is in text value
   {
     newDT->isText = 1;

     //copy value data into string
     while ((*contentLine)[contentIndex] != ':' && (*contentLine)[contentIndex] != ';' && (*contentLine)[contentIndex] != '\0' && (*contentLine)[contentIndex] != '\r')
     {
       copyString[copyStringIndex] = (*contentLine)[contentIndex];
       copyStringIndex++;
       contentIndex++;
     }
     copyString[copyStringIndex] = '\0';

     if((temp = realloc(newDT, sizeof(DateTime) + strlen(copyString) + 1))==NULL)
     {
       free(newDT);
       free(copyString);
       return OTHER_ERROR;
     }
     newDT = temp;
     strcpy(newDT->text, copyString);
     newDT->text[copyStringIndex] = '\0';
     free(copyString);
     copyStringIndex = 0;
   }
   else//date is not in text format
   {
     //copy date
     while ((*contentLine)[contentIndex] != 'T' && (*contentLine)[contentIndex] != 't' && (*contentLine)[contentIndex] != '\0' && (*contentLine)[contentIndex] != 'z' && (*contentLine)[contentIndex] != 'Z' && (*contentLine)[contentIndex] != '\r')
     {
         newDT->date[copyStringIndex] = (*contentLine)[contentIndex];
         contentIndex++;
         copyStringIndex++;
     }
     newDT->date[copyStringIndex] = '\0';
     copyStringIndex = 0;

     if ((*contentLine)[contentIndex] == 'T' || (*contentLine)[contentIndex] == 't')//foud time entry
     {
       contentIndex++;//step over t

       while(copyStringIndex < 6 && (*contentLine)[contentIndex] != '\0' && (*contentLine)[contentIndex] != 'Z' && (*contentLine)[contentIndex] != 'z')
       {
         if ((*contentLine)[contentIndex] == '-')//dashes in input mean two (2) dashes in date string
         {
           newDT->time[copyStringIndex] = '-';
           copyStringIndex++;
           if (copyStringIndex < 6)//make sure there's room for last dash
           {
             newDT->time[copyStringIndex] = '-';
             copyStringIndex++;
           }
           contentIndex++;
         }
         else
         {
           newDT->time[copyStringIndex] = (*contentLine)[contentIndex];
           copyStringIndex++;
           contentIndex++;
         }
       }
       newDT->time[copyStringIndex] = '\0';
     }

     if ((*contentLine)[contentIndex] == 'z' || (*contentLine)[contentIndex] == 'Z')//foud time entry
     {
       newDT->UTC = 1;
     }

     free(copyString);
   }
 }
 else//invalid syntax
 {
   free(newDT);
   *date = NULL;
   return INV_PROP;
 }


 if (newDT->isText == 1 && strcmp(newDT->time, "\0")!= 0 && strcmp(newDT->date, "\0")!= 0)
 {
    free(newDT);
    *date = NULL;
    return INV_PROP;
  }


 *date = newDT;
 return OK;
}
