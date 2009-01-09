#!/usr/bin/perl
require ("genfiles.pl");

genfile("test10M", 10000000+rand(100000));
benchmark('rsyncto("test10M");');
appendfile("test10M", 1000);
benchmark('rsyncto("test10M");');
overwritefile("test10M", 1231413, 1000);
benchmark('rsyncto("test10M");');
insertfile("test10M", 12379, 1000);
benchmark('rsyncto("test10M");');

