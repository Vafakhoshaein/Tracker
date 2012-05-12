delimiter $$

CREATE TABLE `session` (
  `session_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` char(30) DEFAULT NULL,
  `start_time` datetime DEFAULT NULL,
  `end_time` datetime DEFAULT NULL,
  PRIMARY KEY (`session_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1$$

delimiter $$

CREATE TABLE `target` (
  `target_id` int(11) NOT NULL DEFAULT '0',
  `session_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`target_id`,`session_id`),
  KEY `session_id` (`session_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1$$

delimiter $$

CREATE TABLE `trace` (
  `trace_id` int(11) NOT NULL DEFAULT '0',
  `first_frame` int(11) DEFAULT NULL,
  `target_id` int(11) DEFAULT '0',
  `session_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`trace_id`,`session_id`),
  KEY `target_id` (`target_id`),
  KEY `session_id` (`session_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1$$

delimiter $$

CREATE TABLE `trace_node` (
  `xcor` int(11) DEFAULT NULL,
  `ycor` int(11) DEFAULT NULL,
  `height` int(11) DEFAULT NULL,
  `width` int(11) DEFAULT NULL,
  `node_id` int(11) NOT NULL DEFAULT '0',
  `trace_id` int(11) NOT NULL DEFAULT '0',
  `session_id` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`trace_id`,`node_id`,`session_id`),
  KEY `trace_id` (`trace_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1$$

delimiter $$

CREATE TABLE `video_file_reference` (
  `filename` char(100) DEFAULT NULL,
  `startFrame` int(11) DEFAULT NULL,
  `endFrame` int(11) DEFAULT NULL,
  `startTime` datetime DEFAULT NULL,
  `endTime` datetime DEFAULT NULL,
  `session_id` int(11) DEFAULT NULL,
  KEY `session_id` (`session_id`),
  CONSTRAINT `session_id` FOREIGN KEY (`session_id`) REFERENCES `session` (`session_id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=latin1$$


