#include "NextionX.h"

#include <stdio.h>
#include "pico/stdlib.h"

NexComp::NexComp(NexComm &nexComm, uint8_t pageId, uint8_t objId) : _nexComm(&nexComm)
{
	this->_myId.page = pageId;
	this->_myId.obj = objId;
}

void NexComp::setAttr(const char *attr, int32_t num) {
	char cmd[255] ;
	sprintf(cmd, "p[%d].b[%d].%s=%d", this->_myId.page, this->_myId.obj, attr, num) ;
	this->_nexComm->cmdWrite(cmd) ;
}

void NexComp::setAttr(const char *attr, const char *txt) {
	char cmd[255] ;
	sprintf(cmd, "p[%d].b[%d].%s=\"%s\"", this->_myId.page, this->_myId.obj, attr, txt) ;
	this->_nexComm->cmdWrite(cmd) ;
}

void NexComp::setVisible(bool show) {
	char cmd[255] ;
	sprintf(cmd, "vis %d,%d", this->_myId.obj, show) ;
	this->_nexComm->cmdWrite(cmd) ;
}

uint16_t NexComp::getGuid()
{
	return this->_myId.guid;
}

void NexComp::setOnTouch(void (*onPress)(void*, void*), void *arg1, void *arg2) {
	this->_onPrs = onPress;
	this->onPrsArg1 = arg1 ;
	this->onPrsArg2 = arg2 ;
	this->_nexComm->_addCmpList(this);
}

void NexComp::setOnRelease(void (*onRelease)() = nullptr)
{
	this->_onRel = onRelease;
	this->_nexComm->_addCmpList(this);
}
void NexComp::callBack(uint8_t evt)
{
	switch (evt)
	{
	case 0:
		if (this->_onRel != nullptr)
		{
			this->_onRel();
		}
		break;
	case 1:
		if (this->_onPrs != nullptr)
		{
			(this->_onPrs(this->onPrsArg1, this->onPrsArg2));
		}
		break;
	}
}


NexComm::NexComm() {}
void NexComm::begin(uart_inst_t *nexSer, uint32_t nexBaud) {
	uart_set_baudrate(nexSer, nexBaud) ;
	this->_nexSer = nexSer;
	this->cmdWrite("");
	this->cmdWrite("bkcmd=0");
}

void NexComm::begin(uart_inst_t *nexSer) {
	uart_set_baudrate(nexSer, 9600) ;
	this->_nexSer = nexSer;
	this->cmdWrite("");
	this->cmdWrite("bkcmd=0");
}

void NexComm::addDebug(uart_inst_t *dbgSer) {
	this->_dbgSer = dbgSer;
	this->cmdWrite("bkcmd=3");
}

void NexComm::cmdWrite(const char *cmd) {
	this->loop() ; //handle incoming stuff first
	uart_puts(this->_nexSer, cmd) ;
	uart_puts(this->_nexSer, "\xFF\xFF\xFF") ;
	
	if ((this->_dbgSer != nullptr) && (strlen(cmd) > 0)) {
		uart_puts(this->_dbgSer, cmd) ;
		uart_puts(this->_dbgSer, "\r\n") ;
	}
}

void NexComm::loop()
{
	CompId trCmp;
	this->_rLen = this->_readNextRtn();
	if ((this->_rLen > 0) && (this->_dbgSer != nullptr))
	{
		this->_dbgLoop();
	}
	if ((this->_rLen == 4) && (this->_inStr[0] == 0x65))
	{
		trCmp.page = this->_inStr[1];
		trCmp.obj = this->_inStr[2];
		uint8_t listpos = this->_fIdxByGuid(trCmp.guid);
		if (listpos < MAX_LS_LST_LEN)
		{
			this->_lsLst[listpos].comp->callBack(this->_inStr[3]);
		}
	}
}
void NexComm::_addCmpList(NexComp *cmp)
{
	ListEl mycmp;
	mycmp.comp = cmp;
	mycmp.guid = cmp->getGuid();
	if (this->_fIdxByGuid(mycmp.guid) == 0xFF)
	{
		this->_lsLst[this->_lsPtr++] = mycmp;
		if (this->_lsPtr >= MAX_LS_LST_LEN)
		{
			this->_lsPtr = 0;
		}
	}
}

void NexComm::_dbgLoop() {
	char retStr[255] ;
	
	if ((this->_rLen == 1) && ((this->_inStr[0] < 0x25) || (this->_inStr[0] > 0x85))) {
		if (this->_inStr[0] == 1) {
			uart_puts(this->_dbgSer, "Success\r\n") ;
		} else if (this->_inStr[0] < 0x25) {
			sprintf(retStr, "Error %x\r\n", this->_inStr[0]) ;
			uart_puts(this->_dbgSer, retStr) ;
		} else if (this->_inStr[0] > 0x85) {
			sprintf(retStr, "Status %x\r\n", this->_inStr[0]) ;
			uart_puts(this->_dbgSer, retStr) ;
		}
	} else if ((this->_rLen == 4) && (this->_inStr[0] == 0x65)) {
		sprintf(retStr, "%s page %d obj %d\r\n", this->_inStr[3] == 1 ? "Press" : "Release", this->_inStr[1], this->_inStr[2]) ;
		uart_puts(this->_dbgSer, retStr) ;
	}
}

uint8_t NexComm::_readNextRtn() {
	uint8_t msgLen = 0;
	while (uart_is_readable(this->_nexSer) && this->_ffCnt < 3) {
		uint8_t _inByte = uart_getc(this->_nexSer) ;
		this->_inStr[this->_inPtr++] = _inByte ;
		if (_inByte == 255) {
			this->_ffCnt++ ;
		} else {
			this->_ffCnt = 0 ;
			if (this->_inPtr > (MAX_IN_BUF_LEN - 3)) {
				this->_inPtr = MAX_IN_BUF_LEN - 3 ;
			}
		}
	}
	if (this->_ffCnt == 3) {
		msgLen = this->_inPtr - 3 ;
		this->_ffCnt = 0 ;
		this->_inPtr = 0 ;
	}
	return msgLen ;
}

uint8_t NexComm::_fIdxByGuid(uint16_t sguid)
{
	uint8_t retval = 0xFF;
	for (uint8_t i = 0; i < MAX_LS_LST_LEN; i++)
	{
		if (this->_lsLst[i].guid == sguid)
		{
			retval = i;
			break;
		}
	}
	return retval;
}
