import random
import string

from datetime import date, timedelta
from faker import Faker
import basemod

fake = Faker()


def generate_password(length, alphabet=None):
    if alphabet is None:
        alphabet = [
            *string.ascii_lowercase, *string.ascii_uppercase,
            '-', '_', '.', '@', '&']
    return ''.join([random.choice(alphabet) for _ in range(length)])


def get_duration(distance):
    time = (distance / 80) * 3600
    day = int(time // (24 * 3600))
    time = time % (24 * 3600)
    hour = int(time // 3600)
    time %= 3600
    minutes = int(time // 60)
    return f'{day}d:{hour}h:{minutes}m'


def generate_users(n=1):
    genders = {'M': 'male', 'F': 'female'}
    res = []

    for i in range(1, n + 1):
        faker_profile = fake.simple_profile()
        first_name, last_name = faker_profile['name'].split()[-2:]
        res.append({
            'id': i,
            'first_name': first_name,
            'last_name': last_name,
            'email': faker_profile['mail'],
            'login': faker_profile['username'],
            'password': generate_password(30),
            'gender': genders[faker_profile['sex']]
        })
    return res


def generate_routes(n=1):
    res = []

    for _ in range(n):
        distance = random.randint(50, 2000)
        res.append(
            {
                'from': fake.city(),
                'to': fake.city(),
                'distance': distance,
                'duration': get_duration(distance)
            }
        )
    return res


def random_date():
    d = date(2023, 1, 1)

    d += timedelta(days=random.randint(1, 365))

    return d


def generate_trip(k, n):
    res = []

    for _ in range(k):
        res.append(
            {
                'route_id': random.randint(1, n),
                'date': random_date()
            }
        )
    return res


def generate_user_trip(k, l, n):
    res = []

    for _ in range(k):
        res.append({
            'trip_id': random.randint(1, l),
            'user_id': random.randint(1, n)
        })
    return res


def drop_tables():
    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute((f"DROP TABLE IF EXISTS user_trip CASCADE; {basemod.get_hint(i)}"))
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute((f"DROP TABLE IF EXISTS trip CASCADE {basemod.get_hint(i)}"))
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute((f"DROP TABLE IF EXISTS user CASCADE {basemod.get_hint(i)}"))
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute((f"DROP TABLE IF EXISTS route CASCADE {basemod.get_hint(i)}"))


def create_tables():
    for i in range(basemod.SHARD_COUNT):
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute(f"""CREATE TABLE IF NOT EXISTS `user` (
        `id`         INT PRIMARY KEY,
        `first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `last_name`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `login`      VARCHAR(256) NOT NULL,
        `password`   VARCHAR(256) NOT NULL,
        `email`      VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        `gender`     VARCHAR(16) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        KEY `fn` (`first_name`), KEY `ln` (`last_name`)
        ); {basemod.get_hint(i)}""")
        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute(f"""CREATE TABLE IF NOT EXISTS route(
        `id` INT NOT NULL AUTO_INCREMENT,
        `from` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `to` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `duration` TEXT NOT NULL,
        `distance` DECIMAL NOT NULL,
        
        PRIMARY KEY (`id`)
        );""")

        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute(f"""CREATE TABLE IF NOT EXISTS trip(
        `id` INT NOT NULL AUTO_INCREMENT,
        `route_id` INT NOT NULL,
        `date` DATE NOT NULL,
    
        PRIMARY KEY (`id`)
        );""")

        connection = basemod.MySQLConnection(
            host="all-db",
            port="6033",
            database="archdb",
            user="stud",
            password="stud"
        )
        connection.execute(f"""CREATE TABLE IF NOT EXISTS user_trip(
        `id` INT NOT NULL AUTO_INCREMENT,
        `user_id` INT NOT NULL,
        `trip_id` INT NOT NULL,
    
        PRIMARY KEY (`id`)
        );""")

def drop_seq():
    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )
    connection.execute("drop sequence if exists user_id_seq;")


def create_seq(n):
    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )
    connection.execute(f"""create sequence user_id_seq start with {n}""")


def main():
    connection = basemod.MySQLConnection(
        host="all-db",
        port="6033",
        database="archdb",
        user="stud",
        password="stud"
    )

    drop_tables()
    create_tables()
    drop_seq()

    users_count = 5000
    routes_count = 200
    trips_count = 50
    user_trip_count = 100

    connection.insert_values(("INSERT INTO `user` "
                              "(id, `first_name`, `last_name`, `login`, `password`, `email`, `gender`) "
                              "VALUES (%(id)s, %(first_name)s, %(last_name)s, %(login)s, %(password)s, %(email)s, "
                              "%(gender)s)"),
                             generate_users(users_count), lambda x: basemod.get_hash(str(x['id'])))

    connection.insert_without_sharding(("INSERT INTO `route` "
                                        "(`from`, `to`, `distance`, `duration`) "
                                        "VALUES (%(from)s, %(to)s, %(distance)s, %(duration)s)"),
                                       generate_routes(routes_count))

    connection.insert_without_sharding(("INSERT INTO `trip` "
                                        "(`route_id`, `date`) "
                                        "VALUES (%(route_id)s, %(date)s)"), generate_trip(trips_count, routes_count))

    connection.insert_without_sharding(("INSERT INTO `user_trip` "
                                        "(`trip_id`, `user_id`) "
                                        "VALUES (%(trip_id)s, %(user_id)s)"),
                                       generate_user_trip(user_trip_count, trips_count, users_count))

    create_seq(users_count+1)

if __name__ == "__main__":
    main()
