#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Server;
class Client {
   public:
    Client(const std::string& id, const Server& server);
    std::string get_id() const;
    std::string get_publickey() const;
    double get_wallet() const;
    std::string sign(const std::string& txt) const;
    bool transfer_money(const std::string& receiver, const double value) const;
    size_t generate_nonce() const;
    friend void show_wallets(const Server& server);

   private:
    Server const* const server;
    const std::string id;
    std::string public_key;
    std::string private_key;
};

#endif  // CLIENT_H
