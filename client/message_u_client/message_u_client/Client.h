#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <boost/asio.hpp>

class CLI;
class Connection;

enum class ClientStateKeys {
	USERNAME,
	UUID,
	PUB_KEY,
	PRIV_KEY,
	SYM_KEY,
};

class ClientState 
{
public:
	struct OtherClientEntry;
	using store_t = std::unordered_map<ClientStateKeys, std::string>;
	using clients_map_t = std::unordered_map<std::string, OtherClientEntry>;
	using rev_index_t = std::unordered_map<std::string, std::string>;

	struct OtherClientEntry {
		std::string uuid;
		std::string pubKey;
	};

	ClientState(const std::filesystem::path& path);

	void loadFromFile(const std::filesystem::path& path);
	void saveToFile(const std::filesystem::path& path, const std::string& username, const std::string& uuid, const std::string& privKey);
	bool isInitialized();
	std::string getNameByUUID(const std::string& uuid);
	void addClient(const std::string& name, const std::string& uuid);

	void setUsername(const std::string& username);
	void setUUID(const std::string& uuid);
	void setPubKey(const std::string& pubKey);
	void setPubKey(const std::string& username, const std::string& pubKey);
	void setPrivKey(const std::string& privKey);
	void setSymKey(const std::string& symKey);

	const std::string& getUsername();
	const std::string& getUUID();
	const std::string& getUUID(const std::string& username);
	const std::string& getPubKey();
	const std::string& getPubKey(const std::string& username);
	const std::string& getPrivKey();
	const std::string& getSymKey();

private:
	store_t m_store;
	clients_map_t m_nameToClient;
	rev_index_t m_uuidToName;

	bool m_isInitialized{ false };
};

class Client
{
public:
	using context_t = boost::asio::io_context;
	using cli_t = std::unique_ptr<CLI>;
	using connection_t = std::unique_ptr<Connection>;

	Client(context_t& ctx, const std::string& addr, const std::string& port);

	void run();

	~Client();

private:
	CLI& getCLI();
	Connection& getConn();
	ClientState& getState();

	void setupCliHandlers();

	void onCliRegister();
	void onCliReqClientList();
	void onCliReqPubKey();
	void onCliReqPendingMsgs();
	void onCliSendTextMsg();
	void onCliReqSymKey();
	void onCliSendSymKey();

private:
	cli_t m_cli;
	connection_t m_conn;
	ClientState m_state;
};

