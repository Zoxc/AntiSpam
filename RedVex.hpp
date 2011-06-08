#pragma once
#include <windows.h>

class IPacket
{
public:
	enum PacketFlag
	{
		PacketFlag_Dead,
		PacketFlag_Finalized,
		PacketFlag_Hidden,
		PacketFlag_Virtual
	};

	virtual void __stdcall Destroy() = 0;
	virtual void __stdcall SetData(const void* data, int size) = 0;
	virtual void __stdcall ClearData() = 0;
	virtual int __stdcall GetSize() const = 0;
	virtual const void* __stdcall GetData() const = 0;
	virtual IPacket* __stdcall Clone() const = 0;
	virtual bool __stdcall IsFlagSet(PacketFlag flag) const = 0;
	virtual void __stdcall SetFlag(PacketFlag flag) = 0;
	virtual void __stdcall ClearFlag(PacketFlag flag) = 0;
	virtual ~IPacket();
};

enum ModuleKind
{
	RealmModule,
	ChatModule,
	GameModule
};

class IModule
{
public:
	virtual void __stdcall Destroy() = 0;
	virtual void __stdcall OnRelayDataToServer(IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall OnRelayDataToClient(IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall Update() = 0;
	virtual ~IModule() {}
};

class IPacket;
class IModule;

class IProxy
{
public:
	virtual void __stdcall RelayDataToServer(const IPacket* packet, const IModule* owner) = 0;
	virtual void __stdcall RelayDataToClient(const IPacket* packet, const IModule* owner) = 0;
	virtual int __stdcall GetClientSocket() = 0;
	virtual int __stdcall GetServerSocket() = 0;
	virtual IPacket* __stdcall CreatePacket(const void* data, int size) const = 0;
	virtual IProxy* __stdcall GetPeer() = 0;
	virtual ~IProxy() {}
};

typedef void(__stdcall *DestroyPlugin)(void* Info);
typedef IModule*(__stdcall *ModuleCreator)(IProxy* proxy, ModuleKind Kind);

struct PluginInfo
{
	const char *Name;
	const char *Author;
	int SDKVersion;
	DestroyPlugin Destroy;
	ModuleCreator Create;
};

typedef void (__stdcall *WriteLogType)(const char *Text);
typedef HWND *(__stdcall *GetWindowHandleType)();

struct RedVexInfo
{
	WriteLogType WriteLog;
	GetWindowHandleType GetWindowHandle;
};
