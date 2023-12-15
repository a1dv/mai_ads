#include "route.h"

#include <exception>
#include <sstream>

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "../config/config.h"
#include "database.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    Poco::JSON::Object::Ptr Route::toJSON() {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("from", _from);
        root->set("to", _to);
        root->set("distance", _distance);
        root->set("duration", _duration);

        return root;
    }

    std::string& Route::from() {
        return _from;
    }

    std::string& Route::to() {
        return _to;
    }

    void Route::create() {
        try {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement insert(session);

        struct {
          std::string from;
          std::string to;
          std::string duration;
          long distance;
        } entry;

        entry.from = _from;
        entry.to = _to;
        entry.duration = "1h5m";
        entry.distance = 100;

        insert
            << "INSERT INTO route (`from`, `to`, duration, distance) VALUES(?, ?, ?, ?)",
            use(entry.from), use(entry.to), use(entry.duration), use(entry.distance);

        insert.execute();

      } catch (Poco::Data::MySQL::ConnectionException& e) {
        std::cout << "connection:" << e.what() << std::endl;
        throw;
      } catch (Poco::Data::MySQL::StatementException& e) {
        std::cout << "statement:" << e.what() << std::endl;
        throw;
      }
    }

    std::vector<Route> Route::get_by_user_id(long user_id) {
        Poco::Data::Session session = database::Database::get().create_session();
        Statement select(session);

        std::vector<Route> routes;
        Route r;

        select << "SELECT `from`, `to`, distance, duration FROM route "
                  "JOIN trip t on route.id = t.route_id "
                  "JOIN user_trip ut on t.id = ut.trip_id "
                  "WHERE user_id = ?",
                  into(r._from), into(r._to), into(r._distance), into(r._duration),
                  use(user_id), range(0, 1);

        while(!select.done()) {
            if (select.execute()) routes.push_back(r);
        }

        return routes;
    }

    std::optional<Route> Route::get(long id) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);

            Route r;

            select << "SELECT `from`, `to`, distance, duration FROM route "
                      "WHERE id = ?",
                      into(r._from), into(r._to), into(r._distance), into(r._duration),
                      use(id), range(0,1);

            if (select.execute() != 1) {
                return {};
            }

            return r;

        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }


} // namespace database
