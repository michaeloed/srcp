/* $Id: srcp-gl.c 1765 2021-03-02 21:43:13Z gscholz $ */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config-srcpd.h"
#include "srcp-gl.h"
#include "srcp-error.h"
#include "srcp-info.h"
#include "syslogmessage.h"

#define QUEUELEN 50

/* complete GL device group data set */
static gl_t gl[MAX_BUSES];

/* command queues for each bus */
static gl_data_t queue[MAX_BUSES][QUEUELEN];
static pthread_mutex_t queue_mutex[MAX_BUSES];
/* write position for queue writers */
static int out[MAX_BUSES], in[MAX_BUSES];

/* forward declaration of internal functions */
static int queue_len(bus_t busnumber);
static int queue_isfull(bus_t busnumber);

/**
 * isValidGL: checks if a given address could be a valid GL.
 * returns true or false. false, if not all requirements are met.
 */
bool isValidGL(bus_t busnumber, int addr)
{
    /* in bus 0 GL are not allowed */
    /* only num_buses are configured */
    /* number of GL is set */
    /* address must be greater 0 */
    /* but not more than the maximum address on that bus */
    return (busnumber <= num_buses &&
           gl[busnumber].glcount > 0 &&
           addr > 0 &&
           addr <= gl[busnumber].glcount);
}

/**
 * getMaxAddrGL: returns the maximum address for GL on the given bus
 * returns: =0: no GL on that bus, or busid not valid
 *          >0: maximum address
 */
unsigned int getMaxAddrGL(bus_t busnumber)
{
    if (busnumber <= num_buses) {
        return gl[busnumber].glcount;
    }
    return 0;
}

/* there are decoders for 14, 27, 28 and 128 speed steps */
static int calcspeed(int vs, int vmax, int n_fs)
{
    int rs;

    if (vmax == 0)
        return vs;
    if (vs < 0)
        vs = 0;
    if (vs > vmax)
        vs = vmax;
    /* rs = (vs * n_fs) / vmax; */
    /* for test: rs = ((vs * n_fs) / v_max) + 0.5 */
    /* ==> rs = ((2 * vs * n_fs) + v_max) / (2 * v_max) */
    rs = vs * n_fs;             /* vs * n_fs */
    rs <<= 1;                   /* 2 * vs * n_fs */
    rs += vmax;                 /* (2 * vs * n_fs) + v_max */
    rs /= vmax;                 /* ((2 * vs * n_fs) + v_max) / v_max */
    rs >>= 1;                   /* ((2 * vs * n_fs) + v_max) / (2 * v_max) */
    if ((rs == 0) && (vs != 0))
        rs = 1;

    return rs;
}

/* checks whether a GL is already initialized or not
 * returns false even, if it is an invalid address!
 */
bool isInitializedGL(bus_t busnumber, int addr)
{
    if (isValidGL(busnumber, addr)) {
        return (gl[busnumber].glstate[addr].state == glsActive);
    }
    return false;
}

/* Take new locomotive data and make some checks.
 * Lock is ignored! Lock is payed attention to in SRCP procedures, here not
 * necessary (emergency stop)
 */
