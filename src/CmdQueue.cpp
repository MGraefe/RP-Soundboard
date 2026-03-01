// src/CmdQueue.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2016 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "CmdQueue.h"

using Lock = std::lock_guard<std::mutex>;

CmdQueue::CmdQueue() :
	stop(false)
{
}


CmdQueue::~CmdQueue() {}


void CmdQueue::enqueue(std::unique_ptr<Command> cmd)
{
	Lock lock(cmdsMutex);
	cmds.push(std::move(cmd));
}


void CmdQueue::startWorker()
{
	if (running)
		return;

	running = true;
	stop = false;
	std::thread t(&CmdQueue::threadFunc, this);
	thread = std::move(t);
}


void CmdQueue::stopWorker(bool wait)
{
	stop = true;
	if (wait && thread.joinable())
		thread.join();
}


void CmdQueue::threadFunc()
{
	while (!stop)
	{
		cmdsMutex.lock();
		while (!cmds.empty())
		{
			// TODO
		}
	}
}
