#!/usr/bin/perl

use strict;
use autodie;
use Cwd qw(getcwd);
use File::Spec;
use Getopt::Long;
use Pod::Usage;

my $opt_opencl = 0;
my $opt_cg = 0;
my $opt_prod = 0;
my $opt_shared = 0;
my $opt_num_cpu = 1;
my $opt_help = 0;
my $opt_man = 0;
GetOptions(
    'opencl'     => \$opt_opencl,
    'cg'         => \$opt_cg,
    'production' => \$opt_prod,
    'shared'     => \$opt_shared,
    'numcpu=i'   => \$opt_num_cpu,
    'help|?'     => \$opt_help,
    'man'        => \$opt_man,
) or pod2usage(2);
pod2usage(1) if $opt_help;
pod2usage(-exitstatus => 0, -verbose => 2) if $opt_man;

my $MMDAI_CHECKOUT_URI = 'git://github.com/hkrn/MMDAI.git';
my $MMDAI_DIRECTORY = 'MMDAI';
my $VPVL_DIRECTORY = 'libvpvl';
my $VPVL2_DIRECTORY = 'libvpvl2';
my $BULLET_CHECKOUT_URI = 'http://bullet.googlecode.com/svn/tags/bullet-2.77';
my $BULLET_DIRECTORY = 'bullet';
my $ASSIMP_CHECKOUT_URI = 'https://assimp.svn.sourceforge.net/svnroot/assimp/tags/2.0';
my $ASSIMP_DIRECTORY = 'assimp';
my $BUILD_TYPE = $opt_prod ? 'Release' : 'Debug';
my $BUILD_DIRECTORY = lcfirst $BUILD_TYPE;
my $CMAKE_BULLET_ARGS = [
    '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
    '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_shared ? 'ON' : 'FALSE'),
    '-DBUILD_DEMOS:BOOL=OFF',
    '-DBUILD_EXTRAS:BOOL=OFF',
    '-DBUILD_MINICL_OPENCL_DEMOS:BOOL=OFF',
    '-DBUILD_CPU_DEMOS:BOOL=OFF',
];
my $CMAKE_ASSIMP_ARGS = [
    '-DBUILD_ASSIMP_TOOLS:BOOL=ON',
    '-DENABLE_BOOST_WORKAROUND:BOOL=ON',
    '-DCMAKE_BUILD_TYPE:STRING=Debug',
];
my $CMAKE_VPVL_ARGS = [
   '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
   '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_shared ? 'ON' : 'FALSE'),
   '-DVPVL_ENABLE_PROJECT:BOOL=OFF',
   '-DVPVL_OPENGL_RENDERER:BOOL=OFF',
   '-DVPVL_ENABLE_GLSL:BOOL=OFF',
   '-DVPVL_ENABLE_NVIDIA_CG:BOOL=OFF',
   '-DVPVL_LINK_ASSIMP:BOOL=ON',
   '-DVPVL_BUILD_SDL:BOOL=OFF',
   '-DVPVL_LINK_QT:BOOL=OFF',
   '-DVPVL_BUILD_QT_RENDERER:BOOL=OFF',
   '-DVPVL_BUILD_QT_WITH_OPENCV:BOOL=OFF',
   '-DVPVL_ENABLE_OPENCL:BOOL=OFF',
   '-DCMAKE_CXX_FLAGS=-W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings',
];
my $CMAKE_VPVL2_ARGS = [
   '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
   '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_shared ? 'ON' : 'FALSE'),
   '-DVPVL2_ENABLE_NVIDIA_CG:BOOL=' . ($opt_cg ? 'ON' : 'OFF'),
   '-DVPVL2_ENABLE_OPENCL:BOOL=' . ($opt_opencl ? 'ON' : 'OFF'),
   '-DVPVL2_ENABLE_PROJECT:BOOL=ON',
   '-DVPVL2_OPENGL_RENDERER:BOOL=ON',
   '-DVPVL2_ENABLE_GLSL:BOOL=ON',
   '-DVPVL2_LINK_ASSIMP:BOOL=ON',
   '-DVPVL2_BUILD_SDL:BOOL=OFF',
   '-DVPVL2_LINK_QT:BOOL=ON',
   '-DVPVL2_BUILD_QT_RENDERER:BOOL=ON',
   '-DVPVL2_BUILD_QT_WITH_OPENCV:BOOL=OFF',
   '-DCMAKE_CXX_FLAGS=-W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings',
];

sub build {
   my ($directory, $cmake_args, $build_on_plane) = @_;
   chdir $directory;
   if ($build_on_plane) {
       system 'cmake', @$cmake_args;
   }
   else {
       mkdir $BUILD_DIRECTORY unless -d $BUILD_DIRECTORY;
       chdir $BUILD_DIRECTORY;
       system 'cmake', @$cmake_args, '..';
   }
   system 'make', '-j' . $opt_num_cpu;
   chdir ($build_on_plane ? '..' : '../..');
}

# clone MMDAI sources
system 'git', 'clone', $MMDAI_CHECKOUT_URI, $MMDAI_DIRECTORY unless -d $MMDAI_DIRECTORY;
chdir $MMDAI_DIRECTORY;

# checkout bullet sources
system 'svn', 'checkout', $BULLET_CHECKOUT_URI, $BULLET_DIRECTORY unless -d $BULLET_DIRECTORY;
# append LIBRARY_OUTPUT_PATH dynamically
my $path = File::Spec->catdir(getcwd(), $BULLET_DIRECTORY, $BUILD_DIRECTORY, 'lib');
@$CMAKE_BULLET_ARGS = ( @$CMAKE_BULLET_ARGS, '-DLIBRARY_OUTPUT_PATH=' . $path );
build $BULLET_DIRECTORY, $CMAKE_BULLET_ARGS, 0;

# checkout assimp source
system 'svn', 'checkout', $ASSIMP_CHECKOUT_URI, $ASSIMP_DIRECTORY unless -d $ASSIMP_DIRECTORY;
build $ASSIMP_DIRECTORY, $CMAKE_ASSIMP_ARGS, 1;

# build libvpvl
build $VPVL_DIRECTORY, $CMAKE_VPVL_ARGS, 0;

# build libvpvl2
build $VPVL2_DIRECTORY, $CMAKE_VPVL2_ARGS, 0;

__END__

=head1 NAME

build.pl - builds libvpvl/libvpvl2 and dependencies automatically

=head1 SYNOPSIS

./build.pl [options]

 Options:
   -help            brief help message
   -man             full documentation
   -opencl          enable OpenCL build option
   -cg              enable NVIDIA Cg build option
   -production      build as production
   -shared          build as shared library
   -numcpu          specify number of CPU cores

=head1 OPTIONS

=over 8

=item B<-help>

Print a brief help message and exits.

=item B<-man>

Prints the manual page and exits.

=item B<-opencl>

Enables OpenCL build option. This option requires OpenCL headers and libraries to build.

=item B<-cg>

Enables Cg build option. This option requires NVIDIA Cg headers and libraries to build.
For more info, see <http://developer.nvidia.com/cg-toolkit>.

=item B<-production>

Builds libvpvl/libvpvl2 and dependencies as production (no debug symbols) instead of debug.

=item B<-shared>

Builds libvpvl/libvpvl2 and dependencies as shared library instead of static library.

=item B<-numcpu>

Specify number of CPU cores to pass '-j' option in make.

=back

=cut

