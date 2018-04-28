#pragma once
#include "rack.hpp"
using namespace rack;

struct TorpedoPort {
	enum States {
		STATE_QUIESCENT,
		STATE_HEADER,
		STATE_BODY,
		STATE_TRAILER
	};

	Port *_port;
	int _state;
	TorpedoPort(Port * port) {
		_port = port;
		_state = STATE_QUIESCENT;
	}
	int isBusy(void) {
		return (_state != STATE_QUIESCENT);
	}
	
};

struct TorpedoOutputPort : TorpedoPort {
	void abort();
	void send(char *appId, char *message, int messageLength);
	void process();
	virtual void completed();
	TorpedoOutputPort(Port *outPort) : TorpedoPort(outPort) { }
	char *_message;
	int _counter;
};

struct TorpedoInputPort : TorpedoPort {
	void process();
	virtual void received(char *appId, char *message);
	virtual void error();
	TorpedoInputPort(Port *inPort) : TorpedoPort(inPort) { }
	char *_message;
	int _counter;
};
	

