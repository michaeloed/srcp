/* $Id: loopback.c 1788 2024-02-19 06:57:33Z gscholz $ */

/**
 * loopback: simple bus driver without any hardware.
 **/

#include <errno.h>
#include <stdlib.h>

#include "config-srcpd.h"
#include "loopback.h"
#include "toolbox.h"
#include "srcp-fb.h"
#include "srcp-ga.h"
#include "srcp-gl.h"
#include "srcp-sm.h"
#include "srcp-power.h"
#include "srcp-server.h"
#include "srcp-info.h"
#include "srcp-error.h"
#include "syslogmessage.h"

#define __loopback ((LOOPBACK_DATA*)buses[busnumber].driverdata)
#define __loopbackt ((LOOPBACK_DATA*)buses[btd->bus].driverdata)

#define MAX_CV_NUMBER 255


int readconfig_LOOPBACK(xmlDocPtr doc, xmlNodePtr node, bus_t busnumber)
{
    buses[busnumber].driverdata = malloc(sizeof(struct _LOOPBACK_DATA));

    if (buses[busnumber].driverdata == NULL) {
        syslog_bus(busnumber, DBG_ERROR,
                   "Memory allocation error in module '%s'.", node->name);
        return 0;
    }

    buses[busnumber].type = SERVER_LOOPBACK;
    buses[busnumber].init_func = &init_bus_LOOPBACK;
    buses[busnumber].thr_func = &thr_sendrec_LOOPBACK;
    buses[busnumber].init_gl_func = &init_gl_LOOPBACK;
    buses[busnumber].init_ga_func = &init_ga_LOOPBACK;
    strcpy(buses[busnumber].description,
           "GA GL FB SM POWER LOCK DESCRIPTION");

    __loopback->number_fb = 0;  /* max 31 */
    __loopback->number_ga = 256;
    __loopback->number_gl = 80;

    xmlNodePtr child = node->children;
    xmlChar *txt = NULL;

    while (child != NULL) {

        if ((xmlStrncmp(child->name, BAD_CAST "text", 4) == 0) ||
            (xmlStrncmp(child->name, BAD_CAST "comment", 7) == 0)) {
            /* just do nothing, it is only formatting text or a comment */
        }

        else if (xmlStrcmp(child->name, BAD_CAST "number_fb") == 0) {
            txt = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
            if (txt != NULL) {
                __loopback->number_fb = atoi((char *) txt);
                xmlFree(txt);
            }
        }
        else if (xmlStrcmp(child->name, BAD_CAST "number_gl") == 0) {
            txt = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
            if (txt != NULL) {
                __loopback->number_gl = atoi((char *) txt);
                xmlFree(txt);
            }
        }
        else if (xmlStrcmp(child->name, BAD_CAST "number_ga") == 0) {
            txt = xmlNodeListGetString(doc, child->xmlChildrenNode, 1);
            if (txt != NULL) {
                __loopback->number_ga = atoi((char *) txt);
                xmlFree(txt);
            }
        }

        else
            syslog_bus(busnumber, DBG_WARN,
                       "WARNING, unknown tag found: \"%s\"!\n",
                       child->name);;

        child = child->next;
    }

    if (init_GL(busnumber, __loopback->number_gl)) {
        __loopback->number_gl = 0;
        syslog_bus(busnumber, DBG_ERROR,
                   "Can't create array for locomotives");
    }

    if (init_GA(busnumber, __loopback->number_ga)) {
        __loopback->number_ga = 0;
        syslog_bus(busnumber, DBG_ERROR,
                   "Can't create array for accessoires");
    }

    if (init_FB(busnumber, __loopback->number_fb)) {
        __loopback->number_fb = 0;
        syslog_bus(busnumber, DBG_ERROR,
                   "Can't create array for feedback");
    }
    return (1);
}

static int init_lineLoopback(bus_t bus)
{
    return -1;
}

/**
 * cacheInitGL: modifies the gl data used to initialize the device
 **/
