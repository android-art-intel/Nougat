#!/usr/bin/perl

#
# Copyright (C) 2015 Intel Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#

use strict;
use warnings;
use diagnostics;

if (-e './config.inc') {require "./config.inc"; }
else {print "Config file wasn't found!"; exit; }
our %C;
our $quiet=0;
our $benchmarks_directory_name;
if ( not defined $ARGV[0] )
{
    print "\n$C{'white'}Usage:$C{'end'}\t$C{'red'}processing.pl {results path}$C{'end'} $C{'green'}\[basebuild='build name'][%build type%][test]$C{'end'}\n";
    print "\n$C{'white'}Default behaviour:$C{'end'}\n\t\tScript will collect scores and mark as median only values from newly added benchmarks data.\n\t\tIt will ignore already evaluated configurations. To change this add $C{'yellow'}override$C{'end'} argument.\n\t\tIt will ignore real median score if there is already manually changed value in database. To change this add $C{'yellow'}updatemedian$C{'end'} argument. \n";
    print "\n$C{'white'}Description:$C{'end'}\n";
    print "$C{'yellow'}\tbasebuild='build name'$C{'end'} - use specified build as base\n";
    print "$C{'yellow'}\t%buildtype%$C{'end'} - can be one of the following values: $C{'white'}weekly$C{'end'}, $C{'white'}daily$C{'end'}, $C{'white'}tip$C{'end'}, $C{'white'}patch$C{'end'} or $C{'white'}other$C{'end'}\n";
    print "$C{'yellow'}\ttest$C{'end'} - do not add anything in database\n";
    print "$C{'yellow'}\treplace$C{'end'} - delete all previously collected data and replace with new one\n";
    print "$C{'yellow'}\tupdatemedian$C{'end'} - recalculate median values\n";
    print "$C{'yellow'}\toverride$C{'end'} - ignore evaluation status\n";

}
else
{
    my $source_path;
    my $tmp;
    our $donotcollect=0;
    our $replace=0;
    our $updatemedian=0;
    our $backend;
    our %Collectrules;
    our %Config;
    our %Results;
    our @benchmarks;
    our @branches;
    our @backends;
    our @allbenchmarks;
    our @devices;
    our $use_bronze_silver_detection=0;
    my $device_type;
    my $branch;
    our $backend_suffix = "";
    our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
    our $dbhandler="";
    our $verbose = 0;
    our $extraopts="";

    my $backendstr;
    my $benchsuffix;
    my $benchprintsuffix="";
    my $suffixfilter="";
    my $benchressuffix;
    my @availablebenchmarks;
    our @notavailablebenchmarks;

my @all_dirs=();
our $dvmmode="";

my %hostslist;
my @hostslist;

print "Source: $source\n";


require "./modules.inc";
require "./genresultfile.inc";
require "./processresults.inc";
require "./compute.inc";
require "./collectdata.inc";
require "./getrunprops.inc";

$dbhandler = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);
if (not $dbhandler) {print "Can't connect to database. Exiting...\n"; exit;}

print "Loading data:\n";

@branches = LoadBranchesModules_fromDB ();
@backends = LoadBackendsModules_fromDB ();
@devices = LoadDevicesModules_fromDB ();
@allbenchmarks = LoadBenchmarksModules_fromDB ();
@benchmarks = (@allbenchmarks);

if (not defined $ARGV[0]) { print "\e[01;38;05;196m[ Give me source path!!!! Exiting... ]\e[0m\n\n\n "; exit;}
if (scalar @ARGV == 0 ) {  print "\e[01;38;05;196m[ Incorrect source path!!!! Exiting... ]\e[0m\n\n\n "; exit;}

#branch detection
my $argnum= scalar @ARGV;
my @used_args = ();
my %result_dirs = ();
my $begin;
my $end;

