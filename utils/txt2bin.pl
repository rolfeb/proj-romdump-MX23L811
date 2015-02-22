#!/usr/bin/env perl
#
#
use strict;
use warnings;
use Data::Dumper;

while (my $line = <>)
{
    chomp $line;

    my ($bytestr) = ($line =~ m{^([0-9a-f]{64})\s*$})
        or next;

    my @bytes = map { hex($_) } ($bytestr =~ m{(..)}g);
    print pack 'C16', @bytes;
}

exit 0;
