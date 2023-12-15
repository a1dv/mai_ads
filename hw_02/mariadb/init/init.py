import random
import string

from datetime import date, timedelta
from faker import Faker
import mysql.connector

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

    for _ in range(n):
        faker_profile = fake.simple_profile()
        first_name, last_name = faker_profile['name'].split()[-2:]
        res.append({
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


def drop_tables(cursor):
    cursor.execute(("DROP TABLE IF EXISTS user_trip CASCADE"))
    cursor.execute(("DROP TABLE IF EXISTS trip CASCADE"))
    cursor.execute(("DROP TABLE IF EXISTS user CASCADE"))
    cursor.execute(("DROP TABLE IF EXISTS route CASCADE"))


def create_tables(cursor):
    cursor.execute("""CREATE TABLE IF NOT EXISTS `user` (
        `id`         INT NOT NULL AUTO_INCREMENT,
        `first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `last_name`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `login`      VARCHAR(256) NOT NULL,
        `password`   VARCHAR(256) NOT NULL,
        `email`      VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        `gender`     VARCHAR(16) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`)
    );""")

    cursor.execute("""CREATE TABLE IF NOT EXISTS route(
    `id` INT NOT NULL AUTO_INCREMENT,
    `from` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `to` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `duration` TEXT NOT NULL,
    `distance` DECIMAL NOT NULL,
    
    PRIMARY KEY (`id`)
    );""")

    cursor.execute("""CREATE TABLE IF NOT EXISTS trip(
    `id` INT NOT NULL AUTO_INCREMENT,
    `route_id` INT NOT NULL,
    `date` DATE NOT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (`route_id`) REFERENCES route(`id`)
    );
    """)

    cursor.execute("""CREATE TABLE IF NOT EXISTS user_trip(
    `id` INT NOT NULL AUTO_INCREMENT,
    `user_id` INT NOT NULL,
    `trip_id` INT NOT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (`user_id`) REFERENCES user(id),
    FOREIGN KEY (`trip_id`) REFERENCES trip(id)
    );""")


def main():
    connection = mysql.connector.connect(
        host="all-db",
        database="archdb",
        user="stud",
        password="stud"
    )

    c = connection.cursor()

    drop_tables(c)
    create_tables(c)

    users_count = 200
    routes_count = 200
    trips_count = 50
    user_trip_count = 100

    c.executemany(("INSERT INTO `user` "
                   "(`first_name`, `last_name`, `login`, `password`, `email`, `gender`) "
                   "VALUES (%(first_name)s, %(last_name)s, %(login)s, %(password)s, %(email)s, %(gender)s)"),
                  generate_users(users_count))

    c.executemany(("INSERT INTO `route` "
                   "(`from`, `to`, `distance`, `duration`) "
                   "VALUES (%(from)s, %(to)s, %(distance)s, %(duration)s)"), generate_routes(routes_count))

    c.executemany(("INSERT INTO `trip` "
                   "(`route_id`, `date`) "
                   "VALUES (%(route_id)s, %(date)s)"), generate_trip(trips_count, routes_count))

    c.executemany(("INSERT INTO `user_trip` "
                   "(`trip_id`, `user_id`) "
                   "VALUES (%(trip_id)s, %(user_id)s)"), generate_user_trip(user_trip_count, trips_count, users_count))

    connection.commit()


if __name__ == "__main__":
    main()
