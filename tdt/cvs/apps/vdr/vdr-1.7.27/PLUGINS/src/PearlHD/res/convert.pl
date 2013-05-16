#!/usr/bin/perl
open (in,"<$ARGV[$0]") || die $!;
$ref_width = 1920;
$width = $ARGV[1];
if (int($width)){
	$ratio = 1920 / int($width);
	while (<in>){
  		$line= $_;
		$line=~ s/VDRSymbols Sans/VDRSymbols_Sans/g;
		@line = split(/ /, $line );
		foreach (@line){
			@item = split(/=/, $_);
			if (@item[1]){
				if (@item[0] =~/x1/){
					@item[1] =~ s/\"//g;
					$tmp_a="\"".int((@item[1] / $ratio)+0.5)."\"";
					@item[1]=$tmp_a;
				}
				elsif  (@item[0] =~/x2/){
					@item[1] =~ s/\"//g;
					if (int(@item[1])!=-1){
						@item[1]="\"".int((@item[1] / $ratio)+0.5)."\"";
					}
					else {
						@item[1]="\"".@item[1]."\"";
					}
				}
				elsif  (@item[0] =~/y1/){
					@item[1] =~ s/\"//g;
					@item[1]="\"".int((@item[1] / $ratio)+0.5)."\"";
				}
				elsif  (@item[0] =~/y2/){
					@item[1] =~ s/\"//g;
					if (int(@item[1])!=-1){
						@item[1]="\"".int((@item[1] / $ratio)+0.5)."\"";
					}
					else {
						@item[1]="\"".@item[1]."\"";
					}
				}
				elsif  (@item[0] =~/font/){
					if  (@item[1] =~/@/){
					@font = split(/@/, @item[1]);
					@font[1] =~ s/\"//g;
					$newfs = int(@font[1] / $ratio);
					@font[0]=~ s/_/ /g;
					@item[1]=@font[0]."@".$newfs ."\"";}
				}
				print @item[0]."=".@item[1]." ";
			} 
			else {
				print @item[0]." ";
			}
		}
	}
}
sub ceil {
	return int($_[0] + 0.99);
}
close in;