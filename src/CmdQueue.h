// src/CmdQueue.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2016 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#include "SoundInfo.h"


enum class CommandType
{
	play_file,
	stop_playback,
};

class Command
{
  public:
	CommandType getType() const
	{
		return type;
	}

  private:
	CommandType type;
};


class CmdQueue
{
  public:
	CmdQueue();
	~CmdQueue();

	void enqueue(std::unique_ptr<Command> cmd);
	void startWorker();
	void stopWorker(bool wait);

  private:
	void threadFunc();

	std::queue<std::unique_ptr<Command>> cmds;
	std::mutex cmdsMutex;
	std::thread thread;

	std::atomic_bool stop;
	std::atomic_bool running;
};
