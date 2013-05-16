#!/usr/bin/perl

while (defined($_ = <>)) {
	$x = 0; $y = 0;
	$w = 0; $h = 0;

	s/version=0.0.2/version=0.0.3,base=rel/;

	/,x=(\d+)[,;]/ and $x = $1;
	/,y=(\d+)[,;]/ and $y = $1;
	/,width=(\d+)[,;]/ and $w = $1;
	/,height=(\d+)[,;]/ and $h = $1;

	$x2 = $x + $w - 1;
	$y2 = $y + $h - 1;

	s/,x=$x/,x1=$x/;
	s/,y=$y/,y1=$y/;
	s/,width=$w/,x2=$x2/;
	s/,height=$h/,y2=$y2/;

	print $_;
}
