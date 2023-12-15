#include "database.h"
#include "../config/config.h"

namespace database {
    Database::Database() {
        _connection_string += "host=";
        _connection_string += Config::get().get_host();
        _connection_string += ";user=";
        _connection_string += Config::get().get_login();
        _connection_string += ";db=";
        _connection_string += Config::get().get_database();
        _connection_string += ";port=";
        _connection_string += Config::get().get_port();
        _connection_string += ";password=";
        _connection_string += Config::get().get_password();

        std::cout << "Connection string:" << _connection_string << std::endl;
        Poco::Data::MySQL::Connector::registerConnector();
        _pool = std::make_unique<Poco::Data::SessionPool>(
                Poco::Data::MySQL::Connector::KEY, _connection_string);
    }

    Database& Database::get() {
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session() {
        return Poco::Data::Session(_pool->get());
    }

    size_t Database::get_max_shard(){
        return 2;
    }

    std::string Database::get_sharding_hint(long hash){
        size_t shard_number = hash % get_max_shard();

        std::string result = "-- sharding:";
        result += std::to_string(shard_number);
        return result;
    }

    std::vector<std::string> Database::get_all_sharding_hints() {
        std::vector<std::string> res;
        size_t max_shards = get_max_shard();

        res.reserve(max_shards);
        for (size_t i = 0; i < max_shards; ++i) {
            res.push_back(get_sharding_hint(i));
        }

        return res;
    }
}  // namespace database
