#ifndef RXPROG_H
#define RXPROG_H

bool RX62NConnect();
void RX62NDisconnect();

BOOL RX62NQueryBlankCheck();
bool RX62NEraseUserMat();
bool RX62NWriteUserMat(const char *filename);
bool RX62NVerifyUserMat(const char *filename);

#endif

