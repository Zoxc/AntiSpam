#pragma once
#include <list>
#include "RedVex.hpp"
#include "Bayes.h"

class AntiSpamGameModule;

class Player
{
public:
	enum State
	{
		Joined,
		Messaged,
		Spaming,
		Playing
	};

	AntiSpamGameModule &module;

	std::string name;
	std::string account;
	size_t joined;
	size_t messaged;

	std::string log;

	State state;
	bool display_quit_message;

	std::list<IPacket *> events;

	Player(AntiSpamGameModule &module, const std::string &name, const std::string &account);
	~Player();
	
	void flush();
	void evaluate();
	void queue(IPacket *packet);

	size_t elapsed_since_join();
	size_t elapsed_since_message();
};

class AntiSpamGameModule:
	public IModule
{
public:
	AntiSpamGameModule(IProxy* proxy);
	~AntiSpamGameModule();

	void __stdcall Destroy();
	void _stdcall OnRelayDataToServer(IPacket *packet, const IModule *owner);
	void _stdcall OnRelayDataToClient(IPacket *packet, const IModule *owner);
	void _stdcall Update();

	IProxy *proxy;
	std::vector<Player *> players;
	
	Player *find(const std::string &name);
	void chat(const std::string &player, const std::string &message);
};

extern Bayes bayes;
