#!/usr/bin/perl
use strict;
use warnings;
use Device::SerialPort;
use Getopt::Std;

$| = 1;

my $SERIAL_PORT     = '/dev/ttyUSB0';

# Constructor & Basic Values
my $port = Device::SerialPort->new($SERIAL_PORT)
    or die "Can't open $SERIAL_PORT: $!";

$port->baudrate(115200)     or die "fail setting baudrate";
$port->parity("none")       or die "fail setting parity";
$port->databits(8)          or die "fail setting databits";
$port->stopbits(1)          or die "fail setting stopbits";
$port->handshake("none")    or die "fail setting handshake";
$port->dtr_active(1)        or die "fail setting dtr_active";

$port->write_settings       or die "no settings";

for (;;)
{
    if (my $line = $port->input)
    {
        print "$line";
    }
}

exit 0;
