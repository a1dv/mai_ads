#include "cart.h"

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
void Cart::init() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Statement create_stmt(session);
    create_stmt << "CREATE TABLE IF NOT EXISTS `Carts` ("
                << "`id` INT NOT NULL AUTO_INCREMENT,"
                << "`user_id` INT NOT NULL,"
                << "`product_id` INT NOT NULL,"
                << "PRIMARY KEY (`id`), KEY `uid` (`user_id`)"
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

Poco::JSON::Object::Ptr Cart::toJSON() const {
  Poco::JSON::Array product_ids;
  for (const auto& e : get_product_ids()) {
    product_ids.add(e);
  }

  // Oh yeah and that's a memory leak
  // Too bad!
  Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

  root->set("user_id", _user_id);
  root->set("product_ids", product_ids);

  return root;
}

Cart Cart::fromJSON(const std::string& str) {
  Cart cart;
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(str);
  Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

  cart.user_id() = object->getValue<long>("user_id");

  // no i do not care
  auto product_array = object->getArray("product_ids");
  for (size_t i = 0; i < product_array->size(); ++i) {
    cart.product_ids().push_back(product_array->get(i));
  }

  return cart;
}

Cart Cart::read_by_user_id(long id) {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement select(session);
    Cart c;
    select << "SELECT id, user_id, product_id "
              "FROM Carts where user_id=?", use(id);
    select.execute();

    Poco::Data::RecordSet rs(select);

    c.user_id() = id;
    for (auto& row : rs) {
      c.product_ids().push_back(row["product_id"].convert<long>());
    }

    return c;
  }

  catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
  }

  return {};
}

void Cart::save_to_mysql() {
  try {
    Poco::Data::Session session = database::Database::get().create_session();
    Poco::Data::Statement insert(session);

    struct {
      long user_id;
      long product_id;
    } entry;

    insert
        << "INSERT INTO Carts (user_id, product_id) "
           "VALUES(?, ?)",
        use(entry.user_id), use(entry.product_id);

    entry.user_id = _user_id;
    for (const auto& e : _product_ids) {
      entry.product_id = e;
      insert.execute();
    }

    std::cout << "inserted for user:" << _user_id << std::endl;

  } catch (Poco::Data::MySQL::ConnectionException& e) {
    std::cout << "connection:" << e.what() << std::endl;
    throw;
  } catch (Poco::Data::MySQL::StatementException& e) {
    std::cout << "statement:" << e.what() << std::endl;
    throw;
  }
}

long Cart::get_user_id() const { return _user_id; }
const std::vector<long>& Cart::get_product_ids() const { return _product_ids; }

long& Cart::user_id() { return _user_id; }
std::vector<long>& Cart::product_ids() { return _product_ids; }

}
