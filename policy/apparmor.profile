#include <tunables/global>

/srv/agentlab/hw3/bin/legacy_log_parser {
  #include <abstractions/base>

  /srv/agentlab/hw3/bin/legacy_log_parser mr,

  deny network,
  deny capability,

  /srv/agentlab/hw3/inbox/    r,
  /srv/agentlab/hw3/inbox/*   r,

  /srv/agentlab/hw3/markers/   r,
  /srv/agentlab/hw3/markers/*  rw,

  audit deny /opt/agentsec/decoy.txt rwklx,

  audit deny /bin/**      x,
  audit deny /usr/bin/**  x,
  audit deny /sbin/**     x,
  audit deny /usr/sbin/** x,
}
