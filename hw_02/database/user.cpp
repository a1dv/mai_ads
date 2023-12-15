#include "user.h"
#include "../config/config.h"
#include "database.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include <exception>
#include <sstream>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {

void User::init() {
  try {

    Poco::Data::Session session = database::Database::get().create_session();
    Statement create_stmt(session);
    create_stmt << "CREATE TABLE IF NOT EXISTS `user` (`id` INT NOT NULL "
                   "AUTO_INCREMENT,"
                << "`first_name` VARCHAR(256) NOT NULL,"
                << "`last_name` VARCHAR(256) NOT NULL,"
                << "`login` VARCHAR(256) NOT NULL,"
                << "`password` VARCHAR(256) NOT NULL,"
                << "`email` VARCHAR(256) NULL,"
                << "`gender` VARCHAR(16) NULL,"
                << "PRIMARY KEY (`id`),KEY `fn` (`first_name`),KEY `ln` "
                   "(`last_name`));",
        now;
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
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    long id;
    select << "SELECT id FROM user where login=? and password=?", into(id),
        use(login), use(password),
        range(0, 1);  //  iterate over result set one row at a time

    select.execute();
    Poco::Data::RecordSet rs(select);
    if (rs.moveFirst())
      return id;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {

    std::cout << "statement:" << e.what() << std::endl;
  }
  return {};
}
std::optional<User> User::read_by_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    User a;
    select << "SELECT id, first_name, last_name, email, gender,login,password "
              "FROM user where id=?",
        into(a._id), into(a._first_name), into(a._last_name), into(a._email),
        into(a._gender), into(a._login), into(a._password), use(id),
        range(0, 1);  //  iterate over result set one row at a time

    select.execute();
    Poco::Data::RecordSet rs(select);
    if (rs.moveFirst())
      return a;
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
    Statement select(session);
    std::vector<User> result;
    User a;
    select << "SELECT id, first_name, last_name, email, gender, login, password "
              "FROM user",
        into(a._id), into(a._first_name), into(a._last_name), into(a._email),
        into(a._gender), into(a._login), into(a._password),
        range(0, 1);  //  iterate over result set one row at a time

    while (!select.done()) {
      if (select.execute())
        result.push_back(a);
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
    Statement select(session);
    std::vector<User> result;
    User a;
    first_name += "%";
    last_name += "%";
    select << "SELECT id, first_name, last_name, email, gender, login, password "
              "FROM user where first_name LIKE ? and last_name LIKE ?",
        into(a._id), into(a._first_name), into(a._last_name), into(a._email),
        into(a._gender), into(a._login), into(a._password), use(first_name),
        use(last_name),
        range(0, 1);  //  iterate over result set one row at a time

    while (!select.done()) {
      if (select.execute())
        result.push_back(a);
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

    insert
        << "INSERT INTO user (first_name,last_name,email,gender,login,password) "
           "VALUES(?, ?, ?, ?, ?, ?)",
        use(_first_name), use(_last_name), use(_email), use(_gender),
        use(_login), use(_password);

    insert.execute();

    Poco::Data::Statement select(session);
    select << "SELECT LAST_INSERT_ID()", into(_id),
        range(0, 1);  //  iterate over result set one row at a time

    if (!select.done()) {
      select.execute();
    }
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

    std::vector<User> User::get_by_trip_id(long trip_id) {
        Poco::Data::Session session = database::Database::get().create_session();
        Poco::Data::Statement select(session);

        std::vector<User> users;
        User a;

        select << "SELECT first_name, last_name, email, gender, login, password "
                  "FROM user "
                  "JOIN user_trip ON user_id = user.id "
                  "WHERE trip_id = ?",
                into(a._first_name), into(a._last_name), into(a._email),
                into(a._gender), into(a._login), into(a._password), use(trip_id), range(0,1);

        while(!select.done()) {
            if (select.execute()) users.push_back(a);
        }

        return users;
    }
}  // namespace database
