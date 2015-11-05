#!/usr/bin/env perl

use strict;
use warnings;
use Test::More;

use Errno;
use IO::File;
use Net::FTP;

BEGIN {
    unlink '/etc/proftpd/hogehoge.conf' or do {
        die unless $! == Errno::ENOENT;
    }
}

subtest 'allow login' => sub {
    my $ftp = Net::FTP->new('127.0.0.1');
    is($ftp->login('hogehoge', 'hogehoge'), 1);
    is($ftp->message, "User hogehoge logged in\n");
};

subtest 'Include /etc/proftpd/hogehoge.conf, deny login' => sub {

    my $fh = IO::File->new('/etc/proftpd/hogehoge.conf', 'w') or die $!;
    $fh->printf(<<'....');
<Limit LOGIN>
    DenyAll
</Limit>
....
    $fh->close;

    my $ftp = Net::FTP->new('127.0.0.1');
    is($ftp->login('hogehoge', 'hogehoge'), '');
    is($ftp->message, "Login incorrect.\n");
};

subtest 'Include /etc/proftpd/vagrant.conf, allow login' => sub {

    my $fh = IO::File->new('/etc/proftpd/hogehoge.conf', 'w') or die $!;
    $fh->printf(<<'....');
<Limit LOGIN>
</Limit>
....
    $fh->close;

    my $ftp = Net::FTP->new('127.0.0.1');
    is($ftp->login('hogehoge', 'hogehoge'), 1);
    is($ftp->message, "User hogehoge logged in\n");
};

done_testing;
