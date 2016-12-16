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

sub CollectData ()
{
    if (our $donotcollect)
    {
	print "\nSkipping score DB phase\n";
	return;
    }
    else
    {
	print "\n\n=========================== Adding to database ======================================\n\n".`date`;
    }
    our $dbhandler;
    our $dbdb;
    my $query;
    my @fv;

    our %Results;
    our %Config;
    our @benchmarks;
    our $extraopts;###
    our $source;
    our $updatemedian;
    my $modestr= "";
    (my $sec,my  $min,my  $hour,my  $mday,my  $mon,my  $year) = localtime;
    $year += 1900;
    my $propscount = 0;
    if  (defined $Results{"props:count"})
    {
	$propscount = $Results{"props:count"};
    }

	my $db_build_id;
    my $build_type;

    for (my $i = 1; $i<=$propscount; $i++)
    {
	print "\n\tAdding $i configuration from $propscount...(".$Results{"props:$i:branch"}." on ".$Results{"props:$i:device_type"}." with ".$Results{"props:$i:backend"}." backend in ".$Results{"props:$i:mode"}." mode)";
	my $rd = $Results{"props:$i:resultsdir"};
	if ($Config{"rd:".$rd.":build_release"} ne 'OM')
	{
	    if ($Config{"rd:".$rd.":build_release"}=~ /^[0-9].[0-9].[0-9]/)
	    {
		$Config{"rd:".$rd.":build_release"}= $&;
	    }
	}
	if ($Config{"rd:".$rd.":build_release"} eq '' or $Config{"rd:".$rd.":build_release"} eq 'n/a')
	{
	    $Config{"rd:".$rd.":build_release"}="N/A";
	}

	if ($Results{"props:$i:mode"} eq ':mt') {$modestr = "MT";}
	elsif ($Results{"props:$i:mode"} eq ':1c') {$modestr = "1C1T";}
	else  {$modestr = $Results{"props:$i:mode"};}

	my $db_device_id = $Config{"d:name:".$Results{"props:$i:device_type"}};
	my $db_backend_id = $Config{"backends:".$Results{"props:$i:backend"}.":number"};
	my $db_branch_id = $Config{"branches:".$Results{"props:$i:branch"}.":number"};
	my $db_branch_sorted = $Config{"branches:".$Results{"props:$i:branch"}.":sorted"};
	$build_type = $Config{"rd:".$rd.":build_type"};
	$query = $dbhandler->prepare('SELECT id,evaluatedby FROM `'.$dbdb.'`.data_builds WHERE build = "'.$Config{"rd:".$rd.":firmware"}.'" AND device = '.$db_device_id.' and mode = "'.$modestr.'" and extraopts="'.$extraopts.'" and backend='.$db_backend_id.' and source="'.$source.'";');
	$query->execute;
	my $NUM_OF_RECS =  $query->rows;
	if ($NUM_OF_RECS == 0)
	{
	    if ($db_branch_sorted)
	    {
		my $int_build_num=0;
		my $int_patch_num=0;
		
		$query = $dbhandler->prepare('SELECT int_build_num, int_patch_num, build FROM '.$dbdb.'.data_builds WHERE build = "'.$Config{"rd:".$rd.":firmware"}.'" group by int_build_num,int_patch_num;');
		$query->execute;
		$NUM_OF_RECS =  $query->rows;
		if ($NUM_OF_RECS > 1) {die "duplicated entry in database please contact administrator";}
		if ($NUM_OF_RECS == 1)
		{
		    @fv = $query->fetchrow_array;
		    $int_build_num = $fv[0];
		    $int_patch_num = $fv[1];
		}
		else
		{
		    if ($build_type eq "weekly" or $build_type eq "daily")
		    {
			$query = $dbhandler->prepare('SELECT max(int_build_num) FROM '.$dbdb.'.data_builds WHERE branch = '.$db_branch_id.';');
			$query->execute;
			if ($query->rows == 0)
			{$int_build_num = 1;
			}
			else
			{
			    @fv = $query->fetchrow_array;
			    $int_build_num = (defined($fv[0])?$fv[0]:0)+1;
			}
		    }
		    elsif ($build_type eq "patch" or $build_type eq "other")
		    {
#print 'SELECT int_build_num, max(int_patch_num) FROM '.$dbdb.'.data_builds WHERE branch = '.$db_branch_id.' and int_build_num=(SELECT MAX(int_build_num) FROM '.$dbdb.'.data_builds WHERE branch = '.$db_branch_id.' and week="'.$Config{"rd:".$rd.":week_num"}.'" and day="'.$Config{"rd:".$rd.":week_day"}.'");';
			$query = $dbhandler->prepare('SELECT int_build_num, max(int_patch_num) FROM '.$dbdb.'.data_builds WHERE branch = '.$db_branch_id.' and int_build_num=(SELECT MAX(int_build_num) FROM '.$dbdb.'.data_builds WHERE branch = '.$db_branch_id.' and week="'.$Config{"rd:".$rd.":week_num"}.'");');
			$query->execute;
			$NUM_OF_RECS = $query->rows;
			@fv = $query->fetchrow_array;
			if ($NUM_OF_RECS>0 and defined $fv[0] and defined $fv[1])
			{
			    $int_build_num = $fv[0];
			    $int_patch_num = $fv[1]+1;
			}
			else
			{
			    die "No base build in database. Exiting!\n\n";
			}
		    }
		    else
		    {
			die "Strange build type. TBD!";
		    }
		}
		$dbhandler->do('INSERT INTO `'.$dbdb.'`.data_builds (build,run_date,buildreleaseversion,year,week,branch,build_type,day,device,mode,extraopts,backend,source,int_build_num,int_patch_num,data_type) values ("'.$Config{"rd:".$rd.":firmware"}.'", now(),"'.($Config{"rd:".$rd.":build_release"}).'",'.$year.',"'.$Config{"rd:".$rd.":week_num"}.'","'.$db_branch_id.'","'.$build_type.'","'.$Config{"rd:".$rd.":week_day"}.'",'.$db_device_id.',"'.$modestr.'","'.$extraopts.'",'.$db_backend_id.',"'.$source.'",'.$int_build_num.','.$int_patch_num.',"testresult");');
		$db_build_id = $dbhandler->last_insert_id (undef,$dbdb,"data_builds","id");
		$Config{"rd:".$rd.":int_build_num"} = $int_build_num;
		$Config{"rd:".$rd.":int_patch_num"} = $int_patch_num;
		$Config{"rd:".$rd.":db_build_id"} = $db_build_id;

	    }
	    else
	    {
	    $dbhandler->do('INSERT INTO `'.$dbdb.'`.data_builds (build,run_date,buildreleaseversion,year,week,branch,build_type,day,device,mode,extraopts,backend,source,data_type) values ("'.$Config{"rd:".$rd.":firmware"}.'", now(),"'.($Config{"rd:".$rd.":build_release"}).'",'.$year.',"'.$Config{"rd:".$rd.":week_num"}.'","'.$db_branch_id.'","'.$build_type.'","'.$Config{"rd:".$rd.":week_day"}.'",'.$db_device_id.',"'.$modestr.'","'.$extraopts.'",'.$db_backend_id.',"'.$source.'","testresult");'); 
	    $db_build_id = $dbhandler->last_insert_id (undef,$dbdb,"data_builds","id");
	    }
		$Config{"rd:".$rd.":db_build_id"} = $db_build_id;
	}
	elsif  ($NUM_OF_RECS == 1)
	{
	    @fv = $query->fetchrow_array;
	    if (defined($fv[1]) and $fv[1] ne "" and $Config{"force_overwrite"}<1) {print "\t\e[01;38;05;196m[ Skipped ]\e[0m Build was evaluated by $fv[1]"; next;}
	    $db_build_id = $fv[0];
	    $dbhandler->do('UPDATE `'.$dbdb.'`.data_builds SET evaluatedby="",day = "'.$Config{"rd:".$rd.":week_day"}.'" WHERE id = '.$db_build_id.';'); 
	}
	else
	{
	    die "Too many records (builds) returned!";
	}
	$query->finish;
	if ($db_build_id == 0) { die "Error on Insertion build data. exiting.."; }
	if (our $replace) 
	{
	    $dbhandler->do("DELETE FROM data_results WHERE build = '".$db_build_id."';");
	}


	my $benchcount = $Results{"p:$i:b:num"};
	for (my $j = 1; $j<=$benchcount; $j++)
	{
	    my $benchnum = $Results{"p:$i:b:list:$j"};
	    my $vercount= $Results{"p:$i:b:$benchnum:v:num"};
	    for (my $l = 1; $l<=$vercount; $l++)
	    {
		my $vernum = $Results{"p:$i:b:$benchnum:v:list:$l"};
		my $workloadcount = $Results{"p:$i:b:$benchnum:v:$vernum:w:num"};
		for (my $k = 1; $k<=$workloadcount; $k++)
		{
		    my $workloadnum = $Results{"p:$i:b:$benchnum:v:$vernum:w:list:$k"};
		    my $setmedianflag=1;
		    if ($updatemedian)
		    {
			$dbhandler->do("UPDATE `$dbdb`.data_results SET ismedian=0 WHERE build = $db_build_id and benchmark=$benchnum AND workload=$workloadnum;");
#			print ("UPDATE `$dbdb`.data_results SET ismedian=0 WHERE id = $db_build_id and benchmark=$benchnum AND workload=$workloadnum;");
		    }
		    else
		    {
		    $query = $dbhandler->prepare("SELECT count(*) FROM `$dbdb`.data_results WHERE ismedian=1 AND build = $db_build_id and benchmark=$benchnum AND workload=$workloadnum;");
		    $query->execute();
		    @fv = $query->fetchrow_array;
		    if ($fv[0] == 1) {$setmedianflag = 0;}
		    }

		    my $itercount = $Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:iternum"};
#		print "\e[01;38;05;150m[Adding into database]\e[0m $bench (".$Results{"$benchname$mode:iternum"}." iterations).....\n";
		    my $wasmedian = 0;
		    for (my $m=1; $m<=$itercount; $m++)
		    {
			my $runcount = $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:runnum"};
			if ($runcount>0)
			{
			    my $hostname=$Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:hostname"};
			    my $hostid;
			    $query = $dbhandler->prepare('SELECT id FROM `'.$dbdb.'`.data_hosts WHERE hostname = "'.$hostname.'";');
			    $query->execute;
			    my $NUM_OF_RECS = $query->rows;
			    if ($NUM_OF_RECS ==0 )
			    {
				$dbhandler->do('INSERT INTO  `'.$dbdb.'`.data_hosts (hostname) VALUES ("'.$hostname.'");');
				$hostid = $dbhandler->last_insert_id (undef,$dbdb,"data_hosts","id");
			    }
			    elsif ($NUM_OF_RECS == 1 ) { @fv = $query->fetchrow_array;$hostid = $fv[0];}
			    else {die "Too many records (hostids) returned!";}
			    $query->finish;
			    my $db_path ;
			    my $dir=$Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:dir"};
			    $query = $dbhandler->prepare('SELECT id FROM `'.$dbdb.'`.data_run_props WHERE directory = "'.$dir.'";');  #.$&.$'.'";';);
			    $query->execute;
			    $NUM_OF_RECS = $query->rows;
			    if ($NUM_OF_RECS ==0 )
			    {
				$Results{"$dir:cores:freq:min"} = (defined $Results{"$dir:cores:freq:min"}?$Results{"$dir:cores:freq:min"}:'null');
				$Results{"$dir:cores:freq:max"} = (defined $Results{"$dir:cores:freq:max"}?$Results{"$dir:cores:freq:max"}:'null');
				$Results{"$dir:cores:freq:avg"} = (defined $Results{"$dir:cores:freq:avg"}?$Results{"$dir:cores:freq:avg"}:'null');
				$Results{"$dir:cpu:freq:min"} = (defined $Results{"$dir:cpu:freq:min"}?$Results{"$dir:cpu:freq:min"}:'null');
				$Results{"$dir:cpu:freq:max"} = (defined $Results{"$dir:cpu:freq:max"}?$Results{"$dir:cpu:freq:max"}:'null');
				$Results{"$dir:cpu:freq:avg"} = (defined $Results{"$dir:cpu:freq:avg"}?$Results{"$dir:cpu:freq:avg"}:'null');
				$Results{"$dir:cpu:freq:histo:json"} = (defined $Results{"$dir:cpu:freq:histo:json"}?$Results{"$dir:cpu:freq:histo:json"}:'null');
				$Results{"$dir:cpu:corenum"} = (defined $Results{"$dir:cpu:corenum"}?$Results{"$dir:cpu:corenum"}:'null');
				$Results{"$dir:cpu:governer"} = (defined $Results{"$dir:cpu:governer"}?$Results{"$dir:cpu:governer"}:'null');
				$Results{"$dir:cpu:temp:start"} = (defined $Results{"$dir:cpu:temp:start"}?$Results{"$dir:cpu:temp:start"}:'null');
				$Results{"$dir:cpu:temp:end"} = (defined $Results{"$dir:cpu:temp:end"}?$Results{"$dir:cpu:temp:end"}:'null');
				$Results{"$dir:cpu:temp:min"} = (defined $Results{"$dir:cpu:temp:min"}?$Results{"$dir:cpu:temp:min"}:'null');
				$Results{"$dir:cpu:temp:max"} = (defined $Results{"$dir:cpu:temp:max"}?$Results{"$dir:cpu:temp:max"}:'null');
				$Results{"$dir:cpu:temp:avg"} = (defined $Results{"$dir:cpu:temp:avg"}?$Results{"$dir:cpu:temp:avg"}:'null');
				$Results{"$dir:cpu:throttle:count"} = (defined $Results{"$dir:cpu:throttle:count"}?$Results{"$dir:cpu:throttle:count"}:0);
				$Results{"$dir:env:temp"} = (defined $Results{"$dir:env:temp"}?$Results{"$dir:env:temp"}:'null');
				$Results{"$dir:time:length"} = (defined $Results{"$dir:time:length"}?$Results{"$dir:time:length"}:'null');

				$dbhandler->do('INSERT INTO `'.$dbdb.'`.data_run_props (directory,host,cores_freq_min,cores_freq_max,cores_freq_avg,cpu_freq_min,cpu_freq_max,cpu_freq_avg,cpu_freq_histo,cpu_corenum,cpu_governer,cpu_temp_start,cpu_temp_end,cpu_temp_min,cpu_temp_max,cpu_temp_avg,cpu_throttle_count,env_temp,time_length) values ("'.$dir.'", '.$hostid.',"'.$Results{"$dir:cores:freq:min"}.'","'.$Results{"$dir:cores:freq:max"}.'","'.$Results{"$dir:cores:freq:avg"}.'", '.$Results{"$dir:cpu:freq:min"}.','.$Results{"$dir:cpu:freq:max"}.','.$Results{"$dir:cpu:freq:avg"}.', \''.$Results{"$dir:cpu:freq:histo:json"}.'\', '. $Results{"$dir:cpu:corenum"}.",'".$Results{"$dir:cpu:governer"}."',".$Results{"$dir:cpu:temp:start"}.','.$Results{"$dir:cpu:temp:end"}.', '.$Results{"$dir:cpu:temp:min"}.','.$Results{"$dir:cpu:temp:max"}.','.$Results{"$dir:cpu:temp:avg"}.','.$Results{"$dir:cpu:throttle:count"}.', '.$Results{"$dir:env:temp"}.', '.$Results{"$dir:time:length"}.');');
				$db_path = $dbhandler->last_insert_id (undef,$dbdb,"data_run_props","id");
			    }
			    elsif ($NUM_OF_RECS == 1 )
			    {
				@fv = $query->fetchrow_array;
				$db_path = $fv[0];
				$Results{"$dir:cores:freq:min"} = (defined $Results{"$dir:cores:freq:min"}?$Results{"$dir:cores:freq:min"}:'null');
				$Results{"$dir:cores:freq:max"} = (defined $Results{"$dir:cores:freq:max"}?$Results{"$dir:cores:freq:max"}:'null');
				$Results{"$dir:cores:freq:avg"} = (defined $Results{"$dir:cores:freq:avg"}?$Results{"$dir:cores:freq:avg"}:'null');
				$Results{"$dir:cpu:freq:min"} = (defined $Results{"$dir:cpu:freq:min"}?$Results{"$dir:cpu:freq:min"}:'null');
				$Results{"$dir:cpu:freq:max"} = (defined $Results{"$dir:cpu:freq:max"}?$Results{"$dir:cpu:freq:max"}:'null');
				$Results{"$dir:cpu:freq:avg"} = (defined $Results{"$dir:cpu:freq:avg"}?$Results{"$dir:cpu:freq:avg"}:'null');
				$Results{"$dir:cpu:freq:histo:json"} = (defined $Results{"$dir:cpu:freq:histo:json"}?$Results{"$dir:cpu:freq:histo:json"}:'null');
				$Results{"$dir:cpu:corenum"} = (defined $Results{"$dir:cpu:corenum"}?$Results{"$dir:cpu:corenum"}:'null');
				$Results{"$dir:cpu:governer"} = (defined $Results{"$dir:cpu:governer"}?$Results{"$dir:cpu:governer"}:'null');
				$Results{"$dir:cpu:temp:start"} = (defined $Results{"$dir:cpu:temp:start"}?$Results{"$dir:cpu:temp:start"}:'null');
				$Results{"$dir:cpu:temp:end"} = (defined $Results{"$dir:cpu:temp:end"}?$Results{"$dir:cpu:temp:end"}:'null');
				$Results{"$dir:cpu:temp:min"} = (defined $Results{"$dir:cpu:temp:min"}?$Results{"$dir:cpu:temp:min"}:'null');
				$Results{"$dir:cpu:temp:max"} = (defined $Results{"$dir:cpu:temp:max"}?$Results{"$dir:cpu:temp:max"}:'null');
				$Results{"$dir:cpu:temp:avg"} = (defined $Results{"$dir:cpu:temp:avg"}?$Results{"$dir:cpu:temp:avg"}:'null');
				$Results{"$dir:cpu:throttle:count"} = (defined $Results{"$dir:cpu:throttle:count"}?$Results{"$dir:cpu:throttle:count"}:0);
				$Results{"$dir:env:temp"} = (defined $Results{"$dir:env:temp"}?$Results{"$dir:env:temp"}:'null');
				$Results{"$dir:time:length"} = (defined $Results{"$dir:time:length"}?$Results{"$dir:time:length"}:'null');

				$dbhandler->do('UPDATE `'.$dbdb.'`.data_run_props SET directory="'.$dir.'" , host='.$hostid.', cores_freq_min="'.$Results{"$dir:cores:freq:min"}.'",cores_freq_max="'.$Results{"$dir:cores:freq:max"}.'",cores_freq_avg="'.$Results{"$dir:cores:freq:avg"}.'", cpu_freq_min='.$Results{"$dir:cpu:freq:min"}.',cpu_freq_max='.$Results{"$dir:cpu:freq:max"}.',cpu_freq_avg='.$Results{"$dir:cpu:freq:avg"}.', cpu_freq_histo=\''.$Results{"$dir:cpu:freq:histo:json"}.'\', cpu_corenum='. $Results{"$dir:cpu:corenum"}.",cpu_governer='".$Results{"$dir:cpu:governer"}."',cpu_temp_start=".$Results{"$dir:cpu:temp:start"}.',cpu_temp_end='.$Results{"$dir:cpu:temp:end"}.', cpu_temp_min='.$Results{"$dir:cpu:temp:min"}.',cpu_temp_max='.$Results{"$dir:cpu:temp:max"}.',cpu_temp_avg='.$Results{"$dir:cpu:temp:avg"}.',cpu_throttle_count='.$Results{"$dir:cpu:throttle:count"}.', env_temp='.$Results{"$dir:env:temp"}.',time_length ='.$Results{"$dir:time:length"}.' WHERE id= '.$db_path.';');

			    }
			    else {die "Too many records (dirs) returned!";}
			    $query->finish;
			    my $summqueryhead="INSERT INTO data_results (`build`,`directory`,`workload`,`value`,`benchmark`,`ismedian`,`filtered`,`benchmark_internal_version`) SELECT ";
			    my $summquery="";
			    my $updatequery="";
#SELECT id , workload, benchmark FROM data_results WHERE (id=334) HAVING COUNT(*) = 0;
			    for (my $n=1; $n<=$runcount; $n++)
			    {
				$summquery="";
				$summquery .= $summqueryhead."'".$db_build_id."','".$db_path."','".$workloadnum."',".$Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"}.",'".$benchnum."',";
				if (defined $Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} and $setmedianflag)
				{
				    if ($Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} == $Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} and $wasmedian == 0 )
				    {
					$summquery.= "1,";
					$wasmedian=1;
					$updatequery="UPDATE `$dbdb`.data_results SET ismedian=1 WHERE build = $db_build_id and benchmark=$benchnum AND workload=$workloadnum AND value=CAST(".$Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"}." AS DECIMAL(30,4)) AND directory=".$db_path.";";
				    }
				    else
				    {
					$summquery.= "0,";
				    }
				}
				else  { $summquery.= "0,";}
				$summquery.= (defined $Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n:filtered"}?"1":"0").",'$vernum' FROM data_results WHERE (build=".$db_build_id." AND value=CAST(".$Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"}." AS DECIMAL(30,4)) AND directory=".$db_path." AND workload=".$workloadnum." AND benchmark=".$benchnum." )  HAVING COUNT(*) = 0;\n";
				$dbhandler->do($summquery);
				if ($updatequery ne "") {$dbhandler->do($updatequery);}
			    }
			}
		    }
		}
	    }
	}
	print "\t\e[01;38;05;154m[ Added ]\e[0m";
	if (defined $db_build_id and defined $build_type  and ($build_type eq "weekly" or  $build_type eq "daily"))
	{
	    $dbhandler->do("call add_to_variabilitytable($db_build_id);");
	    $dbhandler->do("call calc_calculatable_scores($db_build_id);");
	}

    }
print "\n\n";
    print `date`;
    if (defined $db_build_id and defined $build_type  and( $build_type eq "weekly" or  $build_type eq "daily"))
    {
	$dbhandler->do("call proc_analyzeall();");
#	$dbhandler->do("call create_variabilitytable();");
	$dbhandler->do("call create_currentvariabilitytable();");
    }

}

1;
__DATA__


