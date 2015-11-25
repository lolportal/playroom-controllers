#include "sectors.h"
#include "table-controller-pin-config.h"
#include "sector_test_mode.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define N_ELEMS(a) (sizeof(a)/sizeof(a[0]))

enum
{
  SECTOR_MOST_LEFT = 0,

  SECTOR_MOST_RIGHT = 14,

  SECTOR_NONE = -1,

  SECTOR_INVALID = -2,

};

static bool event_is_ready = false;

static SectorEventData sector_event_data;

static bool wait_for_spin_flag = false;;

static void reset_sector_sensors_state();

void set_wait_for_spin_flag()
{
  noInterrupts();
  wait_for_spin_flag = true;
  reset_sector_sensors_state();
  interrupts();
}

bool is_sector_event_ready(SectorEventData *data)
{
  noInterrupts();
  bool retval = event_is_ready;
  if (event_is_ready)
    *data = sector_event_data;
  interrupts();
  return retval;
}

void reset_sector_event()
{
  noInterrupts();
  event_is_ready = false;
  sector_event_data.reset();
  interrupts();
}

void sectors_init()
{
  Serial.println("Sector pins initialized");

  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];

    if (SECTOR_PIN_ACTIVE_LEVEL == HIGH)
      pinMode(sp.sensor_pin, INPUT);
    else if (SECTOR_PIN_ACTIVE_LEVEL == LOW)
      pinMode(sp.sensor_pin, INPUT_PULLUP);
    else
      Serial.println("Invalid value for sector pin active level.");

    pinMode(sp.number_led_pin, OUTPUT);
    digitalWrite(sp.number_led_pin, LOW);

    pinMode(sp.arrow_led_pin, OUTPUT);
    digitalWrite(sp.arrow_led_pin, LOW);
  }
}

static bool check_number(int n)
{
  if (n >= 0 && (size_t) n < NUMBER_OF_SECTORS)
  {
    return true;
  }
  Serial.print("Invalid sector number: ");
  Serial.println(n);
  return false;
}

static bool check_number_convert_to_internal(int& n)
{
  int internal_number = n - 1;
//  int internal_number = n;
  if (check_number(internal_number))
  {
    n = internal_number;
    return true;
  }
  return false;
}

static int convert_to_external(int n)
{
  if (check_number(n))
  {
    //return n + 1;
    return n;
  }
  return SECTOR_INVALID;
}

bool sectors_all_leds_write(bool on)
{
  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];
    digitalWrite(sp.number_led_pin, on);
    digitalWrite(sp.arrow_led_pin, on);
  }
  return true;
}

bool sector_all_number_leds_write(bool on)
{
  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];
    digitalWrite(sp.number_led_pin, on);
  }
  return true;
}
bool sector_all_arrow_leds_write(bool on)
{
  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];
    digitalWrite(sp.arrow_led_pin, on);
  }
  return true;
}

bool sector_number_led_pin_write(int n, bool on)
{
  if (!check_number_convert_to_internal(n))
    return false;

  SectorPins sp = sector_pins[n];
  pinMode(sp.number_led_pin, OUTPUT);
  digitalWrite(sp.number_led_pin, on);
  return true;
}


bool sector_arrow_led_pin_write(int n, bool on)
{
  if (!check_number_convert_to_internal(n))
    return false;

  SectorPins sp = sector_pins[n];
  pinMode(sp.arrow_led_pin, OUTPUT);
  digitalWrite(sp.arrow_led_pin, on);
  return true;
}

typedef int Sector;

static Sector next_right(const Sector s)
{
  if (s >= SECTOR_MOST_LEFT && s < SECTOR_MOST_RIGHT)
    return s + 1;
  if (s == SECTOR_MOST_RIGHT)
    return SECTOR_MOST_LEFT;
  Serial.print("ERROR: Invalid sector number: ");
  Serial.println(s);
  return SECTOR_INVALID;
}

static Sector prev_left(const Sector s)
{
  if (s > SECTOR_MOST_LEFT && s <= SECTOR_MOST_RIGHT)
    return s - 1;
  if (s == SECTOR_MOST_LEFT)
    return SECTOR_MOST_RIGHT;
  Serial.print("ERROR: Invalid sector number: ");
  Serial.println(s);
  return SECTOR_INVALID;
}

