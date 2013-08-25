#!/usr/bin/perl
use strict;
use warnings;
use IO::File;
use constant DEBUG => 1;

my @allurls;

my $filename = shift;
my $package = ""; # current processing package
my $version; # version of current processing package
my $dir; # dir for current processing package

# command sytax definitions
my $supported_protocols = "https|http|ftp|file|git|svn";
my $make_commands = "nothing|extract|dirextract|patch(time)?(-(\\d+))?|pmove|premove|plink|pdircreate";
my $install_commands = "install -.+|install_file|install_bin";

my $patchesdir .= "\$(buildprefix)/Patches";

sub load ($$);

# File preprocessor begins.
sub load ($$)
{
  my ( $filename, $ignore ) = @_;

  my $fh = new IO::File $filename;

  if ( ! $fh )
  {
    return undef if $ignore;
    die "can't open $filename\n";
  }

  my $foutname;
  ($foutname = $filename) =~ s#(.*).pre#$1#;
  warn "output to $foutname";
  open FILE, "+>", "$foutname";
  
  my $lines = "";
  my $start = 0;
  my $nextpkg = 0;
  while ( <$fh> )
  {
    if ( $_ =~ m#^\]\]END# )
    {
      $start = 0;
      next;
    }
    if ( $_ =~ m#^BEGIN\[\[# )
    {
      $start = 1;
      next;
    }
    #don't touch Makefile conditional
    if (not $start or ($_ =~ m#^(ifdef |ifndef |ifeq |ifneq |else|endif)#) )
    {
      print FILE $_;
      next;
    }
    # remove comments
    $_ =~ s/#.*$//;
    if ($_ =~ m#^\s+$#) {
      next;
    }
    chomp $_;
    $_ =~ s/^\s+//; #remove leading spaces
    $_ =~ s/\s+$//; #remove trailing spaces

    if ($_ =~ m#;$#) {
      if ($nextpkg < 3) {
        die "no info"
      }
      $nextpkg += 1;
      $_ =~ s/;$//;
    }

    if ($nextpkg == 0) {
      process_begin($_);
      $nextpkg += 1;
    } elsif ($nextpkg == 1) {
      process_version($_);
      $nextpkg += 1;
    } elsif ($nextpkg == 2) {
      process_dir($_);
      $nextpkg += 1;
    } elsif ($nextpkg == 3) {
      process_block($_);
    } elsif ($nextpkg == 4) {
      $nextpkg = 0;
    }

  }
  close FILE;
}

sub process_block ($)
{
  warn "$package,$version,$dir  :  $_" if DEBUG;
  my $out;

  $out = "DEPENDS_$package += " . process_depends($_) . "\n";
  print FILE subs_vars($out);
  $out = "PREPARE_$package += " . process_prepare($_) . "\n";
  print FILE subs_vars($out);
  $out = "SRC_URI_$package += " . process_sources($_) . "\n";
  print FILE subs_vars($out);
  $out = "INSTALL_$package += " . process_install($_) . "\n";
  print FILE subs_vars($out);

  print FILE subs_vars(process_download($_) . "\n")
  
}

sub process_rule($) {

  warn "parse: " . $_ . "\n" if DEBUG;

  my $f = "";
  my $l = $_;
  my @l = ();
#  my @l = split( /:/ , $l );
  if ($l =~ m#\;#)
  {
    my @semi = split( /;/ , $l );
    foreach (@semi) {
      my @part = split( m#:(?=/{2})#, $_);
      @l = (@l,@part);
    }
  } else {
    @l = split( /:/ , $l );
  }

#  s#^(\w+)?:($supported_protocols)://([^:]+):.*$#
  
  my $cmd = shift @l;
  my $p;

  if ( $cmd =~ m#^($supported_protocols)# )
  {
    $p = $cmd;    
    $cmd = "extract";
  }
  elsif ( $cmd =~ m#^($make_commands|$install_commands)# )
  {
    $p = shift @l;
  }
  
  my $url = shift @l;
  #print "test $url \n";
  if ( not $url or $url !~ m#^//.*# )
  {
    $p = "none";
  }
  else
  {
    $url = $p . ":" . $url;
  }

  if ( $p ne "none" )
  {
    my @a = split("/", $url);
    $f = $a[-1];
  }

  my %args = ();
  my @argv = ();
  my $arg;
  while($arg = shift @l)
  {
    if ($arg =~ m/(\w+)=(.*)/)
    {
      $args{$1} = $2 ;
      #warn "arg " . $1 . ' = ' . $2 . "\n";
    } else {
      push(@argv, $arg);
      #warn "argv " . $arg . "\n";
    }
  }

  if ( $url and $url =~ m#^svn://# )
  {
      $f = $package . ".svn"
  }
  if ( $url =~ m#^file://# )
  {
      $f = $url;
      $f =~ s#^file://##;
      $f = "$patchesdir/$f";
  }
  elsif ( $url =~ m#^($supported_protocols)# )
  {
      $f = "\$(archivedir)/$f";
  }

  warn "protocol: $p file: $f command: $cmd url: $url\n" if DEBUG;

  return ($p, $f, $cmd, $url, \%args, \@argv);
}


sub process_depends ($)
{
  my $output = "";

    my ($p, $f) = process_rule($_);
    return if ( $p eq "none" );

    if ( $p =~ m#^(file)$# or $p =~ m#^($supported_protocols)$#  )
    {
      $output .= "$f ";
    }
    else
    {
      die "can't recognize protocol " . $_;
    }

  return $output;
}

sub process_dir ($)
{
  $dir = "\$(workprefix)/" . $_;
  my $out = "DIR_$package = $dir\n";
  print FILE subs_vars($out);

  my $output;

#echo '====> prepare $package\' &&
#echo '====> install $package ' &&

  $output =  "PREPARE_$package = ( rm -rf \$(DIR_$package) || /bin/true)" . "\n";
  $output .= "INSTALL_$package = /bin/true\n";

  $output .= "DEPSCLEANUP_$package = rm .deps/$package" . "\n";
  $output .= "DEPSCLEANUP += .deps/$package .deps/$package.do_compile .deps/$package.do_prepare" . "\n";
  $output .= "LIST_CLEAN += $package-clean" . "\n";

  $output .= "DISTCLEANUP_$package = rm -rf \$(DIR_$package)" . "\n";
  $output .= "DISTCLEANUP += \$(DIR_$package)" . "\n";
  $output .= "DEPSDISTCLEANUP_$package = rm .deps/$package .deps/$package.do_compile .deps/$package.do_prepare" . "\n";
  $output .= "LIST_DISTCLEAN += $package-distclean" . "\n";

  $output .= "DEPENDS_$package = \$(DEPDIR)/$package.version_\$(PKGV_$package)-\$(PKGR_$package)" . "\n";

  if ($version =~ m#^git|svn$#)
  {
    $output .= "UPDATEPKGV_$package = [ -d \$(DIR_$package) ] && (touch \$\@ -d `cd \$(DIR_$package) && \$(git_version) && cd -`) || true" . "\n";
    $output .= "LIST_AUTOPKGV += \$(DEPDIR)/$package.version_\$(PKGV_$package)-\$(PKGR_$package)" . "\n";
  } else {
    $output .= "UPDATEPKGV_$package = touch \$\@" . "\n";
  }
  $output .= "\$(DEPDIR)/$package.version_%:\n\t\$(UPDATEPKGV_$package)" . "\n";

  print FILE subs_vars($output);
}


sub process_prepare ($)
{

  my $output = "";
  my $autoversion = "";
  my $updateversion = "";

    my @args = split( /:/, $_ );
    my ($p, $f, $cmd, $url, $opts_ref) = process_rule($_);
    my %opts = %$opts_ref;

    my $subdir = "";
    $subdir = "/" . $opts{"sub"} if $opts{"sub"};
    local @_ = ($p, $f);

    if ( $cmd !~ m#$make_commands# and $p !~ m#(git|svn)# )
    {
      return;
    }
    
      $output .= " && cd \$(workprefix) && ";
    
    if ( ($cmd eq "rpm" || $cmd eq "extract") and $p !~ m#(git|svn)#)
    {
      if ( $_[1] =~ m#\.tar\.bz2$# )
      {
        $output .= "bunzip2 -cd " . $f . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.tar\.gz$# )
      {
        $output .= "gunzip -cd " . $f . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.tgz$# )
      {
        $output .= "gunzip -cd " . $f . " | TAPE=- tar -x";
      }
      elsif ( $_[1] =~ m#\.tar\.xz$# )
      {
      $output .= "tar -xJf " . $f;
      }
      elsif ( $_[1] =~ m#\.exe$# )
      {
        $output .= "cabextract " . $f;
      }
      elsif ( $_[1] =~ m#\.zip$# )
      {
        $output .= "unzip -P $dir " . $f;
      }
      elsif ( $_[1] =~ m#\.src\.rpm$# )
      {
        $output .= "rpm \${DRPM} -Uhv " . $f;
      }
      elsif ( $_[1] =~ m#\.cvs# )
      {
        my $target = $dir;
        if ( @_ > 2 )
        {
          $target = $_[2] 
        }
        $output .= "cp -a " . $f . " " . $target;
      }
      else
      {
        warn "can't recognize type of archive " . $f . " skip";
        $output .= "true";
      }
    }
    elsif ( $p eq "svn" )
    {
      if ( not $opts{"r"} )
      {
         $output .= "(cd " . $f . " && svn update) && ";
      }
      $output .= "(cd " . $f . "; svn up -r " . $opts{"r"} . "; cd -) && " if $opts{"r"};
      $output .= "cp -a " . $f . $subdir . " " . $dir;
      $autoversion = "\$(eval export PKGV_$package = \$(shell cd $f && \$(svn_version)))";
    }
    elsif ( $p eq "git" )
    {
      my $branch = "master";
      $branch = $opts{"b"} if $opts{"b"};
      $output .= "(cd $f && git fetch && git checkout $branch && git pull --rebase origin $branch; cd -) && ";
      $output .= "(cd " . $f . "; git checkout " . $opts{"r"} . "; cd -) && " if $opts{"r"};
      $updateversion = "[ -d $f ] && (true" . $output . "(touch \$(buildprefix)/\$\@ -d `cd $f && \$(git_version) && cd -`) ) || true";
      $output .= "cp -a " . $f . $subdir . " " . $dir;
      $autoversion = "\$(eval export PKGV_$package = \$(shell cd $f && \$(git_version)))";
    }
    elsif ( $cmd eq "nothing" )
    {
      $output .= "cp $f $dir";
    }
    elsif ( $cmd eq "dirextract" )
    {
      $output .= "( mkdir " . $dir . " || /bin/true ) && ";
      $output .= "( cd " . $dir . "; ";

      if ( $_[1] =~ m#\.tar\.bz2$# )
      {
        $output .= "bunzip2 -cd " . $f . " | tar -x";
      }
      elsif ( $_[1] =~ m#\.tar\.gz$# )
      {
        $output .= "gunzip -cd " . $f . " | tar -x";
      }
      elsif ( $_[1] =~ m#\.exe$# )
      {
        $output .= "cabextract " . $f;
      }
      else
      {
        die "can't recognize type of archive " . $f;
      }

      $output .= " )";
    }
    elsif ( $cmd =~ m/patch(time)?(-(\d+))?/ )
    {
      local $_;
      $_ = "-p1 ";
      $_ = "-p$3 " if defined $3;
      $_ .= "-Z " if defined $1;

      if ( $_[1] =~ m#\.bz2$# )
      {
        $output .= "( cd " . $dir . " && chmod +w -R .; bunzip2 -cd " . $f . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.deb\.diff\.gz$# )
      {
        $output .= "( cd " . $dir . "; gunzip -cd " . $f . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.gz$# )
      {
        $output .= "( cd " . $dir . " && chmod +w -R .; gunzip -cd " . $f . " | patch $_ )";
      }
      elsif ( $_[1] =~ m#\.spec\.diff$# )
      {
        $output .= "( cd SPECS && patch $_ < " . $f . " )";
      }
      else
      {
        $output .= "( cd " . $dir . " && chmod +w -R .; patch $_ < " . $f . " )";
      }
    }
    elsif ( $cmd eq "rpmbuild" )
    {
      $output .= "rpmbuild \${DRPMBUILD} -bb -v --clean --target=sh4-linux " . $f;
    }
    elsif ( $cmd eq "pmove" )
    {
      $output .= "mv " . $args[1] . " " . $args[2];
    }
    elsif ( $cmd eq "premove" )
    {
      $output .= "( rm -rf " . $args[1] . " || /bin/true )";
    }
    elsif ( $cmd eq "plink" )
    {
      $output .= "( ln -sf " . $args[1] . " " . $args[2] . " || /bin/true )";
    }
    elsif ( $cmd eq "pdircreate" )
    {
      $output .= "( mkdir -p " . $args[1] . " )";
    }
    else
    {
      die "can't recognize command @_";
    }

  $output .= "\nAUTOPKGV_$package = $autoversion" if $autoversion;
  $output .= "\nUPDATEPKGV_$package = $updateversion" if $updateversion;
  return $output
}

sub process_version ($)
{
  $version = $_;
  my $out;
  $out  = "VERSION_$package = $version\n";
  $out .= "PKGV_$package = \$(VERSION_$package)" . "\n";
  print FILE subs_vars($out);
}

sub process_begin ($)
{
  $package = $_;
}

sub process_install ($)
{

  my $rule = $_;
  my ($p, $f, $cmd, $url, $opts_ref, $dest_ref) = process_rule($rule);
  my @dest = @$dest_ref;

  if ( $cmd =~ m#$make_commands# )
  {
    return "";
  }

  @_ = split ( /:/, $rule );
  $_ = shift @_;

  my $output = " && ";

  if ( $cmd =~ m#$install_commands# )
  {
    if ( $cmd =~ m#install_file|install_bin# )
    {
      $cmd =~ y/a-z/A-Z/ ;
      $output .= "\$\($cmd\) $f @dest";
    }
    else
    {
      $cmd =~ s/install/\$\(INSTALL\)/ ;
      $output .=  "$cmd $f @dest" ;
    }
  }
  elsif ( $_ eq "make" )
  {
    $output .= "\$\(MAKE\) " . join " ", @_;
  }
  elsif ( $_ eq "install" )
  {
    $output .= "\$\(INSTALL\) " . join " ", @_;
  }
  elsif ( $_ eq "rpminstall" )
  {
    $output .= "rpm \${DRPM} --ignorearch -Uhv RPMS/sh4/" . join " ", @_;
  }
  elsif ( $_ eq "shellconfigadd" )
  {
    $output .= "export HCTDINST \&\& HOST/bin/target-shellconfig --add " . join " ", @_;
  }
  elsif ( $_ eq "initdconfigadd" )
  {
    $output .= "export HCTDINST \&\& HOST/bin/target-initdconfig --add " . join " ", @_;
  }
  elsif ( $_ eq "move" )
  {
    $output .= "mv " . join " ", @_;
  }
  elsif ( $_ eq "remove" )
  {
    $output .= "rm -rf " . join " ", @_;
  }
  elsif ( $_ eq "mkdir" )
  {
    $output .= "mkdir -p " . join " ", @_;
  }
  elsif ( $_ eq "link" )
  {
    $output .= "ln -sf " . join " ", @_;
  }
  elsif ( $_ eq "archive" )
  {
    $output .= "TARGETNAME-ar cru " . join " ", @_;
  }
  elsif ( $_ =~ m/^rewrite-(libtool|pkgconfig|dependency)/ )
  {
    $output .= "perl -pi -e \"s,^libdir=.*\$\$,libdir='TARGET/usr/lib',\" ". join " ", @_ if $1 eq "libtool";
    $output .= "perl -pi -e \"s, /usr/lib, TARGET/usr/lib,g if /^dependency_libs/\"  ". join " ", @_ if $1 eq "dependency";
    $output .= "perl -pi -e \"s,^prefix=.*\$\$,prefix=TARGET/usr,\" " . join " ", @_ if $1 eq "pkgconfig";
  }
  else
  {
    die "can't recognize rule \"$rule\"";
  }

  return $output;
}

sub process_uninstall_rule ($)
{
  my $rule = shift;
  my ($p, $f, $cmd) = process_rule($rule);
  
  if ( $cmd =~ m#$make_commands# )
  {
    return "";
  }

  @_ = split ( /:/, $rule );
  $_ = shift @_;

  my $output = "";

  if ( $_ eq "make" )
  {
    $output .= "\$\(MAKE\) " . join " ", @_;
  }
  elsif ( $_ eq "install" )
  {
    $output .= "\$\(INSTALL\) " . join " ", @_;
  }
  elsif ( $_ eq "rpminstall" )
  {
    $output .= "rpm \${DRPM} --ignorearch -Uhv RPMS/sh4/" . join " ", @_;
  }
  elsif ( $_ eq "shellconfigdel" )
  {
    $output .= "export HCTDUNINST \&\& HOST/bin/target-shellconfig --del " . join " ", @_;
  }
  elsif ( $_ eq "initdconfigdel" )
  {
    $output .= "export HCTDUNINST \&\& HOST/bin/target-initdconfig --del " . join " ", @_;
  }
  elsif ( $_ eq "move" )
  {
    $output .= "mv " . join " ", @_;
  }
  elsif ( $_ eq "remove" )
  {
    $output .= "rm -rf " . join " ", @_;
  }
  elsif ( $_ eq "link" )
  {
    $output .= "ln -sf " . join " ", @_;
  }
  elsif ( $_ eq "archive" )
  {
    $output .= "TARGETNAME-ar cru " . join " ", @_;
  }
  elsif ( $_ =~ m/^rewrite-(libtool|pkgconfig)/ )
  {
    $output .= "perl -pi -e \"s,^libdir=.*\$\$,libdir='TARGET/lib',\"  ". join " ", @_ if $1 eq "libtool";
    $output .= "perl -pi -e \"s,^prefix=.*\$\$,prefix=TARGET,\" " . join " ", @_ if $1 eq "pkgconfig";
  }
  else
  {
    die "can't recognize rule \"$rule\"";
  }

  return $output;
}

sub process_sources ($)
{
  my $output = "";

    my ($p, $f, $cmd, $url, $opts_ref) = process_rule($_);
    my %opts = %$opts_ref;
    return if ( $p eq "none" );
    my $rev = "";
    $rev = ":r$opts{'r'}" if $opts{"r"};
    $output .= "$url$rev ";

  return "$output"
}

sub process_download ($)
{

  my $head;
  my $output = "";

    my ($p, $f, $cmd, $url, $opts_ref) = process_rule($_);
    my %opts = %$opts_ref;
    return if ( $p eq "file" || $p eq "none" );
    
    $_ =~ s/$cmd:// if ($cmd ne "");
    
    $f =~ s/\\//;

    my $file = $f;
    $file =~ s/\$\(archivedir\)//;

    my $suburl = subs_vars($_);
    if( $suburl ~~ @allurls )
    {
       #warn $suburl . "\n";
       next;
    }
    push(@allurls, $suburl);
    
    #warn "download: " . $url . "\n";
    
    $head .= " " . $f;
    $output .= $f . ":\n\tfalse";

    if ( $_ =~ m#^ftp://# )
    {
      $output .= " || \\\n\twget -c --passive-ftp -P \$(archivedir) " . $_;
    }
    elsif ( $_ =~ m#^http://# )
    {
      $output .= " || \\\n\twget -c -P \$(archivedir) " . $_;
    }
    elsif ( $_ =~ m#^https://# )
    {
      $output .= " || \\\n\twget -c -P \$(archivedir) " . $_;
    }
#     elsif ( $_ =~ m#^CMD_CVS # )
#     {
#       $output .= " || \\\n\tcd \$(archivedir) && " . $_;
#       my $cvsstring = $_;
#       $cvsstring =~ s/ co / up /;
#       $outputupdate .= "\$(archivedir)" . $file . "up:\n\tfalse";
#       $outputupdate .= " || \\\n\tcd \$(archivedir) && " . $cvsstring;
#     }
    elsif ( $_ =~ m#^svn://# )
    {
      my $tmpurl = $url;
      $url =~ s#svn://#http://# ;
      $output .= " || \\\n\tsvn checkout $url" . " " . $f;
    }
    elsif ( $url =~ m#^git://# )
    {
      my $tmpurl = $url;
      $tmpurl =~ s#git://#$opts{"protocol"}://#  if $opts{"protocol"} ;
      $tmpurl =~ s#ssh://#git\@# if $opts{"protocol"} eq "ssh";
      $output .= " || \\\n\tgit clone $tmpurl " . $f;
      $output .= " -b " . $opts{"b"} if $opts{"b"};
    }

    elsif ( $f =~ m/gz$/ )
    {
      $output .= " || \\\n\twget -c -P \$(archivedir) ftp://ftp.stlinux.com/pub/stlinux/2.0/ST_Linux_2.0/RPM_Distribution/sh4-target-glibc-packages/" . $file;
      $output .= "\n\t\@touch \$\@";
    }
  #   elsif ( $file =~ m/cvs$/ )
  #   {
  #     $output .= " || \\\n\twget -c ftp://xxx.com/pub/tufsbox/cdk/src/" . $file;
  #     $filerep =~ s/\.cvs//;
  #     $output .= "\n\t\@touch -r \$(archivedir) " . $filerep . "/CVS \$\@";
  #     $outputupdate .= "\n\t\@touch -r \$(archivedir) " . $filerep . "/CVS \$\(subst cvsup,cvs,\$\@\)";
  #     $outputupdate .= "\n\n";
  #   }
    else
    {
      $output .= " || \\\n\twget -c -P \$(archivedir) http://tuxbox.berlios.de/pub/tuxbox/cdk/src/" . $file;
      $output .= "\n\t\@touch \$\@";
    }
    $output .= "\n\n";

  return "$output"

}

# $output =~ s#CMD_CVS#\$\(CMD_CVS\)#g;
# $outputupdate =~ s#CMD_CVS#\$\(CMD_CVS\)#g;
# 
# print $head . "\n\n" . $output . "\n\n" . $outputupdate . "\n";

#TODO:
#die "please specify a filename and at least one package" if $#ARGV < 2;


sub subs_vars($)
{
  my $output = shift;
  $output =~ s#TARGETNAME#\$\(target\)#g;
  $output =~ s#TARGETS#\$\(prefix\)\/\$\*cdkroot#g;
  $output =~ s#TARGET#\$\(targetprefix\)#g;
  $output =~ s#HCTDINST#HHL\_CROSS\_TARGET\_DIR\=\$\(prefix\)\/\$\*cdkroot#g;
  $output =~ s#HCTDUNINST#HHL\_CROSS\_TARGET\_DIR\=\$\(targetprefix\)#g;
  $output =~ s#HOST#\$\(hostprefix\)#g;
  $output =~ s#BUILD#\$\(buildprefix\)#g;
  $output =~ s#PKDIR#\$\(packagingtmpdir\)#g;
  $output =~ s#\{PV\}#$version#g;
  $output =~ s#\{PF\}#../Files/$package#g;
  my $dashpackage = $package;
  $dashpackage =~ s#_#\-#g;
  $output =~ s#\{PN\}#$dashpackage#g;
  return $output
}

load ( $filename, 0 );

#print subs_vars($allout) . "\n";
