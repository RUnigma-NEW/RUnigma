#!/usr/bin/perl

if (@ARGV != 2) {
	die "Usage: $0 <in> <out>";
}

open DAT, "<$ARGV[0]" or die $!;
open OUT, ">$ARGV[1]" or die $!;

$| = 1;

system("stty -icanon eol \001");

while (defined($_ = <DAT>)) {
	/\[(\w+)\]/ and print "\nNew Section: $1\n\n";
	/^Item=(\w+),/ and do {
		$i = $1;
		/,x1=(\d+)[,;]/ and do {
			$x1 = $1;
			print "Item $i, x1 (left)?   ([l]/r) > ";
			read STDIN, $c, 1;
			print "\n";
			$nx1 = $x1 - 624;
			s/,x1=$x1/,x1=$nx1/ if $c eq 'r';
		};
		/,y1=(\d+)[,;]/ and do {
			$y1 = $1;
			print "Item $i, y1 (top)?    ([l]/r) > ";
			read STDIN, $c, 1;
			print "\n";
			$ny1 = $y1 - 486;
			s/,y1=$y1/,y1=$ny1/ if $c eq 'r';
		};
		/,x2=(\d+)[,;]/ and do {
			$x2 = $1;
			print "Item $i, x2 (right)?  ([l]/r) > ";
			read STDIN, $c, 1;
			print "\n";
			$nx2 = $x2 - 624;
			s/,x2=$x2/,x2=$nx2/ if $c eq 'r';
		};
		/,y2=(\d+)[,;]/ and do {
			$y2 = $1;
			print "Item $i, y2 (bottom)? ([l]/r) > ";
			read STDIN, $c, 1;
			print "\n";
			$ny2 = $y2 - 486;
			s/,y2=$y2/,y2=$ny2/ if $c eq 'r';
		};
	};
	print OUT $_;
}

system("stty icanon eol ^@");
