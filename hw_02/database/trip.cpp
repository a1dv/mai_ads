#include "trip.h"

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

namespace database {
    Poco::JSON::Object::Ptr Trip::toJSON() {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        Poco::JSON::Array users;

        for (auto a : _users) {
            users.add(User::remove_password(a.toJSON()));
        }

        root->set("date", _date);
        root->set("route", _route.toJSON());
        root->set("users", users);

        return root;
    }



    Route &Trip::route() {
        return _route;
    }

    Poco::Data::Date &Trip::date() {
        return _date;
    }

    std::vector<User> &Trip::users() {
        return _users;
    }

    std::optional<Trip> Trip::get(long id) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);

            Trip t;

            select << "SELECT route_id, date FROM trip "
                      "WHERE id = ?",
                    into(t._route_id), into(t._date), use(id), range(0,1);

            if (select.execute() != 1) {
                return {};
            }

            t.route() = Route::get(t._route_id).value();

            t.users() = User::get_by_trip_id(id);

            return t;
        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Trip::create()  {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            if (!(Route::get(_route_id).has_value())) {
                throw Poco::Data::MySQL::MySQLException("no such route");
            }

            insert << "INSERT INTO trip(route_id, date) VALUES(?, ?)", use(_route_id), use(_date);

            insert.execute();

        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Trip::add_user(long id,long user_id) {
        try{
            if (!get(id).has_value()) {
                throw Poco::Data::MySQL::MySQLException("no such trip");
            }

            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO user_trip(trip_id, user_id) VALUES(?, ?)", use(id), use(user_id);

            insert.execute();
        }catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    long &Trip::route_id() {
        return _route_id;
    }
}