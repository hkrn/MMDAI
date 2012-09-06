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
my $opt_no_bundle = 0;

GetOptions(
    'opencl'     => \$opt_opencl,
    'cg'         => \$opt_cg,
    'march'      => \$opt_march,
    'production' => \$opt_prod,
    'static'     => \$opt_static,
    'nobundle'   => \$opt_no_bundle,
    'numcpu=i'   => \$opt_num_cpu,
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
my $PORTAUDIO_CHECKOUT_URI = 'https://subversion.assembla.com/svn/portaudio/portaudio/trunk';
my $PORTAUDIO_DIRECTORY = 'portaudio' . $SOURCE_DIRECTORY_SUFFIX;
my $PORTAUDIO_REVISION = 1788;
my $LIBJPEG_CHECKOUT_URI = 'http://www.ijg.org/files/jpegsrc.v8d.tar.gz';
my $LIBJPEG_DIRECTORY = 'libjpeg' . $SOURCE_DIRECTORY_SUFFIX;
my $LIBPNG_CHECKOUT_URI = 'git://libpng.git.sourceforge.net/gitroot/libpng/libpng';
my $LIBPNG_DIRECTORY = 'libpng' . $SOURCE_DIRECTORY_SUFFIX;
my $LIBPNG_TAG = 'v1.5.12';
my $DEVIL_CHECKOUT_URI = 'http://downloads.sourceforge.net/project/openil/DevIL/1.7.8/DevIL-1.7.8.tar.gz?r=&ts=&use_mirror=jaist';
my $DEVIL_DIRECTORY = 'devil' . $SOURCE_DIRECTORY_SUFFIX;
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
my $CMAKE_VPVL_ARGS = [
    '-DVPVL_ENABLE_PROJECT:BOOL=OFF',
    '-DVPVL_OPENGL_RENDERER:BOOL=OFF',
    '-DVPVL_ENABLE_GLSL:BOOL=OFF',
    '-DVPVL_ENABLE_NVIDIA_CG:BOOL=OFF',
    '-DVPVL_ENABLE_OPENCL:BOOL=OFF',
    '-DVPVL_LINK_ASSIMP:BOOL=ON',
    '-DVPVL_LINK_QT:BOOL=OFF',
    '-DVPVL_BUILD_SDL:BOOL=OFF',
    '-DVPVL_BUILD_QT_RENDERER:BOOL=OFF',
    '-DCMAKE_CXX_FLAGS=-W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings',
];
my $CMAKE_VPVL2_ARGS = [
    '-DVPVL2_ENABLE_NVIDIA_CG:BOOL=' . ($opt_cg ? 'ON' : 'OFF'),
    '-DVPVL2_ENABLE_OPENCL:BOOL=' . ($opt_opencl ? 'ON' : 'OFF'),
    '-DVPVL2_ENABLE_PROJECT:BOOL=ON',
    '-DVPVL2_ENABLE_GLSL:BOOL=ON',
    '-DVPVL2_OPENGL_RENDERER:BOOL=ON',
    '-DVPVL2_LINK_ASSIMP:BOOL=ON',
    '-DVPVL2_LINK_QT:BOOL=ON',
    '-DVPVL2_LINK_DEVIL:BOOL=ON',
    '-DVPVL2_BUILD_SDL:BOOL=OFF',
    '-DVPVL2_BUILD_QT_RENDERER:BOOL=ON',
    '-DCMAKE_CXX_FLAGS=-W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings',
];
my $SCONS_PORTAUDIO_ARGS = [
    'enableTests=False',
    'enableShared=' . ($opt_static ? 'False' : 'True'),
    'enableStatic=' . ($opt_static ? 'True' : 'False'),
    'enableDebug=' . ($opt_prod ? 'False' : 'True'),
    'enableDebugOutput=' . ($opt_prod ? 'False' : 'True'),
];
my $CONFIGURE_LIBJPEG_ARGS = [
    '--enable-shared',
    '--disable-static',
];
my $CONFIGURE_LIBPNG_ARGS = [
    '--enable-shared',
    '--disable-static',
];
my $CONFIGURE_DEVIL_ARGS = [
    '--enable-ILU=yes',
    '--enable-ILUT=yes',
    '--enable-blp=no',
    '--enable-dcx=no',
    '--enable-dicom=no',
    '--enable-doom=no',
    '--enable-fits=no',
    '--enable-gif=no',
    '--enable-icns=no',
    '--enable-icon=no',
    '--enable-iff=no',
    '--enable-ilbm=no',
    '--enable-iwi=no',
    '--enable-lif=no',
    '--enable-mdl=no',
    '--enable-mp3=no',
    '--enable-pcd=no',
    '--enable-pcx=no',
    '--enable-pic=no',
    '--enable-pix=no',
    '--enable-pnm=no',
    '--enable-psd=no',
    '--enable-psp=no',
    '--enable-pxr=no',
    '--enable-raw=no',
    '--enable-rot=no',
    '--enable-sgi=no',
    '--enable-sun=no',
    '--enable-texture=no',
    '--enable-tpl=no',
    '--enable-utx=no',
    '--enable-vtf=no',
    '--enable-wal=no',
    '--enable-wbmp=no',
    '--enable-wdp=no',
    '--enable-xpm=no',
    '--disable-sdltest',
    '--disable-x11',
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

sub build_with_cmake {
    my ($directory, $cmake_args, $force_dynamic) = @_;
    my @args = (
        @$cmake_args,
        '-DCMAKE_BUILD_TYPE:STRING=' . $BUILD_TYPE,
        '-DBUILD_SHARED_LIBS:BOOL=' . ($opt_static ? 'OFF' : 'ON'),
    );
    if ($opt_march) {
        push @args, '-DCMAKE_OSX_ARCHITECTURES=i386;x86_64';
    }
    if ($opt_static and !$force_dynamic) {
        push @args, '-DCMAKE_CXX_FLAGS=-fvisibility=hidden -fvisibility-inlines-hidden';
    }
    chdir $directory;
    mkdir $BUILD_DIRECTORY unless -d $BUILD_DIRECTORY;
    chdir $BUILD_DIRECTORY;
    system 'cmake', @args, '..';
    system 'make', '-j' . $opt_num_cpu;
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
build_with_cmake $BULLET_DIRECTORY, $CMAKE_BULLET_ARGS, 0;
chdir $base_directory;

# checkout assimp source
system 'svn', 'checkout', $ASSIMP_CHECKOUT_URI, $ASSIMP_DIRECTORY unless -d $ASSIMP_DIRECTORY;
my $regex = $opt_static ? 's/ADD_LIBRARY\(\s*assimp\s+SHARED/ADD_LIBRARY(assimp STATIC/gxms'
                        : 's/ADD_LIBRARY\(\s*assimp\s+STATIC/ADD_LIBRARY(assimp SHARED/gxms';
system 'perl', '-pi', '-e', $regex, File::Spec->catfile($ASSIMP_DIRECTORY, 'code', 'CMakeLists.txt');
build_with_cmake $ASSIMP_DIRECTORY, $CMAKE_ASSIMP_ARGS, 1;
my $assimp_dir = File::Spec->catdir($base_directory, $ASSIMP_DIRECTORY, $BUILD_DIRECTORY);
my $assimp_lib_dir = File::Spec->catdir($assimp_dir, 'lib');
mkdir $assimp_lib_dir unless -d $assimp_lib_dir;
map { copy $_, $assimp_lib_dir } grep { (fileparse($_, '.so', '.dylib'))[2] ne '' } glob File::Spec->catdir($assimp_dir, 'code') . '/*';

if ($^O eq 'MacOS' or $^O eq 'darwin') {
    my $install_name = File::Spec->catfile($assimp_lib_dir, 'libassimp.dylib');
    system 'install_name_tool', '-id', $install_name, $install_name;
}
chdir $base_directory;

# checkout portaudio
system 'svn', 'checkout', '-r', $PORTAUDIO_REVISION, $PORTAUDIO_CHECKOUT_URI, $PORTAUDIO_DIRECTORY unless -d $PORTAUDIO_DIRECTORY;
build_with_scons $PORTAUDIO_DIRECTORY, $SCONS_PORTAUDIO_ARGS;
chdir $base_directory;

# save current environment variables
my %env_backup = %ENV;
$ENV{'PATH'} = '/usr/bin:/bin';
$ENV{'PKG_CONFIG_PATH'} = '/usr/lib/pkgconfig';

if ($opt_static && !$opt_no_bundle) {
    # checkout libjpeg
    unless (-d $LIBJPEG_DIRECTORY) {
        system 'wget', $LIBJPEG_CHECKOUT_URI;
        system 'tar', '-xvzf', 'jpegsrc.v8d.tar.gz';
        system 'mv', 'jpeg-8d', $LIBJPEG_DIRECTORY;
    }
    make_library($base_directory, $LIBJPEG_DIRECTORY, $CONFIGURE_LIBJPEG_ARGS, [ 'libjpeg.dylib', 'libjpeg.8.dylib' ]);
    
    # checkout libpng
    system 'git', 'clone', $LIBPNG_CHECKOUT_URI, $LIBPNG_DIRECTORY unless -d $LIBPNG_DIRECTORY;
    chdir $LIBPNG_DIRECTORY;
    system 'git', 'checkout', $LIBPNG_TAG;
    chdir $base_directory;
    make_library($base_directory, $LIBPNG_DIRECTORY, $CONFIGURE_LIBPNG_ARGS, [ 'libpng.dylib', 'libpng15.15.dylib' ]);
}

# checkout devil
unless (-d $DEVIL_DIRECTORY) {
    system 'wget', $DEVIL_CHECKOUT_URI;
    system 'tar', '-jxvf', 'DevIL-1.7.8.tar.gz';
    system 'mv', 'devil-1.7.8', $DEVIL_DIRECTORY;
    chdir $base_directory;
}

my $libjpeg_path = File::Spec->catdir($base_directory, $LIBJPEG_DIRECTORY);
my $libpng_path = File::Spec->catdir($base_directory, $LIBPNG_DIRECTORY);
if ($opt_march) {
    my $libjpeg_lib32_path = File::Spec->catdir($libjpeg_path, $BUILD_DIRECTORY . '_i386');
    my $libjpeg_lib64_path = File::Spec->catdir($libjpeg_path, $BUILD_DIRECTORY . '_native');
    my $libpng_lib32_path = File::Spec->catdir($libpng_path, $BUILD_DIRECTORY . '_i386');
    my $libpng_lib64_path = File::Spec->catdir($libpng_path, $BUILD_DIRECTORY . '_native');
    my $ldflags32 = '-L' . $libjpeg_lib32_path . '/lib -L' . $libpng_lib32_path . '/lib';
    my $cflags32  = $ldflags32 . ' -I' . $libjpeg_lib32_path . '/include -I' . $libpng_lib32_path . '/include'; 
    my $ldflags64 = '-L' . $libjpeg_lib64_path . '/lib -L' . $libpng_lib64_path . '/lib';
    my $cflags64 = $ldflags64 . ' -I' . $libjpeg_lib64_path . '/include -I' . $libpng_lib64_path . '/include'; 
    $ENV{'CFLAGS32'} = $ENV{'CXXFLAGS32'} = $cflags32;
    $ENV{'LDFLAGS32'} = $ldflags32;
    $ENV{'CFLAGS64'} = $ENV{'CXXFLAGS64'} = $cflags64;
    $ENV{'LDFLAGS64'} = $ldflags64;
}
else {
    my $libjpeg_lib64_path = File::Spec->catdir($libjpeg_path, $BUILD_DIRECTORY . '_native');
    my $libpng_lib64_path = File::Spec->catdir($libpng_path, $BUILD_DIRECTORY . '_native');
    my $ldflags64 = '-L' . $libjpeg_lib64_path . '/lib -L' . $libpng_lib64_path . '/lib';
    my $cflags64 = $ldflags64 . ' -I' . $libjpeg_lib64_path . '/include -I' . $libpng_lib64_path . '/include'; 
    $ENV{'CFLAGS64'} = $ENV{'CXXFLAGS64'} = $cflags64;
    $ENV{'LDFLAGS64'} = $ldflags64;
}
chdir $base_directory;
make_library($base_directory, $DEVIL_DIRECTORY,
	$opt_prod ? $CONFIGURE_DEVIL_ARGS : [ @$CONFIGURE_DEVIL_ARGS, '--enable-debug' ],
    [ 'libIL.dylib', 'libIL.1.dylib', 'libILU.dylib', 'libILU.1.dylib', 'libILUT.dylib', 'libILUT.1.dylib' ]);
%ENV = %env_backup;

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
build_with_cmake $VPVL_DIRECTORY, $CMAKE_VPVL_ARGS, 0;
chdir $base_directory;

# build libvpvl2
build_with_cmake $VPVL2_DIRECTORY, $CMAKE_VPVL2_ARGS, 0;
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
   -nobundle        doesn't bundle libraries (libjpeg and libpng)
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

=item B<-nobundle>

Builds DevIL without building libjpeg and libpng (for Linux platform).

=item B<-numcpu>

Specify number of CPU cores to pass '-j' option in make.

=back

=cut

