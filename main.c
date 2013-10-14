#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

/* local objects */
#include "config.h"
#include "aldl-io/config.h"
#include "aldl-io/aldl-io.h"
#include "configfile/varstore.h"
#include "configfile/configfile.h"

/* plugins */
#include "debugif/debugif.h"

/* ------- GLOBAL----------------------- */

aldl_conf_t *aldl; /* aldl data structure */
aldl_commdef_t *comm; /* comm specs */
char *config_file; /* path to config file */

/* ------- LOCAL FUNCTIONS ------------- */

/* run main aldl aquisition event loop */
int aldl_acq();

/* run cleanup rountines for aldl and serial crap */
int aldl_finish();

/* allocate all base data structures */
int aldl_alloc();

/* load all base config data, but no packet definitions. */
int load_config_a(char *filename);

/* load all packet definitions, and allocate as needed */
int load_config_b(char *filename);

#ifdef PREPRODUCTION
/* debug-only fallback config */
void fallback_config();

/* placeholder error handler */
void tmperror(char *str);
#endif

int main() {
  /* ------- SETUP AND LOAD CONFIG -------------------*/

  aldl_alloc(); /* perform initial allocations */

  aldl->state = ALDL_CONNECTING; /* initial connection state */

  load_config_a("/project/lt1.conf"); /* load 1st stage config */

  fallback_config(); /* REMOVE ME */

  load_config_b("/project/lt1.conf"); /* allocate and load stage b conf */

  /* FIXME this needs to come from load_config or switch to autodetct */
  char *serialport = "d:002/002";

  serial_init(serialport); /* init serial port */

  /* ------- EVENT LOOP STUFF ------------------------*/

  aldl_acq(); /* start main event loop */

  /* ------- CLEANUP ----------------------------------*/

  aldl_finish(comm);

  return 0;
}

int aldl_alloc() {
#ifdef VERBLOSITY
  printf("performing initial allocation\n");
#endif
  aldl = malloc(sizeof(aldl_conf_t));
  if(aldl == NULL) tmperror("out of memory 1055"); /* FIXME */
  comm = malloc(sizeof(aldl_commdef_t));
  if(comm == NULL) tmperror("out of memory 1055"); /* FIXME */
  aldl->comm = comm; /* link */
  return 0;
}

int load_config_a(char *filename) {
#ifdef VERBLOSITY
  printf("load stage a config\n");
#endif

  return 0;
}

int load_config_b(char *filename) {
#ifdef VERBLOSITY
  printf("load stage b config\n");
#endif
  /* allocate space to store packet definitions */
  comm->packet = malloc(sizeof(aldl_packetdef_t) * comm->n_packets);
  if(comm->packet == NULL) tmperror("out of memory 1055"); /* FIXME */

  /* !! get packet definitions here, or this flunks due to missing length */
  /* a placeholder packet, lt1 msg 0 */
  comm->packet[0].length = 64;
  comm->packet[0].enable = 1;
  comm->packet[0].id = 0x00;
  comm->packet[0].msg_len = 0x57;
  comm->packet[0].msg_mode = 0x01;
  comm->packet[0].commandlength = 5;
  comm->packet[0].offset = 3;
  comm->packet[0].timer = 50;
  generate_pktcommand(&comm->packet[0],comm);

  int x = 0;
  for(x=0;x<comm->n_packets;x++) { /* allocate data storage */
    comm->packet[x].data = malloc(comm->packet[x].length);
    if(comm->packet[x].data == NULL) tmperror("out of memory 1055"); /* FIXME */
  };

  aldl->def = malloc(sizeof(aldl_define_t) * aldl->n);
  if(aldl->def == NULL) tmperror("out of memory 1055"); /* FIXME */
  /* get data definitions here !! */

  /* allocate space for records */
  return 0;
}

int aldl_acq() {
  /* this is a broken routine that should be used for testing only. */
  int npkt = 0;
  aldl_packetdef_t *pkt = NULL;
  aldl_reconnect(comm); /* this shouldn't return without a connection .. */
  aldl->state = ALDL_CONNECTED;
  printf("connection successful?... !\n");
  while(1) {
  for(npkt=0;npkt < comm->n_packets;npkt++) {
    printf("acquiring packet %i\n",npkt);
    pkt = &comm->packet[npkt];
    aldl_get_packet(pkt);
  };
  debugif_iterate(aldl);
  };
  return 0;
}

int aldl_finish() {
  serial_close();
  return 0;
}

void tmperror(char *str) {
  printf("FATAL ERROR: %s",str);
  exit(1);
}

#ifdef PREPRODUCTION
void fallback_config() {
  sprintf(comm->ecmstring, "EE");
  comm->checksum_enable = 1;
  comm->pcm_address = 0xF4;
  comm->idletraffic = 0x00;
  comm->idledelay = 10;
  comm->chatterwait = 1;
  comm->shutupcommand = generate_shutup(0x56,0x08,comm);
  comm->shutupfailwait = 500;
  comm->shutupcharlimit = 20;
  comm->shutuprepeat = 3;
  comm->shutuprepeatdelay = 75;
  comm->n_packets = 1;
}
#endif

