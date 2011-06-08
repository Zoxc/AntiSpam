#include "GameModule.hpp"
#include <sstream>
#include <fstream>
#include <algorithm>

extern RedVexInfo *Funcs;

Bayes bayes;

Player::Player(AntiSpamGameModule &module, const std::string &name, const std::string &account) : module(module), name(name), account(account), joined(GetTickCount()), state(Joined), display_quit_message(false)
{
}

Player::~Player()
{
	for(auto i = events.begin(); i != events.end(); ++i)
		delete *i;
}

size_t Player::elapsed_since_join()
{
	return GetTickCount() - joined;
}

size_t Player::elapsed_since_message()
{
	return GetTickCount() - messaged;
}

void Player::flush()
{
	for(auto i = events.begin(); i != events.end(); ++i)
	{
		module.proxy->RelayDataToClient(*i, &module);
		delete *i;
	}

	events.clear();
}

void Player::evaluate()
{
	if(bayes.Categorize(log) == -1)
	{
		state = Spaming;
	}
	else
	{
		state = Playing;
		flush();
	}
}

void Player::queue(IPacket *packet)
{
	if(state != Player::Playing)
	{
		events.push_back(packet->Clone());

		packet->SetFlag(IPacket::PacketFlag_Dead);
	}
}

AntiSpamGameModule::AntiSpamGameModule(IProxy *proxy) :	proxy(proxy)
{
}

AntiSpamGameModule::~AntiSpamGameModule()
{
	for(auto i = players.begin(); i != players.end(); ++i)
		delete *i;
}

void __stdcall AntiSpamGameModule::Destroy()
{
	delete this;
}

void AntiSpamGameModule::OnRelayDataToServer(IPacket *packet, const IModule *owner)
{
	const unsigned char* bytes = static_cast<const unsigned char*>(packet->GetData());
	int packetId = bytes[1];

}

Player *AntiSpamGameModule::find(const std::string &name)
{
	for(auto i = players.begin(); i != players.end(); ++i)
	{
		Player *player = *i;

		if(_strcmpi(player->name.c_str(), name.c_str()) == 0)
			return player;
	}

	return nullptr;
}

void AntiSpamGameModule::OnRelayDataToClient(IPacket *packet, const IModule *owner)
{
	const char* bytes = static_cast<const char*>(packet->GetData());
	int packetId = bytes[0];

	if(owner == this)
		return;

	switch(packetId)
	{
		case 0x26:
		{
			if(bytes[1] == 1)
			{
				std::string name = &bytes[10];

				Player *player = find(name);

				if(player)
				{
					std::string message = &bytes[name.length() + 11];
					
					if(player->state != Player::Playing)
					{
						player->queue(packet);

						if(player->state == Player::Joined)
						{
							player->state = Player::Messaged;
							player->messaged = GetTickCount();
						}

						if(player->state == Player::Messaged)
							player->log += message + "\n";
					}
				}
			}
		}
		break;

		case 0x5A:
		{
			switch(bytes[1])
			{
				case 0:
				case 2:
				case 3:
				{
					std::string name = &bytes[8];
					std::string account = &bytes[24];

					Player *player = find(name);

					if(player)
						player->account = account;

					if(bytes[1] == 2)
					{
						// A player is joining

						if(!player)
						{
							player = new Player(*this, name, account);

							players.push_back(player);
						}
						
						player->queue(packet);
					}
					else if(player)
					{
						std::stringstream state;
						state << player->state;
						
						// A player is leaving
						
						if(player->state == Player::Messaged)
							player->evaluate();

						if(player->state == Player::Spaming)
						{
							if(!player->display_quit_message)
								packet->SetFlag(IPacket::PacketFlag_Dead);
						}
						else
							player->flush();

						players.erase(std::remove(players.begin(), players.end(), player), players.end());
						delete player;
					}
				}
				break;

				default:
					break;
			}
		}
		break;

		case 0x5B:
		{
			std::string name = &bytes[8];

			Player *player = find(name);
			
			if(!player)
			{
				player = new Player(*this, name, "");

				players.push_back(player);
			}
		}
		break;

		default:
			break;
	}
}

void AntiSpamGameModule::Update()
{
	for(auto i = players.begin(); i != players.end(); ++i)
	{
		Player *player = *i;

		switch(player->state)
		{
			case Player::Joined:
			{
				if(player->elapsed_since_join() > 45000)
				{
					player->flush();
					
					player->state = Player::Playing;
				}
				else if((player->elapsed_since_join() > 8000) && !player->display_quit_message)
				{
					player->flush();
					player->display_quit_message = true;
				}
			}
			break;

			case Player::Messaged:
			{
				if(player->elapsed_since_message() > 3000)
					player->evaluate();
			}
			break;

			case Player::Spaming:
			{
				if(player->elapsed_since_join() > 45000)
				{
					player->flush();

					chat("ÿc1AntiSpamÿc ", "ÿc4" + player->name + " was mistaken for a spambot.");

					player->state = Player::Playing;
				}
			}
			break;

			default:
				break;
		}
	}
}

void AntiSpamGameModule::chat(const std::string &player, const std::string &message)
{
	std::string data = std::string("\x26\x01\x00\x02\x00\x00\x00\x00\x00\x05", 10) + player + std::string("\x00", 1) + message + std::string("\x00", 1);

	IPacket *packet = proxy->CreatePacket(data.data(), data.length());
	packet->SetFlag(IPacket::PacketFlag_Hidden);
	proxy->RelayDataToClient(packet, this);

	delete packet;
}
