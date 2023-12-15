#include "product.h"

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
void Product::init() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Statement create_stmt(session);
    create_stmt << "CREATE TABLE IF NOT EXISTS `Products` ("
                << "`id` INT NOT NULL AUTO_INCREMENT,"
                << "`name` VARCHAR(1024) NOT NULL,"
                << "`count` INT NOT NULL,"
                << "`value` INT NOT NULL,"
                << "PRIMARY KEY (`id`), KEY `an` (`name`)"
                << ");",
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

Poco::JSON::Object::Ptr Product::toJSON() const {
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("id", _id);
  root->set("name", _name);
  root->set("count", _count);
  root->set("value", _value);

  return root;
}

Product Product::fromJSON(const std::string& str) {
  Product product;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  product.id() = object->getValue<long>("id");
  product.name() = object->getValue<std::string>("name");
  product.count() = object->getValue<long>("count");
  product.value() = object->getValue<long>("value");

  return product;
}

std::optional<Product> Product::read_by_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    Product p;
    select << "SELECT id, name, count, value "
              "FROM Products where id=?",
        into(p._id), into(p._name), into(p._count), into(p._value), use(id),
        range(0, 1);  //  iterate over result set one row at a time

    select.execute();
    Poco::Data::RecordSet rs(select);
    if (rs.moveFirst()) {
      return p;
    }
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

std::vector<Product> Product::read_all() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Statement select(session);
    std::vector<Product> result;
    Product p;
    select << "SELECT id, name, count, value "
              "FROM Products",
        into(p._id), into(p._name), into(p._count), into(p._value),
        range(0, 1);  //  iterate over result set one row at a time

    while (!select.done()) {
      if (select.execute())
        result.push_back(p);
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

void Product::save_to_mysql() {

  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    insert
        << "INSERT INTO Products (name, count, value) "
           "VALUES(?, ?, ?)",
        use(_name), use(_count), use(_value);

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

long Product::get_id() const { return _id; }
const std::string& Product::get_name() const { return _name; }
long Product::get_count() const { return _count; }
long Product::get_value() const { return _value; }

long& Product::id() { return _id; }
std::string& Product::name() { return _name; }
long& Product::count() { return _count; }
long& Product::value() { return _value; }

}
