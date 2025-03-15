#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <filesystem>
#include <boost/asio.hpp>

// Forward declarations
class CLI;
class Connection;

// Enum class for the client state keys
enum class ClientStateKeys {
	USERNAME, // To access the username
	UUID, // To access the UUID
	PUB_KEY, // To access the public key
	PRIV_KEY, // To access the private key
};

// Classs that represents the client state
// The client state class is used to store the client's state information such as the username, UUID, public key, and private key.
// The client state also stores data about other users such as their public keys, symmetric keys, and UUIDs.
class ClientState 
{
public:
	// Forward declaration and type aliases
	struct ClientEntry;
	using store_t = std::unordered_map<ClientStateKeys, std::string>;
	using clients_map_t = std::unordered_map<std::string, ClientEntry>; // maps a username to a client entry
	using rev_index_t = std::unordered_map<std::string, std::string>; // maps a UUID to a username

	// Client entry for the other clients, stores their UUID, public key and symmetric key
	struct ClientEntry {
		std::string uuid{};
		std::optional<std::string> pubKey;
		std::optional<std::string> symKey;
	};

	// Constructs the client state from a file
	ClientState(const std::filesystem::path& path);

	// Loads the client state from a file
	void loadFromFile(const std::filesystem::path& path);
	
	// Saves the client state to a file
	void saveToFile(const std::filesystem::path& path);

	// Checks if the client state is initialized
	bool isInitialized();

	// Checks if a client has a symmetric key
	bool hasSymKey(const std::string& username);

	// Gets the username by a uuid
	std::string getNameByUUID(const std::string& uuid);

	// Adds a client to the client state
	void addClient(const std::string& name, const std::string& uuid);

	// Sets the username
	void setUsername(const std::string& username);

	// Sets the UUID
	void setUUID(const std::string& uuid);

	// Sets the public keys of the current client
	void setPubKey(const std::string& pubKey);

	// Sets the public key of another client
	void setPubKey(const std::string& username, const std::string& pubKey);

	// Sets the private key of the current client
	void setPrivKey(const std::string& privKey);

	// Sets the symmetric key of another client
	void setSymKey(const std::string& username, const std::string& symKey);

	// Gets the username
	const std::string& getUsername();

	// Gets the UUID in unhexed format
	std::string getUUIDUnhexed();

	// Gets the UUID of the current client
	const std::string& getUUID();

	// Gets the public key of the another client
	const std::string& getUUID(const std::string& username);

	// Gets the public key of the current client
	const std::string& getPubKey();

	// Gets the public key of another client
	const std::optional<std::string>& getPubKey(const std::string& username);

	// Gets the private key of the current client
	const std::string& getPrivKey();

	// Gets the symmetric key of another client
	const std::optional<std::string>& getSymKey(const std::string& username);

private:
	// Get a client by its username
	ClientEntry& getClient(const std::string& username);

private:
	store_t m_store; // The store that holds the current client state information
	clients_map_t m_nameToClient; // Maps a username to a client entry
	rev_index_t m_uuidToName; // Maps a UUID to a username

	bool m_isInitialized{ false };
};

// This class is the main class that represents the client, it is responsible for handling the client's CLI, connection, and state.
class Client
{
public:
	using context_t = boost::asio::io_context;
	using cli_t = std::unique_ptr<CLI>;
	using connection_t = std::unique_ptr<Connection>;

	Client(context_t& ctx, const std::string& addr, const std::string& port);

	// This function runs the client
	void run();

	~Client();

private:
	// Gets the cli object
	CLI& getCLI();

	// Gets the connection object
	Connection& getConn();

	// Gets the state object
	ClientState& getState();

	// Binds the cli handlers, the handlers are the clients logic
	void setupCliHandlers();

	// Called on a register event.
	void onCliRegister();

	// Called on request for a client list
	void onCliReqClientList();

	// Called on request public key
	void onCliReqPubKey();

	// Called on request for pending messages
	void onCliReqPendingMsgs();

	// Called on sending a text message
	void onCliSendTextMsg();

	// Called on requesting a symmetric key
	void onCliReqSymKey();

	// Called on sending a symmertric key
	void onCliSendSymKey();

	// Called on sending a file
	void onCliSendFile();

private:
	cli_t m_cli;
	connection_t m_conn;
	ClientState m_state;
};

