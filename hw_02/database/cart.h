#ifndef CART_H
#define CART_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class Cart {
 private:
  long _user_id;
  std::vector<long> _product_ids;

 public:
  static Cart fromJSON(const std::string& str);

  long get_user_id() const;
  const std::vector<long>& get_product_ids() const;

  long& user_id();
  std::vector<long>& product_ids();

  static void init();
  static Cart read_by_user_id(long id);
  void save_to_mysql();

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
