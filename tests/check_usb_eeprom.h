/**
 * @file
 * @author Meike Vocke <meike.vocke\@avionic-design.de>
 * @date 2016
 *
 * @brief Provide testsuite for usb_eeprom
 *
 * @copyright GPLv3
 */

#ifndef CHECK_USB_EEPROM_H
#define CHECK_USB_EEPROM_H

/**
 * @brief Add eeprom test cases to the given suite
 *
 * @param eeprom_suite Suite the test cases should be added
 * @return 0 on success
 */
int eeprom_suite(Suite *eeprom_suite);

#endif /* CHECK_USB_EEPROM_H */