bool sectors_show_playing_sector(int actual_sector, int playing_sector)
{
  if (!check_number_convert_to_internal(actual_sector) || !check_number_convert_to_internal(playing_sector))
    return false;

  if (actual_sector == playing_sector)
  {
    sector_number_led_pin_write(actual_sector, true);
  }
  else
  {
    do
    {
      sector_arrow_led_pin_write(actual_sector, true);
      actual_sector = next_right(actual_sector);
    } while (actual_sector != playing_sector);
    sector_number_led_pin_write(playing_sector, true);
  }
  return true;
}

static Sector check_enabled_sector()
{
  int on_count = 0;
  Sector enabled_sectors[2];
  for (Sector i = 0; i < (Sector) N_ELEMS(sector_pins); ++i)
  {
    const SectorPins sp = sector_pins[i];
    bool on = (digitalRead(sp.sensor_pin) == SECTOR_PIN_ACTIVE_LEVEL);
    if (on)
    {
      ++on_count;
      if (on_count >= 3)
        break;
      enabled_sectors[on_count - 1] = i;
    }
  }

  switch (on_count)
  {
  case 0:
    return SECTOR_NONE;
  case 1:
    return enabled_sectors[0];
  case 2:
    if ((enabled_sectors[1] == next_right(enabled_sectors[0])) ||
      (enabled_sectors[1] == prev_left(enabled_sectors[0])))
    {
      return enabled_sectors[0];
    }
    Serial.print("sectors not adjacent: ");
    Serial.print(enabled_sectors[0]);
    Serial.print(" ");
    Serial.println(enabled_sectors[1]);
    return SECTOR_INVALID;
  default:
    Serial.println("too many sectors are on");
    return SECTOR_INVALID;
  }
}

static bool check_sector_is_on(Sector s)
{
  if (!check_number(s))
    return false;
  return digitalRead(sector_pins[s].sensor_pin) == SECTOR_PIN_ACTIVE_LEVEL;
}

typedef enum
{
  STATE_READY,
  STATE_WAIT_FOR_DIRECTION,
  STATE_WAIT_FOR_NEXT_SECTOR,
  STATE_WAIT_FOR_STOP,
} State;

typedef enum
{
  DIRECTION_RIGHT,
  DIRECTION_LEFT,
} Direction;

static Sector prev_sector;

static Sector correct_sectors_counter;

static State state = STATE_READY;

unsigned long prev_time;

Direction dir;

static void reset_sector_sensors_state()
{
  state = STATE_READY;
  correct_sectors_counter = 0;
  sector_all_number_leds_write(false);
  sector_all_arrow_leds_write(false);
}

void read_sector_sensors_and_light_their_leds()
{  
  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];
    digitalWrite(sp.number_led_pin, digitalRead(sp.sensor_pin));
  }
}

void read_sector_sensors_and_light_all_leds()
{  
  for (size_t i = 0; i < N_ELEMS(sector_pins); ++i)
  {
    SectorPins sp = sector_pins[i];
    digitalWrite(sp.number_led_pin, digitalRead(sp.sensor_pin));
    digitalWrite(sp.arrow_led_pin, digitalRead(sp.sensor_pin));
  }
}

//#define SECTORS_DEBUG_MSG

