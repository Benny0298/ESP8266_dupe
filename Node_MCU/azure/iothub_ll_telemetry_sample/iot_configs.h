// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            "HUAWEI_P30"
#define IOT_CONFIG_WIFI_PASSWORD        "zGmT!@.651926"

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */
// #define DEVICE_CONNECTION_STRING    "HostName=myEsp.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=beTs0TG91Od3LnBcAf+fXshcvsM7yCtw5DGmXjbJsgE="
 #define DEVICE_CONNECTION_STRING    "HostName=myEsp.azure-devices.net;DeviceId=myEsp;SharedAccessKey=BVMFvho+q3bpejx/Q66TwwdYnhBqTxUgCUQDyzeF2Ds="

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_HTTP

#endif /* IOT_CONFIGS_H */