int enqueueGL(bus_t busnumber, int addr, int dir, int speed, int maxspeed,
              int f)
{
    if (!isValidGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    int result;
    struct timeval now;

    if (!isInitializedGL(busnumber, addr)) {
        cacheInitGL(busnumber, addr, 'P', 1, 14, 1);
        syslog_bus(busnumber, DBG_WARN, "GL default init for %d-%d",
                busnumber, addr);
    }
    if (queue_isfull(busnumber)) {
        syslog_bus(busnumber, DBG_WARN, "GL command queue full");
        return SRCP_TEMPORARILYPROHIBITED;
    }

    result = pthread_mutex_lock(&queue_mutex[busnumber]);
    if (result != 0) {
        syslog_bus(busnumber, DBG_ERROR,
                "pthread_mutex_lock() failed: %s (errno = %d).",
                strerror(result), result);
    }

    /* copy all known (INIT) values to queue */
    queue[busnumber][in[busnumber]] = gl[busnumber].glstate[addr];

    /* update remaining new variables */
    queue[busnumber][in[busnumber]].speed =
        calcspeed(speed, maxspeed, gl[busnumber].glstate[addr].n_fs);

    queue[busnumber][in[busnumber]].direction = dir;
    queue[busnumber][in[busnumber]].funcs = f;
    gettimeofday(&now, NULL);
    queue[busnumber][in[busnumber]].tv = now;
    queue[busnumber][in[busnumber]].id = addr;
    in[busnumber]++;
    if (in[busnumber] == QUEUELEN)
        in[busnumber] = 0;

    result = pthread_mutex_unlock(&queue_mutex[busnumber]);
    if (result != 0) {
        syslog_bus(busnumber, DBG_ERROR,
                "pthread_mutex_unlock() failed: %s (errno = %d).",
                strerror(result), result);
    }

    /* Restart thread to send GL command */
    resume_bus_thread(busnumber);
    return SRCP_OK;
}

int queue_GL_isempty(bus_t busnumber)
{
    return (in[busnumber] == out[busnumber]);
}

static int queue_len(bus_t busnumber)
{
    if (in[busnumber] >= out[busnumber])
        return in[busnumber] - out[busnumber];
    else
        return QUEUELEN + in[busnumber] - out[busnumber];
}

/* maybe, 1 element in the queue cannot be used.. */
static int queue_isfull(bus_t busnumber)
{
    return queue_len(busnumber) >= QUEUELEN - 1;
}

/** result is next item or -1, updates fifo pointer! */
int dequeueNextGL(bus_t busnumber, gl_data_t *gld)
{
    if (in[busnumber] == out[busnumber])
        return -1;

    *gld = queue[busnumber][out[busnumber]];
    out[busnumber]++;
    if (out[busnumber] == QUEUELEN)
        out[busnumber] = 0;
    return out[busnumber];
}

int cacheGetGL(bus_t busnumber, int addr, gl_data_t *gld)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_NODATA;

    *gld = gl[busnumber].glstate[addr];
    return SRCP_OK;
}

/**
 * cacheSetGL is called from the hardware drivers to keep the
 * the data and the info mode informed. It is called from
 * within the SRCP SET Command code.
 * It respects the TERM function.
*/
int cacheSetGL(bus_t busnumber, int addr, gl_data_t l)
{
    if (!isValidGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    char msg[1000];
    gl[busnumber].glstate[addr] = l;
    gettimeofday(&gl[busnumber].glstate[addr].tv, NULL);
    if (gl[busnumber].glstate[addr].state == glsTerm) {
        snprintf(msg, sizeof(msg), "%lu.%.3lu 102 INFO %ld GL %d\n",
                gl[busnumber].glstate[addr].tv.tv_sec,
                gl[busnumber].glstate[addr].tv.tv_usec / 1000,
                busnumber, addr);
        bzero(&gl[busnumber].glstate[addr], sizeof(gl_data_t));
    }
    else {
        cacheInfoGL(busnumber, addr, msg);
    }
    enqueueInfoMessage(msg);
    return SRCP_OK;
}

int cacheInitGL(bus_t busnumber, int addr, const char protocol,
                int protoversion, int n_fs, int n_func)
{
    if (!isValidGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    char msg[1000];
    gl_data_t tgl;
    memset(&tgl, 0, sizeof(tgl));
    int rc = bus_supports_protocol(busnumber, protocol);
    if (rc != SRCP_OK) {
        return rc;
    }
    gettimeofday(&tgl.inittime, NULL);
    tgl.tv = tgl.inittime;
    tgl.n_fs = n_fs;
    tgl.n_func = n_func;
    tgl.protocolversion = protoversion;
    tgl.protocol = protocol;
    tgl.id = addr;
    if (buses[busnumber].init_gl_func)
        rc = (*buses[busnumber].init_gl_func) (&tgl);
    if (rc == SRCP_OK) {
        gl[busnumber].glstate[addr] = tgl;
        gl[busnumber].glstate[addr].state = glsActive;
        cacheDescribeGL(busnumber, addr, msg);
        enqueueInfoMessage(msg);
    }
    return rc;
}


int cacheTermGL(bus_t busnumber, int addr)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_NODATA;

    gl_data_t *gld = &gl[busnumber].glstate[addr];

    /*we are terminating now*/
    gld->state = glsTerm;

    /* first brake if current speed != 0 */
    if (gld->speed != 0) {
        gld->speed = 0;
        gld->direction = 2;
        enqueueGL(busnumber, addr, gld->direction, gld->speed,
                gld->n_fs, gld->funcs);
    }

    /* second terminate GL */
    char msg[256];
    gettimeofday(&gld->tv, NULL);
    snprintf(msg, sizeof(msg), "%lu.%.3lu 102 INFO %ld GL %d\n",
            gld->tv.tv_sec, gld->tv.tv_usec / 1000, busnumber, addr);
    enqueueInfoMessage(msg);

    /* third clear GL data */
    memset(gld, 0, sizeof(gl_data_t));

    return SRCP_OK;
}

/*
 * RESET a GL to its defaults
 */
int resetGL(bus_t busnumber, int addr)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_NODATA;

    enqueueGL(busnumber, addr, 0, 0, 1, 0);
    return SRCP_OK;
}


