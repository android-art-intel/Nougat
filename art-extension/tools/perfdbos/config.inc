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

our $source="NSK";

#Database connection settings
our $dbhost = "nstdrlel258";
our $dbport = "3306";
our $dbuser = "perfdbuser";
our $dbpass = "perfdbpass";
our $dbdb = "perfdb";


#Database tables
our %dbtables;
$dbtables{'list_benchmarks'} = "list_benchmarks";
$dbtables{'list_workloads'}  = "list_workloads";
$dbtables{'list_branches'} = "list_branches";
$dbtables{'list_backends'} = "list_backends";
$dbtables{'list_devices'} = "list_devices";
$dbtables{''} = "";




our $benchmarks_directory_name="benchmarks";

#Files for parcing
our %files;
$files{'scores'} = "scores.csv";


# Other
our $compare_script_URL= "compare.pl";

# Colors
our %C;
$C{'end'}="\e[0m";
$C{'green'}="\e[01;38;05;2m";
$C{'red'}="\e[01;38;05;1m";
$C{'yellow'}="\e[01;38;05;3m";
$C{'blue'}="\e[01;38;05;4m";
$C{'pink'}="\e[01;38;05;5m";
$C{'cyan'}="\e[01;38;05;6m";
$C{'white'}="\e[01;38;05;7m";
$C{'darkgrey'}="\e[01;38;05;8m";

1;