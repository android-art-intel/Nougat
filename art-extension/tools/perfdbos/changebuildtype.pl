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

#!/usr/bin/perl
use strict;
use warnings;
use diagnostics;
use DBI;
require "./config.inc";
our %C;
our $quiet=0;
if ( not defined $ARGV[0] )
{
    print "Usage:\t$C{'red'}changebuildtype.pl {build name}$C{'end'} $C{'green'}\[build type]$C{'end'}\nIf no build type was given then current type will be printed.\n\n";
    print "Available types are:\n$C{'yellow'}\tweekly\n\tdaily\n\ttip\n\tpatch\n\tother$C{'end'}\n";
}
else
{
    our $dbhost;our $dbport;our $dbuser;our $dbpass;our $dbdb;our $source;
    my $dbh = DBI->connect("DBI:mysql:$dbdb:$dbhost:$dbport",$dbuser,$dbpass);
    my $query;
    print "Searching for $C{'white'}'$ARGV[0]'$C{'end'} build...\n";
    ($query = $dbh->prepare("SELECT id,build,build_type,IF(not isnull(evaluatedby) and evaluatedby <>'','evaluated','') FROM data_builds WHERE build='$ARGV[0]';"))->execute;
    if ($query->rows)
    {
	print "Build was found.\nResults:\n";
	while (my @val = $query->fetchrow_array)
	{
	    print "  $val[0]\t$val[1]\t$C{'yellow'}$val[2]$C{'end'}\t$C{'green'}$val[3]$C{'end'}\n";
	}

	if (defined $ARGV[1])
	{
	    print "\n";
	    if ($ARGV[1] eq "weekly" or $ARGV[1] eq "daily" or $ARGV[1] eq "tip" or $ARGV[1] eq "patch" or $ARGV[1] eq "other")
	    {
		print "Changing build type on $C{'white'}$ARGV[1]$C{'end'}...\n";
		if ($dbh->do("UPDATE data_builds SET build_type='$ARGV[1]' WHERE build='$ARGV[0]';")) { print "$C{'green'}Done$C{'end'}\n";}
		else {print "$C{'red'}Error: SQL Update error$C{'end'}\n";}
	    }
	    else { print "$C{'red'}Error: unsupported build type.$C{'end'}\nAvailable types are:\n$C{'yellow'}\tweekly\n\tdaily\n\ttip\n\tpatch\n\tother$C{'end'}\n"; }

	}
    }
    else {print "$C{'red'}Error: No such build in database$C{'end'}\n";}
    print "\n";
    exit 1;

}

1;

