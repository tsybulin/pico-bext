#pragma once

#define MAX_IN_BUF_LEN 14
#define MAX_LS_LST_LEN 16

#include <string.h>
#include "hardware/uart.h"

/*
 * CompId declaration
 */

typedef union CompId_t
{
	uint16_t guid;
	struct
	{
		uint8_t page;
		uint8_t obj;
	};
} CompId;

/*
 * Forward declaration (needed for the reference in NexComp)
 */
class NexComm;

/* 
 * NexComp declaration
 */
class NexComp
{
public:
	NexComp(NexComm&, uint8_t, uint8_t);
	void setAttr(const char *, int32_t);
	void setAttr(const char *, const char*) ;
	void setVisible(bool show) ;
	uint16_t getGuid();
	void setOnTouch(void (*)(void*, void*), void*, void*);
	void setOnRelease(void (*)());
	void callBack(uint8_t);

private:
	NexComm *_nexComm;
	CompId _myId;
	void (*_onPrs)(void*, void*) = nullptr;
	void *onPrsArg1 = nullptr ;
	void *onPrsArg2 = nullptr ;
	void (*_onRel)() = nullptr;
};

/*
 * ListEl declaration
 */
typedef struct ListEl_t
{
	uint16_t guid;
	NexComp *comp;
} ListEl;

/* 
 * NexComm declaration
 */
class NexComm {
public:
	NexComm();
	void begin(uart_inst_t *);
	void begin(uart_inst_t *, uint32_t);

	void addDebug(uart_inst_t *);
	void cmdWrite(const char *);
	void loop();

protected:
	void _addCmpList(NexComp *);
	uint8_t _inStr[MAX_IN_BUF_LEN] = {0};
	uint8_t _rLen = 0;
	uart_inst_t *_nexSer = nullptr;
	uart_inst_t *_dbgSer = nullptr;
	char *_tag;

private:
	void _nexInit();
	void _dbgInit();
	uint8_t _readNextRtn();
	uint8_t _fIdxByGuid(uint16_t);
	void _dbgLoop();
	uint8_t _inPtr = 0;
	uint8_t _ffCnt = 0;
	uint8_t _lsPtr = 0;
	ListEl _lsLst[MAX_LS_LST_LEN];

friend NexComp;
};
