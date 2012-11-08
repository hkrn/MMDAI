#!/usr/bin/perl

use strict;
use warnings;
use autodie;
use Cwd;
use File::Basename;
use File::Copy;
use File::Path qw(make_path);
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
my $opt_clang = 0;
my $opt_print_flags = 0;

GetOptions(
    'opencl'     => \$opt_opencl,
    'cg'         => \$opt_cg,
    'march'      => \$opt_march,
    'production' => \$opt_prod,
    'static'     => \$opt_static,
    'clang'      => \$opt_clang,
    'numcpu=i'   => \$opt_num_cpu,
    'flags'      => \$opt_print_flags,
    'help|?'     => \$opt_help,
    'man'        => \$opt_man,
) or pod2usage(2);
pod2usage(1) if $opt_help;
pod2usage(-exitstatus => 0, -verbose => 2) if $opt_man;

my $SOURCE_DIRECTORY_SUFFIX = '-src';
my $MMDAI_CHECKOUT_URI = 'git://github.com/hkrn/MMDAI.git';
my $MMDAI_DIRECTORY = 'MMDAI';
my $VPVL_DIRECTORY = 'libvpvl';
my $VPVL2_DIRECTORY = 'libvpvl2';
my $BULLET_CHECKOUT_URI = 'http://bullet.googlecode.com/svn/tags/bullet-2.77';
my $BULLET_DIRECTORY = 'bullet' . $SOURCE_DIRECTORY_SUFFIX;
my $ASSIMP_CHECKOUT_URI = 'https://assimp.svn.sourceforge.net/svnroot/assimp/tags/2.0';
my $ASSIMP_DIRECTORY = 'assimp' . $SOURCE_DIRECTORY_SUFFIX;
my $NVTT_CHECKOUT_URI = 'http://nvidia-texture-tools.googlecode.com/svn/trunk';
my $NVTT_DIRECTORY = 'nvtt' . $SOURCE_DIRECTORY_SUFFIX;
my $NVTT_REVISION = 1357;
my $PORTAUDIO_CHECKOUT_URI = 'https://subversion.assembla.com/svn/portaudio/portaudio/trunk';
my $PORTAUDIO_DIRECTORY = 'portaudio' . $SOURCE_DIRECTORY_SUFFIX;
my $PORTAUDIO_REVISION = 1788;
my $LIBAV_CHECKOUT_URI = 'git://git.libav.org/libav.git';
my $LIBAV_DIRECTORY = 'libav' . $SOURCE_DIRECTORY_SUFFIX;
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
];
my $CMAKE_NVTT_ARGS = [
];
my $CMAKE_VPVL_ARGS = [
    '-DVPVL_ENABLE_PROJECT:BOOL=OFF',
    '-DVPVL_OPENGL_RENDERER:BOOL=OFF',
    '-DVPVL_ENABLE_GLSL:BOOL=OFF',
    '-DVPVL_ENABLE_NVIDIA_CG:BOOL=OFF',
    '-DVPVL_ENABLE_OPENCL:BOOL=OFF',
    '-DVPVL_LINK_ASSIMP:BOOL=ON',
    '-DVPVL_LINK_QT:BOOL=OFF',
    '-DVPVL_BUILD_QT_RENDERER:BOOL=OFF',
];
my $CMAKE_VPVL2_ARGS = [
    '-DVPVL2_ENABLE_NVIDIA_CG:BOOL=' . ($opt_cg ? 'ON' : 'OFF'),
    '-DVPVL2_ENABLE_OPENCL:BOOL=' . ($opt_opencl ? 'ON' : 'OFF'),
    '-DVPVL2_ENABLE_PROJECT:BOOL=ON',
    '-DVPVL2_ENABLE_GLSL:BOOL=ON',
    '-DVPVL2_OPENGL_RENDERER:BOOL=ON',
    '-DVPVL2_LINK_ASSIMP:BOOL=ON',
    '-DVPVL2_LINK_QT:BOOL=ON',
    '-DVPVL2_LINK_NVTT:BOOL=ON',
    '-DVPVL2_BUILD_QT_RENDERER:BOOL=' . ($opt_static ? 'OFF' : 'ON'),
];
my $SCONS_PORTAUDIO_ARGS = [
    'enableTests=False',
    'enableShared=' . ($opt_static ? 'False' : 'True'),
    'enableStatic=' . ($opt_static ? 'True' : 'False'),
    'enableDebug=' . ($opt_prod ? 'False' : 'True'),
    'enableDebugOutput=' . ($opt_prod ? 'False' : 'True'),
];
my $CONFIGURE_LIBAV_ARGS = [
    '--enable-shared',
    '--disable-static',
    '--disable-ffmpeg',
    '--disable-avconv',
    '--disable-avplay',
    '--disable-avprobe',
    '--disable-avserver',
    '--disable-ffmpeg',
    '--disable-network',
    '--disable-bzlib',
    '--disable-libfreetype',
    '--disable-libopenjpeg',
    '--disable-decoders',
    '--disable-decoder=h264',
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

sub rewrite_cmake_flags {
    my ($cmake_args) = @_;
    my @args = (
        @$cmake_args,
        '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
        '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_static ? 'OFF' : 'ON'),
    );
    if ($opt_march) {
        push @args, '-DCMAKE_OSX_ARCHITECTURES="i386;x86_64"';
    }
    if ($opt_static) {
        push @args, '-DCMAKE_CXX_FLAGS=\'-fvisibility=hidden -fvisibility-inlines-hidden\'';
    }
    return @args;
}

sub rewrite_scons_flags {
    my ($scons_args) = @_;
    my @args = @$scons_args;
    if ($opt_march) {
        push @args, (
            'customCFlags=-arch i386 -arch x86_64',
            'customCxxFlags=-arch i386 -arch x86_64',
            'customLinkFlags=-arch i386 -arch x86_64',
        )
    }
    return @args;
}

sub build_with_cmake {
    my ($directory, $cmake_args) = @_;
    chdir $directory;
    mkdir $BUILD_DIRECTORY unless -d $BUILD_DIRECTORY;
    chdir $BUILD_DIRECTORY;
    my %ENV_backup = %ENV;
    if ($opt_clang) {
        $ENV{'CC'} = 'clang';
        $ENV{'CXX'} = 'clang++';
    }
    system 'cmake', rewrite_cmake_flags($cmake_args), '..';
    system 'make', '-j' . $opt_num_cpu;
    %ENV = %ENV_backup;
}

sub build_with_scons {
    my ($directory, $scons_args) = @_;
    chdir $directory;
    system 'scons', rewrite_scons_flags($scons_args);
    system 'scons', 'install';
}

sub build_with_configure {
    my ($directory, $configure_args, $do_install, $do_clean) = @_;
    my @args = @$configure_args;
    chdir $directory;
    system 'make clean >/dev/null 2>&1'; # ensure cleaning compiled object files
    system './configure', @args;
    system 'make', '-j' . $opt_num_cpu;
    system 'make', 'install' if $do_install;
    system 'make', 'clean' if $do_clean;
}

sub make_universal_binary {
    my ($path_i386, $path_x86_64, $path_universal, $libraries) = @_;
    if ($opt_march) {
        my $dh;
        my $path_universal_lib = File::Spec->catdir($path_universal, 'lib');
        opendir $dh, $path_universal_lib;
        my @files = map { File::Spec->catfile($path_universal_lib, $_) }
            grep { my $file = $_; scalar(grep { $file eq $_ } @$libraries) == 0 }
            File::Spec->no_upwards(readdir $dh);
        closedir $dh;
        foreach my $library (@$libraries) {
            my $i386_file = File::Spec->catfile($path_i386, 'lib', $library);
            my $x86_64_file = File::Spec->catfile($path_x86_64, 'lib', $library);
            my $universal_file = File::Spec->catfile($path_universal_lib, $library);
            system 'lipo', '-create', '-output', $universal_file, '-arch', 'i386', $i386_file, '-arch', 'x86_64', $x86_64_file;
        }
        for my $file (@files) {
            -f $file and unlink $file;
            -l $file and unlink $file;
        }
    }
}

sub make_library {
    my ($base_directory, $directory, $configure_args, $output_filenames) = @_;
    my $orig_cflags = $ENV{'CFLAGS'} || '';
    my $orig_cxxflags = $ENV{'CXXFLAGS'} || '';
    my $orig_ldflags = $ENV{'LDFLAGS'} || '';
    if ($opt_march) {
        my $path_i386 = File::Spec->catdir($base_directory, $directory, $BUILD_DIRECTORY . '_i386');
        my $path_x86_64 = File::Spec->catdir($base_directory, $directory, $BUILD_DIRECTORY . '_native');
        make_path $path_i386 unless -d $path_i386;
        $ENV{'CFLAGS'} = ($ENV{'CFLAGS32'} || '') . ' -arch i386';
        $ENV{'CXXFLAGS'} = ($ENV{'CXXFLAGS32'} || '') . ' -arch i386';
        $ENV{'LDFLAGS'} = ($ENV{'LDFLAGS32'} || '') . ' -arch i386';
        build_with_configure $directory, [ '--prefix=' . $path_i386, @$configure_args ], 1, 1;
        chdir $base_directory;
        $ENV{'CFLAGS'} = $orig_cflags;
        $ENV{'CXXLAGS'} = $orig_cxxflags;
        $ENV{'LDLAGS'} = $orig_ldflags;
        $ENV{'CFLAGS'} = ($ENV{'CFLAGS64'} || '') . ' -arch x86_64';
        $ENV{'CXXFLAGS'} = ($ENV{'CXXFLAGS64'} || '') . ' -arch x86_64';
        $ENV{'LDFLAGS'} = ($ENV{'LDFLAGS64'} || '') . ' -arch x86_64';
        make_path $path_x86_64 unless -d $path_x86_64;
        build_with_configure $directory, [ '--prefix=' . $path_x86_64, @$configure_args ], 1, 1;
        chdir $base_directory;
        make_universal_binary($path_i386, $path_x86_64, $path_x86_64, $output_filenames);
    }
    else {
        $ENV{'CFLAGS'} = $ENV{'CFLAGS64'} || '';
        $ENV{'CXXFLAGS'} = $ENV{'CXXFLAGS64'} || '';
        $ENV{'LDFLAGS'} = $ENV{'LDFLAGS64'} || '';
        my $path_x86_64 = File::Spec->catdir($base_directory, $directory, $BUILD_DIRECTORY . '_native');
        make_path $path_x86_64 unless -d $path_x86_64;
        build_with_configure $directory, [ '--prefix=' . $path_x86_64, @$configure_args ], 1, 1;
        chdir $base_directory;
    }
    $ENV{'CFLAGS'} = $orig_cflags;
    $ENV{'CXXLAGS'} = $orig_cxxflags;
    $ENV{'LDLAGS'} = $orig_ldflags;
}

if ($opt_print_flags) {
    my $output = ' -DLIBRARY_OUTPUT_PATH=`pwd`/lib';
    print '[bullet]', "\n", 'cmake ', join(' ', rewrite_cmake_flags($CMAKE_BULLET_ARGS)), $output, "\n\n";
    print '[assimp]', "\n", 'cmake ', join(' ', rewrite_cmake_flags($CMAKE_ASSIMP_ARGS)), "\n\n";
    print '[nvtt]', "\n", 'cmake ', join(' ', rewrite_cmake_flags($CMAKE_NVTT_ARGS)), $output, "\n\n";
    print '[vpvl]', "\n", 'cmake ', join(' ', rewrite_cmake_flags($CMAKE_VPVL_ARGS)), "\n\n";
    print '[vpvl2]', "\n", 'cmake ', join(' ', rewrite_cmake_flags($CMAKE_VPVL2_ARGS)), "\n\n";
    print '[portaudio]', "\n", 'scons ', join(' ', rewrite_scons_flags($SCONS_PORTAUDIO_ARGS)), "\n\n";
    print '[libav]', "\n", './configure ', join(' ', @$CONFIGURE_LIBAV_ARGS), "\n\n";
    exit(0);
}

# clone MMDAI sources
system 'git', 'clone', $MMDAI_CHECKOUT_URI, $MMDAI_DIRECTORY unless -d $MMDAI_DIRECTORY;
chdir $MMDAI_DIRECTORY;
# to resolve symlink, use Cwd::cwd()
my $base_directory = Cwd::cwd();
chdir $base_directory;
system 'git', 'pull', '--rebase';

# checkout bullet sources
system 'svn', 'checkout', $BULLET_CHECKOUT_URI, $BULLET_DIRECTORY unless -d $BULLET_DIRECTORY;
# append LIBRARY_OUTPUT_PATH dynamically
my $path = File::Spec->catdir($base_directory, $BULLET_DIRECTORY, $BUILD_DIRECTORY, 'lib');
@$CMAKE_BULLET_ARGS = ( @$CMAKE_BULLET_ARGS, '-DLIBRARY_OUTPUT_PATH=' . $path );
build_with_cmake $BULLET_DIRECTORY, $CMAKE_BULLET_ARGS;
chdir $base_directory;

# checkout assimp source
system 'svn', 'checkout', $ASSIMP_CHECKOUT_URI, $ASSIMP_DIRECTORY unless -d $ASSIMP_DIRECTORY;
my $regex = $opt_static ? 's/ADD_LIBRARY\(\s*assimp\s+SHARED/ADD_LIBRARY(assimp STATIC/gxms'
                        : 's/ADD_LIBRARY\(\s*assimp\s+STATIC/ADD_LIBRARY(assimp SHARED/gxms';
system 'perl', '-pi', '-e', $regex, File::Spec->catfile($ASSIMP_DIRECTORY, 'code', 'CMakeLists.txt');
build_with_cmake $ASSIMP_DIRECTORY, $CMAKE_ASSIMP_ARGS;
my $assimp_dir = File::Spec->catdir($base_directory, $ASSIMP_DIRECTORY, $BUILD_DIRECTORY);
my $assimp_lib_dir = File::Spec->catdir($assimp_dir, 'lib');
mkdir $assimp_lib_dir unless -d $assimp_lib_dir;
map { copy $_, $assimp_lib_dir } grep { (fileparse($_, '.so', '.dylib'))[2] ne '' } glob File::Spec->catdir($assimp_dir, 'code') . '/*';

if ($^O eq 'MacOS' or $^O eq 'darwin') {
    my $install_name = File::Spec->catfile($assimp_lib_dir, 'libassimp.dylib');
    system 'install_name_tool', '-id', $install_name, $install_name;
}
chdir $base_directory;

# checkout nvidia texture tools sources
system 'svn', 'checkout', '-r', $NVTT_REVISION, $NVTT_CHECKOUT_URI, $NVTT_DIRECTORY unless -d $NVTT_DIRECTORY;
# build directory should be same as configure does
$path = File::Spec->catdir($base_directory, $NVTT_DIRECTORY, 'build-' . $BUILD_DIRECTORY, 'lib');
# append LIBRARY_OUTPUT_PATH dynamically
@$CMAKE_NVTT_ARGS = ( @$CMAKE_NVTT_ARGS, '-DLIBRARY_OUTPUT_PATH=' . $path );
build_with_cmake $NVTT_DIRECTORY, $CMAKE_NVTT_ARGS;
chdir $base_directory;

# checkout portaudio
system 'svn', 'checkout', '-r', $PORTAUDIO_REVISION, $PORTAUDIO_CHECKOUT_URI, $PORTAUDIO_DIRECTORY unless -d $PORTAUDIO_DIRECTORY;
my $path_portaudio_native = File::Spec->catdir($base_directory, $PORTAUDIO_DIRECTORY, $BUILD_DIRECTORY . '_native');
my $new_SCONS_PORTAUDIO_ARGS = [ @$SCONS_PORTAUDIO_ARGS, 'prefix="' . $path_portaudio_native . '"' ];
make_path $path_portaudio_native unless -d $path_portaudio_native;
build_with_scons $PORTAUDIO_DIRECTORY, $new_SCONS_PORTAUDIO_ARGS;
chdir $base_directory;

# checkout libav
system 'git', 'clone', $LIBAV_CHECKOUT_URI, $LIBAV_DIRECTORY unless -d $LIBAV_DIRECTORY;
chdir $LIBAV_DIRECTORY;
system 'git', 'checkout', $LIBAV_TAG;
chdir $base_directory;

if ($opt_march) {
    # build libav for i386
    my @i386_args = @$CONFIGURE_LIBAV_ARGS;
    my $path_i386 = File::Spec->catdir($base_directory, $LIBAV_DIRECTORY, $BUILD_DIRECTORY . '_i386');
    push @i386_args, (
        '--prefix=' . $path_i386,
        '--arch=i386',
        '--cc=clang -m32',
    );
    push @i386_args, '--disable-debug' if $opt_prod;
    make_path $path_i386 unless -d $path_i386;
    build_with_configure $LIBAV_DIRECTORY, \@i386_args, 1, 1;
    chdir $base_directory;
    # build libav for x86_64
    my @x86_64_args = @$CONFIGURE_LIBAV_ARGS;
    my $path_x86_64 = File::Spec->catdir($base_directory, $LIBAV_DIRECTORY, $BUILD_DIRECTORY . '_native');
    push @x86_64_args, (
        '--prefix=' . $path_x86_64,
        '--arch=x86_64',
        '--cc=clang',
    );
    push @x86_64_args, '--disable-debug' if $opt_prod;
    make_path $path_x86_64 unless -d $path_x86_64;
    build_with_configure $LIBAV_DIRECTORY, \@x86_64_args, 1, 1;
    chdir $base_directory;
    # create universal binary with lipo
    make_universal_binary($path_i386, $path_x86_64, $path_x86_64,
        ['libavcodec.dylib', 'libavformat.dylib', 'libavutil.dylib', 'libswscale.dylib']);
}
else {
    my @args = @$CONFIGURE_LIBAV_ARGS;
    my $path_libav = File::Spec->catdir($base_directory, $LIBAV_DIRECTORY, $BUILD_DIRECTORY . '_native');
    push @args, '--prefix=' . $path_libav;
    push @args, '--disable-debug' if $opt_prod;
    build_with_configure $LIBAV_DIRECTORY, \@args, 1, 1;
}
chdir $base_directory;

# build libvpvl
build_with_cmake $VPVL_DIRECTORY, $CMAKE_VPVL_ARGS;
chdir $base_directory;

# build libvpvl2
build_with_cmake $VPVL2_DIRECTORY, $CMAKE_VPVL2_ARGS;
chdir $base_directory;

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
   -clang           compile sources with clang instead of gcc
   -flags           print all (common) build flags to each libraries
   -numcpu=<core>   specify number of CPU cores

=head1 DESCRIPTION

This script helps building libvpvl/libvpvl2 and dependencies.

 ./build.pl -opencl -cg -march -static -production # for MacOSX build
 ./build.pl -cg -static -production -clang # for Linux build

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

=item B<-clang>

Compiles sources on CMake with clang/clang++ instead of gcc/g++ for NVTT (NVIDIA texture tools).

=item B<-numcpu>

Specify number of CPU cores to pass '-j' option in make.

=back

=cut

