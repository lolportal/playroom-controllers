// 
// 
// 

#include "request_processing.h"
#include "sectors.h"
#include "MySerial.h"
#include <request_names.h>

static bool verify_request(InputRqParsingOutput& data);

typedef enum
{
  ERROR_NONE,
  ERROR_SECTOR_ATTR_NOT_FOUND,
  ERROR_BAD_SECTOR_NUMBER,
  ERROR_INVALID_RQ_TYPE,
  ERROR_INVALID_SECTOR_NUMBER,
  ERROR_REQUEST_PROCESSING,
}
RqVerfyError;

static RqVerfyError error_code;

#define BAD_RQ "Request processing error: "

const char *get_request_error()
{
  switch (error_code)
  {
  case ERROR_INVALID_RQ_TYPE:
    return BAD_RQ"Invalid request type";
  case ERROR_SECTOR_ATTR_NOT_FOUND:
    return BAD_RQ"Attribute \"sector\" not found";
  case ERROR_BAD_SECTOR_NUMBER:
    return BAD_RQ"Attribute \"sector\" value incorrect";
  case ERROR_REQUEST_PROCESSING:
    return BAD_RQ"Internal error, probably incorrect arguments?";
  case ERROR_INVALID_SECTOR_NUMBER:
    return BAD_RQ"Invalid sector number parsing";
  case ERROR_NONE:
    return BAD_RQ"No error";
  default:
    return BAD_RQ"Unexpected error";
  }
}

#undef BAD_RQ

void print_request(const InputRqParsingOutput& data)
{

  MySerial.println(get_request_name(data.request_type));

  if (data.id_attr_found)
  {
    MySerial.print("Sector: ");
    MySerial.println(data.id_attr_buf);
  }

}

static bool process_alive_request()
{
  return true; // always successful
}

static bool process_barrel_wait_request()
{
  set_wait_for_spin_flag();
  return true;
}

static bool process_request(InputRqParsingOutput& data)
{
  bool ok;
  int arg_sector_number = 0;
  int arg_playing_sector = 0;
  
  if (data.id_attr_found)
  {
    // Depending on a request type, id attribute can contain one or two numbers.
    int scan_result = sscanf(data.id_attr_buf, "%d,%d", &arg_sector_number, &arg_playing_sector);
  
    switch (scan_result)
    {
    case 2:
      break;
    case 1:
      arg_playing_sector = arg_sector_number;
      break;
    default:
      error_code = ERROR_INVALID_SECTOR_NUMBER;
      return false;
    }
  }
  
  bool(*no_arg_handler)() = NULL;
  bool(*single_arg_handler)(bool) = NULL;
  bool(*two_arg_handler)(int, bool) = NULL;
  bool(*two_int_handler)(int, int) = NULL;

  bool arg_on;

  switch (data.request_type)
  {
  case RQ_TYPE_ALIVE:
    no_arg_handler = process_alive_request;
    break;
  case RQ_TYPE_SECTOR_NUMBER_ON:
    two_arg_handler = sector_number_led_pin_write;
    arg_on = true;
    break;
  case RQ_TYPE_SECTOR_NUMBER_OFF:
    two_arg_handler = sector_number_led_pin_write;
    arg_on = false;
    break;
  case RQ_TYPE_SECTOR_ARROW_ON:
    two_arg_handler = sector_arrow_led_pin_write;
    arg_on = true;
    break;
  case RQ_TYPE_SECTOR_ARROW_OFF:
    two_arg_handler = sector_arrow_led_pin_write;
    arg_on = false;
    break;
  case RQ_TYPE_SECTOR_NUMBER_ALL_ON:
    single_arg_handler = sector_all_number_leds_write;
    arg_on = true;
    break;
  case RQ_TYPE_SECTOR_NUMBER_ALL_OFF:
    single_arg_handler = sector_all_number_leds_write;
    arg_on = false;
    break;
  case RQ_TYPE_SECTOR_ARROW_ALL_ON:
    single_arg_handler = sector_all_arrow_leds_write;
    arg_on = true;
    break;
  case RQ_TYPE_SECTOR_ARROW_ALL_OFF:
    single_arg_handler = sector_all_arrow_leds_write;
    arg_on = false;
    break;
  case RQ_TYPE_SECTORS_ALL_ON:
    single_arg_handler = sectors_all_leds_write;
    arg_on = true;
    break;
  case RQ_TYPE_SECTORS_ALL_OFF:
    single_arg_handler = sectors_all_leds_write;
    arg_on = false;
    break;
  case RQ_TYPE_SECTOR_PLAY:
    two_int_handler = sectors_show_playing_sector;
    break;
  case RQ_TYPE_BARREL_WAIT:
    no_arg_handler = process_barrel_wait_request;
    break;
  default:
    error_code = ERROR_INVALID_RQ_TYPE;
    return false;
  }

  if (no_arg_handler)
  {
    ok = no_arg_handler();
  }
  else if (single_arg_handler)
  {
    ok = single_arg_handler(arg_on);
  }
  else if (two_arg_handler)
  {
    ok = two_arg_handler(arg_sector_number, arg_on);
  }
  else if (two_int_handler)
  {
    ok = two_int_handler(arg_sector_number, arg_playing_sector);
  }
  else
    ok = false; // should be impossible to get here
  if (!ok)
    error_code = ERROR_REQUEST_PROCESSING;
  return ok;
}

static bool verify_request(InputRqParsingOutput& data)
{
  switch (data.request_type)
  {
  case RQ_TYPE_ALIVE:
  case RQ_TYPE_SECTOR_NUMBER_ON:
  case RQ_TYPE_SECTOR_NUMBER_OFF:
  case RQ_TYPE_SECTOR_NUMBER_ALL_ON:
  case RQ_TYPE_SECTOR_NUMBER_ALL_OFF:
  case RQ_TYPE_SECTOR_ARROW_ON:
  case RQ_TYPE_SECTOR_ARROW_OFF:
  case RQ_TYPE_SECTOR_ARROW_ALL_ON:
  case RQ_TYPE_SECTOR_ARROW_ALL_OFF:
  case RQ_TYPE_BARREL_WAIT:
  case RQ_TYPE_SECTORS_ALL_ON:
  case RQ_TYPE_SECTORS_ALL_OFF:
  case RQ_TYPE_SECTOR_PLAY:
    break;
  default:
    error_code = ERROR_INVALID_RQ_TYPE;
    return false;
  }

  data.id_attr_buf[sizeof(data.id_attr_buf) - 1] = '\0';

  switch (data.request_type)
  {
  case RQ_TYPE_SECTOR_NUMBER_ON:
  case RQ_TYPE_SECTOR_NUMBER_OFF:
  case RQ_TYPE_SECTOR_ARROW_ON:
  case RQ_TYPE_SECTOR_ARROW_OFF:
  case RQ_TYPE_SECTOR_PLAY:
  {
    if (!data.id_attr_found)
    {
      error_code = ERROR_SECTOR_ATTR_NOT_FOUND;
      return false;
    }

    size_t sector_attr_len = strlen(data.id_attr_buf);

    if (sector_attr_len == 0 || sector_attr_len > sizeof(data.id_attr_buf) - 1)
    {
      error_code = ERROR_BAD_SECTOR_NUMBER;
      return false;
    }
  }
    break;
  default:
    break;
  }

  return true;
}

bool verify_and_process_request(InputRqParsingOutput& data)
{
  error_code = ERROR_NONE;

  bool ok = verify_request(data) && process_request(data);
  
  return ok;
}
