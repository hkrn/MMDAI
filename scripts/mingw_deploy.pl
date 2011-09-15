#!/usr/bin/perl

use strict;
use File::Copy qw(copy);

my $MINGW_ROOT = '/usr/i686-pc-mingw32/sys-root/mingw';
my $MINGW_BIN = $MINGW_ROOT . '/bin';
my $MINGW_LIB = $MINGW_ROOT . '/lib';

my $release = $ARGV[0] eq '-release' ? 1 : 0;
my $deploy = $ARGV[1] eq '-deploy' ? 1 : 0;

sub copy_dll {
  my ($path, $dir, $targets) = @_;
  foreach my $target (@$targets) {
    $target .= '.dll';
    my $src = $path . '/'. $target;
    my $dest = $dir . '/' . $target;
    unless (-e $src) {
      warn 'source: ', $src, ' doesn\'t exists.', "\n";
      next;
    }
    my $ret = $deploy ? copy($src, $dest)
                      : symlink($src, $dest);
    print 'deploying ', $target, '...', "\n";
    warn 'failed deploying ', $target, "\n" unless $ret;
  }
}

system('rm -f */*.dll');

my @targets_in_bin = (
  'libgcc_s_sjlj-1',
  'libpng14-14',
  'libportaudio-2',
  'libstdc++-6',
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
  'glew32',
  'libHTSEngine',
  'libOpenJTalk',
  'libjulius',
);

my @targets_bullet = (
  'libBulletCollision',
  'libBulletDynamics',
  'libBulletSoftBody',
  'libLinearMath',
);

my $vpvl_version = '0.3.0';
my @targets_vpvl = (
  sprintf('%s.%s', ($release ? 'vpvl' : 'vpvl_debug'), $vpvl_version),
);

my $dir = $release ? 'release' : 'debug';
my $bullet_lib = '../bullet/' . $dir . '-mingw/lib';
my $vpvl_lib = '../libvpvl/' . $dir . '-mingw/lib';

copy_dll($MINGW_BIN, $dir, \@targets_in_bin);
copy_dll($MINGW_LIB, $dir, \@targets_in_lib);
copy_dll($bullet_lib, $dir, \@targets_bullet);
copy_dll($vpvl_lib, $dir, \@targets_vpvl);

if ($deploy) {
  my $cmd = <<"EOS";
mkdir MMDAI;
cp $dir/*.exe $dir/*.dll MMDAI;
cp -r $MINGW_ROOT/lib/qt4/plugins MMDAI;
cd MMDAI/plugins;
rm -rf bearer graphicssystems iconengines script sqldrivers;
find -name '*d4.dll*' -exec rm -f {} \\;
cd ../..;
zip -1 -r MMDAI.zip MMDAI;
rm -rf MMDAI;
EOS
  system($cmd);
}

