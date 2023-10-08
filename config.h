/*
  Project Name:   signal_tower
  Description:    public (non-secret) configuration data
*/

// Configuration Step 1: Create and/or configure secrets.h. Use secrets_template.h as guide to create secrets.h

// Configuration Step 2: Set debug message output
// comment out to turn off; 1 = summary, 2 = verbose
#define DEBUG 2

#define LAMPPIN_RED     15
#define LAMPPIN_ORANGE  13
#define LAMPPIN_GREEN   12
#define LAMPPIN_BLUE    14
// #define LAMPPIN_WHITE


// Configuration variables that are less likely to require changes

const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";

// Masks for each light's status bits in the overall status indicator
uint8_t tower_state;
uint8_t red_mask    = 0b1000;
uint8_t orange_mask = 0b0100;
uint8_t green_mask  = 0b0010;
uint8_t blue_mask   = 0b0001;

#define CONNECT_ATTEMPT_LIMIT 3 // max connection attempts to internet services
#define CONNECT_ATTEMPT_INTERVAL 10 // seconds between internet service connect attempts