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
require "./config.inc";
our %C;
our $quiet = 1;
our $evaluationmode = 0;
foreach my $arg (@ARGV)
{
if ($arg eq '-q') {$quiet = 1;}
if ($arg eq '-e') {$evaluationmode = 1;}
}
my $ARGS = "@ARGV";
$ARGS =~ s/-q//g;
$ARGS =~ s/-e//g;
$ARGS =~ s/ /','/g;
#print "$ARGS";
if ( not defined $ARGV[0] )
{
    print "Usage:\t$C{'red'}compare.pl$C{'end'} $C{'green'}\[{build name}]*$C{'end'} $C{'white'}\[-q]\[-e]\n\t-q for quite mode output = link only\n\t-e for evaluation mode.$C{'end'}\n";
}
else
{
    our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
    our $compare_script_URL;
    my $dbh = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);
    my $query;
    if ($quiet==0) {print "Searching for $C{'white'}'$ARGS'$C{'end'} builds...\n";}
$"="','";
#print "SELECT group_concat(id,'e') FROM data_builds WHERE build in('$ARGS') order by device, backend, branch, int_build_num;\n";

    ($query = $dbh->prepare("SELECT id,device,backend,branch FROM data_builds WHERE build in('@ARGV') order by device, backend, branch, int_build_num;"))->execute;
    if ($query->rows)
    {
	if ($quiet==0) {print "Builds were found.\nResult:\n";}
	my $builds_str="";
	my $prev="";
	my $comma="";
	my $wasnotb=0;
	while (my @val = $query->fetchrow_array)
	{
	    my $b="";
	    if ($prev ne "$val[1]_$val[2]" and @ARGV !=1) {$prev = "$val[1]\_$val[2]"; $b='b';}
	    else { $wasnotb=1;  }
	    $builds_str.="$comma$val[0]$b"."e";
	    $comma=',';
	}
	if ($wasnotb==0) { $builds_str =~ s/b//g;}
	$evaluationmode= ($evaluationmode?"evaluationmode":"");
	exec ("./$compare_script_URL builds=$builds_str benchid=not,0 $evaluationmode");
    }
    else {print "$C{'red'}Error: No such build in database$C{'end'}\n";}
#    print "\n";
    exit 1;

}

1;

