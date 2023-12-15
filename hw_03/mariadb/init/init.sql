CREATE TABLE IF NOT EXISTS `user` (
        `id`         INT NOT NULL AUTO_INCREMENT,
        `first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `last_name`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
        `login`      VARCHAR(256) NOT NULL,
        `password`   VARCHAR(256) NOT NULL,
        `email`      VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        `gender`     VARCHAR(16) CHARACTER SET utf8 COLLATE utf8_unicode_ci NULL,
        PRIMARY KEY (`id`), KEY `fn` (`first_name`), KEY `ln` (`last_name`)
    );

CREATE TABLE IF NOT EXISTS route(
    `id` INT NOT NULL AUTO_INCREMENT,
    `from` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `to` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `duration` INT NOT NULL,
    `length` DECIMAL NOT NULL,
    
    PRIMARY KEY (`id`)
);

CREATE TABLE IF NOT EXISTS trip(
    `id` INT NOT NULL AUTO_INCREMENT,
    `route_id` INT NOT NULL,
    `date` TIMESTAMP NOT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (`route_id`) REFERENCES route(`id`)
);

CREATE TABLE IF NOT EXISTS user_trip(
    `id` INT NOT NULL AUTO_INCREMENT,
    `user_id` INT NOT NULL,
    `trip_id` INT NOT NULL,

    PRIMARY KEY (`id`),
    FOREIGN KEY (`user_id`) REFERENCES user(id),
    FOREIGN KEY (`trip_id`) REFERENCES trip(id)
);