void cacheCleanGL(bus_t bus)
{
    for (unsigned int i = 1; i <= gl[bus].glcount; i++) {
        bzero(&gl[bus].glstate[i], sizeof(gl_data_t));
    }
}


int cacheDescribeGL(bus_t busnumber, int addr, char *msg)
{
    if (!isInitializedGL(busnumber, addr)) {
        strcpy(msg, "");
        return SRCP_NODATA;
    }

    gl_data_t* gld = &gl[busnumber].glstate[addr];
    sprintf(msg, "%lu.%.3lu 101 INFO %ld GL %d %c %d %d %d\n",
            gld->inittime.tv_sec,
            gld->inittime.tv_usec / 1000,
            busnumber, addr, gld->protocol,
            gld->protocolversion,
            gld->n_fs,
            gld->n_func);
    return SRCP_INFO;
}

int cacheInfoGL(bus_t busnumber, int addr, char *msg)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_NODATA;

    char line[MAXSRCPLINELEN];
    gl_data_t* gld = &gl[busnumber].glstate[addr];

    sprintf(msg, "%lu.%.3lu 100 INFO %ld GL %d %d %d %d %d",
            gld->tv.tv_sec, gld->tv.tv_usec / 1000,
            busnumber, addr, gld->direction,
            gld->speed, gld->n_fs,
            (gld->funcs & 0x01) ? 1 : 0);

    for (int i = 1; i < gld->n_func; i++) {
        snprintf(line, sizeof(line), "%s %d", msg,
                ((gld->funcs >> i) & 0x01) ? 1 : 0);
        strcpy(msg, line);
    }
    snprintf(line, sizeof(line), "%s\n", msg);
    strcpy(msg, line);

    return SRCP_INFO;
}

/* has to use a semaphore, must be atomized! */
int cacheLockGL(bus_t busnumber, int addr, long int duration,
                sessionid_t sessionid)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    char msg[256];
    gl_data_t *gld = &gl[busnumber].glstate[addr];

    if (gld->locked_by == sessionid || gld->locked_by == 0) {
        gld->locked_by = sessionid;
        gld->lockduration = duration;
        gettimeofday(&gld->locktime, NULL);
        describeLOCKGL(busnumber, addr, msg);
        enqueueInfoMessage(msg);
        return SRCP_OK;
    }
    return SRCP_DEVICELOCKED;
}

int cacheGetLockGL(bus_t busnumber, int addr, sessionid_t * session_id)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    *session_id = gl[busnumber].glstate[addr].locked_by;
    return SRCP_OK;
}

int describeLOCKGL(bus_t busnumber, int addr, char *reply)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    gl_data_t *gld = &gl[busnumber].glstate[addr];

    sprintf(reply, "%lu.%.3lu 100 INFO %ld LOCK GL %d %ld %ld\n",
            gld->locktime.tv_sec,
            gld->locktime.tv_usec / 1000,
            busnumber, addr, gld->lockduration,
            gld->locked_by);
    return SRCP_OK;
}

int cacheUnlockGL(bus_t busnumber, int addr, sessionid_t sessionid)
{
    if (!isInitializedGL(busnumber, addr))
        return SRCP_WRONGVALUE;

    gl_data_t *gld = &gl[busnumber].glstate[addr];

    if (gld->locked_by == 0 || gld->locked_by == sessionid) {
        char msg[256];
        gld->locked_by = 0;
        gettimeofday(&gld->locktime, NULL);
        sprintf(msg, "%lu.%.3lu 102 INFO %ld LOCK GL %d\n",
                gld->locktime.tv_sec,
                gld->locktime.tv_usec / 1000,
                busnumber, addr);
        enqueueInfoMessage(msg);
        return SRCP_OK;
    }
    return SRCP_DEVICELOCKED;
}

