#include <EEPROM.h>

/*  EEPROM contents:
 *   
 *   address  |      data        |     data size
 *  -------------------------------------------------
 *          0 | game type        | 1
 *          1 | die sidedness    | 1
 *    2 - 297 | discord url      | 296
 *  298 - 366 | discord host     | 67
 *  365 - 414 | discord bot name | 50
 *  415 - 446 | wifi ssid        | 32
 *  447 - 510 | wifi password    | 63
 */

// see the table above
void EEPROM_map_init()
{
  EEPROM_map =
  {
    {0,     1},
    {1,     1},
    {2,   296},
    {298,  67},
    {365,  50},
    {415,  32},
    {447,  63}
  };
}

// get a single block of data from EEPROM
String load_block(EEPROM_data data)
{
  String load_string;
  for(int i = 0; i < data.msize; i++)
  {
    byte c = EEPROM.read(data.maddr + i);
    if(c != 0x17)
    {
      load_string.concat(char(c));
    }
    else
    {
      break;
    }
    yield();
  }
  return load_string;
}


// save a single data point to EEPROM
void EEPROM_save_block(String val, EEPROM_data data)
{
  for(int i = 0; i < data.msize; i++)
  {
    if(i < val.length())
    {
      EEPROM.write(data.maddr + i, val.charAt(i));
    }
    else
    {
      EEPROM.write(data.maddr + i, 0x17);
    }
    yield();
  }
}


// save everything specified in the map
void EEPROM_save_data()
{
  EEPROM.begin(511);

  // device state info
  EEPROM.write(0, device_state.game_type);
  EEPROM.write(1, device_state.die_sidedness);

  // discord data
  EEPROM_save_block(discord_data.url, EEPROM_map.url);
  EEPROM_save_block(discord_data.host, EEPROM_map.host);
  EEPROM_save_block(discord_data.bot_name, EEPROM_map.bot_name);
  
  // wifi credentials
  EEPROM_save_block(wifi_credentials.ssid, EEPROM_map.ssid);
  EEPROM_save_block(wifi_credentials.password, EEPROM_map.password);

  EEPROM.end();
}

// load everything specified in the map, update state structs
void EEPROM_load_data()
{
  EEPROM.begin(512);

  // device state info
  device_state.game_type     = EEPROM.read(0);
  device_state.die_sidedness = EEPROM.read(1);

  // discord data
  discord_data.url       = load_block(EEPROM_map.url);
  discord_data.host      = load_block(EEPROM_map.host);
  discord_data.bot_name  = load_block(EEPROM_map.bot_name);

  // wifi credentials
  wifi_credentials.ssid     = load_block(EEPROM_map.ssid);
  wifi_credentials.password = load_block(EEPROM_map.password);

  EEPROM.end(); 
}

