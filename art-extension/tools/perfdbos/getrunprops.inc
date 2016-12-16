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
use Date::Parse;
use DBI;
sub getrunprops ($$)
{
my ($fullpath,$path) = @_;
if ( defined $path)
{
#print " 1=".$fullpath."\t2=".$path."\n";
my $filexists= `find $fullpath -name "*-device.log" -print`;
chomp ($filexists);
my $report="";
our %Results;
my $val;
my $first=1;
my $count = 0;
my $um=0;
my $corenum = 0;
$Results{"$path:time:length"} = 0;
$Results{"$path:cpu:freq:histo"}="";
$Results{"$path:cpu:corenum"}=0;
$Results{"$path:cpu:freq:min"}=0;
$Results{"$path:cpu:freq:max"}=0;
$Results{"$path:cpu:freq:avg"}=0;
$Results{"$path:cores:freq:min"}=0;
$Results{"$path:cores:freq:max"}=0;
$Results{"$path:cores:freq:avg"}=0;
$Results{"$path:cpu:temp:start"}=0;
$Results{"$path:cpu:temp:end"}=0;
$Results{"$path:cpu:temp:min"}=0;
$Results{"$path:cpu:temp:max"}=0;
$Results{"$path:cpu:temp:avg"}=0;
$Results{"$path:cpu:throttle:count"}=0;
$Results{"$path:cpu:throttle:count/sec"}=0;

if ( $filexists ne "" && defined $path)
{
my @timesstr = `find $fullpath -name '*-device.log'  -exec cat {} \\; | grep 'cpu:' | grep -v 'cannot bind'|grep -v 'cpu: coretemp: -1 core_throttle_count: n'|grep -v 'coretemp: core_throttle_count:'|  awk -F'[' '{print \$2}' |awk -F'] ' '{print \$1}'`;
my @freqsstr = `find $fullpath -name "*-device.log"  -exec cat {} \\; | grep 'cpu:' | grep -v 'cannot bind'|grep -v 'cpu: coretemp: -1 core_throttle_count: n'|grep -v 'coretemp: core_throttle_count:'| awk -F'] ' '{print \$2}' | grep -v '\\['| awk -F'cpu: ' '{print \$2}'|awk -F' coretemp: ' '{print \$1}'`;
my @tempsstr = `find $fullpath -name "*-device.log"  -exec cat {} \\; | grep 'cpu:' | grep -v 'cannot bind'|grep -v 'cpu: coretemp: -1 core_throttle_count: n'|grep -v 'coretemp: core_throttle_count:'| awk -F'] ' '{print \$2}' | grep -v '\\['| awk -F'coretemp: ' '{print \$2}'| awk -F' core_throttle' '{print \$1}' | grep -v 'core_throttle_count'`;
my @throtsstr = `find $fullpath -name "*-device.log"  -exec cat {} \\; | grep 'cpu:' | grep -v 'cannot bind'|grep -v 'cpu: coretemp: -1 core_throttle_count: n'|grep -v 'coretemp: core_throttle_count:'| awk -F'] ' '{print \$2}' | grep -v '\\['| awk -F' core_throttle_count: ' '{print \$2}'`;
my %tmpcores =();
my %tmpfreqs =();
if (defined $freqsstr[0])
{
$corenum= split / /, $freqsstr[0];
}
#if (not (isdigit($corenum) and $corenum >0)) {$corenum=-1;}
if ($corenum >0)
{
#print $corenum;
$Results{$path.":cpu:corenum"}= $corenum;
#freqs

foreach my $str (@freqsstr)
{
    chomp ($str);
    my @tmp =  split / /, $str;
    if (@tmp==$corenum)
    {
	$um=0;
	for (my $i=0;$i<$corenum;$i++)
	{	
	
	    if ($tmp[$i] =~ /[0-9]/)
	    {
		$tmp[$i]=eval($tmp[$i]);
		$tmpfreqs{$tmp[$i]}++;
		if ($first)
		{
		    $tmpcores{"$i:avg"}=0;
		    $tmpcores{"$i:min"}=$tmp[$i];
		    $tmpcores{"$i:max"}=$tmp[$i];
	    }
	    else
	    {
		if ($tmp[$i] <$tmpcores{"$i:min"}){ $tmpcores{"$i:min"}=$tmp[$i];}
		if ($tmp[$i] >$tmpcores{"$i:max"}){ $tmpcores{"$i:max"}=$tmp[$i];}
	    }
	    $tmpcores{"$i:avg"}+=eval($tmp[$i]);
	    $um+=$tmp[$i];
	    }
	}
	if ($first)
	{
	    $Results{"$path:cpu:freq:min"}=$um;
	    $Results{"$path:cpu:freq:max"}=$um;
	}
	else
	{
	    if ($Results{"$path:cpu:freq:min"} > $um) {$Results{"$path:cpu:freq:min"}=$um;}
	    if ($Results{"$path:cpu:freq:max"} < $um) {$Results{"$path:cpu:freq:max"}=$um;}
	}
    $count++;
    $first=0;
    }
}
$Results{"$path:cores:freq:min"}="";
$Results{"$path:cores:freq:max"}="";
$Results{"$path:cores:freq:avg"}="";
$um=0;
for (my $i=0;$i<$corenum;$i++)
{
    $um+= (defined ($tmpcores{"$i:avg"})?$tmpcores{"$i:avg"}:0);
    if ($i>0)
    {
	$Results{"$path:cores:freq:min"}.=",";
	$Results{"$path:cores:freq:max"}.=",";
	$Results{"$path:cores:freq:avg"}.=",";
    }
    $Results{"$path:cores:freq:avg"}.=eval((defined($tmpcores{"$i:avg"})?$tmpcores{"$i:avg"}:0)/$count);
    $Results{"$path:cores:freq:min"}.=(defined($tmpcores{"$i:min"})?$tmpcores{"$i:min"}:0);
    $Results{"$path:cores:freq:max"}.=(defined($tmpcores{"$i:max"})?$tmpcores{"$i:max"}:0);
}
$Results{"$path:cpu:freq:avg"}=$um/$count;
$Results{"$path:cores:freq:avg"}="[".$Results{"$path:cores:freq:avg"}."]";
$Results{"$path:cores:freq:min"}="[".$Results{"$path:cores:freq:min"}."]";
$Results{"$path:cores:freq:max"}="[".$Results{"$path:cores:freq:max"}."]";

$,  = ' ';
my @hash_keys = keys %tmpfreqs;
@hash_keys = sort {$a <=> $b} (@hash_keys);
if (@hash_keys > 20 )
{
    my $minfreq = $hash_keys[0];
    my $maxfreq = $hash_keys[-1];
    my $step = int(($maxfreq-$minfreq)/20);
    my %newtmpfreqs= ();
    my $border = int($minfreq+$step);
    my $curfreq=int($minfreq);
    $newtmpfreqs{$curfreq}=0;
    foreach my $key(@hash_keys)
    {
	if ($key>=$border)
	{
	    $curfreq=int($key);
	    $border=int($curfreq+$step);
	    $newtmpfreqs{$curfreq}=0;
	    if ($border>=$maxfreq) {$curfreq=int($maxfreq);}
	}
	$newtmpfreqs{$curfreq}+=$tmpfreqs{$key};
    }
    %tmpfreqs= %newtmpfreqs;
    @hash_keys = keys %tmpfreqs;
    @hash_keys = sort {$a <=> $b} (@hash_keys);
}

my $maxnum=-1;

foreach my $key(@hash_keys)
    { $maxnum+=$tmpfreqs{$key}; }
#my $symb="\x{2588}";
if ($maxnum>0)
{
my $symb="|";
my $symb2=" ";
my $strlen=20;
$first=1;
$Results{"$path:cpu:freq:histo"}= "\nFrequency histogram:\n";
$Results{"$path:cpu:freq:histo:json"}.="";
foreach my $key(@hash_keys)
{
my $repeatnum = $tmpfreqs{$key}/$maxnum*100/5;
$Results{"$path:cpu:freq:histo"}.="$key:\t[";
if (not $first) {$Results{"$path:cpu:freq:histo:json"}.=',';}
else {$first=0;}
$Results{"$path:cpu:freq:histo:json"}.="{\"$key\":$tmpfreqs{$key}}";
$Results{"$path:cpu:freq:histo"}.= "$symb" x $repeatnum;
if ($strlen-$repeatnum>0) {
if ($repeatnum!=0) {$Results{"$path:cpu:freq:histo"}.="$symb2";}
$Results{"$path:cpu:freq:histo"}.="$symb2"x($strlen-$repeatnum+1)};
$Results{"$path:cpu:freq:histo"}.= "] ($tmpfreqs{$key})\n";
}
$Results{"$path:cpu:freq:histo:json"}= '['.$Results{"$path:cpu:freq:histo:json"}.']';

}

#temp
$first=1;
$count=0;
$val=0;
$Results{"$path:cpu:temp:avg"}=0;
foreach my $str (@tempsstr)
{
    chomp ($str);
    $val= eval($str);
    if (defined ($val) && $val >0)
    {
	if ($first)
	{
	    $Results{"$path:cpu:temp:start"}=$val;
	    $Results{"$path:cpu:temp:min"}=$val;
	    $Results{"$path:cpu:temp:max"}=$val;
	}
	else
	{
	    if ($Results{"$path:cpu:temp:min"} > $val) {$Results{"$path:cpu:temp:min"}=$val;}
	    if ($Results{"$path:cpu:temp:max"} < $val) {$Results{"$path:cpu:temp:max"}=$val;}
	}
	$Results{"$path:cpu:temp:avg"}+=$val;
	$count++;
	$first=0;
    }
}
$Results{"$path:cpu:temp:end"}=(defined ($val)?$val:0);
$Results{"$path:cpu:temp:start"}=(defined($Results{"$path:cpu:temp:start"})?$Results{"$path:cpu:temp:start"}:0);
$Results{"$path:cpu:temp:min"}=(defined($Results{"$path:cpu:temp:min"})?$Results{"$path:cpu:temp:min"}:0);
$Results{"$path:cpu:temp:max"}=(defined($Results{"$path:cpu:temp:max"})?$Results{"$path:cpu:temp:max"}:0);
$Results{"$path:cpu:temp:avg"}=($count==0?0:$Results{"$path:cpu:temp:avg"}/$count);

#time
chomp ($timesstr[0]);
chomp ($timesstr[-1]);
$Results{"$path:time:length"}=str2time($timesstr[-1])-str2time($timesstr[0]);



#throtts
$Results{"$path:cpu:throttle:count"}=-1;
$Results{"$path:cpu:throttle:count/sec"}=-1;
chomp ($throtsstr[0]);
if ($throtsstr[0] ne "n/a")
{
    chomp ($throtsstr[-1]);
    my @tmp1 = split / /, $throtsstr[0];
    my @tmp2 = split / /, $throtsstr[-1];
    my $diff=0;
    if (defined ($tmp2[0]) && defined ($tmp1[0]))
    {
	for (my $i=0;$i<$corenum;$i++)
	{
	    if (defined ($tmp2[$i]) && defined ($tmp1[$i]))
	    {
		$diff+=$tmp2[$i]-$tmp1[$i];
	    }
	}
    }
    $Results{"$path:cpu:throttle:count"}=$diff;
    $Results{"$path:cpu:throttle:count/sec"}=($Results{"$path:time:length"}>0?$diff/$Results{"$path:time:length"}:-1);
}

#print "\n=============================\n";
}
$report.=$Results{"$path:cpu:freq:histo"}."\n";
$report.="cpu:corenum:\t".$Results{"$path:cpu:corenum"}."\n";
$report.="cpu:freq:min:\t".$Results{"$path:cpu:freq:min"}."\n";
$report.="cpu:freq:max:\t".$Results{"$path:cpu:freq:max"}."\n";
$report.="cpu:freq:avg:\t".$Results{"$path:cpu:freq:avg"}."\n";
$report.="cores:freq:min:\t".$Results{"$path:cores:freq:min"}."\n";
$report.="cores:freq:max:\t".$Results{"$path:cores:freq:max"}."\n";
$report.="cores:freq:avg:\t".$Results{"$path:cores:freq:avg"}."\n";
$report.="cpu:temp:start:\t".$Results{"$path:cpu:temp:start"}."\n";
$report.="cpu:temp:end:\t".$Results{"$path:cpu:temp:end"}."\n";
$report.="cpu:temp:min:\t".$Results{"$path:cpu:temp:min"}."\n";
$report.="cpu:temp:max:\t".$Results{"$path:cpu:temp:max"}."\n";
$report.="cpu:temp:avg:\t".$Results{"$path:cpu:temp:avg"}."\n";
$report.="cpu:throttle:count:\t".$Results{"$path:cpu:throttle:count"}."\n";
$report.="cpu:throttle:count/sec:\t".$Results{"$path:cpu:throttle:count/sec"}."\n";
$report.="time:length:\t".$Results{"$path:time:length"}."\n";

#`rm $path/runprops.histo`;
#`rm $path/runprops.histogram;`;
#exit;
}

$filexists=`find $fullpath -name "system_info1.log" -print`;
chomp($filexists);
if ( $filexists ne "")
{
my @governers=`find $fullpath -name 'system_info1.log'  -exec cat {} \\; | grep governor | grep -v 'cannot bind'| awk '{print \$9}' | awk -F',' '{print \$1}'`;
#my @governers=`find $fullpath -name 'system_info1.log'  -exec cat {} \\; | grep 'scaling_governor:' | grep -v 'cannot bind'|  awk -F'governor: ' '{print $\2}' | cut -d ',' -f 1`;
#|grep -v '/system/bin/sh' 
my $envtemp=`find $fullpath -name 'system_info1.log'  -exec cat {} \\; | grep thermal_zone | grep -v 'cannot bind'| grep battery| awk -F'battery ' '{print \$2}'`;

#governers
$val="";
$um="";
$first=1;
foreach my $str (@governers)
{
    chomp($str);
    if (not $first) {$um.='","';}
    else {$val=$str;}
    $um.=($str);
    $first=0;
}
$_=$um;
$count=s/$val//ig;

if (defined ($corenum) and $count < $corenum ) {$um= '["'.$um.'"]';}
else {$um=$val;}
$Results{"$path:cpu:governer"}=$um;


#envtemp
chomp ($envtemp);

$envtemp=($envtemp=~ /[0-9]/?$envtemp:-1);
$Results{"$path:env:temp"}=$envtemp;

$report.="cpu:governer:\t".$Results{"$path:cpu:governer"}."\n";
$report.="env:temp:\t".$Results{"$path:env:temp"}."\n";

}

if ($report ne "")
{
$report.= "\n";
`echo '$report' > $fullpath/runprops.histogram;`;
}
}
}
1;


