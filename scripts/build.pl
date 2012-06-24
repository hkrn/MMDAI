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
my $opt_static = 0;
my $opt_num_cpu = 1;
my $opt_help = 0;
my $opt_man = 0;
my $opt_march = 0;
GetOptions(
    'opencl'     => \$opt_opencl,
    'cg'         => \$opt_cg,
    'march'      => \$opt_march,
    'production' => \$opt_prod,
    'static'     => \$opt_static,
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
my $PORTAUDIO_CHECKOUT_URI = 'https://subversion.assembla.com/svn/portaudio/portaudio';
my $PORTAUDIO_DIRECTORY = 'portaudio';
my $PORTAUDIO_REVISION = 1788;
my $LIBAV_CHECKOUT_URI = 'git://git.libav.org/libav.git';
my $LIBAV_DIRECTORY = 'libav';
my $LIBAV_TAG = 'v0.8.3';
my $BUILD_TYPE = $opt_prod ? 'Release' : 'Debug';
my $BUILD_DIRECTORY = lcfirst $BUILD_TYPE;
my $CMAKE_BULLET_ARGS = [
    '-DBUILD_DEMOS:BOOL=OFF',
    '-DBUILD_EXTRAS:BOOL=OFF',
    '-DBUILD_MINICL_OPENCL_DEMOS:BOOL=OFF',
    '-DBUILD_CPU_DEMOS:BOOL=OFF',
];
my $CMAKE_ASSIMP_ARGS = [
    '-DBUILD_ASSIMP_TOOLS:BOOL=OFF',
    '-DENABLE_BOOST_WORKAROUND:BOOL=ON',
    '-DCMAKE_BUILD_TYPE:STRING=Debug',
];
my $CMAKE_VPVL_ARGS = [
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
my $SCONS_PORTAUDIO_ARGS = [
    'enableTests=False',
    'enableShared=' . ($opt_static ? 'False' : 'True'),
    'enableStatic=' . ($opt_static ? 'True' : 'False'),
    'enableDebug=' . ($opt_prod ? 'False' : 'True'),
    'enableDebugOutput=' . ($opt_prod ? 'False' : 'True'),
];
my $CONFIGURE_LIBAV_ARGS = [
    '--disable-bzlib',
    '--disable-libfreetype',
    '--disable-libopenjpeg',
    '--disable-decoders',
    '--enable-decoder=flac',
    '--enable-decoder=pcm_s16le',
    '--disable-encoders',
    '--enable-encoder=png',
    '--enable-encoder=pcm_s16le',
    '--disable-parsers',
    '--disable-demuxers',
    '--enable-demuxer=aiff',
    '--enable-demuxer=flac',
    '--enable-demuxer=wav',
    '--disable-muxers',
    '--enable-muxer=mov',
    '--disable-protocols',
    '--enable-protocol=file',
    '--disable-filters',
    '--disable-bsfs',
    '--disable-indevs',
    '--disable-outdevs',
    '--enable-zlib',
];

sub build_with_cmake {
    my ($directory, $cmake_args, $build_on_plane) = @_;
    my @args = (
        @$cmake_args,
        '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
        '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_static ? 'OFF' : 'ON'),
    );
    if ($opt_march) {
        push @args, '-DCMAKE_OSX_ARCHITECTURES=i386;x86_64';
    }
    chdir $directory;
    if ($build_on_plane) {
        system 'cmake', @args;
    }
    else {
        mkdir $BUILD_DIRECTORY unless -d $BUILD_DIRECTORY;
        chdir $BUILD_DIRECTORY;
        system 'cmake', @args, '..';
    }
    system 'make', '-j' . $opt_num_cpu;
    chdir ($build_on_plane ? '..' : '../..');
}

sub build_with_scons {
    my ($directory, $scons_args) = @_;
    my @args = @$scons_args;
    if ($opt_march) {
        push @args, (
            'customCFlags=-arch i386 -arch x86_64',
            'customCxxFlags=-arch i386 -arch x86_64',
            'customLinkFlags=-arch i386 -arch x86_64',
        )
    }
    chdir $directory;
    system 'scons', @args;
    chdir '..';
}

sub build_with_configure {
    my ($directory, $configure_args, $do_install, $do_clean) = @_;
    my @args = @$configure_args;
    if ($opt_static) {
        push @args, (
            '--enable-static',
            '--disable-shared',
        );
    }
    else {
        push @args, (
            '--enable-shared',
            '--disable-static',
        );
    }
    chdir $directory;
    system './configure', @args;
    system 'make', '-j' . $opt_num_cpu;
    system 'make', 'install' if $do_install;
    system 'make', 'clean' if $do_clean;
    chdir '..';
}

# clone MMDAI sources
system 'git', 'clone', $MMDAI_CHECKOUT_URI, $MMDAI_DIRECTORY unless -d $MMDAI_DIRECTORY;
system 'git', 'pull', '--rebase';
chdir $MMDAI_DIRECTORY;

# checkout bullet sources
system 'svn', 'checkout', $BULLET_CHECKOUT_URI, $BULLET_DIRECTORY unless -d $BULLET_DIRECTORY;
# append LIBRARY_OUTPUT_PATH dynamically
my $path = File::Spec->catdir(getcwd(), $BULLET_DIRECTORY, $BUILD_DIRECTORY, 'lib');
@$CMAKE_BULLET_ARGS = ( @$CMAKE_BULLET_ARGS, '-DLIBRARY_OUTPUT_PATH=' . $path );
build_with_cmake $BULLET_DIRECTORY, $CMAKE_BULLET_ARGS, 0;

# checkout assimp source
system 'svn', 'checkout', $ASSIMP_CHECKOUT_URI, $ASSIMP_DIRECTORY unless -d $ASSIMP_DIRECTORY;
build_with_cmake $ASSIMP_DIRECTORY, $CMAKE_ASSIMP_ARGS, 1;

# checkout portaudio
system 'svn', 'checkout', '-r', $PORTAUDIO_REVISION, $PORTAUDIO_CHECKOUT_URI, $PORTAUDIO_DIRECTORY unless -d $PORTAUDIO_DIRECTORY;
build_with_scons $PORTAUDIO_DIRECTORY, $SCONS_PORTAUDIO_ARGS;

system 'git', 'clone', $LIBAV_CHECKOUT_URI, $LIBAV_DIRECTORY unless -d $LIBAV_DIRECTORY;
chdir $LIBAV_DIRECTORY;
system 'git', 'checkout', $LIBAV_TAG;
chdir '..';
if ($opt_march) {
    my @i386_args = @$CONFIGURE_LIBAV_ARGS;
    my $path_i386 = File::Spec->catdir(getcwd(), $LIBAV_DIRECTORY, $BUILD_DIRECTORY, 'libav_i386');
    push @i386_args, '--prefix=' . $path_i386;
    system 'mkdir', '-p', $path_i386;
    build_with_configure $LIBAV_DIRECTORY, \@i386_args, 1, 1;
    my @x86_64_args = @$CONFIGURE_LIBAV_ARGS;
    my $path_x86_64 = File::Spec->catdir(getcwd(), $LIBAV_DIRECTORY, $BUILD_DIRECTORY, 'libav_x86_64');
    push @x86_64_args, '--prefix=' . $path_x86_64;
    build_with_configure $LIBAV_DIRECTORY, \@x86_64_args, 1, 1;
}
else {
    my @args = @$CONFIGURE_LIBAV_ARGS;
    my $path_libav = File::Spec->catdir(getcwd(), $LIBAV_DIRECTORY, $BUILD_DIRECTORY, 'libav');
    push @args, '--prefix=' . $path_libav;
    build_with_configure $LIBAV_DIRECTORY, \@args, 1, 1;
}

# build libvpvl
build_with_cmake $VPVL_DIRECTORY, $CMAKE_VPVL_ARGS, 0;

# build libvpvl2
build_with_cmake $VPVL2_DIRECTORY, $CMAKE_VPVL2_ARGS, 0;

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
   -march           enable building multiple architectures
   -production      build as production
   -static          build as static library
   -numcpu=<core>   specify number of CPU cores

=head1 DESCRIPTION

This script helps building libvpvl/libvpvl2 and dependencies.

 ./build.pl -opencl -march -static -production # for MacOSX build
 ./build.pl -production # for Linux build

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

=item B<-march>

Enable building multiple architectures (i386 and x86_64). This option affects only on MacOSX.

=item B<-production>

Builds libvpvl/libvpvl2 and dependencies as production (no debug symbols) instead of debug.

=item B<-static>

Builds libvpvl/libvpvl2 and dependencies as static library instead of dynamic shared library.

=item B<-numcpu>

Specify number of CPU cores to pass '-j' option in make.

=back

=cut

