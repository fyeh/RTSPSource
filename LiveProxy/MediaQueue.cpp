#include "stdafx.h"
#include "MediaQueue.h"
#include "trace.h"


/**
The queue of video frames.

Initalize the queue with the specified size
\param queueSize The size of the video queue
*/
CMediaQueue::CMediaQueue(int queueSize)
{
	count = 0;
	size=queueSize;
	head = tail = NULL;
	ptr = pstatic = (MediaQueue *)malloc(sizeof(MediaQueue)*queueSize);
	ZeroMemory(pstatic, sizeof(MediaQueue)*queueSize);
	head = ptr;
	readpos = writepos = head;
	for (int i = 1; i < queueSize; i++)
	{
		ptr->next = pstatic+i;
		ptr = ptr->next;
	}

	tail = ptr;
	tail->next =  head;
	ptr = head;
	hFrameListLock = CreateMutex(NULL,FALSE,NULL);
	hRecvEvent     = CreateEvent(NULL, TRUE, FALSE, NULL);
}

/**
Cleanup the queue
*/
CMediaQueue::~CMediaQueue()
{
	CloseHandle(hRecvEvent);
	CloseHandle(hFrameListLock);
		
	//empty the queue	
	while (count >= 0)
	{
		//remove a frame so we can add one			
		count--;
		if (readpos!=NULL && readpos->frame != NULL)
		{
			free(readpos->frame);
			readpos->frame = NULL;
		}
		readpos = readpos->next;
	}
	free(pstatic);

}


/**
Add a new video frame to the queue
*/
void CMediaQueue::put(FrameInfo* frame)
{	  
	if(ptr == NULL)
		return; 

	WaitForSingleObject(hFrameListLock,INFINITE);
	if (count >= size)
	{
		//remove a frame so we can add one			
		count = size-1;
		if (readpos->frame)
		{
			free(readpos->frame);
			readpos->frame = NULL;
		}
		readpos = readpos->next;
	}

	//add the frame
	writepos->frame = frame;
	writepos =  writepos->next;
	count++;

	if (count <=1)
	{
		SetEvent(hRecvEvent);
	}
	ReleaseMutex(hFrameListLock); 		
}

/**
Remove a video frame from the queue
*/
FrameInfo* CMediaQueue::get()
{
	FrameInfo* frame = NULL;
		
	if (count < 1)
	{
		TRACE_WARN("No frames in queue, waiting");
		WaitForSingleObject(hRecvEvent, 500);
	}

	ResetEvent(hRecvEvent);
		
	WaitForSingleObject(hFrameListLock,INFINITE);
	if(count > 0)
	{			 
		frame = readpos->frame;
		readpos->frame =  NULL;
		readpos = readpos->next;  	
		count--;			
	}else{
		TRACE_ERROR("No frames in queue");
	}

	ReleaseMutex(hFrameListLock); 
	return(frame);
}

/**
Empty the queue
*/
void CMediaQueue::reset()
{
	WaitForSingleObject(hFrameListLock,INFINITE);
	ptr = readpos;
	while (ptr->frame)
	{
		free(ptr->frame);
		ptr->frame = NULL;
		ptr = ptr->next;
	}		
	writepos = readpos;
	count  = 0;
	ReleaseMutex(hFrameListLock); 

}