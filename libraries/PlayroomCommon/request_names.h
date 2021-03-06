// request_names.h

#ifndef _REQUEST_NAMES_h
#define _REQUEST_NAMES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

typedef enum
{
  RQ_TYPE_INVALID,
  RQ_TYPE_ALIVE,
  RQ_TYPE_SECTOR_NUMBER_ON,
  RQ_TYPE_SECTOR_NUMBER_OFF,
  RQ_TYPE_SECTOR_NUMBER_ALL_ON,
  RQ_TYPE_SECTOR_NUMBER_ALL_OFF,
  RQ_TYPE_SECTOR_ARROW_ON,
  RQ_TYPE_SECTOR_ARROW_OFF,
  RQ_TYPE_SECTOR_ARROW_ALL_ON,
  RQ_TYPE_SECTOR_ARROW_ALL_OFF,
  RQ_TYPE_LOCKBOX_OPEN,
  RQ_TYPE_LOCKBOX_CLOSE,
  RQ_TYPE_LOCKBOX_ALL_OPEN,
  RQ_TYPE_LOCKBOX_ALL_CLOSE,
  RQ_TYPE_TOP_LIGHT,
  RQ_TYPE_SURROUND_LIGHT,
  RQ_TYPE_LOCKBOX_LIGHT,
  RQ_TYPE_BARREL_WAIT,
  RQ_TYPE_DOOR_LOCK,
  RQ_TYPE_DOOR_UNLOCK,
  RQ_TYPE_CURTAIN_OPEN,
  RQ_TYPE_CURTAIN_CLOSE,
  RQ_TYPE_SECTORS_ALL_ON,
  RQ_TYPE_SECTORS_ALL_OFF,
  RQ_TYPE_SECTOR_PLAY,

} RqType;


const char* get_request_name(RqType rq_type);

RqType get_request_type(const char *str);

#endif

