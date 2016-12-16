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

sub GenerateFile ($$$)
{
    my ($destpath,$patchnum,$rd) = @_;
    our ($dbhandler,%Results, $source, $dbdb,@benchmarks,%Config);
    my $sql="";
    my ($bt,$bt1,$bt2);
    my ($db_build_id, $weekbranchid);
    my $weeknum=$Config{"rd:".$rd.":week_num"};
    my $weekday = $Config{"rd:".$rd.":week_day"};
    if ($Config{"rd:".$rd.":build_type"} eq "patch") {$bt=1;}
    elsif ($Config{"rd:".$rd.":build_type"} eq "daily") {}
    my $destfile;
    my $query; my @fv;
    my $prevbenchmark="";
    my $prevworkload="";
    my $backend="";
    my ($value,$weekval, $patchval, $status)=("","","","");
    my $device="";
    my $capt;
    my $benchname;
    my $percentage;
    my $var;
    my ($weeknum1, $weeknum2);
    my $weekbuildtype ='"daily","weekly"';
    my $i;
    if (defined $bt) { $weeknum1 = "Patch"; $weeknum2 = "Base";}
    else { $weeknum1 = "Tested build"; $weeknum2 = "Base build"; }
    if ($Config{"useanotherbranch"} ne "") {  $weekbranchid = $Config{"useanotherbranch"};    }
    else    { $weekbranchid = $Config{"rd:".$rd.":branchid"};}
    my $weekbuildname = "and build <> '".$Config{"rd:".$rd.":firmware"}."'";
    if ($Config{"useanotherbuild"} ne "") {  $weekbuildname = "and build = '".$Config{"useanotherbuild"}."'"; $weekbuildtype = '"'.$Config{"useanotherbuildtype"}.'"';    }
    my $propscount = 0;
    if  (defined $Results{"props:count"}) { $propscount = $Results{"props:count"}; }
    my @backendssorted;
    for ($i = 1; $i<=$propscount; $i++) { @backendssorted = (@backendssorted, $Results{"props:$i:backendid"}) }
    @backendssorted = sort {$a <=> $b } @backendssorted;
    @backendssorted = (0,@backendssorted);
    for ($i = 1; $i<=$propscount; $i++)
    {
	$Results{"props:sorted:$i"}=$i;
	my $k = $Results{"props:sorted:$i"};
	if( $Results{"props:$k:resultsdir"} == $rd)
	{
	    my $modestr="";
	    if ($Results{"props:$k:mode"} eq ':mt') {$modestr = "MT";}
	    elsif ($Results{"props:$k:mode"} eq ':1c') {$modestr = "1C1T";}
	    else  {$modestr = $Results{"props:$k:mode"};}
	    $sql ="SELECT test_id,base_id FROM
		(SELECT id as test_id, int_build_num, int_patch_num FROM $dbdb.data_builds WHERE device = '".$Config{"d:name:".$Results{"props:$k:device_type"}}."' and source='$source'  AND mode = '$modestr' AND backend =  ".($Config{"backends:".$Results{"props:$k:backend"}.":number"})." and branch=".$Config{"rd:".$rd.":branchid"}." and build='".$Config{"rd:".$rd.":firmware"}."' ) AS test,
		(SELECT id as base_id, int_build_num, int_patch_num FROM $dbdb.data_builds WHERE device = '".$Config{"d:name:".$Results{"props:$k:device_type"}}."' and source='$source'  AND mode = '$modestr' AND backend =  ".($Config{"backends:".$Results{"props:$k:backend"}.":number"})." and branch=$weekbranchid $weekbuildname) AS base
		ORDER  BY base.int_build_num desc LIMIT 1;";
	    $query = $dbhandler->prepare($sql);
	    $query->execute;
	    if ($query->rows > 0)
	    {
		(my $test_id, my $base_id) = $query->fetchrow_array;
		$sql="SELECT * FROM `tmp_variabilityvalues` WHERE `build`=$base_id LIMIT 1";
		$query = $dbhandler->prepare($sql);
		$query->execute;
		if ($query->rows <= 0)
		{
		   $dbhandler->do("call add_to_variabilitytable($base_id);");
		}
		$sql="SELECT test.build as 'testbuild',base.build as 'basebuild',test.benchmark as 'benchmark', test.workload as 'workload',
		    test.JPSPrimary as 'ismain',if(test.lessisbetter=1,  round((1-test.result/base.result)*100.0,2) , round((test.result/base.result - 1)*100.0,2)) as 'percentage',
		    test.result as 'testresult',base.result as 'weeklyresult',base.week as 'week',test.branch as 'branch',
		    test.device as 'device',test.backend as 'backend',test.mode as 'mode',var.variability as 'var', test.day as 'day'
		    FROM
			(SELECT * FROM view_filteredresults WHERE buildid=$test_id) AS test,
			(SELECT * FROM view_filteredresults WHERE buildid=$base_id) AS base,
			(SELECT * FROM tmp_variabilityvalues WHERE build=$base_id) AS var
		    WHERE test.workloadid = base.workloadid AND base.workloadid = var.workload AND test.workloadid !=434 AND test.workloadid !=435 GROUP BY test.workloadid ORDER BY  test.workloadid;";
#		print "\n$sql\n";
		$query = $dbhandler->prepare($sql);
		$query->execute;
		my $prstr;
		my $prstrmain;
		open my $out, '>', '/tmp/f1.tmp';
		open my $out2, '>', '/tmp/f2.tmp';
		open my $out3, '>', '/tmp/f3.tmp';
		my $old_handle = select $out;
		if ($query->rows > 0)
		{
		    my $firstrow = 1;
		    my $wasbenchprinted=0;
		    my $prevbench="";
		    while (@fv = $query->fetchrow_array)
		    {
			if ($firstrow )
			{
			    $prstr = "The patch successfully passed standard patch testing.\n\nJPS Primary workloads scores changes for ".$fv[10]." on ".$fv[0]." as compared to the ".$fv[1]." build are:";
			    $prstrmain = "The patch successfully passed standard patch testing.\n\nJPS Primary workloads scores changes on ".$fv[0]." as compared to the ".$fv[1]." build are:";
			    if ($bt) {print $out2 "\n---------------------------------------------\nPatched build $fv[0] compared with weekly $fv[1] on $fv[9] on $fv[10] device\n";} #_$fv[8]$fv[15]
			    else {print $out2 "Weekly build $fv[0] on $fv[10] device compared with build $fv[1]\nVariability values are in square brackets.\n";}
			    $firstrow = 0;
			    $destfile="full_".$Results{"props:$k:backend"}."_".$Config{"rd:".$rd.":week_num"}.$Config{"rd:".$rd.":week_day"}."_".$patchnum.$Results{"props:$k:device_type"}."_".$Results{"props:$k:modestr"}.".txt";
			    print $out2 "\n$weeknum1:\t$fv[0]\n$weeknum2:\t$fv[1]\n\n";
			    $bt1="$weeknum1 score";
			    $bt2="$weeknum2 score";
			}
			if ($backend ne $fv[11] or $device ne $fv[10] )
			{   $backend = $fv[11];
			    $device = $fv[10];
			    print $out "\n";
			    $capt = $fv[11]." on ".$fv[10].":";
			    select $out; $~='REPORT_PATCH_HEAD'; write $out;
			    select $out2; $~='REPORT_FULL_HEAD'; write $out2;
			}
			$weekval = $fv[7];
			if ($fv[2] ne $prevbench)
			{
			    $prevbench = $fv[2];
			    $wasbenchprinted=0;
			    close $out3;
			    close $out;
			    `cat /tmp/f3.tmp >> /tmp/f1.tmp;`;
			    open $out, '>>', '/tmp/f1.tmp';
			    open $out3, '>', '/tmp/f3.tmp';
			}
			if (-$fv[5] > $fv[13] and -$fv[5]>0.5) { $status = "[!]"; }
			elsif (-$fv[5]>10) { $status = "[?]"; }
			else {$status = "";}

			if ($fv[4] == 1)
			{
			    $benchname = "$fv[2].$fv[3]";
			    $percentage = $fv[5];
			    $value = $fv[6];
			    $var =  $fv[13];
			    select $out; $~='REPORT_PATCH'; write $out;
			    $wasbenchprinted=1;
			}
			elsif ($status ne "") 
			{
			    if ( $wasbenchprinted)  {$benchname = " |-> $fv[2].$fv[3]";}
			    else {$benchname = "$fv[2].$fv[3]";}
			    $percentage = $fv[5];
			    $value = $fv[6];
			    $var = $fv[13];
			    if ($wasbenchprinted) { select $out; $~='REPORT_PATCH'; write $out; }
			    else { select $out3; $~='REPORT_PATCH'; write $out3; }
			}
			$benchname = "$fv[2].$fv[3]";
			$percentage = $fv[5];
			$var = $fv[13];
			$patchval = $fv[6];
			select $out2; $~='REPORT_FULL'; write $out2;
		    }
		    close $out3 or die;
		    close $out;
		    `cat /tmp/f3.tmp >> /tmp/f1.tmp;`;
		    open $out, '>>', '/tmp/f1.tmp';
		    select $out; $~='REPORT_PATCH_END'; write $out;
		    select $out2; $~='REPORT_FULL_END'; write $out2;
		    select $old_handle;
		    close $out or die;
		    close $out2 or die;

		    if ( -e "$destpath$destfile")  {  `rm $destpath$destfile`;}
		    `echo "" > $destpath$destfile`;
		    `chmod ugo+rw $destpath$destfile  >/dev/null 2>/dev/null`;
		    if (not -e "$destpath/Performance_results.txt") {`echo "$prstrmain" > $destpath/Performance_results.txt `;}
		    `echo "$prstr" > $destpath$destfile;`;
		    `cat /tmp/f1.tmp >> $destpath$destfile;`;
		    `echo "(Numbers in brackets are variability of the scores measured on base build).\n" >> $destpath$destfile;`;
		    `cat /tmp/f1.tmp >>$destpath/Performance_results.txt;`;
		    `chmod ugo+rw $destpath/Performance_results.txt >/dev/null 2>/dev/null`;
		    `cat /tmp/f2.tmp >> $destpath$destfile;`;
		    `rm /tmp/f1.tmp;rm /tmp/f2.tmp;rm /tmp/f3.tmp;`;
		}
	    }
	}
    }
# ==========================================================================================
format REPORT_PATCH_HEAD =
@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        $capt
 ------------------------------------------------------------------------------------------------------
 |   |       Benchmark/workload         | @||||||||||||||| |   Change   |  Variab  | @||||||||||||||| |
                                                                    $bt1, $bt2
 ------------------------------------------------------------------------------------------------------
.
format REPORT_PATCH =
 |@<<| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< | @###########.### | @######.#% | [@##.#%] | @###########.### |
  $status, $benchname,    $value,  $percentage,   $var, $weekval
.
format REPORT_PATCH_END =
 ------------------------------------------------------------------------------------------------------
.

format REPORT_FULL_HEAD =
@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        $capt
 ------------------------------------------------------------------------------------------------------
 |   |       Benchmark/workload         |   Change   |  Variab  | @||||||||||||||| | @||||||||||||||| |
                                                                    $bt1, $bt2
 ------------------------------------------------------------------------------------------------------
.
format REPORT_FULL =
 |@<<| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< | @######.#% | [@##.#%] | @###########.### | @###########.### |
  $status, $benchname,                $percentage,   $var,        $patchval,   $weekval
.
format REPORT_FULL_END =
 ------------------------------------------------------------------------------------------------------
.
}




