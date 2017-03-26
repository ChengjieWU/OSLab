#!/usr/bin/perl

open(SIG, $ARGV[0]) || die "open $ARGV[0]: $!";

$n = sysread(SIG, $buf, 10 * 1024 * 1024);

if($n > 10 * 1024 * 1024){
  print STDERR "ERROR: kernel binary file too large: $n bytes (max 10*1024*1024)\n";
  exit 1;
}

print STDERR "OK: kernel binary file is $n bytes (max 10 * 1024 * 1024)\n";

$p = 10*1024*1024 - $n;
$buf .= "\0" x $p;

open(SIG, ">$ARGV[0]") || die "open >$ARGV[0]: $!";
print SIG $buf;
close SIG;
