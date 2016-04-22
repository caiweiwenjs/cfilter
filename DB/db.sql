CREATE DATABASE IF NOT EXISTS `pcms`;

USE pcms;

CREATE TABLE IF NOT EXISTS `user_printer` (
	`user_name` CHAR(32) NOT NULL,
	`printer_name` VARCHAR(255) NOT NULL,
	PRIMARY KEY (`user_name`, `printer_name`)
) ENGINE=InnoDB DEFAULT CHARSET=UTF8;

CREATE TABLE IF NOT EXISTS `print_log` ( 
	`id` INT(11) AUTO_INCREMENT PRIMARY KEY,
	`status` INT NOT NULL,
	`user_name` CHAR(32) NOT NULL,
	`printer_name` VARCHAR(255) NOT NULL,
	`file_name` VARCHAR(255) NOT NULL,
	`title` VARCHAR(255) NOT NULL,
	`options` VARCHAR(255) NOT NULL,
	`copies` INT NOT NULL,
	`submit_time` DATETIME NOT NULL,
	`print_time` DATETIME NULL
) ENGINE=InnoDB DEFAULT CHARSET=UTF8;