sub GenerateDetailedFile ($$$)
{
    my ($destpath,$patchnum,$rd) = @_;
    my $destfile;
    our $dbhandler;
    our %Config; 
    our %Results; 
    our $source;
    our $dbdb;
    my $query; my @fv;
    my $prevbenchmark="";
    my $prevworkload="";
#    if (not  $destpath  =~ /benchmarks/) { $destpath.="/benchmarks/";} 
    my $propscount = 0;
    if  (defined $Results{"props:count"})
    {
	$propscount = $Results{"props:count"};
    }

    for (my $i = 1; $i<=$propscount; $i++)
    {
if(    $Results{"props:$i:resultsdir"} == $rd)
{

    my $modestr="";
    if ($Results{"props:$i:mode"} eq ':mt') {$modestr = "MT";}
    elsif ($Results{"props:$i:mode"} eq ':1c') {$modestr = "1C1T";}
    else  {$modestr = $Results{"props:$i:mode"};}


	$destfile="details_".$Results{"props:$i:backend"}."_".$Config{"rd:".$rd.":week_num"}.$Config{"rd:".$rd.":week_day"}."_".$patchnum.$Results{"props:$i:device_type"}."_".$Results{"props:$i:modestr"}.".txt";
	my $sql= 'SELECT build,backend,device, benchmark,mode,workload,workloadid, view_allresults.directory, data_hosts.hostname, group_concat(if(ismedian = 1, concat(" [", result, "]"), concat(" ", result))) as scores, source
	      FROM '.$dbdb.'.view_allresults, '.$dbdb.'.data_run_props,'.$dbdb.'.data_hosts 
	      WHERE mode = "'.$modestr.'" AND backend = "'.$Results{"props:$i:backend"}.'" and build = "'.$Config{"rd:".$rd.":firmware"}.'" and deviceid = '.$Config{"d:name:".$Results{"props:$i:device_type"}}.' and directoryid = data_run_props.id and data_hosts.id = data_run_props.host and source="'.$source.'"
	      GROUP BY backendid ,mode, benchmarkid , workload , device, directory order by device , backendid , mode DESC , benchmarkid , workloadid , directory, source;';
#	`echo "$sql\n\n" >> ss2`;
	$query = $dbhandler->prepare($sql);
	$query->execute;

	my $NUM_OF_FIELDS =  $query->{NUM_OF_FIELDS};
	for (my $i = 0;$i < $NUM_OF_FIELDS; $i++) {@fv = (@fv, $query->{NAME}[$i]);}
	$fv[0] = $Config{"rd:".$rd.":firmware"};
	$fv[6] = $Results{"props:$i:device_type"};
	open my $out, '>', "$destpath$destfile";
	my $old_handle  = select $out;
	$~='REPORT_DETAILED_HEAD';   write $out;
	while (@fv = $query->fetchrow_array) 
	{ 
	    if ( $prevbenchmark ne $fv[3]) {$prevbenchmark = $fv[3];  $prevworkload = $fv[5];   $~='REPORT_DETAILED_DIV1';   write $out;     $~='REPORT_DETAILED_BODY';}
	    elsif ( $prevworkload ne $fv[5]) {$prevworkload = $fv[5];    $~='REPORT_DETAILED_DIV2';   write $out;     $~='REPORT_DETAILED_BODY';}
	    $fv[9] =~ s/^ //g;
	    write $out; 
	}
    $~='REPORT_DETAILED_TAIL';
    write $out;
    select $old_handle;
    close $out or die;
    `chmod ugo+rw $destpath$destfile  >/dev/null 2>/dev/null`;
}
    }
#`ln -s $destpath$destfile $destpath/../$destfile`;
format REPORT_DETAILED_HEAD =
All scores on ^<<<<<<<<<<<< device for build @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<:
                    $fv[6],        $fv[0]
Note: (Median numbers are in square brackets.)
|===================|===============|====|==============|===================|=================================|========|==============================================|==========================================================================|
|^||||||||||||||||| |^||||||||||||| |^|| |^|||||||||||| |^||||||||||||||||| |   ^|||||||||||||||||||||||||||| |^|||||| |^|||||||||||||||||||||||||||||||||||||||||||| |   ^|||||||||||||||||||||||||||||||||                                     |
  $fv[1],         $fv[2],   $fv[4],        $fv[3],            $fv[5],               $fv[8],              $fv[10],                 $fv[9],                              $fv[7]
.
format REPORT_DETAILED_DIV1 =
|===================|===============|====|==============|===================|=================================|========|==============================================|==========================================================================|
.
format REPORT_DETAILED_DIV2 =
|-------------------|---------------|----|--------------|-------------------|---------------------------------|--------|----------------------------------------------|--------------------------------------------------------------------------|
.


format REPORT_DETAILED_BODY =
|^||||||||||||||||~~|^||||||||||||| |^<<<|^<<<<<<<<<<<<<|^<<<<<<<<<<<<<<<<<<|^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<|^|||||| |^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<|^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<    |
     $fv[1],         $fv[2],  $fv[4],       $fv[3],            $fv[5],               $fv[8],             $fv[10],               $fv[9],                   $fv[7]
.
format REPORT_DETAILED_TAIL =
|===================|===============|====|==============|===================|=================================|========|==============================================|==========================================================================|
.

}

1;
__DATA__


