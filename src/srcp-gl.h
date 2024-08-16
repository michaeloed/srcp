/* $Id: srcp-gl.h 1764 2021-03-02 21:29:30Z gscholz $ */

#ifndef _SRCP_GL_H
#define _SRCP_GL_H

#include <stdbool.h>
#include <sys/time.h>

#include "config-srcpd.h"
#include "srcp-session.h"

enum gl_state_t {glsNone = 0, glsActive, glsTerm};

/* GL device data set */
typedef struct {
    enum gl_state_t state;    /* 0==none, 1==active, 2==terminating */
    char protocol;
    int protocolversion;
    int n_func;               /* number of functions */
    int n_fs;                 /* number of speed steps*/
    int id;                   /* address  */
    int speed;                /* target speed scaled to        0..14 */
    int direction;            /* 0/1/2                               */
    unsigned int funcs;       /* Fx..F1, F                           */
    struct timeval tv;        /* Last time of change                 */
    struct timeval inittime;
    struct timeval locktime;
    sessionid_t locked_by;    /* locking session */
    long int lockduration;
} gl_data_t;

/* GL device group data set */
typedef struct {
    unsigned int glcount;
    gl_data_t *glstate;
} gl_t;

void startup_GL();
void shutdown_GL();
int init_GL(bus_t busnumber, unsigned int count);
unsigned int getMaxAddrGL(bus_t busnumber);
bool isInitializedGL(bus_t busnumber, int addr);
bool isValidGL(bus_t busnumber, int addr);
int enqueueGL(bus_t busnumber, int addr, int dir, int speed,
        int maxspeed, int f);
int queue_GL_isempty(bus_t busnumber);
int dequeueNextGL(bus_t busnumber, gl_data_t * l);
int cacheGetGL(bus_t busnumber, int addr, gl_data_t * l);
int cacheSetGL(bus_t busnumber, int addr, gl_data_t l);
int cacheInfoGL(bus_t busnumber, int addr, char *info);
int cacheDescribeGL(bus_t busnumber, int addr, char *msg);
int cacheInitGL(bus_t busnumber, int addr, const char protocol,
                int protoversion, int n_fs, int n_func);
int cacheTermGL(bus_t busnumber, int addr);
void cacheCleanGL(bus_t bus);
int cacheLockGL(bus_t busnumber, int addr, long int duration,
                sessionid_t sessionid);
int cacheGetLockGL(bus_t busnumber, int addr, sessionid_t * sessionid);
int cacheUnlockGL(bus_t busnumber, int addr, sessionid_t sessionid);
void unlock_gl_bysessionid(sessionid_t sessionid);
void unlock_gl_bytime(void);
int describeLOCKGL(bus_t bus, int addr, char *reply);
void debugGL(bus_t busnumber, int start, int end);

#endif
