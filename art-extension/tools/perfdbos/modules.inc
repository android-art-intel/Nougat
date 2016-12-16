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
our $dbdb;
our $dbhandler;
our $verbose;
our %Config;
our %dbtables;
my $query;
my @fv;

sub LoadBranchesModules_fromDB ()
{
    my @branches= ();
    print "Branches....\n";
    $query = $dbhandler->prepare("SELECT `id`, `name`, `alias`, `use_bronze_silver_detection`, `inverse_detection` FROM `$dbdb`.`$dbtables{'list_branches'}`;");
    $query->execute;
    my $branchid;
    my $branchname;
    while (@fv = $query->fetchrow_array)
    {
        $branchid=$fv[0];
        $branchname=$fv[1];
        @branches = (@branches,$branchname); #name
        $Config{"branches:".$branches[-1].":number"}=$branchid; #num
        $Config{"branches:".$branches[-1].":name"} = $branchname; #name
        $Config{"branches:".$branches[-1].":sorted"} = 1; #sorted
        $Config{"branches:".$branches[-1].":use_bronze_silver_detection"} =$fv[3]; #use_bronze_silver_detection
        $Config{"branches:".$branches[-1].":inverse_detection"} =$fv[4]; #inverse_detection
        $Config{"branches:".$branchid.":name"}=$branches[-1]; #num
        if (defined $fv[2]) #alias
        {
            @branches = (@branches,$fv[2]); #name
            $Config{"branches:".$branches[-1].":alias"} = 1;
            $Config{"branches:".$branches[-1].":aliasfor"} = $branchname;
            $Config{"branches:".$branches[-1].":number"}=$branchid;
            $Config{"branches:".$branches[-1].":name"} = $fv[2]; #name
            $Config{"branches:".$branches[-1].":sorted"} = 1; #sorted
        }
    }
    $query->finish;
    return @branches;
}


sub LoadBenchmarksModules_fromDB ()
{
    my @benchmarks = ();
    print "Benchmarks & workloads....\n";
    $query = $dbhandler->prepare("
    SELECT
        `$dbtables{'list_benchmarks'}`.`name`,
        `$dbtables{'list_benchmarks'}`.`id`,
        `$dbtables{'list_workloads'}`.`name`,
        `$dbtables{'list_workloads'}`.`order`,
        `$dbtables{'list_workloads'}`.`ismain`,
        `$dbtables{'list_workloads'}`.`id`,
        `$dbtables{'list_workloads'}`.`criteria`,
        `$dbtables{'list_workloads'}`.`lessisbetter`
    FROM (`$dbdb`.`$dbtables{'list_workloads'}` JOIN `$dbdb`.`$dbtables{'list_benchmarks'}`)
    WHERE
        `$dbtables{'list_workloads'}`.`benchmark` = `$dbtables{'list_benchmarks'}`.`id`
        AND `$dbtables{'list_workloads'}`.`calculate` = 0
    ;");

    $query->execute;
    my $workloadname;
    my $workloadid;
    my $order;
    my $benchname;
    my $benchid;
    while (@fv = $query->fetchrow_array)
    {
        $benchname = $fv[0]; #benchname
        $benchid = $fv[1]; #benchid
        if (not defined $Config{"bid:".$benchid.":name"})
        {
            @benchmarks = (@benchmarks,$benchname); #benchname
            $Config{"b:".$benchmarks[-1].":number"}=$benchid; #benchid
            $Config{"bid:".$benchid.":name"}=$benchname;
            $Config{"b:".$benchmarks[-1].":value:benchnamewithnum"} = $benchid."-".$benchname;
        }

        $workloadname = $fv[2]; #workloadname
        $order = $fv[3];
        $workloadid = $fv[5];
        $Config{"b:".$benchname.":".$workloadname.":order"}=$order; # worder
        if ($fv[4] eq "1")
        {
            $Config{"b:".$benchname.":mainworkload:name"}=$workloadname;
            $Config{"b:".$benchname.":mainworkload:num"}=$order;
        }
        $Config{"b:".$benchname.":".$workloadname.":id"}=$workloadid;
        $Config{"b:".$benchname.":".$workloadname.":criteria"}=$fv[6]; # criteria
        $Config{"b:".$benchname.":id:".$workloadid.":criteria"}=$fv[6]; # criteria
        $Config{"b:".$benchname.":".$workloadname.":lessisbetter"}=$fv[7];
        $Config{"b:".$benchname.":id:".$workloadid.":lessisbetter"}=$fv[7];
        $Config{"b:".$benchname.":".$order.":id"}=$workloadid;
        $Config{"b:".$benchname.":".$order.":name"}=$workloadname; # wname
        if (not defined $Config{"b:".$benchname.":workloadnum"} or $order > $Config{"b:".$benchname.":workloadnum"} )
        {
            $Config{"b:".$benchname.":workloadnum"} = $order;
        }
    }
    $query->finish;
    return @benchmarks;
}



sub LoadBackendsModules_fromDB ()
{
    my @backends= ();
    print "Backends....\n";
    $query = $dbhandler->prepare("SELECT `id`,`name`,`vm_type`,`vm_backend`,`vm_mode` FROM `$dbdb`.`$dbtables{'list_backends'}`;");
    $query->execute;
    while (@fv = $query->fetchrow_array)
    {
        @backends = (@backends,$fv[1]); #name
        $Config{"backends:".$backends[-1].":number"}=$fv[0]; #id
        $Config{"backends:".$backends[-1].":vm_type"} = $fv[2]; #vm_type
        $Config{"backends:".$backends[-1].":vm_backend"} = $fv[3]; #vm_backend
        $Config{"backends:".$backends[-1].":vm_mode"} = $fv[4]; #vm_mode
        $Config{"backends:detect:".$fv[2].lc($fv[3]).$fv[4].":name"} = $fv[1]; #name
    }
    $query->finish;
    return @backends;
}



sub LoadDevicesModules_fromDB ()
{
    my @devices= ();
    print "Devices....\n";
    $query = $dbhandler->prepare("
    SELECT `name`,`id`, GROUP_CONCAT(`name` SEPARATOR ',') AS `devicelist`
    FROM `$dbdb`.`$dbtables{'list_devices'}`
    WHERE `enabled` = 1
    GROUP BY `cpu`, `devicegroup`
    ORDER BY `id`, `mainingroup` DESC;
    ");
    $query->execute;
    my $devid;
    while (@fv = $query->fetchrow_array)
    {
        @devices = (@devices, $fv[0]);
        $devid=$fv[1];
        $Config{"d:id:".$devid}=$devices[-1];
        @fv=split (",",$fv[2]);
        foreach my $t (@fv)
        {
            $Config{"d:name:".$t}=$devid;
            $Config{"d:name:".$t."64"}=$devid;
            $Config{"d:name:".$t."_64"}=$devid;
            $Config{"d:name:".$t."_64P"}=$devid;
        }
    }
    return @devices;
}


1;
__DATA__;

