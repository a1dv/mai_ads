#ifndef AUTHOR_H
#define AUTHOR_H

#include <optional>
#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
class User {
 private:
  long _id;
  std::string _first_name;
  std::string _last_name;
  std::string _email;
  std::string _gender;
  std::string _login;
  std::string _password;

 public:
  static User fromJSON(const std::string& str);

  long get_id() const;
  const std::string& get_first_name() const;
  const std::string& get_last_name() const;
  const std::string& get_email() const;
  const std::string& get_gender() const;
  const std::string& get_login() const;
  const std::string& get_password() const;

  long& id();
  std::string& first_name();
  std::string& last_name();
  std::string& email();
  std::string& gender();
  std::string& login();
  std::string& password();

  static void init();
  static std::optional<User> read_by_id(long id);
  static std::optional<long> auth(std::string& login, std::string& password);
  static std::vector<User> read_all();
  static std::vector<User> search(std::string first_name,
                                  std::string last_name);
  void save_to_mysql();

  static Poco::JSON::Object::Ptr remove_password(Poco::JSON::Object::Ptr src);
  static std::vector<User> get_by_trip_id(long trip_id);

  Poco::JSON::Object::Ptr toJSON() const;
};
}  // namespace database

#endif
