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
use DBI;
use JSON;
#use LWP::UserAgent;
#use Data::Dumper qw(Dumper);
our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
require "config.inc";
our $dbh = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);

my $outstr = "";

if ($dbh)
{
my $benchmarksquery;my @brarecnum;my $bacquery;my @bacrecnum;my $benchmarkswcount;
my $show_variability=1;
my $show_delete_buttons=0;
my $use_median_variability=0;
my $drop_limit=0;
my $show_degradations_only=0;
my $patches=0;
my $hidevariability=0;
my $JIRA=0;
    my $query;
    my @fv;
    my $multiplebackends=0;
    my $cachedname = "_db_compare";

        $outstr .=  "<HTML>\n<HEAD>\n<meta http-equiv='content-type' content='text/html; charset=UTF-8'>\n
<STYLE>
    .other {background-color: rgba(234, 234, 234, 1);}
    .up th, .up td
    {
     border-top-style : double;
    border-top-width: medium;
    }
    .maintable td
    {
        font-size: small;
        text-align: center;
    }

    .rb {border-right: 1px solid black;}
    .lb{border-left: 1px solid black;}

    .maintable th
    {
        text-align: center;
        padding: 0 10 0 10;
        background-color: rgba(131, 174, 253, 0.23);
    }
    .maintable th a
    {
        color: black;
        text-decoration: none;
        font-weight: normal;
    }

    .firstcolumn 
    {
        max-width: 250px;
        min-width: 150px;
        font-size: small;
        font-weight: normal;
    }
    .base {background-color: rgba(181, 177, 100, 0.31) !important;}
    .caption {font-weight: bolder;}



.comment{
border: 5px solid transparent;
border-top: 4px solid red;
border-right: 4px solid red;
width: 0px;
height: 0px;
display: inline-block;
vertical-align: top;
cursor:help;
/*margin-top: -2px;*/
}


table.transp {
    display: block;
}
table.transp tbody tr {
    width: 100px;
}
table.transp .label {
    height: 40px;
    text-align: center;
    line-height: 100%;
}

table.transp tr,
table.transp thead {
    display: block;
    float: left;
}
table.transp tr td,
table.transp tr th {
    display: block;
}

.evaluatecheckbox {
    width: 16px;
    height: 16px;
    border: 0;
    border-radius:8px;
    background-color: transparent;
    background-repeat: no-repeat;
    float: right;
    margin: 2px 0 0 0;
}
.evaluated0 { background-position: center -0px; 
    box-shadow: 0px 0px 2px 1px grey inset;
}
.evaluated1 { background-position: center -16px;
    box-shadow: 0px 0px 2px 1px green inset;
 }


td .max::after
{
content: 'MAX';
color: red;
    text-shadow: 0px 0px 8px #FF3F3F;
    font-weight: bolder;
}

.maxcolumn {    background: #EAD1D1;}
th.maxcolumn{    background: #D4B9B9;}
</STYLE>
</HEAD>
<BODY><div id='includedContent'>";
    my $benchorder = "";
    my @args=();

    my $use_only_main_scores=0;


    my $args="";
    my $showids=0;
    my $args2="";
    my $addmaxcolumn=0;
    my $benchmarklist = "";

    my $URL = $ARGV[0];
    $URL=~ s/=;/;/g;
    $URL=~ s/=$//g;
    $URL=~ s/;/&/g;
    $URL=~ s/%2C/,/g;
    $URL=~ s/%7C/\|/g;

    $URL=~ s/&hidevariability//g;
    $URL=~ s/&JIRA//g;

my $tmp;
my $builds;
foreach $tmp (@ARGV)
{
    chomp ($tmp);
    if ($tmp =~ "builds=") { $builds = $'; }
    elsif ($tmp =~ "bench=")
    {
	$benchmarklist = $';
	if ((index $benchmarklist,"'" )<0)
	{
	    $benchmarklist=~ s/,/','/g;
	    $benchmarklist = "'".$benchmarklist."'";
	}
	$benchmarklist = " benchmark in (".$benchmarklist.") ";
	@args=(@args,$benchmarklist);
    }
    elsif ($tmp eq 'showids'){ $showids = 1;}
    elsif ($tmp =~ "benchid=")
    {
	my $benchids=$';
	my $addnot=0;
	while ($benchids=~ /[0-9]*\.\.[0-9]*/)
	{
	    my $beg = $`;
	    my $end = $';
	    my $seq = $&;
	    $seq =~ /\.\./;
	    $benchids = $beg.(join(',', ($` .. $'))).$end;
	}
	if ($benchids=~ /not,/) {$addnot=1; $benchids=~ s/not,//g;}
	if ($benchids=~ /not/)  {$addnot=1; $benchids=~ s/not//g;}
	@args=(@args," benchmarkid ".($addnot?"not":"")." in (".$benchids.") ");
    }
    elsif ($tmp =~ "droplimit=")
    {
	$drop_limit=$';
    }
    elsif  ($tmp=~ "benchorder=")
    {
	$benchorder=$';
    }
    elsif (scalar @args >0 )
    {
        $args = join "OR", @args;
	$args = " AND (".$args.") ";
	@args=();
    }
    elsif ($tmp eq 'main') {@args=(@args," ismain=1 "); $use_only_main_scores=1;}
    elsif ($tmp eq 'max') { $addmaxcolumn=1;}
    elsif ($tmp eq 'evaluationmode') {$show_variability=0;$use_median_variability=1;$show_degradations_only=1;}
    elsif ($tmp eq 'hidenonbasevariability') {$show_variability=0;}
    elsif ($tmp eq 'usemedianvariability') {$use_median_variability=1;}
    elsif ($tmp eq 'showdegaradationsonly') {$show_degradations_only=1;}
    elsif ($tmp eq 'patches') {$patches=1;}
    elsif ($tmp eq 'showdegradationsonly') {$show_degradations_only=1;}
    elsif ($tmp eq 'hidevariability') {$hidevariability=1;$show_variability=0}
    elsif ($tmp eq 'JIRA') { $JIRA=1}
    elsif ($tmp =~ "workloadid=")
    {
	my $workloadids=$';
	my $addnot=0;
	while ($workloadids=~ /[0-9]*\.\.[0-9]*/)
	{
	    my $beg = $`;
	    my $end = $';
	    my $seq = $&;
	    $seq =~ /\.\./;
	    $workloadids = $beg.(join(',', ($` .. $'))).$end;
	}

	if ($workloadids=~ /not,/) {$addnot=1; $workloadids=~ s/not,//g;}
	if ($workloadids=~ /not/)  {$addnot=1; $workloadids=~ s/not//g;}
	@args=(@args," workloadid ".($addnot?"not":"")." in (".$workloadids.") ");
    }
    else {
        print  "<HTML>\n<HEAD>\n<meta http-equiv='content-type' content='text/html; charset=UTF-8'>\n
<h2>Help page</h2>
For evaluation purposes you need only two arguments: <b>builds</b> and <b>evaluationmode</b>.<br>
Two additional arguments with &quot;not,&quot; at the beginning allow to hide evaluated rows - <b>benchid</b> and <b>workloadid</b><br>

<br>
Example - list of degradations on WW47 with filtered Quadrant benchmark and Antutu 5.6 2D graphics workload:<br>
compare.pl builds=7262b,7370,7261b,7371,7260b,7372,7257b,7375_7259b,7373,7258b,7374,7266b,7377,7264b,7382,7265b,7376,7263b,7381 benchid=not,4,51..66 workloadid=not,208 evaluationmode max droplimit=0.5
<br>
<p>
<h3>List of all arguments:</h3>
<ul>
    <li><b>builds</b>=7262b,7370e,7261b,7371,7260b,7372_7257b,7375,7259b,7373,7258b,7374,7266b,7377,7264b,7382,7265b,7376,7263b,7381
	<ul><li>Mandatory argument - list of build ids. By default first build considered as base
	<li>Splitters:
	    <ul><li><b>,</b> - standard horisontal separator
	    <li><b>_</b> - Vertical separator
	    </ul>
	</li>
	<li>Modificators:
	    <ul><li><b>b</b> - mark build as <b>B</b>ase
	    <!--li><b>e</b> - <b>E</b>xtended output - add links for particular build to weekly results and details pages<--!>
	    <li><b>l</b> - use <b>L</b>atest build of the same configuration instead of which was given.
	    <li><b>h</b> - <b>H</b>ide this build.
	    <li><b>p</b> - Show all <b>P</b>atches based on given build.
	    </ul>
	</li>
	</ul>

    <li><b>benchid</b>=not,1
	<ul><li>Optional argument -  list on benchmark ids
	<li> <b>..</b> - sequence of ids. Ex: From 51 to 63 - 51..63; 
	<li>Modificators:
	<ul><li>not, - as a first item in list to inverse argument to filter some benchmarks</ul>
	</ul>
    <li><b>benchorder</b>=1,6,3
	<ul><li>Optional argument -  list on benchmark ids to sort output. example: benchorder=1,11,5,4,41,7,12,10,49,50,16,40&benchid=1,11,5,4,41,7,12,10,49,50,16,40</ul>
    <li><b>workloadid</b>=not,16
	<ul><li>Optional argument -  list on workload ids
	<li> <b>..</b> - sequence of ids. Ex: From 251 to 263 - 251..263;
	<li>Modificators:
	    <ul><li>not, - as a first item in list to inverse argument meaning to filter some benchmarks</ul>
	</ul>
    <li><b>main</b>
	<ul><li>Display only main workloads</ul>
    <li><b>showids</b>
	<ul><li>Displays benchmarks and workloads <b>ids</b></ul>
    <li><b>hidenonbasevariability</b>
	<ul><li>Hide variability columns for non-base builds</ul>
    <li><b>showdegradationsonly</b>
	<ul><li>Hide rows without degradations</ul>
    <li><b>usemedianvariability</b>
	<ul><li>To use variability for last three builds instead of variability of base build: If score dropped more than both values then it will be marked as red, in case of one excess  as yellow</ul>
    <li><b>evaluationmode</b>
	<ul><li>This is a combination of last three argument: hidenonbasevariability& showdegradationsonly & usemedianvariability</ul>
    <li><b>droplimit</b>
	<ul><li>Show only degradations which are larger than this limit</ul>
    <li><b>hidevariability</b>
	<ul><li>Do not display variability columns</ul>
    <li><b>JIRA</b>
	<ul><li>Produces Wiki's format output for JIRA bugs</ul>
    <li><b>help</b>
	<ul><li>Show this page</ul>
    <li><b>max</b>
	<ul><li>Add column with maximum score among displayed builds</ul>

</ul>
</p>

<hr>";

exit;}

}
    if (scalar @args >0 )
    {
        $args2 = join "OR", @args;
	$args2 = " AND (".$args2.") ";
    }
    $args = $args.$args2;
    my $builds_orig = "";
my $allbuilds = $builds;
my $firstiter = 1;
foreach my $builds (split(/_/,$allbuilds))
{
    if ($benchorder ne "") {$benchorder="ORDER BY IF(FIELD(benchmarkid,".$benchorder.")=0,1,0), FIELD(benchmarkid,".$benchorder.") "};
    ($benchmarksquery = $dbh->prepare("SELECT * FROM view_workloads_info $benchorder ;"))->execute;
    ($benchmarkswcount = $dbh->prepare ("SELECT count(`$dbdb`.`list_workloads`.`id`) AS `workloadid` FROM `$dbdb`.`list_workloads` GROUP BY `benchmark` ORDER BY `$dbdb`.`list_workloads`.`benchmark`;"))->execute;

    if ($builds =~ /[0-9,{,},p,b,e,l,h,release,max]+/)
    {
	$builds = $&;
	while ($builds =~ /[0-9,b,e,l,h,release]+[p]+/)
	{
	    my $st = $`;
	    my $tmp = $&;
	    my $fn = $';
	    $tmp =~/[0-9]+/;

	    my $patchbuildsquery;
	    ($patchbuildsquery= $dbh->prepare("SELECT concat(',',group_concat(id,'e')) FROM data_builds, (SELECT branch,device,mode,backend,int_build_num FROM data_builds where id = $&) as base
WHERE data_builds.branch = base.branch AND
data_builds.device = base.device AND
data_builds.mode = base.mode AND
data_builds.backend = base.backend AND
data_builds.int_build_num = base.int_build_num AND
data_builds.int_patch_num > 0;"))->execute;
	    my @res = $patchbuildsquery->fetchrow_array;
	    $tmp.='b'.$res[0];
	    $builds = $st.$tmp.$fn;
	    $builds =~ s/p//;
	}

	if ($builds =~ /[0-9]+[l]/)
	{
	    while ($builds =~ /[0-9]+[l]+/)
	    {
		my $bname= $&;
		$bname =~ s/l//g;
	  ($query = $dbh->prepare("
SELECT id,data_builds.int_release_num FROM data_builds,(
SELECT branch,backend,device,`mode`,`source`,`int_release_num`,`int_build_num` FROM data_builds where id = $bname) as params WHERE
data_builds.branch = params.branch
AND data_builds.int_build_num >= params.int_build_num
AND data_builds.backend = params.backend
AND data_builds.device = params.device
AND data_builds.`mode` = params.`mode`
AND data_builds.int_patch_num = 0
ORDER BY data_builds.int_build_num desc LIMIT 1;
"))->execute;
	    @fv = $query->fetchrow_array;
	    $bname.='l';
	    $builds =~ s/$bname/$fv[0]/;
	    }
	}
	my $userelease = 0;
	if ($builds =~ /release/g) { $userelease=1;}
	my $usebase = 0;
	if ($builds =~ /base/g) { $usebase=1;}
	$builds =~ s/,release//g;
	$builds =~ s/release,//g;
	$builds =~ s/,base//g;
	$builds =~ s/base,//g;
	$builds =~ /^[0-9]+/g;
	my @basebuild = ($&);
	my %basebuildscheck;
	my %extendedoutput;
	$basebuildscheck{$&}=1;

	$builds =~ s/,[0-9]+[h]//g;
	$builds =~ s/[0-9]+[h],//g;
	$builds_orig = $builds;
	if ($userelease)
	{
	  ($query = $dbh->prepare("
SELECT id,data_builds.int_release_num FROM data_builds,(
SELECT branch,backend,device,`mode`,`source`,`int_release_num`,`int_build_num` FROM data_builds where id = $basebuild[0]) as params WHERE
data_builds.branch = params.branch
AND ((data_builds.int_release_num < params.int_release_num and params.int_release_num >0) or params.int_release_num = 0)
AND data_builds.int_build_num < params.int_build_num
AND data_builds.backend = params.backend
AND data_builds.device = params.device
AND data_builds.`mode` = params.`mode`
AND data_builds.int_patch_num = 0
ORDER BY data_builds.int_release_num desc;
"))->execute;
#AND data_builds.`source` = params.`source`
	    @basebuild= ();
	    %basebuildscheck = ();
	    my $endflag=1;
	    my $releasenum=-1;
	    $"=',';
	    while (@fv = $query->fetchrow_array and $endflag)
	    {
		if ($endflag==1)
		{
		    $releasenum = $fv[1];
		    $endflag = 2;
		}
		
		if ($releasenum != $fv[1])
		{
		    $endflag = 0;
		}
		else
		{
		    @basebuild = (@basebuild,$fv[0]);
		    $basebuildscheck{$fv[0]}=1;
		    if (not $builds =~ /$fv[0]/) {$builds.=",$fv[0]";}
		}
	    }
	}
	elsif ($usebase)
	{
	  ($query = $dbh->prepare("
SELECT id,data_builds.int_build_num FROM data_builds,(
SELECT branch,backend,device,`mode`,`source`,`int_build_num` FROM data_builds where id = $basebuild[0]) as params WHERE
data_builds.branch = params.branch
AND ((data_builds.int_build_num < params.int_build_num and params.int_build_num >0) or params.int_build_num = 0)
AND data_builds.int_build_num < params.int_build_num
AND data_builds.backend = params.backend
AND data_builds.device = params.device
AND data_builds.`mode` = params.`mode`
AND data_builds.int_patch_num = 0
ORDER BY data_builds.int_build_num desc;
"))->execute;
	    @basebuild= ();
	    %basebuildscheck = ();
	    my $endflag=1;
	    my $releasenum=-1;
	    $"=',';
	    while (@fv = $query->fetchrow_array and $endflag)
	    {
		if ($endflag==1)
		{
		    $releasenum = $fv[1];
		    $endflag = 2;
		}
		if ($releasenum != $fv[1]) { $endflag = 0; }
		else
		{
		    @basebuild = (@basebuild,$fv[0]);
		    $basebuildscheck{$fv[0]}=1;
		    if (not $builds =~ /$fv[0]/) {$builds.=",$fv[0]";}
		}
	    }
	}
	elsif ($builds =~ /[0-9]+[b]/)
	{
	    %basebuildscheck = ();
	    @basebuild= ();
	    while ($builds =~ /[0-9]+[b]/)
	    {
	    @basebuild = (@basebuild,$&);
	    $basebuild[-1]=~ s/b//g;
	    $basebuildscheck{$basebuild[-1]}=1;
	    $builds =~ s/b//;
	    }
	}
	while ($builds =~ /[0-9]+[e]/)
	{
	    my $tmp = $&;
	    if ($tmp =~ s/e//g)
	    {
		$extendedoutput{$tmp}=1;
	    }
		$builds =~ s/e//;
	}
#$outstr .=  "<br>$builds";
	$builds =~ s/p//g;
	$builds =~ s/b//g;
	$builds =~ s/e//g;
#$outstr .=  "<br>$builds";
#$outstr .=  "<br>$builds_orig";
    ($query = $dbh->prepare("
SELECT data_builds.id, build, IF(build_type='weekly' OR build_type='daily',1,0) as isweekly , IF(build_type='patch',1,0) as ispatch, build_type,run_date,week,day,'',list_backends.name,list_devices.name,list_branches.name,mode,source, IF(build_type='target',1,0) as 'is_target',data_builds.device,data_builds.branch,data_builds.backend,  IF (not (isnull(evaluatedby) or evaluatedby = ''),1,0) as 'evaluated'
FROM data_builds, list_devices, list_branches, list_backends
WHERE list_devices.id = data_builds.device AND list_branches.id = data_builds.branch AND list_backends.id = data_builds.backend AND data_builds.id IN ($builds)
ORDER BY ".(scalar @basebuild == 1?"data_builds.id = $basebuild[0] DESC,":"")." FIELD(data_builds.id,$builds),  source DESC,int_build_num ASC, int_patch_num;"))->execute;

if ($query->rows > 0)
{
    if ($JIRA) { $outstr .=  "||^Benchmark^||^Workload^|"; }
    else
    {
    $outstr .=  "
<TABLE class='maintable' >
<colgroup>
<col style='max-width: 150px; min-width: 150px;'>
<col span='2' style='max-width: 145px; min-width: 125px; position: absolute;'>
</colgroup>
<TR><TH rowspan=2 class='firstcolumn' ><b>Benchmark:</b></TH><TH rowspan=2 colspan=2 class='firstcolumn'><b>Workload:</b></TH>";
    }
}
else { $outstr .=  "<center><h1>No data</h1></center>";}
my $weeklyname="";
my %Results=();
my %Weeklybuilds=();
my %Variability=();
my %Variability_med=();
my @buildids=();
my $style;
my $currentweeklybuild=0;
my $buildids="0";
my %Builds=();
my @res;
    while (@fv = $query->fetchrow_array)
    {
	$Builds{$fv[0].":buildsinfo"}="build id=$fv[0]";
	$buildids.=",$fv[0]";
	$Builds{$fv[0].":buildsinfo"}.="\nbuild type = $fv[4]";
	$Builds{$fv[0].":buildsinfo"}.="\nbuild name=$fv[1]";
	$Builds{$fv[0].":buildsinfo"}.="\nbackend=$fv[9]";
	$Builds{$fv[0].":buildsinfo"}.="\ndevice=$fv[10]";
	$Builds{$fv[0].":buildsinfo"}.="\nbranch=$fv[11]";
	$fv[7]=(defined $fv[7]?"$fv[7]":"");
	$fv[12]=(defined $fv[12]?"$fv[12]":"");
	$Builds{$fv[0].":buildsinfo"}.="\nmode=$fv[12]";
	$fv[13]=(defined $fv[13]?"$fv[13]":"");
	$Builds{$fv[0].":buildsinfo"}.="\nsource=$fv[13]";
	$Builds{$fv[0].":weeklyname"} = $fv[6].$fv[7].$fv[8];
	if ($JIRA) {$Builds{$fv[0].":capt"}=($fv[14]==1?'<b>[Target]</b> ':'')."<nobr>".$fv[1]."<br>".$fv[9];}
	else {$Builds{$fv[0].":capt"}=($fv[14]==1?'<b>[Target]</b> ':'')."<nobr>".$fv[1]." ($fv[13])<br>".$fv[9];}
	$Builds{$fv[0].":capt"}=~ s/_userdebug//g;
	$Builds{$fv[0].":capt"}=~ s/_WW/<br>WW/g;
	$Builds{$fv[0].":capt"}=~ s/\_$Builds{$fv[0].":weeklyname"}/ <br><b>$Builds{$fv[0].":weeklyname"}<\/b> /g;
	$Builds{$fv[0].":capt"}.= "<br>$fv[10]";
	if ($JIRA) {$Builds{$fv[0].":capt"}=~ s/ABT_//g; $Builds{$fv[0].":capt"}=~ s/_ND/ (ND)/g;} #  $Builds{$fv[0].":capt"}=~ s/-BRONZE/ BRONZE/g;
	$Builds{$fv[0].":evaluated"}= "$fv[18]";

	my $buildvarquery;
	($buildvarquery = $dbh->prepare("SELECT workload,ROUND(`variability`,1) as variability FROM tmp_variabilityvalues where build=$fv[0] order by workload;"))->execute;
	while (@res = $buildvarquery->fetchrow_array)
		{ $Variability{$fv[0]."w".$res[0]}=$res[1];}

	if (exists $basebuildscheck{$fv[0]})
	{
	    if ($use_median_variability)
	    {
		my $buildmedvarquery;
		($buildmedvarquery = $dbh->prepare("SELECT workload,FLOOR(`variability`)+1 FROM $dbdb.tmp_currentvariabilityvalues where source='$fv[13]' and branch=$fv[16] and backend=$fv[17] and device=$fv[15] and mode='$fv[12]' order by benchmark,workload;"))->execute;
		while (@res = $buildmedvarquery->fetchrow_array) 
		    { $Variability_med{$fv[0]."w".$res[0]}=$res[1];}
	    }
	}
	@buildids=(@buildids,$fv[0]);

	my $buildresquery;
	($buildresquery = $dbh->prepare("SELECT workloadid,result,float_value FROM view_filteredresults where buildid=$fv[0] ".$args." order by workloadid;"))->execute;
	while (@res = $buildresquery->fetchrow_array) 
	    { $Results{$fv[0]."w".$res[0]}=$res[1]; 
		if ($res[2] ==0) {$Results{$fv[0]."w".$res[0]} =~ s/\.0000//;}
	    }

    }
my $argnum=0;
    foreach my $a (split(',',$builds_orig))
    {
	$argnum++;
	my $columns=2;
	$a=~ /[0-9]+/;
	my $buildid = $&;
	my $buildinfo=$Builds{$buildid.":buildsinfo"};
	my $capt = $Builds{$buildid.":capt"};
	if ($a=~ /b/)
	{
	    $Weeklybuilds{$currentweeklybuild}=$buildid;
	    $currentweeklybuild=$buildid;
	    $buildinfo.="\n*Used as base build*";
	    $Results{$argnum.":".$buildid."s"}='base';
	}
	else
	{
	    $Results{$argnum.":".$buildid."s"}='other';
	}

	$Results{$argnum.":".$buildid."wb"}=$currentweeklybuild;
	if ( $a=~ /e/ && !$JIRA) { $columns++;}
	if (!$hidevariability && !$JIRA) { $columns++;}
	if ( $a=~ /b/) { $columns--;}
	if ($JIRA){ $columns--;}
	if (!($a=~ /b/) && $hidevariability && $show_degradations_only) { $columns++;}

	if (!$show_variability && $use_median_variability && !( $a=~ /b/) ) {$columns--};
	$buildinfo.="\nbuild num = ".((defined ($fv[0]) and defined $Builds{$fv[0].":weeklyname"} )?$Builds{$buildid.":weeklyname"} :"");
        my $style= ($a =~ /b/? 'base':'other');
	if ($JIRA) { $capt=~ s/<br>/ \\\\ /g;  $outstr .=   ($style eq 'base'?" |":"")."|^$capt^"."| |"x($columns); }
	else {$outstr .=  "<TH class='$style' colspan=$columns><div class='evaluated".$Builds{$buildid.":evaluated"}." evaluatecheckbox'></div><center><a  href='/perfdb/builddetails.php?buildid=$buildid' target='_blank' title='$buildinfo'>$capt</a></center></TH>";}

    }

    if (!$JIRA && $addmaxcolumn==1)
    {
	$outstr .=  "<TH class='maxcolumn' colspan=2><center>MAX</center></TH>";
    }
	
    if ($JIRA) { $outstr .=  "<br>|| || ";}
    else {  $outstr .=  "</TR>";}

    my $prevbenchmark = "";
    my $prevweeklybuild = -1;
    my $rowclass="";

    if (!$JIRA) { $outstr .=  "<TR>";}
    my $prevbuildtype="";
    foreach my $a (split(',',$builds_orig))
    {
#	$outstr .=  "<br>$a";
        my $style= ($a =~ /b/? 'base':'other');
#	if ($JIRA) {$outstr .=  "|^Score^"}
	if ($JIRA) {$outstr .=  ($style eq 'base'?($prevbuildtype eq 'base'?"":"| ")."|":"")."|^Score^"}
#	if ($JIRA) {$outstr .=  ($style eq 'base'?"|e |":"")."|^Score^"}
	else { $outstr .=  "<TD class='$style'><p class='caption'>Score</p></TD>";}
	if (($a=~ /b+/ || $show_variability) && !$hidevariability)
		{ if ($JIRA) { $outstr .=  "";} else {$outstr .=  "<TD class='$style'><p class='caption'>Variability</p></TD>";}}
        if (!($a =~ /b+/)) {if ($JIRA) {$outstr .=  "|^&#9650;&#9660;^";} else {$outstr .=  "<TD class='$style'><p class='caption'>&#9650;&#9660;</p></TD>";}}
#// to add extra column uncomment this
#	if ($a =~ /e+/) {if (!$JIRA) {$outstr .=  "<TD class='$style'>Links</TD>"}}
	$prevbuildtype=$style;
    }
    if (!$JIRA && $addmaxcolumn ==1)
    {
	$outstr .=  "<TD class='maxcolumn'><p class='caption'>Max score</p></TD><TD class='maxcolumn'><p class='caption'>Build</p></TD>";
    }

    if ($JIRA) { $outstr .=  "||<br>";}
    else {  $outstr .=  "</TR>";}
  $prevbuildtype="";
#    $outstr .=  "<br>$builds";
    my $benchstr="";
    my $wcount=0;
    my $wskipped=0;
    my $strtoprint="";
    my $prevworkloadnum=0;
    my $oneworkloadadded=0;
    my $prevbenchmarkid = 0;
    my $prevbenchmarkgroup = 0;
    my @wnum=0;
    my $bestscore = -1;
    my $bestscorebuildname = "";
    while (@fv = $benchmarksquery->fetchrow_array)
    {
        my $scoresavailable = 0;
	my $localstr="";
	if ($prevbenchmark ne $fv[1])
	{
	    $rowclass="up";
	    @wnum=$benchmarkswcount->fetchrow_array;
	    if ($JIRA) { $benchstr = "||^$prevbenchmark^";}
	    else { $benchstr .= ($wcount)."><b title='benchmark_id=$prevbenchmarkid'>".($showids?"<b>[".$prevbenchmarkid."]</b> ":"")."$prevbenchmark</b>".($show_delete_buttons?" <a onclick='setLoc(location.href,\"benchid\",$prevbenchmarkid);'>[X]</a>":"")."</TH>";}
	    if ($wcount>0){ $outstr .=  $benchstr.$strtoprint; }
	    $wskipped=0;
	    $wcount =0;
#	    $prevworkloadnum = $wnum[0];
	    $prevbenchmark = $fv[1];
	    $prevbenchmarkgroup=$fv[12];
	    $prevbenchmarkid = $fv[0];
	    $benchstr = "<TR class='$rowclass'><TH class='firstcolumn' rowspan="; 
	    $strtoprint="";
	    $oneworkloadadded=0;
	    $bestscore = -1;
	    $bestscorebuildname = "";
	}
	elsif ($oneworkloadadded)
	{
	    if (!$JIRA){ $localstr .= "<TR>";
	    $bestscore = -1;
	    $bestscorebuildname = "";
	    }
	}
	if ($JIRA) {$localstr .= ($oneworkloadadded?"|| ":"")."||".($fv[6]?"*^$fv[4]^*</b>":"^$fv[4]^");}
	else {$localstr .= "<TH class='aleft firstcolumn'><span title='workload_id=$fv[3]'>".($showids?"<b>[".$fv[3]."]</b> ":"").($fv[6]?"<b>$fv[4]*</b>":"$fv[4]")."</span></TH><TH class='rb'>".($fv[7]?"<font color='red'>&#8681;</font>":"&#8679;")."</TH>";}
	my $workloadid=$fv[3];
	my $var =0;
	my $varstr ="";
	my $medvar =0;
	my $medvarstr ="";
	my $diff="";
	my $wasdrop=0;
	$argnum=0;
	foreach my $a (split(',',$builds_orig))
	    {$argnum++; $extendedoutput{$argnum.":".$a.'_'.$fv[0]}=0;}
	$argnum=0;
	foreach my $a (split(',',$builds_orig))
	{
	    $argnum++;
	    my $b = $a;
	    $a=~ /[0-9]+/;
	    $a= $&;
	    my $result="<font color='grey'>n/a</font>";
	    my $usebold=0;
	    my $basebuild = $Results{$argnum.":".$a."wb"};
	    my $style=$Results{$argnum.":".$a."s"};
	    my $basevar=0;
	    my $medbasevar=0;
	
	    $style =~ s/patch/wb$basebuild/g;
	    if (exists $Results{$a."w".$fv[3]})
	    {
		$result = $Results{$a."w".$fv[3]};
		if (exists $Variability{$basebuild."w".$fv[3]}) { $basevar=$Variability{$basebuild."w".$fv[3]};}
		else {$basevar=0;}
		if (exists $Variability{$a."w".$fv[3]} ) { $var=$Variability{$a."w".$fv[3]}; $varstr="[".$Variability{$a."w".$fv[3]}."%]";}
		else {$var=0; $varstr="[n/a]";}
		if ($use_median_variability)
		{
		    if (exists $Variability_med{$basebuild."w".$fv[3]}) { $medbasevar=$Variability_med{$basebuild."w".$fv[3]};}
		    else {$medbasevar=int($basevar)+1;}
		    if (exists $Variability_med{$a."w".$fv[3]} ) { $medvar=$Variability_med{$a."w".$fv[3]}; $medvarstr="[".$Variability_med{$a."w".$fv[3]}."%]";}
		    else {$medvar=int($var)+1; $medvarstr="[".$medvar."%]";}# $medvarstr="[n/a]";}

		}
		if ($fv[6]) {$varstr ="<b>".$varstr."</b>";}
		if ((( $show_variability || $basebuild == $a)&& !$hidevariability) || ($JIRA && $basebuild == $a))
		{
		    if ($JIRA){ if ($hidevariability){$varstr =" ";}else {$varstr =" ".$varstr;}}
		    else {$varstr ="<TD class='$style rb'>".(($use_median_variability && $basebuild == $a)?"Med: $medvarstr<br>Curr:":"").$varstr."</TD>";}}
		else {$varstr="";}
		if ($bestscore <0) {$bestscore=$result;$bestscorebuildname = "$a"; }
			if ($fv[7]) { if ($result<$bestscore) {$bestscore=$result; $bestscorebuildname = "$a";}  } #to recheck
			else { if ($result>$bestscore) {$bestscore=$result; $bestscorebuildname = "$a";}}

		if ($a == $basebuild)
		{
		    $basebuild=$Weeklybuilds{$a};
		    $diff="";
		    $usebold=1;
		}
		else
		{
		
		    if (exists ($Results{$basebuild."w".$fv[3]}))
		    {
			#lessisbetter
			my $addid="";
			
			if ($fv[7]) {$diff = (1.0-$result/$Results{$basebuild."w".$fv[3]})*100;   } #to recheck
			else {$diff = ($result/$Results{$basebuild."w".$fv[3]}-1.0)*100; }
			my $class="'czr'";
			my $sign="&#8776;";
			if ($diff < 0)
			{	if ($diff<=-$drop_limit)
				{
				    $addid=" id='b$a\_w$workloadid' ";
				    if ($basevar<=-$diff and (($medbasevar<=-$diff and $use_median_variability) or not $use_median_variability))
				    {
					$class="'cmb'";$sign="&#9660;"; $scoresavailable = 1;$wasdrop=1;
				    }
				    elsif ($use_median_variability and ($basevar<=-$diff or $medbasevar<=-$diff))
				    {
					$class="'cms'";$sign="&#9660;"; $scoresavailable = 1; $wasdrop=1;
				    }
				    elsif (($basevar/3)<=-$diff) 
				    {
					$class="'cms'";$sign="&#9660;"; if ($show_degradations_only) {$class="'czr'"; } else {$scoresavailable = 1;}
				    
				    } #$diff="hide";
				    else { if ($show_degradations_only) {} else{$scoresavailable = 1;}} #$diff="hide";
				}
			}
			else
			{
			    if ($basevar<=$diff) {$class="'cpb'"; $sign="&#9650;";}
			    elsif (($basevar/3)<=$diff) {$class="'cps'";$sign="&#9650;";}
			    if ($show_degradations_only) {$class="'czr'"; } else {$scoresavailable = 1;} #$diff="hide";
			}
#			$diff = ($diff eq "hide"?"<font color=lightgrey><i>".sprintf("%.2f%%",$diff)."</i></font>":sprintf("%.2f%%",$diff)."$sign");
			if ($JIRA) {$diff = sprintf("%.2f%%",$diff);}
			else {$diff = sprintf("%.2f%%",$diff)."$sign";}

			if ($JIRA){ $diff = "|^{color:".($class eq "'cmb'"?"red":($class eq "'cms'"?"orange":($class eq "'cpb'"?"green":($class eq "'cps'"?"darkseagreen":"grey"))))."}".$diff."{color}^";}
			else
			{
			if ($fv[6]) {$diff ="<b>".$diff."</b>";}
			$diff = "<td class='$style rb' $addid><p class=$class >".$diff."</p></td>";
			}
		    }
		    else  {
			if ($JIRA) {$diff="|^{color:grey}n/a{color}^"; $scoresavailable = 1;}
			else { $diff="<td class='$style rb'><font color='grey'>n/a</font></td>"; $scoresavailable = 1;}
		    }
		}
	    }
	    else
	    {
		if (( $show_variability || $basebuild == $a )&& !$hidevariability) {$varstr="<td class='$style rb'><font color='grey'>[n/a]</font></td>";}
		else {$varstr="";}
		if ($a != $basebuild)
		{
    		    if ($JIRA) {$diff="|^{color:grey}n/a{color}^";}
		    else {$diff = "<td class='$style rb'><font color='grey'>n/a</font></td>";}
		}
		else {$diff="";}
	    }
	    my $rowspan=$fv[11];
	    if (!$JIRA)
	    {
#to use extra column uncomment this
#	    if ($b =~ /e/ and $extendedoutput{$argnum.":".$b.'_'.$fv[0]}==0) {$diff.= "<TD class='$style rb '><a  href='/perfdb/builddetails.php?buildid=$a' target='_blank'>details</a></TD>"; $extendedoutput{$argnum.":".$b.'_'.$fv[0]}=1;}
	    }
	    if ($use_only_main_scores){ $rowspan=1;}
	    if ($fv[6] and $use_only_main_scores==0) {$result ="<b>".$result."</b>";}
	    if($JIRA){    $localstr .= (($style eq 'base')?($prevbuildtype eq 'base'?"":"| ")."|":"")."|^".$result."^".$varstr.$diff;}
	    else{    $localstr .="<TD class='$style lb'>".$result."</TD>$varstr$diff";}
$prevbuildtype=$style;
	}
	

    if (!$JIRA && $addmaxcolumn==1)
    {
#	my $toreplace = "$bestscore<span class='max'></span>";
	$localstr =~ s/$bestscore/$bestscore\<sup\>\<span class='max'\>\<\/span\>\<\/sup\>/;
$bestscorebuildname =$Builds{$bestscorebuildname.":capt"};
	    $bestscorebuildname=~ s/\<br\>/ , /g;
	$localstr .=  "<TD class='maxcolumn'><b>$bestscore</b></TD><TD class='maxcolumn'>$bestscorebuildname</TD>";
    }

	if ($JIRA) {$localstr .= "|<br>"}
	else {$localstr .="</TR>";}
	    $bestscore = -1;
	    $bestscorebuildname = "";

	if ( $scoresavailable )
	{
	    if (not $show_degradations_only  or ($show_degradations_only && $wasdrop==1))
	    {
	    $oneworkloadadded=1;
	    $wcount++;
	    $strtoprint.=$localstr;
	    }
	}
	else { $wskipped++;
}
    }
	    $rowclass="up";
	    @wnum=$benchmarkswcount->fetchrow_array;
	    if ($wcount>0)
	    {
		if ($JIRA) {$benchstr = "||^$prevbenchmark^";}
		else { $benchstr .= ($wcount)."><b title='benchmark_id=$prevbenchmarkid'>$prevbenchmark</b></TH>";}
	    $outstr .=  $benchstr.$strtoprint; 
	    }
	    $wskipped=0;
	    $wcount =0;
	    $strtoprint="";
	    $oneworkloadadded=0;
#    $"="\n ";
    my $ans="";

    $outstr .=  "<STYLE>".$ans."</STYLE>".($JIRA?"<br><br>":($firstiter==0?"<hr>":""));
$firstiter = 0;
}
else { $outstr .=  "<center><h1>Incorrect parameters</h1></center>";}

}

if ($JIRA){  $outstr .=  "<br>Arguments were: [compare.pl $URL]<br>";}
else {    $URL=~ s/&showids//g;$outstr .=  "</TABLE>\n<br>Arguments for JIRA's format: <b>compare.pl $URL hidevariability JIRA</b></div>";}

$outstr .=  "</BODY>\n</HTML>";


#$dbh->disconnect;
print "$outstr";
}
else {print "Can't connect!";}

1;
__DATA__
