#! /usr/bin/env perl -w
use 5.10.0;
use strict;
use FindBin;
use lib "$FindBin::Bin/../../../vendor/openssl/";
use lib "$FindBin::Bin/../../../vendor/openssl/util/perl";
use File::Basename;
use File::Spec::Functions qw/:DEFAULT abs2rel rel2abs/;
use File::Copy;
use File::Path qw/make_path/;
use with_fallback qw(Text::Template);

# Read configdata from ../openssl/configdata.pm that is generated
# with ../openssl/Configure options arch

# CFX mention: `perl Configure VC-WIN64A-masm`
# (using `mingw-w64-x86_64-perl`)
use configdata;

my $asm = "asm";
my $arch = "VC-WIN64A-masm";

my $src_dir = "../../../vendor/openssl";
my $arch_dir = "./";
my $base_dir = "$arch_dir/";

my $is_win = ($arch =~/^VC-WIN/);
my $makefile = "Makefile";
# Generate arch dependent header files with Makefile
my $buildinf = "crypto/buildinf.h";
my $progs = "apps/progs.h";
my $cmd1 = "cd $src_dir; nmake -f $makefile clean build_generated $buildinf $progs;";
system($cmd1) == 0 or die "Error in system($cmd1)";

# Copy and move all arch dependent header files into config/archs
make_path("$base_dir/crypto/include/internal", "$base_dir/include/openssl",
          {
           error => \my $make_path_err});
if (@$make_path_err) {
  for my $diag (@$make_path_err) {
    my ($file, $message) = %$diag;
    die "make_path error: $file $message\n";
  }
}
#copy("$src_dir/configdata.pm", "$base_dir/") or die "Copy failed: $!";
copy("$src_dir/include/openssl/opensslconf.h",
     "$base_dir/openssl/") or die "Copy failed: $!";
move("$src_dir/include/crypto/bn_conf.h",
     "$base_dir/crypto/") or die "Move failed: $!";
move("$src_dir/include/crypto/dso_conf.h",
     "$base_dir/crypto/") or die "Move failed: $!";
copy("$src_dir/$buildinf",
     "$base_dir/crypto/") or die "Copy failed: $!";
move("$src_dir/$progs",
     "$base_dir") or die "Copy failed: $!";

# read openssl source lists from configdata.pm
my @libapps_srcs = ();
foreach my $obj (@{$unified_info{sources}->{'apps/libapps.a'}}) {
    push(@libapps_srcs, ${$unified_info{sources}->{$obj}}[0]);
}

my @libssl_srcs = ();
foreach my $obj (@{$unified_info{sources}->{libssl}}) {
  push(@libssl_srcs, ${$unified_info{sources}->{$obj}}[0]);
}

my @libcrypto_srcs = ();
my @generated_srcs = ();
foreach my $obj (@{$unified_info{sources}->{libcrypto}}) {
  my $src = ${$unified_info{sources}->{$obj}}[0];
  # .S files should be preprocessed into .s
  if ($unified_info{generate}->{$src}) {
    # .S or .s files should be preprocessed into .asm for WIN
    $src =~ s\.[sS]$\.asm\ if ($is_win);
    push(@generated_srcs, $src);
  } else {
    push(@libcrypto_srcs, $src);
  }
}

my @apps_openssl_srcs = ();
foreach my $obj (@{$unified_info{sources}->{'apps/openssl'}}) {
  push(@apps_openssl_srcs, ${$unified_info{sources}->{$obj}}[0]);
}

# Generate all asm files and copy into config/archs
foreach my $src (@generated_srcs) {
  $src =~ s/\\/\//g;
  my $cmd = "pushd $src_dir; CC=cl ASM=masm nmake -f $makefile $src;" .
    "cp --parents $src ../../code/vendor/openssl/asm/; popd";
  system("$cmd") == 0 or die "Error in system($cmd)";
}

# Clean Up
my $cmd2 ="cd $src_dir; nmake -f $makefile clean; nmake -f $makefile distclean;";
system($cmd2) == 0 or die "Error in system($cmd2)";
