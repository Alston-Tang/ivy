//
// Created by tang on 1/20/19.
//

#ifndef IVY_CONTROLLER_H
#define IVY_CONTROLLER_H

#include <glog/logging.h>
#include <unordered_map>
#include <unordered_set>
#include <folly/MPMCQueue.h>
#include "util.h"

namespace ivy {

enum ConnectionAction {
    CLOSE,
    ESTABLISH,
};

struct ConnectionTrait {
    ConnectionAction action;
    int fd;
    uint64_t id;
};

typedef folly::MPMCQueue<ConnectionTrait> PeerSyncQueue;
typedef std::unordered_map<int, uint64_t> ConnectionsType;
typedef std::unordered_map<uint64_t, std::unordered_set<int>> ConnectionsRevType;
typedef folly::MPMCQueue<ivy::message::Raw> RawMessageQueue;

inline void update_connection(ConnectionTrait &trait, ConnectionsType &connections) {
    switch (trait.action) {
        case ConnectionAction::CLOSE:
            if (!connections.count(trait.fd)) {
                LOG(WARNING) << "Ignore close action as the fd " << trait.fd << " not exists in local connections. id: "
                             << id_to_printable(trait.id);
                return;
            }
            if (connections[trait.fd] != trait.id) {
                LOG(WARNING) << "Ignore close action as incoming id " << id_to_printable(trait.id)
                             << " is different from local id " << id_to_printable(connections[trait.fd]);
                return;
            }
            connections.erase(trait.fd);
            LOG(INFO) << "Connection " << id_to_printable(trait.id) << " is closed according to peer's report";
            return;
        case ConnectionAction::ESTABLISH:
            if (connections.count(trait.fd)) {
                LOG(WARNING) << "Ignore establish action as fd " << trait.fd
                             << " is already in local connections. incoming id: " << id_to_printable(trait.id)
                             << " local id: " << id_to_printable(trait.id);
                return;
            }
            connections[trait.fd] = trait.id;
            LOG(INFO) << "Connection to " << id_to_printable(trait.id) << "is established according to peer report";
            return;
        default:
            LOG(ERROR) << "Invalid action";
            return;
    }
}

inline void update_connection_rev(ConnectionTrait &trait, ConnectionsRevType &connections) {
    switch (trait.action) {
        case ConnectionAction::CLOSE:
            if (!connections.count(trait.id) || !connections[trait.id].count(trait.fd)) {
                LOG(WARNING) << "Ignore close action as the id " << id_to_printable(trait.id) << "and fd " << trait.fd
                             << " not exists in local connections.";
                return;
            }
            connections[trait.id].erase(trait.fd);
            LOG(INFO) << "Connection to " << id_to_printable(trait.id) << " with fd " << trait.fd
                      << " is closed according to peer's report";
            return;
        case ConnectionAction::ESTABLISH:
            if (connections.count(trait.id) && connections[trait.id].count(trait.fd)) {
                LOG(WARNING) << "Ignore establish action as fd " << trait.fd << " and id " << id_to_printable(trait.id)
                             << " is already in local connections.";
                return;
            }
            connections[trait.id].insert(trait.fd);
            LOG(INFO) << "Connection to " << id_to_printable(trait.id) << " with fd " << trait.fd
                      << " is established according to peer report";
            return;
        default:
            LOG(ERROR) << "Invalid action";
            return;
    }
}

}

#endif //IVY_CONTROLLER_H
