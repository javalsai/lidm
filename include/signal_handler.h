#ifndef SIGNALHANDLERH_
#define SIGNALHANDLERH_

// handle SIGTERM by sending SIGTERM to all children, resulting
// in a graceful graphical shutdown
void setup_sigterm();

#endif /* SIGNALHANDLERH_ */