int init_gl_LOOPBACK(gl_data_t * gl)
{
    switch (gl->protocol) {
        case 'L':
        case 'S':              /*TODO: implement range checks */
            return (gl->n_fs == 31) ? SRCP_OK : SRCP_WRONGVALUE;
            break;
        case 'P':
            return SRCP_OK;
            break;
        case 'M':
            switch (gl->protocolversion) {
                case 1:
                    return (gl->n_fs == 14) ? SRCP_OK : SRCP_WRONGVALUE;
                    break;
                case 2:
                    return ((gl->n_fs == 14) ||
                            (gl->n_fs == 27) ||
                            (gl->n_fs == 28)) ? SRCP_OK : SRCP_WRONGVALUE;
                    break;
            }
            return SRCP_WRONGVALUE;
            break;
        case 'N':
            switch (gl->protocolversion) {
                case 1:
                    return ((gl->n_fs == 14) || (gl->n_fs == 28) ||
                            (gl->n_fs == 128)) ? SRCP_OK : SRCP_WRONGVALUE;
                    break;
                case 2:
                    return ((gl->n_fs == 14) || (gl->n_fs == 28) ||
                            (gl->n_fs == 128)) ? SRCP_OK : SRCP_WRONGVALUE;
                    break;
            }
            return SRCP_WRONGVALUE;
            break;
    }
    return SRCP_UNSUPPORTEDDEVICEPROTOCOL;
}

/**
 * initGA: modifies the ga data used to initialize the device
 **/
int init_ga_LOOPBACK(ga_data_t *ga)
{
    if ((ga->protocol == 'M') || (ga->protocol == 'N')
        || (ga->protocol == 'P') || (ga->protocol == 'S'))
        return SRCP_OK;
    return SRCP_UNSUPPORTEDDEVICEPROTOCOL;
}

/* Initialisiere den Bus, signalisiere Fehler
 * Einmal aufgerufen mit busnummer als einzigem Parameter
 * return code wird ignoriert (vorerst)
 */
int init_bus_LOOPBACK(bus_t i)
{
    static char *protocols = "LSPMN";

    buses[i].protocols = protocols;

    syslog_bus(i, DBG_INFO,
               "loopback start initialization (verbosity = %d).",
               buses[i].debuglevel);
    if (buses[i].debuglevel == 0) {
        syslog_bus(i, DBG_INFO, "loopback open device %s (not really!).",
                   buses[i].device.file.path);
        buses[i].device.file.fd = init_lineLoopback(i);
    }
    else {
        buses[i].device.file.fd = -1;
    }
    syslog_bus(i, DBG_INFO, "loopback initialization done.");
    return 0;
}


static void handle_power_command(bus_t bus)
{
    buses[bus].power_changed = 0;
    char msg[110];

    infoPower(bus, msg);
    enqueueInfoMessage(msg);
    buses[bus].watchdog++;
}


