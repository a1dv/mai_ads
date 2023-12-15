import mysql.connector
from mysql.connector import errorcode

SHARD_COUNT = 2

def get_hint(shard_id):
    shard_id = shard_id % SHARD_COUNT
    return f"-- sharding:{shard_id}"

def get_hash(astr):
    p = 53
    m = 61566613

    roll = 0
    for ch in astr[::-1]:
        roll = (roll * p) % m + ord(ch)
    return roll


class MySQLConnection:
    def __init__(self, *, host, port, database, user, password):
        self.cnx = mysql.connector.connect(
            host=host,
            port=port,
            database=database,
            user=user,
            password=password)
        self.cursor = self.cnx.cursor()
        self.cnx.autocommit = True

    def __del__(self):
        self.cursor.close()
        self.cnx.close()

    def get(self, command):
        try:
            self.cursor.execute(command)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
                print("already exists.")
            else:
                print(err.msg)
        else:
            print("get: OK")

        return(list(self.cursor))

    def execute(self, command):
        try:
            self.cursor.execute(command)
        except mysql.connector.Error as err:
            if err.errno == errorcode.ER_TABLE_EXISTS_ERROR:
                print("already exists.")
            else:
                print(err.msg)
        else:
            print("execute: OK")

    def insert_values(self, insert_command, value_list, hash_func):
        for value in value_list:
            hint = get_hint(hash_func(value))
            query = insert_command + hint
            print(query)
            self.cursor.execute(query, value)

    def insert_without_sharding(self, query, value_list):
        for value in value_list:
            print(query)
            self.cursor.execute(query, value)
