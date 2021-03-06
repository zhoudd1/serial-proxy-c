#ifndef SERVER_H
#define SERVER_H

#include "ae.h"
#include "serial.h"

#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Error codes */
#define C_OK  (0)
#define C_ERR (-1)

/* Log levels */
#define LL_DEBUG (0)
#define LL_INFO  (1)
#define LL_WARN  (2)
#define LL_ERROR (3)

#define LOG_MAX_LEN (1024)

/* Static server configuration */
#define CONFIG_DEFAULT_HZ                    (10)
#define CONFIG_MIN_HZ                        (1)
#define CONFIG_MAX_HZ                        (500)
#define CONFIG_DEFAULT_PID_FILE              ("/var/run/sproxyd.pid")
#define CONFIG_DEFAULT_DAEMONIZE             (0)
#define CONFIG_DEFAULT_SYSLOG_ENABLED        (0)
#define CONFIG_DEFAULT_MAX_CLIENTS           (1000)
#define CONFIG_DEFAULT_VERBOSITY             (LL_ERROR)
#define CONFIG_DEFAULT_RECONNECT_INTERVAL_MS (5000)
#define CONFIG_MIN_RECONNECT_INTERVAL_MS     (1000)
#define CONFIG_MAX_RECONNECT_INTERVAL_MS     (3600000) /* 24 hours */
#define CONFIG_DEFAULT_SERIAL_CONFIG_FILE    ("serial.ini")

/* Convert milliseconds to cronloops based on server HZ value */
#define run_with_period(_ms_) if ((_ms_ <= 1000/server.hz) || \
    !(server.cronloops%((_ms_)/(1000/server.hz))))

#define strlcpy(dst, src, size) \
    (snprintf(dst, size, "%s", src))

struct sproxyServer {
    pid_t pid;                  /* Main process PID*/
    char *pidfile;              /* PID file path */
    char *logfile;              /* Log file */
    char *configfile;           /* System config file */
    int shutdown;               /* Signal to shutdown */
    int reload;                 /* Signal to reload config */
    int daemonize;              /* True if running as a daemon */
    int verbosity;              /* Logging level */
    int syslog;                 /* Is syslog enabled? */
    int maxclients;             /* Max concurrent clients */
    int cronloops;              /* Number of times the cron function run */
    int reconnect_interval;     /* Number of milliseconds to wait before
                                   reconnecting serial devices */
    long long cron_event_id;    /* Cron task id */
    aeEventLoop *el;
    int hz;                     /* Timer event frequency */
    char *serial_configfile;    /* Serial config file */
    struct serialState serial;  /* State of serial devices */
};

extern struct sproxyServer server;

/**
 * @brief Logging helper that accepts a formatted string and any number of
 *        arguments to fill it.
 *
 * @param[in] level - Logging level of message
 * @param[in] fmt - String to substitute with given variable arguments
 * @param[in] ... - Variable arguments
 */
void serverLog(int level, const char *fmt, ...);

/**
 * @brief Logging helper to print given message.
 *
 * @param[in] level - Logging level of message
 * @param[in] msg - Message to log
 */
void serverLogRaw(int level, const char *msg);

/**
 * @brief Logging helper to print errno diagnostics.
 *
 * @param[in] level - Logging level of message
 * @param[in] fmt - String to substitute with given variable arguments
 * @param[in] ... - Variable arguments
 */
void serverLogErrno(int level, const char *fmt, ...);

/**
 * @brief Log level to string.
 *
 * @param[in] level - Log level
 *
 * @return Log level string or NULL if level is invalid
 */
const char *serverLogLevel(int level);

/**
 * @brief Load server configuration from given file.
 *
 * @param[in] filename - File name containing server configuration
 */
void serverLoadConfig(const char *filename);

/**
 * @brief Set serial default configuration and load overrides from file.
 */
void serialInit(void);

/**
 * @brief Attempt a clean shutdown by closing all serialNode devices.
 */
void serialTerm(void);

/**
 * @brief Called before every event loop iteration. Reset all serialNode
 *        receive buffers.
 */
void serialBeforeSleep(void);

/**
 * @brief Called at a specified interval, will attempt to reconnect all
 *        serialNode that are disconnected.
 */
void serialCron(void);

/**
 * @brief Load serial configuration from given file.
 *
 * @param[in] filename - File name containing serial configuration
 */
void serialLoadConfig(const char *filename);

#endif
