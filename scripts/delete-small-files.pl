#!/usr/bin/perl
use strict;
use warnings;
mkdir "trash";
opendir(my $dh, ".") || die;
while (readdir $dh) {
	next unless (/^20/);
	my $sfz = `du -hs "$_"`;
	my @sz = split(/(K|M| )/, $sfz);
	rename($_, "trash/$_") if $sz[1] eq 'K' or $sz[0] < 1;
}
closedir $dh;
