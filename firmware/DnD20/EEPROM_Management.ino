#include <EEPROM.h>
/* [ int, int, int, | int, int, int, int, int, | String, String, String, String, String ]
struct device_data
{
  int   mode;
  int   game_type;
  int   die_sidedness;
}

struct discord_info
{
  String url;
  String host;
  String bot_name;
}

struct wifi_info
{
  String ssid;
  String password;
}*/

/*  EEPROM contents:
 *   
 *   address  |      data        |     data size
 *  -------------------------------------------------
 *          0 | game mode        | 1      
 *          1 | game type        | 1
 *          2 | die sidedness    | 1
 *    3 - 298 | discord url      | 296
 *  299 - 365 | discord host     | 67
 *  366 - 415 | discord bot name | 50
 *  416 - 447 | wifi ssid        | 32
 *  448 - 511 | wifi password    | 63
 */


void EEPROM_map_init()
{
  EEPROM_map =
  {
    {0,     1},
    {1,     1},
    {2,     1},
    {3,   296},
    {299,  67},
    {366,  50},
    {416,  32},
    {448,  63}
  };
}


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


void EEPROM_save_data()
{
  EEPROM.begin(512);

  // device state info
  EEPROM.write(0, device_state.mode);
  EEPROM.write(1, device_state.game_type);
  EEPROM.write(2, device_state.die_sidedness);

  // discord data
  EEPROM_save_block(discord_data.url, EEPROM_map.url);
  EEPROM_save_block(discord_data.host, EEPROM_map.host);
  EEPROM_save_block(discord_data.bot_name, EEPROM_map.bot_name);
  
  // wifi credentials
  EEPROM_save_block(wifi_credentials.ssid, EEPROM_map.ssid);
  EEPROM_save_block(wifi_credentials.password, EEPROM_map.password);

  EEPROM.end();
}

void EEPROM_load_data()
{
  EEPROM.begin(512);

  // device state info
  device_state.mode = EEPROM.read(0);
  device_state.game_type = EEPROM.read(1);
  device_state.die_sidedness = EEPROM.read(2);

  // discord data
  discord_data.url = load_block(EEPROM_map.url);
  discord_data.host = load_block(EEPROM_map.host);
  discord_data.bot_name = load_block(EEPROM_map.bot_name);

  // wifi credentials
  wifi_credentials.ssid = load_block(EEPROM_map.ssid);
  wifi_credentials.password = load_block(EEPROM_map.password);

  EEPROM.end(); 
}

