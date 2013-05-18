#!/usr/bin/perl

use strict;
use warnings;

my $DISPLAYS = {
	Channel => 'channelInfo',
	Volume => 'volume',
	Message => 'message',
	Replay => 'replayInfo',
	ReplayMode => 'replaySmall',
	Menu => 'menu',
};

my $TOKENS = {
	DateTimeF => '{DateTime:%s}',
	DateTime => '{DateTime:%a %e.%m. %H\:%M}',
	Date => '{DateTime:%e.%m.}',
	Time => '{DateTime:%H\:%M}',
	ChannelNumberName => '{ChannelNumber} {ChannelName}',
	PresentDateTimeF => '{PresentStartDateTime:%s}',
	PresentStartTime => '{PresentStartDateTime:%H\:%M}',
	PresentDate => '{PresentStartDateTime:%e.%m.}',
	PresentVPSTime => '{PresentVPSDateTime:%H\:%M}',
	PresentEndTime => '{PresentEndDateTime:%H\:%M}',
	PresentDuration => '{PresentDuration:%H\:%M}',
	PresentTextDescription => '{PresentText}\n\n{PresentDescription}',
	FollowingStartTime => '{FollowingStartDateTime:%H\:%M}',
	FollowingEndTime => '{FollowingEndDateTime:%H\:%M}',
	FollowingDuration => '{FollowingDuration:%H\:%M}',
	FollowingTextDescription => '{FollowingText}\n\n{FollowingDescription}',
	FastFwd => '{IsFastForward}',
	ReplayTime => '{ReplayPosition}',
	ReplayDuration => '{ReplayDuration}',
	MenuRed => '{ButtonRed}',
	MenuGreen => '{ButtonGreen}',
	MenuYellow => '{ButtonYellow}',
	MenuBlue => '{ButtonBlue}',
	MenuGroups => '{MenuGroup}',
	MenuItems => '{MenuItem}',
};

my $ALIGNS = {
	0 => 'left',
	1 => 'center',
	2 => 'right'
};

my $SYMBOLS = {
	Teletext => '{HasTeletext}',
	Audio => '{HasMultilang}',
	Dolby => '{HasDolby}',
	Radio => '{IsRadio}',
	Encrypted => '{IsEncrypted}',
	Recording => '{IsRecording}',
	
	Play => '{IsPlaying}',
	Pause => '{IsPausing}',
	FastFwd => '{IsFastForward}',
	FastRew => '{IsFastRewind}',
	SlowFwd => '{IsSlowForward}',
	SlowRew => '{IsSlowRewind}',
};

my $data = { Skin => [] };
my $section = "Skin";

while (defined($_ = <>)) {
	chomp $_;
	$_ =~ s/;$//;
	next if /^\s*$/;
	next if /^\s*#/;

	if ($_ =~ /\[([^\]]+)\]/) {
		$section = $1;
		$data->{$section} = [];
	} else {
		my @params = split(/,/, $_);
		my $item = {};
		foreach my $param (@params) {
			my @param = split(/=/, $param);
			$item->{$param[0]} = $param[1];
		}
		push @{$data->{$section}}, $item;
	}
}

print "<?xml version=\"1.0\"?>\n";

# skin tag
print "<skin version=\"1.0\"";
print " name=\"" . $data->{Skin}[0]{name} . "\"";
if (defined($data->{Skin}[0]{base}) && $data->{Skin}[0]{base} eq 'abs') {
	print " screenBase=\"absolute\"";
} else {
	print " screenBase=\"relative\"";
}
print ">\n";