foreach my $tbr (@branches)
{
    $tmp = $Config{"branches:".$tbr.":name"}."_";
    for (my $i=0;$i<$argnum;$i++)
    {
	if ($ARGV[$i] =~ /$tmp/)
	{
	    $Config{"rd:".$i.":branch"} = $Config{"branches:".$tbr.":name"};
	    if (-e $ARGV[$i])
	    {
		print "Path with results:".$ARGV[$i]."\n";
		@used_args = (@used_args,$i);
		$source_path = $ARGV[$i];
		chomp ($source_path);
		$source_path .= "/";
		$result_dirs{"orig:".$i} = $source_path;
		$source_path =~ s/$benchmarks_directory_name//;
		$result_dirs{$i} = $source_path;
		
# $Config{"branches:".$Results{"props:$i:branch"}.":number"};
		$Config{"rd:".$i.":branchid"} = $Config{"branches:".$tbr.":number"};

#weeknum detection
my $detector=0;
		if ($source eq "NSK")
		{
		    if ( $source_path =~ /[(KK),(LP),(MD),(ND)]+_[W,R]+[0-9]*[a-z,.,0-9]*/ ) {  $Config{"rd:".$i.":week_num"} = $&; $begin=$`; $end=$'; $detector=1; }  #weeknum detection
		    else { print "\e[01;38;05;196m[ Week num not found!!! Exiting... ]\e[0m\n"; exit(0); }
		    if ($Config{"branches:".$tbr.":use_bronze_silver_detection"}==1 )
		    {
			$use_bronze_silver_detection=1;
			my $suff1= "-BRONZE";
			my $suff2= "-SILVER";
			if ($Config{"branches:".$tbr.":inverse_detection"}==1)
			{
			    my $rrr = $suff1;
			    $suff1 = $suff2;
			    $suff2 = $rrr;
			}
			$backend_suffix = $suff2;
			if ($source_path =~ /bronze/)
			{
			    $backend_suffix = "-BRONZE";
			}
			if ($source_path =~ /silver/)
			{
			    $backend_suffix = "-SILVER";
			}
		    }
		    if ($source_path =~ /$tmp/  )
		    {
			$Config{"rd:".$i.":firmware"} =$&.$'; #$&
			$Config{"rd:".$i.":firmware"}=~ s/\///g;
			$end =~ s/userdebug//g;
			$end =~ s/eng//g;
			$end =~ s/\///g;
			$end = $`;
			$tmp = $end =~ /([0-9]+_[0-9]+_){1,}/g;
			if ($tmp ne "")
			{
			    $Config{"rd:".$i.":build_type"} = "patch";
			    $Config{"rd:".$i.":patch_num"} = $&.$';
			    chop ($Config{"rd:".$i.":patch_num"});
			    $Config{"rd:".$i.":patch_num"} =~ s/\///g;
			}
			$Config{"rd:".$i.":buildtag"} = $`;
			$Config{"rd:".$i.":buildtag"} =~ s/_//;
			$Config{"rd:".$i.":buildtag"} =~ s/\///g;
			if ($detector==1)
			{
			    $Config{"rd:".$i.":week_num"} =~ /[W,R]+[0-9]*/g;
			    $Config{"rd:".$i.":week_num"} = $&;
			    $tmp = $';
			    $Config{"rd:".$i.":week_num"} = $&;
			}
			else {die;}
			$Config{"rd:".$i.":week_day"} = lc($tmp);
			if (not defined $Config{"rd:".$i.":build_type"} )
			{
			    if ($tmp eq ""){   $Config{"rd:".$i.":build_type"} = "weekly";}
			    else { $Config{"rd:".$i.":build_type"} = "daily";}
			}
			
			if ($Config{"rd:".$i.":buildtag"}=~ /-tip-/)
			{ $Config{"rd:".$i.":build_type"} = "daily";
			}
		    }
#		    else {  print "\e[01;38;05;196m[ Trying another branch...]\e[0m\n";  }
		    else {  print "\e[01;38;05;196m[ Can't parse path!!! Exiting... ]\e[0m\n"; exit(0); }
		    $Config{"rd:".$i.":buildtag"}  =~ s/-bronze//;
		}
		else
		{
		die "\n[unknown source]\n";
		}

		if ($source_path =~ /[(201),0-9]+[0-1]+[0-9]+(-RC)+[0-9]*/ and $Config{"branches:".$tbr.":staging_branch"} > 0 )
		{
		$tbr = $Config{"branches:".$Config{"branches:".$tbr.":staging_branch"}.":name"};
		print "Staging branch detected -  ".$tbr." (".$Config{"branches:".$tbr.":number"}.")\n";
		$Config{"use_staging"} = 1;

		$Config{"rd:".$i.":branch"} = $Config{"branches:".$tbr.":name"};
		$Config{"rd:".$i.":branchid"} = $Config{"branches:".$tbr.":number"};
		}
		if ($Config{"rd:".$i.":week_num"} eq "") { die "EEEnNNNDDD Problems with path parser!";}

	    }
	}
    }
}
$Config{"rd:num"} = scalar @used_args;

if (scalar @used_args == 0 ) { print "\e[01;38;05;196m[ Give me correct source path!!!! Exiting... ]\e[0m\n\n\n "; exit;}


$Config{"useanotherbranch"}= "";
$Config{"useanotherbuild"}= "";
$Config{"int_build_num"}= "auto";
$Config{"force_overwrite"}= 0;

foreach $tmp (@ARGV)
{
    my $used_flag = 0;
    foreach my $elem (@used_args)
    {
	if ($tmp eq $ARGV[$elem])
	{
	    $used_flag = 1;
	}
    }
    if (not $used_flag)
    {
	chomp ($tmp);
	if ($tmp eq "test"){$donotcollect = 1;}
	elsif ($tmp =~ "basebranch=")
	{
	    if (defined ($Config{"branches:".$'.":number"})){ $Config{"useanotherbranch"}=$Config{"branches:".$'.":number"};}
	}
	elsif ($tmp =~ "basebuild=")
	{
	    my $query ;
	    my $bb=$';
	    $bb =~ s/\///;
	    my @fv;
	    $query = $dbhandler->prepare("SELECT branch,build_type,int_build_num,int_patch_num FROM $dbdb.data_builds where build='$bb' LIMIT 1;");
	    $query->execute;
	    while ( @fv = $query->fetchrow_array)
	    {
		$Config{"useanotherbuild"}=$bb;
		$Config{"useanotherbranch"}=$fv[0];
		$Config{"useanotherbuildtype"}=$fv[1];
	    }
	    $query->finish;
	    if ($Config{"useanotherbuild"} eq "") {print "\e[01;38;05;196m[ No such base build! Exiting... ]\e[0m\n\n"; exit;}
	}
	elsif ($tmp eq "replace"){$replace = 1;}
	elsif ($tmp eq "updatemedian"){$updatemedian = 1;}
	elsif ($tmp eq "weekly"){$Config{"global:build_type"}='weekly'}
	elsif ($tmp eq "daily"){$Config{"global:build_type"}='daily'}
	elsif ($tmp eq "tip"){$Config{"global:build_type"}='daily'}
	elsif ($tmp eq "patch"){$Config{"global:build_type"}='patch'}
	elsif ($tmp eq "other"){$Config{"global:build_type"}='other'}
	elsif ($tmp eq "overwrite"){$Config{"force_overwrite"} = 1;}
	elsif ($tmp eq "override"){$Config{"force_overwrite"} = 1;}
	else
	{
	    if (not defined ($device_type))
	    {
		foreach my $dev (@devices)
		{
		    chomp ($dev);
		    if ($dev eq $tmp) {$device_type = $tmp;}
		    elsif (defined $Config{"d:shortcut:".$dev}) 
		    {
			if ($Config{"d:shortcut:".$dev} =~ /^$tmp[|]/i or $Config{"d:shortcut:".$dev} =~ /[|]$tmp[|]/i or $Config{"d:shortcut:".$dev} =~ /[|]$tmp$/i or $Config{"d:shortcut:".$dev} =~ /^$tmp$/i)
			{$device_type = $dev;}
		    }
		}
	    }
	}
    }
}
if ($source_path eq "") { print "\nNo source path specified!\n"; exit; }
if (not defined ($device_type)) { $device_type = "auto";}
if (not defined ($backend)) { $backend="auto"; }


foreach my $rd (@used_args)
{
    $source_path = $result_dirs{$rd};
    print "\n=================================================================================\n";
    print "\e[01;38;05;27m[INFO]\e[0m Results path = $source_path\n";
    print "\e[01;38;05;27m[INFO]\e[0m Week number = ".$Config{"rd:".$rd.":week_num"}."\n";
    print "\e[01;38;05;27m[INFO]\e[0m Week day = ".$Config{"rd:".$rd.":week_day"}."\n";
    print "\e[01;38;05;27m[INFO]\e[0m Branch = ".$Config{"rd:".$rd.":branch"}." \n";
    print "\e[01;38;05;27m[INFO]\e[0m Firmware = ".$Config{"rd:".$rd.":firmware"}."\n";
    print "\e[01;38;05;27m[INFO]\e[0m Devices = $device_type\n";
    print "\e[01;38;05;27m[INFO]\e[0m Backends = $backend\n";
    print "\e[01;38;05;27m[INFO]\e[0m Build type = ".$Config{"rd:".$rd.":build_type"}."\n";
    print "\e[01;38;05;27m[INFO]\e[0m Build tag = ".$Config{"rd:".$rd.":buildtag"}."\n";
    print "\e[01;38;05;27m[INFO]\e[0m Patch number = ".(exists $Config{"rd:".$rd.":patch_num"}?$Config{"rd:".$rd.":patch_num"}:"")."\n\n";
#exit;

    @all_dirs=();
    if ( -e "$source_path/$benchmarks_directory_name")
    {
	my @tmpdirs = `find $source_path/$benchmarks_directory_name -name "scores.csv" -print`;
	foreach my $dir (@tmpdirs) { $dir=~ s/scores.csv//;chomp ($dir);}
	@all_dirs=@tmpdirs;
    }

if (scalar @all_dirs == 0) {die "!!! No benchmarks folder was found..."; }
    my $prevdevice="";
    my $prevbackend="";
    my %iternum;
    my $mode="";

    foreach my $dir (@all_dirs)
    {
	my @dirparts = split '/',$dir;
	my $localdir = $dirparts[-2]."/".$dirparts[-1];
	my $hostname= "";
	if (-e "$dir/run.opts")
	{
	    $hostname = `cat $dir/run.opts | grep "android.host" | awk '{print \$2}'`;
	    chomp ($hostname);
	    $hostname .= '-'.`cat $dir/run.opts | grep "android.serial" | awk '{print \$2}'`;
	}
	else { die ;}
	chomp ($hostname);
	print "\e[01;38;05;40m[Processing]\e[0m  $hostname  Dir $localdir: ";
	my $result;
	my $errflag=1;
	my $backendid=0;
	$device_type = `cat $dir/run.opts |grep android.device.type | awk '{print \$2}'`;
	chomp($device_type);
	if ( defined $Config{"d:name:".$device_type} ) {}
	else { die "!!!TBD: add code to add new devices automaticaly!!!";}

	$branch =`cat $dir/run.opts |grep android.build.branch | awk '{print \$2}'`;
	chomp($branch);
	if ($Config{"use_staging"})
	{
	    $branch = ($Config{"branches:".$branch.":staging_branch"}>0?$Config{"branches:".$Config{"branches:".$branch.":staging_branch"}.":name"}:$branch);
	}
	if ( defined $Config{"branches:".$branch.":number"} ) {}
	else { die "!!!TBD: add code to add new branches automaticaly ($branch)!!!";}
	
	my $vm_runtime = `cat $dir/run.opts |grep vm.runtime | awk '{print \$2}'`;
	chomp($vm_runtime);
	my $vm_backend = `cat $dir/run.opts |grep vm.backend | awk '{print \$2}'`;
	chomp($vm_backend);
	my $vm_mode    = `cat $dir/run.opts |grep vm.mode | awk '{print \$2}'`;
	chomp($vm_mode);
	$backend = $vm_runtime.lc($vm_backend.$backend_suffix).$vm_mode;
	if ( defined $Config{"backends:detect:".$backend.":name"} ) { $backend =$Config{"backends:detect:".$backend.":name"}; $backendid =$Config{"backends:".$backend.":number"};}
	else
	{
		my $addbackend= $vm_runtime."-".$vm_backend.$backend_suffix;
		my $addbitdebth;
		$vm_runtime =~ /64/;
		if ($& eq "64") {$addbitdebth=64;}
		else {$addbitdebth=32;}
		my $addvmtype = $vm_runtime;
		my $addvmbackend = $vm_backend.$backend_suffix;
		my $addvmmode = $vm_mode;

		print "\n****************************************************
		Unknown backend: $backend\n".
		"INSERT INTO `$dbdb`.list_backends (name,bitdepth,vm_type,vm_backend,vm_mode,comment) VALUES ('$addbackend',$addbitdebth,'$addvmtype','$addvmbackend','$addvmmode','auto');\n";
		$dbhandler->do("INSERT INTO `$dbdb`.list_backends (name,bitdepth,vm_type,vm_backend,vm_mode,comment) VALUES ('$addbackend',$addbitdebth,'$addvmtype','$addvmbackend','$addvmmode','auto');");
		my $backendid = $dbhandler->last_insert_id (undef,$dbdb,"list_backends","id");
		$Config{"backends:".$addbackend.":number"}= $backendid;
		$Config{"backends:".$addbackend.":vm_backend"} = $addvmbackend;
		$Config{"backends:".$addbackend.":vm_mode"} = $addvmmode;
		$Config{"backends:".$addbackend.":vm_type"} = $addvmtype;
		$Config{"backends:detect".$addvmtype.$addvmbackend.$addvmmode.":name"} = $addbackend;
		print "new backendid = $backendid\n

		Adding new backend into database
		*****************************************************\n\n";
		@backends = LoadBackendsModules_fromDB ();
	}

	
	if ( `cat $dir/run.opts | grep cpu.mode | awk '{print \$2}'` eq "1C1T" ) {$mode = ":1c";}
	else {$mode = ":mt";}


	if (defined $result and ($result eq "error" or $result eq "skip") ) {  $iternum{$mode}--; next;}

	if (-e "$dir/scores.csv" ) { $result="scores.csv"; print "\t\e[01;38;05;40m[OK]\e[0m result file was found, processing.... ";}
	else{print "\t\e[01;38;05;154m[Skipping]\e[0m cannot find result file, skipping....\n"; $result = "error";}
	
	if ($result eq "error" or $result eq "skip" ) {  $iternum{$mode}--; next;}
	$result = collectResults ($dir,$result,$hostname,$device_type,$branch,$backend,$mode,$rd,$backendid);
	if ($result eq "error" or $result eq "skip" ) {  $iternum{$mode}--; next;}
#	print "\n\e[01;38;05;2m[ OK  ]\e[0m $bench";
##rd:".$i.":
#exit;

    }
#`echo "" > Results.log`;foreach my $word (sort keys %Results) {   `echo "$word \t\t $Results{$word}" >> Results.log`;  }
#`echo "" > clconfig.log`;foreach my $word (sort keys %Config)  {   `echo "$word \t\t $Config{$word}" >> clconfig.log`;  }


}
if (defined $Results{"props:count"})
{
    getMedianRunNums ();
    CollectData (); 
    my $patchtmp="";
    print "\n===================== Creating text reports  ======================================\n";
    foreach my $rd (@used_args)
    {
        my $bn = $Config{"rd:".$rd.":firmware"};
        my $bbn = $Config{"useanotherbuild"};
        my $links="";
        if (defined $Config{"useanotherbuild"} and $Config{"useanotherbuild"} ne "")
        {
            $links = `./getlink.pl $bn $bbn  -q`;
            `echo '$bn\t$links' >> /nfs/ins/proj/slt/qa/android/results/perflinks.txt`;
        }

        $source_path = $result_dirs{$rd};
        if ($Config{"rd:".$rd.":build_type"} eq "patch"){$patchtmp.=$Config{"rd:".$rd.":patch_num"};}
        if (not  $source_path  =~ /^$benchmarks_directory_name/) {$source_path .="/$benchmarks_directory_name/";}
        `rm -f $source_path/Performance_results.txt`;
        `rm -f $source_path/../Performance_results.txt`;
	print "\n\n$source_path/,$patchtmp,$rd\n\n";
        GenerateDetailedFile ("$source_path/",$patchtmp,$rd);
        GenerateFile ("$source_path/",$patchtmp,$rd);

        if (-e "$source_path/Performance_results.txt") 
        {
            `echo "\n(Numbers in brackets are variability of the scores measured for recent builds).\n" >> $source_path/Performance_results.txt `;
            if (defined $Config{"useanotherbuild"}) {  `echo '$links' >> $source_path/Performance_results.txt\n\n`;}
            `cp $source_path/Performance_results.txt $source_path/../Performance_results.txt`;
        }
    }
}
else
{
    print "\nNo scores were found\n";
}
$dbhandler->disconnect;
print "\n========================== THE END OF BENCHMARKS PROCESSING =======================\n";
}
exit;






