// Implementation of "StreamClientState":
#pragma once
#include "stdafx.h"
#include "StreamClientState.h"
#include "live.h"

/**
Stream client constructor.
Clear out of public propertices
*/
StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

/**
Stream client destructor.
Clean up the env
*/
StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}