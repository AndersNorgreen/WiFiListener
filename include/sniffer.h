#ifndef SNIFFER_H
#define SNIFFER_H

void setUpEspWiFi();
void setChannel(int channel);
void bumpChannel();
void setUpdateDelay(int delay);
void scan();
void cycleChannelsScan();

#endif // SNIFFER_H