static void handle_sm_command(bus_t bus)
{
    struct _SM smtmp;
    /* registers 1-4 == CV#1-4; reg5 == CV#29; reg 7-8 == CV#7-8 */
    int reg6 = 1;
    static int cv[MAX_CV_NUMBER + 1];

    dequeueNextSM(bus, &smtmp);
    session_lock_wait(bus);
    smtmp.value &= 255;

    switch (smtmp.command) {
        case GET:
            smtmp.value = -1;
            switch (smtmp.type) {
                case REGISTER:
                    if ((smtmp.typeaddr > 0) && (smtmp.typeaddr < 9)) {
                        if (smtmp.typeaddr < 5 || smtmp.typeaddr > 6) {
                            smtmp.value = cv[smtmp.typeaddr];
                            if (smtmp.typeaddr < 5)
                                reg6 = 1;
                        }
                        else if (smtmp.typeaddr == 5) {
                            cv[29] = smtmp.value = cv[29];
                        }
                        else {
                            smtmp.value = reg6;
                        }
                    }
                    break;
                case PAGE:
                case CV:
                    if ((smtmp.typeaddr >= 0) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        smtmp.value = cv[smtmp.typeaddr];
                    }
                    break;
                case CV_BIT:
                    if ((smtmp.typeaddr >= 0) && (smtmp.bit >= 0)
                        && (smtmp.bit < 8) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        smtmp.value = (1 &
                                       (cv[smtmp.typeaddr] >> smtmp.bit));
                    }
                    break;
            }
            break;
        case SET:
            switch (smtmp.type) {
                case REGISTER:
                    if ((smtmp.typeaddr > 0) && (smtmp.typeaddr < 9)) {
                        if (smtmp.typeaddr < 5 || smtmp.typeaddr > 6) {
                            if (smtmp.typeaddr < 5)
                                reg6 = 1;
                            cv[smtmp.typeaddr] = smtmp.value;
                        }
                        else if (smtmp.typeaddr == 5) {
                            cv[29] = smtmp.value;
                        }
                        else {
                            reg6 = smtmp.value;
                        }
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
                case PAGE:
                case CV:
                    if ((smtmp.typeaddr >= 0) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        cv[smtmp.typeaddr] = smtmp.value;
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
                case CV_BIT:
                    if ((smtmp.typeaddr >= 0) && (smtmp.bit >= 0)
                        && (smtmp.bit < 8) && (smtmp.value >= 0) &&
                        (smtmp.value <= 1) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        if (smtmp.value) {
                            cv[smtmp.typeaddr] |= (1 << smtmp.bit);
                        }
                        else {
                            cv[smtmp.typeaddr] &= ~(1 << smtmp.bit);
                        }
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
            }
            setSM(bus, smtmp.type, smtmp.addr, smtmp.typeaddr, smtmp.bit, smtmp.value, 0);
            break;
        case VERIFY:
            switch (smtmp.type) {
                case REGISTER:
                    if ((smtmp.typeaddr > 0) && (smtmp.typeaddr < 9)) {
                        if (smtmp.typeaddr < 5 || smtmp.typeaddr > 6) {
                            if (smtmp.typeaddr < 5)
                                reg6 = 1;
                            if (smtmp.value != cv[smtmp.typeaddr])
                                smtmp.value = -1;
                        }
                        else if (smtmp.typeaddr == 5) {
                            if (cv[29] != smtmp.value)
                                smtmp.value = -1;
                        }
                        else {
                            if (reg6 != smtmp.value)
                                smtmp.value = -1;
                        }
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
                case PAGE:
                case CV:
                    if ((smtmp.typeaddr >= 0) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        if (smtmp.value != cv[smtmp.typeaddr])
                            smtmp.value = -1;
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
                case CV_BIT:
                    if ((smtmp.typeaddr >= 0) && (smtmp.bit >= 0)
                        && (smtmp.bit < 8) &&
                        (smtmp.typeaddr <= MAX_CV_NUMBER)) {
                        if (smtmp.value != (1 &
                                            (cv[smtmp.typeaddr] >>
                                             smtmp.bit)))
                            smtmp.value = -1;
                    }
                    else {
                        smtmp.value = -1;
                    }
                    break;
            }
            break;
    }
    session_endwait(bus, smtmp.value);

    buses[bus].watchdog++;
}


static void handle_gl_command(bus_t bus)
{
    gl_data_t gltmp, glakt;
    int addr;

    dequeueNextGL(bus, &gltmp);
    addr = gltmp.id;
    cacheGetGL(bus, addr, &glakt);

    if (gltmp.direction == 2) {
        gltmp.speed = 0;
        gltmp.direction = glakt.direction;
    }
    cacheSetGL(bus, addr, gltmp);
    buses[bus].watchdog++;
}


static void handle_ga_command(bus_t busnumber)
{
    ga_data_t gatmp;
    int addr, i;
    struct timeval akt_time;

    dequeueNextGA(busnumber, &gatmp);
    addr = gatmp.id;

    gettimeofday(&akt_time, NULL);
    gatmp.tv[gatmp.port] = akt_time;
    setGA(busnumber, addr, gatmp);

    if (gatmp.action && (gatmp.activetime > 0)) {
        for (i = 0; i < 50; i++) {
            if (__loopback->tga[i].id == 0) {
                gatmp.t = akt_time;
                gatmp.t.tv_sec += gatmp.activetime / 1000;
                gatmp.t.tv_usec += (gatmp.activetime % 1000) * 1000;
                if (gatmp.t.tv_usec > 1000000) {
                    gatmp.t.tv_sec++;
                    gatmp.t.tv_usec -= 1000000;
                }
                __loopback->tga[i] = gatmp;
                break;
            }
        }
    }
    buses[busnumber].watchdog++;
}


/*thread cleanup routine for this bus*/
static void end_bus_thread(bus_thread_t * btd)
{
    int result;

    syslog_bus(btd->bus, DBG_INFO, "Loopback bus terminated.");

    result = pthread_mutex_destroy(&buses[btd->bus].transmit_mutex);
    if (result != 0) {
        syslog_bus(btd->bus, DBG_WARN,
                   "pthread_mutex_destroy() failed: %s (errno = %d).",
                   strerror(result), result);
    }

    result = pthread_cond_destroy(&buses[btd->bus].transmit_cond);
    if (result != 0) {
        syslog_bus(btd->bus, DBG_WARN,
                   "pthread_mutex_init() failed: %s (errno = %d).",
                   strerror(result), result);
    }

    free(buses[btd->bus].driverdata);
    free(btd);
}

/*main thread routine for this bus*/
void *thr_sendrec_LOOPBACK(void *v)
{
    int addr, ctr;
    struct timeval akt_time, cmp_time;
    ga_data_t gatmp;
    int last_cancel_state, last_cancel_type;

    bus_thread_t *btd = (bus_thread_t *) malloc(sizeof(bus_thread_t));

    if (btd == NULL)
        pthread_exit((void *) 1);
    btd->bus = (bus_t) v;
    btd->fd = -1;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_cancel_state);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_cancel_type);

    /*register cleanup routine */
    pthread_cleanup_push((void *) end_bus_thread, (void *) btd);

    /* initialize tga-structure */
    for (ctr = 0; ctr < 50; ctr++)
        __loopbackt->tga[ctr].id = 0;

    syslog_bus(btd->bus, DBG_INFO, "Loopback bus started (device = %s).",
               buses[btd->bus].device.file.path);

    /*enter endless loop to process work tasks */
    while (true) {

        buses[btd->bus].watchdog = 1;

        /*POWER action arrived */
        if (buses[btd->bus].power_changed == 1)
            handle_power_command(btd->bus);

        /*SM action arrived */
        if (!queue_SM_isempty(btd->bus))
            handle_sm_command(btd->bus);

        /* loop shortcut to prevent processing of GA, GL (and FB)
         * without power on; arriving commands will flood the command
         * queue */
        if (buses[btd->bus].power_state == 0) {

            /* wait 1 ms */
            if (usleep(1000) == -1) {
                syslog_bus(btd->bus, DBG_ERROR,
                           "usleep() failed: %s (errno = %d)\n",
                           strerror(errno), errno);
            }
            continue;
        }

        /*GL action arrived */
        if (!queue_GL_isempty(btd->bus))
            handle_gl_command(btd->bus);


        /* handle delayed switching of GAs (there is a better place) */
        gettimeofday(&akt_time, NULL);
        for (ctr = 0; ctr < 50; ctr++) {
            if (__loopbackt->tga[ctr].id > 0) {
                cmp_time = __loopbackt->tga[ctr].t;

                /* switch off time reached? */
                if (cmpTime(&cmp_time, &akt_time)) {
                    gatmp = __loopbackt->tga[ctr];
                    addr = gatmp.id;
                    gatmp.action = 0;
                    setGA(btd->bus, addr, gatmp);
                    __loopbackt->tga[ctr].id = 0;
                }
            }
        }

        /*GA action arrived */
        if (!queue_GA_isempty(btd->bus))
            handle_ga_command(btd->bus);

        /*FB action arrived */
        /* currently nothing to do here */
        buses[btd->bus].watchdog++;

        /* busy wait and continue loop */
        /* wait 1 ms */
        if (usleep(1000) == -1) {
            syslog_bus(btd->bus, DBG_ERROR,
                       "usleep() failed: %s (errno = %d)\n",
                       strerror(errno), errno);
        }
    }

    /*run the cleanup routine */
    pthread_cleanup_pop(1);
    return NULL;
}
