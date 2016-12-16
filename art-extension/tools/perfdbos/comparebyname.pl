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
our $quiet = 0;
if (defined $ARGV[2] and $ARGV[2] eq '-q') {$quiet = 1;}
if ( not defined $ARGV[0] or not defined $ARGV[1] )
{
    print "Usage:\t$C{'red'}getlink.pl$C{'end'} $C{'green'}\{build name} {base build name}$C{'end'} $C{'white'}\[-q]\n\t-q for quite mode.output = link only$C{'end'}\n";
}
else
{
    our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
    our $compare_script_URL;
    my $dbh = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);
    my $query;
    if ($quiet==0) {print "Searching for $C{'white'}'$ARGV[0]'$C{'end'} build...\n";}
    ($query = $dbh->prepare("
SELECT group_concat(base.id,'b,', data_builds.id,'e')
FROM data_builds join (SELECT id,build,backend,device FROM data_builds WHERE build='$ARGV[1]') as base
WHERE data_builds.build='$ARGV[0]'
AND data_builds.backend = base.backend
AND data_builds.device = base.device ORDER BY data_builds.backend;"))->execute;
    if ($query->rows)
    {
	if ($quiet==0) {print "Builds were found.\nResult:\n";}
	while (my @val = $query->fetchrow_array)
	{
	    print "$compare_script_URL builds=$val[0]";
	}
    }
    else {print "$C{'red'}Error: No such build in database$C{'end'}\n";}
    print "\n";
    exit 1;

}

1;

