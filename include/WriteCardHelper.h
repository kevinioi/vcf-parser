/**
 * @file PropertyHelper.c
 * @author Kevin ioi
 * @date Oct 2018
 * @brief File containing the functions needed to
 */
 
#ifndef _PRINTCARDHELPER_H
#define  _PRINTCARDHELPER_H


VCardErrorCode openFileWrite(FILE **fp, const char *fileName);

VCardErrorCode writeProperty(FILE **fp, Property *writeProp);

VCardErrorCode propListToString(char **contentLine, List *values);

VCardErrorCode paramListToString(char **contentLine, List *parameters);

VCardErrorCode writeOptionalProps(FILE **fp, List *props);

VCardErrorCode writeDateTime(char **contentLine, DateTime *writeDate);


#endif
