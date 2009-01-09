#!/usr/bin/perl
require ("genfiles.pl");

genfile("test100M", 100000000+rand(1000000));
benchmark('rsyncto("test100M");');
appendfile("test100M", 1000);
benchmark('rsyncto("test100M");');
overwritefile("test100M", 1231413, 1000);
benchmark('rsyncto("test100M");');
insertfile("test100M", 12379, 1000);
benchmark('rsyncto("test100M");');

