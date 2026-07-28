/**
 * @file    app_cfg.h
 * @brief   应用级全局配置（版本号、WiFi、服务器、任务周期）
 * @author  zry
 * @date    2026-07-17
 * @version V1.0.0
 *
 * @note    业务参数
 * @copyright (c) 2026 zry. All rights reserved.
 */
 
#ifndef APP_CFG_H
#define APP_CFG_H


/* Global typedef */

/* Global define */

/* 固件版本 */
#define APP_FW_VERSION_MAJOR    (1U)
#define APP_FW_VERSION_MINOR    (0U)
#define APP_FW_VERSION_PATCH    (0U)

/* WiFi 配置 */
#define APP_WIFI_SSID           "MySSID"
#define APP_WIFI_PASSWORD       "MyPassword"

/* 服务器配置 */
#define APP_SERVER_IP           "192.168.1.100"
#define APP_SERVER_PORT         (8080U)

/* 任务周期 (ms) */
#define APP_TASK_1MS_PERIOD     (1U)
#define APP_TASK_10MS_PERIOD    (10U)
#define APP_TASK_100MS_PERIOD   (100U)
#define APP_TASK_1000MS_PERIOD  (1000U)

/* Global Variable */



#endif /* APP_CFG_H */
