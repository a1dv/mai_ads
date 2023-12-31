#include "user.h"
#include "../config/config.h"
#include "database.h"
#include "../helper.h"
#include "cache.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include <functional>
#include <exception>
#include <sstream>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {

    void User::init() {
        try {

            Poco::Data::Session session = database::Database::get().create_session();
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement create_stmt(session);
                create_stmt << "CREATE TABLE IF NOT EXISTS `user` (`id` INT NOT NULL, "
                            << "`first_name` VARCHAR(256) NOT NULL,"
                            << "`last_name` VARCHAR(256) NOT NULL,"
                            << "`login` VARCHAR(256) NOT NULL,"
                            << "`password` VARCHAR(256) NOT NULL,"
                            << "`email` VARCHAR(256) NULL,"
                            << "`gender` VARCHAR(16) NULL,"
                            << "PRIMARY KEY (`id`),KEY `fn` (`first_name`),KEY `ln` "
                               "(`last_name`));"
                            << hint,
                        now;
            }
        }

        catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("gender", _gender);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    Poco::JSON::Object::Ptr  User::remove_password(Poco::JSON::Object::Ptr src) {
        if (src->has("password"))
            src->set("password", "*******");
        return src;
    }

    User User::fromJSON(const std::string& str) {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.gender() = object->getValue<std::string>("gender");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    std::optional<long> User::auth(std::string& login, std::string& password) {
        try {
            std::cout << "Trying to auth " << login << "::" << password << std::endl;
            Poco::Data::Session session = database::Database::get().create_session();
            long id;

            std::vector<std::string> shards = database::Database::get_all_sharding_hints();
            for (const std::string &shard: shards) {
                std::cout << "Session created" << std::endl;
                Poco::Data::Statement select(session);
                select << "SELECT id FROM user where login = ? and password = ? " << shard,
                        into(id),
                        use(login),
                        use(password),
                        range(0, 1);

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()) {
                    return id;
                }
            }
        } catch (Poco::Data::DataException &e) {
            std::cout << "Exception: " << e.what() << " :: " << e.message() << std::endl;
            return {};
        }
        return {};
    }
    std::optional<User> User::read_by_id(long id,bool use_cache) {
        if (use_cache) {
            std::optional<User> opt_user = read_from_cache_by_id(id);
            if (opt_user) {
                return opt_user;
            }
        }

        try {
            Poco::Data::Session session = database::Database::get().create_session();
            User a;
            Poco::Data::Statement select(session);
            select << "SELECT id, first_name, last_name, email, gender,login,password "
                      "FROM user where id=?" << Database::get_sharding_hint(get_hash(std::to_string(id))),
                    into(a._id), into(a._first_name), into(a._last_name), into(a._email),
                    into(a._gender), into(a._login), into(a._password), use(id),
                    range(0, 1);  //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) {
                if (use_cache) a.save_to_cache();
                return a;
            }


        }

        catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }

    std::vector<User> User::read_all() {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<User> result;
            User a;
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement select(session);
                select << "SELECT id, first_name, last_name, email, gender, login, password "
                          "FROM user" + hint,
                        into(a._id), into(a._first_name), into(a._last_name), into(a._email),
                        into(a._gender), into(a._login), into(a._password),
                        range(0, 1);  //  iterate over result set one row at a time

                while (!select.done()) {
                    if (select.execute())
                        result.push_back(a);
                }
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string first_name, std::string last_name) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();

            std::vector<User> result;

            first_name += "%";
            last_name += "%";
            std::vector<std::string> hints = database::Database::get_all_sharding_hints();

            for (const std::string &hint: hints) {
                Statement select(session);
                User a;
                select << "SELECT id, first_name, last_name, email, gender, login, password "
                          "FROM user where first_name LIKE ? and last_name LIKE ?" + hint,
                        into(a._id), into(a._first_name), into(a._last_name), into(a._email),
                        into(a._gender), into(a._login), into(a._password), use(first_name),
                        use(last_name),
                        range(0, 1);  //  iterate over result set one row at a time

                while (!select.done()) {
                    if (select.execute())
                        result.push_back(a);
                }
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_mysql() {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            _id = generate_id();
            insert
                    << "INSERT INTO user (id, first_name,last_name,email,gender,login,password) "
                       "VALUES(?, ?, ?, ?, ?, ?, ?)" << Database::get_sharding_hint(get_hash(std::to_string(_id))),
                    use(_id), use(_first_name), use(_last_name), use(_email), use(_gender),
                    use(_login), use(_password);

            std::cout << insert.toString() << std::endl;

            insert.execute();

            save_to_cache();

            std::cout << "inserted:" << _id << std::endl;
        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    const std::string& User::get_login() const {
        return _login;
    }

    const std::string& User::get_password() const {
        return _password;
    }

    std::string& User::login() {
        return _login;
    }

    std::string& User::password() {
        return _password;
    }

    long User::get_id() const {
        return _id;
    }

    const std::string& User::get_first_name() const {
        return _first_name;
    }

    const std::string& User::get_last_name() const {
        return _last_name;
    }

    const std::string& User::get_email() const {
        return _email;
    }

    const std::string& User::get_gender() const {
        return _gender;
    }

    long& User::id() {
        return _id;
    }

    std::string& User::first_name() {
        return _first_name;
    }

    std::string& User::last_name() {
        return _last_name;
    }

    std::string& User::email() {
        return _email;
    }

    std::string& User::gender() {
        return _gender;
    }

    long User::generate_id() {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement select(session);

        long id;

        select << "SELECT nextval(user_id_seq)", into(id), range(0,1);

        if (!select.done()) {
            select.execute();
        }

        return id;
    }

    std::vector<User> User::get_by_ids(const std::vector<long>& ids) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<User> users;

            std::vector<std::string> hints = database::Database::get_all_sharding_hints();
            for (const std::string &hint: hints) {
                for (auto id : ids) {
                    Statement select(session);
                    User a;
                    select << "SELECT first_name, last_name, email, gender, login, password "
                              "FROM user WHERE id = ? " << hint, use(id), into(a._first_name), into(a._last_name), into(a._email),
                            into(a._gender), into(a._login), into(a._password), range(0,1);

                    std::cout << select.toString() << std::endl;

                    select.execute();
                    Poco::Data::RecordSet rs(select);
                    if (rs.moveFirst())
                        users.push_back(a);
                }
            }


            return users;
        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException& e) {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_cache() const {
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        database::Cache::get().put(_id, message);
    }

    std::optional<User> User::read_from_cache_by_id(long id) {
        try {
            std::string result;
            if (database::Cache::get().get(id, result))
                return fromJSON(result);
            else
                return {};
        }
        catch (std::exception &err) {
            return {};
        }
    }

}  // namespace database
