#!/usr/bin/perl
my $pat = `lsusb | grep ASUS`;
my ($bus, $dev) = ($pat =~ /Bus (\d+) Device (\d+)/);
printf "Killing device %d,%d\n", $bus, $dev;
system "./usbreset /dev/bus/usb/$bus/$dev";
