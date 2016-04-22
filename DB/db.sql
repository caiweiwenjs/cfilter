CREATE DATABASE IF NOT EXISTS `pcms`;

USE pcms;

CREATE TABLE IF NOT EXISTS `user_printer` (
	`user_name` CHAR(32) NOT NULL,
	`printer_name` VARCHAR(255) NOT NULL,
	PRIMARY KEY (`user_name`, `printer_name`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8;