void sectors_process_sensors()
{
  if (sector_test_mode_is_enabled())
  {
    read_sector_sensors_and_light_all_leds();
    return;
  }

  if (!wait_for_spin_flag)
    return;
  
  read_sector_sensors_and_light_their_leds();
  
  // at this point s is either a valid sector number or a SECTOR_NONE.
  if (state == STATE_READY)
  {
    Sector s = check_enabled_sector();
    switch (s)
    {
    case SECTOR_NONE:
      return;
    case SECTOR_INVALID:
      reset_sector_sensors_state();
      return;
    default:
      if (!check_number(s))
      {
        reset_sector_sensors_state();
        return;
      }
      prev_time = millis();
      prev_sector = s;
      state = STATE_WAIT_FOR_DIRECTION;
      #ifdef SECTORS_DEBUG_MSG
      Serial.print(prev_time);
      Serial.print(": First sector: ");
      Serial.println(s);
      #endif
      return;
    }
  }
  else if (state == STATE_WAIT_FOR_DIRECTION)
  {
    if (millis() - prev_time >= 2 * SECTOR_TURN_TIME_MS)
    {
      #ifdef SECTORS_DEBUG_MSG
      Serial.println("Waited for direction detection for too long...");
      #endif
      reset_sector_sensors_state();
      return;
    }

    Sector second_right_sector = next_right(next_right(prev_sector));
    Sector second_left_sector = prev_left(prev_left(prev_sector));

    Sector s = check_enabled_sector();
    bool right_is_on = (digitalRead(sector_pins[second_right_sector].sensor_pin) == SECTOR_PIN_ACTIVE_LEVEL);
    bool left_is_on = (digitalRead(sector_pins[second_left_sector].sensor_pin) == SECTOR_PIN_ACTIVE_LEVEL);

    if (s == prev_sector)
    {
      prev_time = millis();
      return;
    }

    if (s == SECTOR_NONE || (!right_is_on && !left_is_on))
    {
      return;
    }
    if (right_is_on && left_is_on)
    {
      #ifdef SECTORS_DEBUG_MSG
      Serial.println("ERROR: Both right and left movement are detected.");
      #endif
      reset_sector_sensors_state();
      return;
    }
    // at this point either right_is_on == true or left_is_on == true.
    
    
    if (right_is_on)
    {
      dir = DIRECTION_RIGHT;
      prev_sector = second_right_sector;
    }
    else
    {
      dir = DIRECTION_LEFT;
      prev_sector = second_left_sector;
    }

    prev_time = millis();
    state = STATE_WAIT_FOR_NEXT_SECTOR;
    correct_sectors_counter = 0; // maybe should be + 2
    
    #ifdef SECTORS_DEBUG_MSG
    Serial.print(prev_time);
    Serial.print(": Sector after first: ");
    Serial.println(s);
    Serial.print("Direction: ");
    Serial.println(dir == DIRECTION_RIGHT ? "RIGHT" : "LEFT");
    #endif
    return;
  }
  else if (state == STATE_WAIT_FOR_NEXT_SECTOR)
  {
    if (millis() - prev_time >= SECTOR_TURN_TIME_MS)
    {
      #ifdef SECTORS_DEBUG_MSG
      Serial.println("Waited too long for next sector");
      #endif
      reset_sector_sensors_state();
      return;
    }

    Sector expected_sector;
    Sector next_expected_sector;
    if (dir == DIRECTION_RIGHT)
    {
      expected_sector = next_right(prev_sector);
      next_expected_sector = next_right(expected_sector);
    }
    else
    {
      expected_sector = prev_left(prev_sector);
      next_expected_sector = prev_left(expected_sector);
    }

    bool ok = check_sector_is_on(expected_sector);
    
    if (!ok)
    {
      ok = check_sector_is_on(next_expected_sector);
      if (ok)
        expected_sector = next_expected_sector;
    }
    if (ok)
    {
      prev_time = millis();
      prev_sector = expected_sector;
      ++correct_sectors_counter;

      if (correct_sectors_counter >= FULL_TURN_LIMIT)
      {
        event_is_ready = true;
        sector_event_data.set_event_started();
        state = STATE_WAIT_FOR_STOP;
        correct_sectors_counter = 0;
        return;
      }
    }
    
  }
  else if (state == STATE_WAIT_FOR_STOP)
  {
    Sector s = check_enabled_sector();
    if (millis() - prev_time >= SECTOR_TURN_TIME_MS)
    {
      switch (s)
      {
      case SECTOR_NONE:
        event_is_ready = true;
        sector_event_data.set_event_stopped(convert_to_external(prev_sector));
        wait_for_spin_flag = false;
        break;
      case SECTOR_INVALID:
        #ifdef SECTORS_DEBUG_MSG
        Serial.println("Invalid sector detected while waiting for STOP");
        #endif
        break;
      default:
        if (!check_number(s))
        {
          #ifdef SECTORS_DEBUG_MSG
          Serial.println("Wrong number sector detected while waiting for STOP");
          #endif
          break;
        }
        event_is_ready = true;
        sector_event_data.set_event_stopped(convert_to_external(s));
        wait_for_spin_flag = false;
        break;
      }
      reset_sector_sensors_state();
      return;
    }
    if (s != SECTOR_NONE && s != prev_sector && check_number(s))
    {
      prev_time = millis();
      prev_sector = s;
    }

  }
  else
  {
    #ifdef SECTORS_DEBUG_MSG
    Serial.print("Invalid sector state: ");
    Serial.println(state);
    #endif
    reset_sector_sensors_state();
  }
}
