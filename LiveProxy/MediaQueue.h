#pragma once

/**
The type, size and timestamp of the frame
In live mode time is always 0
*/
typedef struct __FrameHeader
{
	long TimeStamp;
	long FrameType;
	long FrameLen;
}FrameHeader;

/**
The frame header and the buffer
this is what we queue up
*/
typedef struct __FrameInfo
{
    FrameHeader frameHead;
	char* pdata;
}FrameInfo;


/**
This is the queue of video frames
*/
typedef struct __MediaQueue
{
	FrameInfo *frame;
	struct __MediaQueue *next;
}MediaQueue;


/**
This class manages the queue of of video frames
*/
class CMediaQueue
{
	MediaQueue *head, *tail;
	MediaQueue *writepos, *readpos;
	MediaQueue *ptr, *pstatic;
	HANDLE		hFrameListLock;
	HANDLE  	hRecvEvent;
	int			count;
	int			size;

public:
	CMediaQueue(int queueSize);
	~CMediaQueue();

	void put(FrameInfo* frame);
	FrameInfo* get();
	void reset();

public:
	/**
	The capacity of the queue
	*/
	int get_Size()
	{		
		return(size);
	}

	/**
	The number of frames in the queue
	*/
	int get_Count()
	{		
		return(count);
	}

	/**
	Helper to see if queue is empty
	*/
	int get_isEmpty()
	{		
		return(count<=0?1:0);
	}

	/**
	remove all items from the queue
	*/
	int empty()
	{		
		SetEvent(hRecvEvent);
		return(count=0);
	}
};
