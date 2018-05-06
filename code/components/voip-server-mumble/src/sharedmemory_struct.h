#include <stdint.h>
#include "types.h"

typedef struct
{
  char username[121];
  char ipaddress[INET6_ADDRSTRLEN];
  char channel[121];
  char os[121], release[121], os_version[121];
  int tcp_port, udp_port;
  bool_t bUDP, authenticated, deaf, mute, self_deaf, self_mute, recording, bOpus;
  int availableBandwidth;
  uint32_t online_secs, idle_secs;
  uint8_t hash[20];
  bool_t isAdmin;
  bool_t isSuppressed;
  float UDPPingAvg, UDPPingVar, TCPPingAvg, TCPPingVar;
  uint32_t UDPPackets, TCPPackets;
} shmclient_t;

typedef struct
{
  int shmtotal_size, shmclient_size;
  int clientcount, server_max_clients;
  unsigned int umurmurd_pid;
  uint8_t alive;
  shmclient_t client[];
} shm_t;
