#include "server.h"

#include <stdlib.h>

#include <iterator>
#include <sstream>

#include "client.h"
#include "crypto.h"
using std::getline;
using std::istringstream;
using std::make_shared;
using std::ostringstream;
using std::shared_ptr;
using std::string;

std::vector<std::string> pending_trxs;

void apply_pending_trxs(std::map<std::shared_ptr<Client>, double>& clients, std::vector<std::string>& pending_trxs);

Server::Server() : clients() {
}

shared_ptr<Client> Server::add_client(const string& id) {
    string actual_id = id;
    while (this->get_client(actual_id) != nullptr) {
        auto postfix = rand() % 9000 + 1000;
        actual_id = id + std::to_string(postfix);
    }
    auto client = make_shared<Client>(actual_id, *this);
    clients.emplace(std::make_pair(client, 5));
    return client;
}

shared_ptr<Client> Server::get_client(const string& id) const {
    for (const auto& ele : clients) {
        auto client = ele.first;
        if (client->get_id() == id) {
            return client;
        }
    }
    return nullptr;
}

double Server::get_wallet(const string& id) const {
    const auto client = get_client(id);
    if (client == nullptr) {
        throw std::runtime_error(id + "not exist when getting wallet");
    }
    return clients.at(client);
}

bool Server::parse_trx(const string& trx, string& sender, string& receiver, double& value) {
    istringstream is(trx);
    if (!getline(is, sender, '-') || !getline(is, receiver, '-') ||
        !(is >> value) || !is.eof() || is.bad()) {
        throw std::runtime_error("parse_trx failed");
    }
    return true;
}

bool Server::add_pending_trx(const string& trx, const string& signature) const {
    string sender_id, receiver_id;
    double value;
    if (!parse_trx(trx, sender_id, receiver_id, value)) {
        return false;
    }
    // verify sender's and receiver's identity
    auto sender = get_client(sender_id);
    if (sender == nullptr) {
        return false;
    }
    auto receiver = get_client(receiver_id);
    if (receiver == nullptr) {
        return false;
    }
    if (!crypto::verifySignature(sender->get_publickey(), trx, signature)) {
        return false;
    }
    // check if wallet has enough money
    if (sender->get_wallet() < value) {
        return false;
    }
    pending_trxs.push_back(trx);
    return true;
}

size_t Server::mine() {
    if (clients.empty()) {
        throw std::runtime_error("clients array are empty");
    }
    // concatenate all strings in pending_trxs without delimter
    ostringstream os;
    std::copy(pending_trxs.begin(), pending_trxs.end(),
              std::ostream_iterator<string>(os));
    string mempool = os.str();
    auto client_iter = clients.cbegin();
    while (1) {
        if (client_iter == clients.cend()) {
            client_iter = clients.cbegin();
        }
        auto client = client_iter->first;
        auto nounce = client->generate_nonce();
        string final = mempool + std::to_string(nounce);
        string hash{crypto::sha256(final)};
        if (hash.substr(0, 10).find("000") != string::npos) {
            std::cout << client->get_id() << "win this round" << std::endl;
            clients.at(client) += 6.25;
            // apply pendind trxs
            for (const auto& trx : pending_trxs) {
                string sender_id, receiver_id;
                double value;
                parse_trx(trx, sender_id, receiver_id, value);
                auto receiver = get_client(receiver_id);
                auto sender = get_client(sender_id);
                clients.at(receiver) += value;
                clients.at(sender) -= value;
            }
            pending_trxs.clear();
            return nounce;
        }
    }
    ++client_iter;
}

// void apply_pending_trxs(std::map<std::shared_ptr<Client>, double>& clients, std::vector<std::string>& pending_trxs) {
//     for (const auto& trx : pending_trxs) {
//         string sender_id, receiver_id;
//         double value;
//         Server::parse_trx(trx, sender_id, receiver_id, value);
//     }
// }