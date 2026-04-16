
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// https://github.com/espressif/esp-idf/blob/release/v5.5/examples/bluetooth/bluedroid/ble/gatt_client/tutorial/Gatt_Client_Example_Walkthrough.md
void gattc_client_main(void);

bool traverse_send_peer(uint16_t len, uint8_t* value);

#ifdef __cplusplus
}
#endif
