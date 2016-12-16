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
our $quiet=1;
if (defined $ARGV[0] )
{
    our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
    require "./config.inc";
    my $dbh = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);
    my $query;
    ($query = $dbh->prepare("SELECT  MIN(not (isnull(evaluatedby) or evaluatedby = '') ) FROM data_builds WHERE build='$ARGV[0]';"))->execute;
    my $result=0;
    my @val = $query->fetchrow_array;
    if (defined $val[0]) 
    {
	$result = $val[0];
	if ($result == 0)
	{
	    print "Results were not evaluated yet\n";
	}
	else
	{
	    print "Results were evaluated\n";
	    exit 0;
	}
    }
    else {print "Error: No such build in database\n";}
    exit 1;
    $dbh->disconnect;
}
else {print "Give build name as an argument!\n";exit 1;}
1;

