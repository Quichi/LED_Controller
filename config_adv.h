/*
 * config_adv.h
 *
 *  Created on: 05.03.2017
 *      Author: quichi
 */

#ifndef CONFIG_ADV_H_
#define CONFIG_ADV_H_

#define PWM_SCALING_FACTOR 	4

/* Rooms */
#define LIVINGROOM 	10
#define BEDROOM 	20
#define ALLROOMS	255

/* Message Definess */
#define SET_BRIGHTNESS_ID		0
#define SEND_OWN_STATUS_ID		1
#define SWITCH_OFF_ID			2
#define SWITCH_ON_ID			3
#define SET_DEFAULTS_ID			4
#define SET_STATUSMESSAGES_ID 	5
#define REQUEST_DEFAULTS_ID 101
#define RESPONSE_DEFAULTS_ID 201

#define STATUS_ID			0xFF
#define DO_NOTHING			0xFF

/* EEPROM Definitions */
#define EEPROM_BYTES_USED 3
#define CONFIGURATION_ADDR 0

typedef struct {
	uint8_t default_rgb[3];
	bool status_messages;
}configuration;



#endif /* CONFIG_ADV_H_ */
