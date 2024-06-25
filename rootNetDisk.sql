/*
 Navicat Premium Data Transfer

 Source Server         : mysql
 Source Server Type    : MySQL
 Source Server Version : 80037 (8.0.37-0ubuntu0.22.04.3)
 Source Host           : 192.168.214.128:3306
 Source Schema         : rootNetDisk

 Target Server Type    : MySQL
 Target Server Version : 80037 (8.0.37-0ubuntu0.22.04.3)
 File Encoding         : 65001

 Date: 22/06/2024 09:27:25
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for files
-- ----------------------------
DROP TABLE IF EXISTS `files`;
CREATE TABLE `files`  (
  `fileId` int NOT NULL AUTO_INCREMENT,
  `parentId` int NULL DEFAULT NULL,
  `ownerId` int NULL DEFAULT NULL,
  `fileName` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `fileType` char(1) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `fileSize` int NULL DEFAULT NULL,
  `fileHash` char(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `modifyTime` time NULL DEFAULT NULL,
  PRIMARY KEY (`fileId`) USING BTREE,
  INDEX `fkey`(`ownerId` ASC) USING BTREE,
  CONSTRAINT `fkey` FOREIGN KEY (`ownerId`) REFERENCES `users` (`userId`) ON DELETE RESTRICT ON UPDATE RESTRICT
) ENGINE = InnoDB AUTO_INCREMENT = 3 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of files
-- ----------------------------
INSERT INTO `files` VALUES (0, NULL, 1, 'version3', 'd', 4096, NULL, '15:53:00');
INSERT INTO `files` VALUES (1, 0, 1, 'data', 'd', 4096, NULL, '15:53:00');
INSERT INTO `files` VALUES (2, 1, 1, '2.txt', 'r', 70464307, NULL, '15:53:00');

-- ----------------------------
-- Table structure for sha1_of_files
-- ----------------------------
DROP TABLE IF EXISTS `sha1_of_files`;
CREATE TABLE `sha1_of_files`  (
  `sha1_id` int NULL DEFAULT NULL,
  `sha1` char(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of sha1_of_files
-- ----------------------------

-- ----------------------------
-- Table structure for users
-- ----------------------------
DROP TABLE IF EXISTS `users`;
CREATE TABLE `users`  (
  `userId` int NOT NULL AUTO_INCREMENT,
  `userName` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `salt` varchar(12) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `cryptPasswd` varchar(98) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NULL DEFAULT NULL,
  `curDirectory` int NULL DEFAULT 0,
  PRIMARY KEY (`userId`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 17 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of users
-- ----------------------------
INSERT INTO `users` VALUES (1, 'test', NULL, NULL, 1);
INSERT INTO `users` VALUES (2, 'yang', '123456789', '987654321', 0);
INSERT INTO `users` VALUES (3, 'temp', '1234', '5678', 0);
INSERT INTO `users` VALUES (4, 'temp1', NULL, NULL, 0);
INSERT INTO `users` VALUES (6, 'lll', '1234', '5678', 0);
INSERT INTO `users` VALUES (7, 'll2', '1234', '5678', 0);
INSERT INTO `users` VALUES (8, 'qwe', '$6$?1@foGUF$', '$6$?1@foGUF$r.u2fWZkXEmlqb7y68PzkkoybpL/3TUNK1gdQ4eK6UhGtol4mNJJjo2olwj4tNZNxON1IXWgsIaKh8/gluTUb1', 0);
INSERT INTO `users` VALUES (9, 'adsf', '$6$TktpumZ4$', '$6$TktpumZ4$fzX/EkrHHesZ7E.YwlWGyp9sNZz7cDrSmvW8L857x6qcYpw.1olYNEf7qp94mtElF3PvGzvDH1bYSGMNZ4Coq1', 0);
INSERT INTO `users` VALUES (10, 'qaz', '$6$V6D2mub1$', '$6$V6D2mub1$QfGbfaQHR2ob74iHJX8tjlCMxsY3EKysBRhQDOc2Yx48HP6qEd6127QVQOr7TFURV4Ye/k1XeX12hpfzY67If1', 0);
INSERT INTO `users` VALUES (11, 'sdfsdfds', '$6$LkS1HocP$', '$6$LkS1HocP$q/IvLH0PNuAWA0uK62BM.X4G27GYml0Jmwz2cGo23v1y6ZL6Qlt1Iy7HZRfH41xVQlE.wy06JoYDc..8YcHAU0', 0);
INSERT INTO `users` VALUES (12, 'tang', '$6$XFdm<F6N$', '$6$XFdm<F6N$I3bYFEVZ9l5yye7W0WquGrpcUoRkcJIhqfZwx55ElHt1wpgRg1/Ie7SzbEj3Kd.55tAssgVcsHdMkD4cd1q/J1', 0);
INSERT INTO `users` VALUES (13, 'ting', '$6$uQArWJHH$', '$6$uQArWJHH$pGvjUEJpBJ7VFAnSBHak9MtESze2br9flixL//ktgSwaZcEO3M7BL//UAGMEifsTtNkt30Jv40aPUxkBWoWKz0', 0);
INSERT INTO `users` VALUES (14, 'dsfsd', '$6$yIEMNwzd$', '$6$yIEMNwzd$pAIZzL29GX13x5i2xhpU/17akBfac2jqOd7lVuj323EF0Q8pUPw8.Opy6R7l2yuAAsZRycC5bLUCleM9zl2cC1', 0);
INSERT INTO `users` VALUES (15, 'qsc', '$6$bK?N6AH3$', '$6$bK?N6AH3$oOY/1ljydXYqOz8Mpwr3K3P43PP/wjlyfl/FP32OX4bN1Ntq6B96OfoSz8OqmNkNY9PBO0VsQCkOUN4WLaN0u/', 0);
INSERT INTO `users` VALUES (16, 'testuser', '$6$1?YvxGLu$', '$6$1?YvxGLu$/EKu1P4Ml.BGBq9Z.ZR4zXj4OesJN78ArEl4hid1s1r2.b4Lz5rN3sJKxOQ.3hleRz0ZcvAexVYBuDVY7r11t.', 0);

SET FOREIGN_KEY_CHECKS = 1;
