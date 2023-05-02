#include "client.h"

#include <stdlib.h>

#include <sstream>
#include <string>

#include "crypto.h"
#include "server.h"

using std::string;
Client::Client(const string& id, const Server& server) : server(&server), id(id) {
    crypto::generate_key(public_key, private_key);
}

string Client::get_id() const {
    return id;
}

std::string Client::get_publickey() const {
    return public_key;
}

double Client::get_wallet() const {
    return server->get_wallet(id);
}

std::string Client::sign(const std::string& txt) const {
    return crypto::signMessage(private_key, txt);
}

bool Client::transfer_money(const std::string& receiver, const double value) const {
    std::ostringstream oss;
    oss << id << "-" << receiver << "-" << value;
    const string trx = oss.str();
    std::cout << trx << std::endl;
    const string signature = crypto::signMessage(private_key, trx);
    return server->add_pending_trx(trx, signature);
}

size_t Client::generate_nonce() const {
    return rand() % 1000000;
}
