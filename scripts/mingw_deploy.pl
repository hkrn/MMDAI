#!/usr/bin/perl

use strict;
use File::Copy qw(copy);

my $mingw_root = '/usr/i686-pc-mingw32/sys-root/mingw';
my $mingw_bin = $mingw_root . '/bin';
my $mingw_lib = $mingw_root . '/lib';

my $release = $ARGV[0] eq '-release' ? 1 : 0;
my $deploy = $ARGV[1] eq '-deploy' ? 1 : 0;

system('rm -f *.dll');

my @targets_in_bin = (
  'libgcc_s_sjlj-1',
  'libstdc++-6',
  'libpng14-14',
  'zlib1'
);
if ($release) {
  @targets_in_bin = (
    @targets_in_bin,
    'QtCore4',
    'QtGui4',
    'QtOpenGL4',
  );
}
else {
  @targets_in_bin = (
    @targets_in_bin,
    'QtCored4',
    'QtGuid4',
    'QtOpenGLd4',
  );
}

my @targets_in_lib = (
  'libglee',
  'libBulletCollision',
  'libBulletDynamics',
  'libLinearMath',
  'libMMDAI',
  'libMMDME',
);

foreach my $target (@targets_in_bin) {
  $target .= '.dll';
  $deploy ? copy($mingw_bin . '/' . $target, $target)
          : symlink($mingw_bin . '/' . $target, $target);
  print 'deploying ', $target, '...', "\n";
}

foreach my $target (@targets_in_lib) {
  $target .= '.dll';
  $deploy ? copy($mingw_lib . '/' . $target, $target)
          : symlink($mingw_lib . '/' . $target, $target);
  print 'deploying ', $target, '...', "\n";
}

if ($deploy) {
  system('mkdir QtMMDAI; cp *.exe *.dll QtMMDAI; zip -r QtMMDAI.zip QtMMDAI; rm -rf QtMMDAI');
}