foreach my $display (keys %$data) {
	next if (!defined($DISPLAYS->{$display}));

	# display tag
	print "  <display id=\"" . $DISPLAYS->{$display} . "\">\n";

	foreach my $window (@{$data->{$display}}) {
		next if ($window->{Item} ne 'Background');

		print "    <window x1=\"" . $window->{x1} . "\" x2=\"" . $window->{x2}
				. "\" y1=\"" . $window->{y1} . "\" y2=\"" . $window->{y2}
				. "\" bpp=\"" . ($window->{bpp} || 4) . "\"/>\n";
	}

	foreach my $item (@{$data->{$display}}) {
		local $_ = $item->{Item};
		/^Background$/ and do {
			if (defined($item->{bg})) {
				print "    <rectangle x1=\"" . $item->{x1} . "\" x2=\"" 
						. $item->{x2} . "\" y1=\"" . $item->{y1} . "\" y2=\"" 
						. $item->{y2} . "\" color=\"" . $item->{bg} . "\"/>\n";
			}
			if (defined($item->{path})) {
				print "    <image x=\"" . $item->{x1} . "\" y=\"" . $item->{y1}
						. "\" path=\"" . $item->{path} . "\"";

				if (defined($item->{alpha})) {
					print " alpha=\"" . $item->{alpha} . "\"";
				}
				print "/>\n";
			}
			next;
		};
		/^Image$/ and do {
			print "    <image x=\"" . $item->{x1} . "\" y=\"" . $item->{y1}
					. "\" path=\"" . $item->{path} . "\"";
			if (defined($item->{alpha})) {
				print " alpha=\"" . $item->{alpha} . "\"";
			}
			print "/>\n";
			next;
		};
		/^Rectangle$/ and do {
			$item->{x1} ||= 0;
			$item->{y1} ||= 0;
			$item->{x2} ||= -1;
			$item->{y2} ||= -1;

			print "    <rectangle x1=\"" . $item->{x1} . "\" x2=\"" 
					. $item->{x2} . "\" y1=\"" . $item->{y1} . "\" y2=\"" 
					. $item->{y2} . "\" color=\"" . $item->{fg} . "\"";
			if (defined($item->{display})) {
				if (defined($TOKENS->{$item->{display}})) {
					print " condition=\"" . $TOKENS->{$item->{display}} . "\"";
				} else {
					print " condition=\"{" . $item->{display} . "}\"";
				}
			}
			print "/>\n";
			next;
		};
		/^Logo$/ and do {
			print "    <image x=\"" . $item->{x1} . "\" y=\"" . $item->{y1}
					. "\"";
			if (defined($item->{alpha})) {
				print " alpha=\"" . $item->{alpha} . "\"";
			}
			if ($item->{display} eq 'ChannelName') {
				my $p = $item->{path} . "/{" . $item->{display} . "}."
						. $item->{type};
				print " condition=\"file('$p')\"" . " path=\"$p\"";
			} elsif ($item->{display} eq 'ReplayMode') {
				my $p = $item->{path} . "/{" . $item->{display} . "}."
						. $item->{type};
				print " condition=\"file('$p')\" path=\"$p\"";
			}
			print "/>\n";
			next;
		};
		/^Text$/ and do {
			$item->{x1} ||= 0;
			$item->{y1} ||= 0;
			$item->{x2} ||= -1;
			$item->{y2} ||= -1;

			print "    <text x1=\"" . $item->{x1} . "\" x2=\"" 
					. $item->{x2} . "\" y1=\"" . $item->{y1} . "\" y2=\"" 
					. $item->{y2} . "\" color=\"" . $item->{fg} . "\"";
			if (defined($item->{align})) {
				print " align=\"" . $ALIGNS->{$item->{align}} . "\"";
			}
			if (defined($item->{font})) {
				print " font=\"" . $item->{font} . "\"";
			}
			print ">";
			if ($item->{display} eq 'DateTimeF' 
					|| $item->{display} eq 'PresentDateTimeF') {
				my $f = $item->{format};
				$f =~ s/^"|"$//;
				$f =~ s/([:\{\}])/\\$1/g;
				print sprintf($TOKENS->{$item->{display}}, $f);
			} elsif (defined($TOKENS->{$item->{display}})) {
				print $TOKENS->{$item->{display}};
			} else {
				print "{" . $item->{display} . "}";
			}
			print "</text>\n";
			next;
		};
		/^Symbol$/ and do {
			print "    <image x=\"" . $item->{x1} . "\" y=\"" . $item->{y1}
					. "\"";
			if (defined($item->{alpha})) {
				print " alpha=\"" . $item->{alpha} . "\"";
			}
			print " condition=\"" . $SYMBOLS->{$item->{display}} . "\" path=\""
					. $item->{path} . "\"/>\n";
			if (defined($item->{altpath})) {
				print "    <image x=\"" . $item->{x1} . "\" y=\"" 
						. $item->{y1} . "\"";
				if (defined($item->{alpha})) {
					print " alpha=\"" . $item->{alpha} . "\"";
				}
				print " condition=\"not(" . $SYMBOLS->{$item->{display}} . ")\""
						." path=\"" . $item->{path} . "\"/>\n";
			}
			next;
		};
		/^Progress$/ and do {
			print "    <progress x1=\"" . $item->{x1} . "\" x2=\"" 
					. $item->{x2} . "\" y1=\"" . $item->{y1} . "\" y2=\"" 
					. $item->{y2} . "\" color=\"" . $item->{fg} . "\"";
			if (defined($item->{bg})) {
				print " bgcolor=\"" . $item->{bg} . "\"";
			}
			if ($item->{display} eq 'VolumeCurrent') {
				print " current=\"{VolumeCurrent}\" total=\"{VolumeTotal}\"";
			}
			if ($item->{display} eq 'PresentDuration') {
				print " current=\"{PresentProgress}\" "
						. " total=\"{PresentDuration}\"";
			}
			if ($item->{display} eq 'ReplayTime') {
				print " current=\"{ReplayPositionIndex}\" "
						. " total=\"{ReplayDurationIndex}\"";
			}
			print "/>\n";
			next;
		};
	}

	# display end tag
	print "  </display>\n";
}

# skin end tag
print "</skin>\n";
