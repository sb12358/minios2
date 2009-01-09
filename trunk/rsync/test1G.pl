#!/usr/bin/perl
require ("general.pl");

genfile("test1G", 1000000000+rand(10000000));
benchmark('rsyncto("test1G");');
appendfile("test1G", 1000);
benchmark('rsyncto("test1G");');
overwritefile("test1G", 12345678, 1000);
benchmark('rsyncto("test1G");');
insertfile("test1G", 123456789, 1000);
benchmark('rsyncto("test1G");');

