#ifndef __GPLUGIN_H
#define __GPLUGIN_H

class gPlugin : public gControl
{
public:
	gPlugin(gContainer *parent);
	void plug(int id, bool prepared = false);
	void discard();
//"Properties"
	int client();

	int getBorder() { return getFrameBorder(); }
	void setBorder(int vl) { setFrameBorder(vl); }

//"Signals"
	void (*onPlug)(gControl *sender);
	void (*onUnplug)(gControl *sender);
	void (*onError)(gControl *sender);
};

#endif