/**
 * called when a session is terminating
 */
void unlock_gl_bysessionid(sessionid_t sessionid)
{
    syslog_session(sessionid, DBG_DEBUG, "Unlocking GLs by session-id");
    for (bus_t i = 0; i <= num_buses; i++) {
        int number = getMaxAddrGL(i);
        for (int j = 1; j <= number; j++) {
            if (gl[i].glstate[j].locked_by == sessionid) {
                cacheUnlockGL(i, j, sessionid);
            }
        }
    }
}

/**
 * called once per second to unlock
 */
void unlock_gl_bytime(void)
{
    for (bus_t i = 0; i <= num_buses; i++) {
        int number = getMaxAddrGL(i);
        for (int j = 1; j <= number; j++) {
            if (gl[i].glstate[j].lockduration > 0
                && gl[i].glstate[j].lockduration-- == 1) {
                cacheUnlockGL(i, j, gl[i].glstate[j].locked_by);
            }
        }
    }
}

/**
 * First initialisation after program start up
 */
void startup_GL()
{
    for (bus_t i = 0; i < MAX_BUSES; i++) {
        in[i] = 0;
        out[i] = 0;
        gl[i].glcount = 0;
        gl[i].glstate = NULL;

        int result = pthread_mutex_init(&queue_mutex[i], NULL);
        if (result != 0) {
            syslog_bus(0, DBG_ERROR,
                       "pthread_mutex_init() failed: %s (errno = %d).",
                       strerror(result), result);
        }
    }
}

/*destroy all occupied mutexes*/
void shutdown_GL()
{
    for (bus_t i = 0; i < MAX_BUSES; i++) {
        free(gl[i].glstate);
        int result = pthread_mutex_destroy(&queue_mutex[i]);
        if (result != 0) {
            syslog_bus(0, DBG_ERROR,
                       "pthread_mutex_destroy() failed: %s (errno = %d).",
                       strerror(result), result);
        }
    }
}

/**
 * allocate memory for all single GL devices used by this bus,
 * called by configuration routines
 * return values:
 *    1: bus > maxbuses, memory accoration error
 *    0: success
 */
int init_GL(bus_t busnumber, unsigned int count)
{
    syslog_bus(busnumber, DBG_INFO, "init GL: %d", count);
    if (busnumber >= MAX_BUSES)
        return 1;

    if (count > 0) {
        gl[busnumber].glstate = malloc((count + 1) * sizeof(gl_data_t));
        if (gl[busnumber].glstate == NULL)
            return 1;
        gl[busnumber].glcount = count;
        for (unsigned int i = 0; i <= count; i++) {
            bzero(&gl[busnumber].glstate[i], sizeof(gl_data_t));
        }
    }
    return 0;
}

void debugGL(bus_t busnumber, int start, int end)
{
    gl_data_t *gld;

    syslog_bus(busnumber, DBG_WARN, "debug GLSTATE from %d to %d", start,
               end);
    for (int i = start; i <= end; i++) {
        gld = &gl[busnumber].glstate[i];
        syslog_bus(busnumber, DBG_WARN, "GLSTATE for %d/%d", busnumber, i);
        syslog_bus(busnumber, DBG_WARN, "state %d", gld->state);
        syslog_bus(busnumber, DBG_WARN, "protocol %c", gld->protocol);
        syslog_bus(busnumber, DBG_WARN, "protocolversion %d",
                   gld->protocolversion);
        syslog_bus(busnumber, DBG_WARN, "n_func %d", gld->n_func);
        syslog_bus(busnumber, DBG_WARN, "n_fs %d", gld->n_fs);
        syslog_bus(busnumber, DBG_WARN, "id %d", gld->id);
        syslog_bus(busnumber, DBG_WARN, "speed %d", gld->speed);
        syslog_bus(busnumber, DBG_WARN, "direction %d", gld->direction);
        syslog_bus(busnumber, DBG_WARN, "funcs %d", gld->funcs);
        syslog_bus(busnumber, DBG_WARN, "lockduration %ld",
                   gld->lockduration);
        syslog_bus(busnumber, DBG_WARN, "locked_by %ld", gld->locked_by);
    }
}
