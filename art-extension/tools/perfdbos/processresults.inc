#!/usr/bin/perlB

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

sub collectResults ($$$$$$$)
{
    our %Results;
    our %Config;
    my ($result_dir, $result_file, $hostname, $device_type, $branch, $backend, $mode, $rd,$backendid) = @_;
    my $benchnum;
    my $workloadnum;
    my $propscount = 0;
    my $propnum =-5;
    my $parsed_dirname;
    if (exists($Results{"props:count"}))
    {
        $propscount = $Results{"props:count"};
    }
    else
    {
        $Results{"props:count"} = 0;
    }
    #print "\n>>>>>>propscount=".$propscount."\trd=".$rd;

    for (;$propscount >0 and $propnum <0; $propscount --)
    {
        #print "\n>>propscount=".$propscount."\tConfig{d:name:.Results{props:propscount:device_type}}=".$Config{"d:name:".$Results{"props:$propscount:device_type"}}."\tResults{props:propscount:device_type}}=".$Results{"props:$propscount:device_type"}."\tConfig{d:name:.device_type}=".$Config{"d:name:".$device_type}."\tdevice_type=".$device_type."\trd=".$rd."\tResults{props:propscount:resultsdir}=".$Results{"props:$propscount:resultsdir"};
        if ( $Config{"d:name:".$Results{"props:$propscount:device_type"}} == $Config{"d:name:".$device_type})
        {
            if($Results{"props:$propscount:branch"} eq $branch)
            {
                if( $Results{"props:$propscount:backend"} eq $backend)
                {
                    if($Results{"props:$propscount:mode"} eq $mode)
                    {
                        if($Results{"props:$propscount:resultsdir"} == $rd)
                        {
                            $propnum = $propscount;
                        }
                    }
                }
            }
        }
    }

    if ($propnum < 0)
    {
        $Results{"props:count"}++;
        $propnum = $Results{"props:count"};
        $Results{"props:$propnum:device_type"}=$device_type;
        $Results{"props:$propnum:branch"}=$branch;
	$Results{"props:$propnum:backendid"}=$backendid;
        $Results{"props:$propnum:backend"}=$backend;
        $Results{"props:$propnum:mode"}=$mode;
        $Results{"props:$propnum:modestr"}=$mode;
        $Results{"props:$propnum:modestr"}=~ s/://g;
        $Results{"props:$propnum:resultsdir"} = $rd;
        if ( -e "$result_dir/system_info1.log"){ $Config{"rd:".$rd.":build_release"} = `cat $result_dir/system_info1.log | grep ro.build.version.release | awk -F'[' '{print \$3}' | awk -F']' '{print \$1}'`;}
        else {$Config{"rd:".$rd.":build_release"} ="n/a";}
        chomp $Config{"rd:".$rd.":build_release"};
    }
    my $criteria="original";
    my $iternum;
    open (my $tmpfile ,"<" ,"$result_dir$result_file");
    if ((-s $tmpfile)== 0) { close $tmpfile; print "\e[01;38;05;196m[ Error ]\e[0m Result file is empty!!! \n"; return "error"; }
    my $fl = my  $j  = 0;
    print "\n";
    my %uniq= ();
    while (my $ss=<$tmpfile>)
    {
        chomp($ss);
        ##uniq
        if (defined $uniq{$ss}) {next;}
        else {$uniq{$ss}=1;}
        if ($ss=~/$criteria/)
        {
            if ($ss=~/Infinity/ or $ss=~/NaN/ or $ss=~/NOTVALID/ or $ss=~/;;;;;;-;-;-;-;-;-;-/ or $ss=~/;run1;/ or $ss eq "") {next; }
            if ($result_dir=~ /\/benchmarks_robo\//)
            {
                my $resnum = 3+`echo "$ss" | awk -F';' '{print \$3}' | sed s/.*\\(// | sed s/\\).*//`;
                my $commandstr= `echo "$ss" | awk -F';' 'BEGIN {ORS=""}{for (k=1;k<=$resnum;k++){print \$k FS}}'`;
                if ($commandstr=~ /SPECjvm2008/)
                {
                    if ($commandstr=~ / 1T/)
                    {
                        $commandstr=~ s/ 1T//;
                        $commandstr=~ s/SPECjvm2008/SPECjvm2008-1T/;
                    }
                    if ($commandstr=~ / 4T/)
                    {
                        $commandstr=~ s/ 4T//;
                        $commandstr=~ s/SPECjvm2008/SPECjvm2008-4T/;
                    }
                }
                $ss = $commandstr;
            }
            $ss=~ s/ 1c;/;/;
            my @vals = split (";",$ss);
            # TODO: parse modification number
#	    print "$vals[0],$vals[1],$vals[2],";
	    my $ver = "";
            if ($vals[0] =~ /\/[0-9,a-z,A-Z,.]*/)
	    {
	    $ver = $&;
	    $ver =~ s/\///;
            $vals[0] =~ s/\/[0-9,a-z,A-Z,.]*//;
	    }
#	    print "[$ver]\n";
            if (exists ($Config{"b:$vals[0]:number"}) )
            {
                $benchnum=$Config{"b:$vals[0]:number"};
                #print ">>b=".$benchnum." ";
                if (exists ( $Config{"b:".$vals[0].":".$vals[1].":id"}))
                {

                    if(not exists (  $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver}))
                    {
                        $Results{"p:".$propnum.":b:num"}++;
			$Results{"p:".$propnum.":b:".$benchnum.":v:".$ver}='v';
                	if(not exists (  $Results{"p:".$propnum.":b:".$benchnum.":v:num"}))
                	{
                    	    $Results{"p:".$propnum.":b:".$benchnum.":v:num"}=0;
                	}
                        $Results{"p:".$propnum.":b:".$benchnum.":v:num"}++;
                        $Results{"p:".$propnum.":b:".$benchnum.":v:list:".$Results{"p:".$propnum.":b:".$benchnum.":v:num"}}=$ver;
                        $Results{"p:".$propnum.":b:list:".$Results{"p:".$propnum.":b:num"}}=$benchnum;
##                        $Results{"p:".$propnum.":b:list:".$Results{"p:".$propnum.":b:num"}}=$benchnum;
                    }

                    $workloadnum=$Config{"b:".$vals[0].":".$vals[1].":id"};
                    #print ">>w=".$workloadnum." ";
                    if(not exists (  $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum}))
                    {
                        $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:num"}++;
                        $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:list:".$Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:num"}}=$workloadnum;
                    }
                    #print $vals[1]."\n";
                    $vals[2]=~ m/\([0-9]*\)/;
                    my $benchreznum=$&;
                    $benchreznum=~ s/[\(\)]//g;
                    $Results{"p:".$propnum.":b:".$benchnum}='b';
                    $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum} = 'w';
                    if(not exists($Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":iternum"})) { $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":iternum"}=0;}
                    $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":iternum"}++;
                    $iternum = $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":iternum"};
                    $result_dir=~ m/[0-9,_,a-z,A-Z]*\/$/;
                    $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":$iternum:dir"} = $&;
		    $parsed_dirname = $&;
                    $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":$iternum:hostname"} = $hostname;
                    my $actualrunnum = 0;
                    for (my $i=1; $i <= $benchreznum; $i++)
                    {
                        #print "r$i=".$vals[2+$i]." ";
                        if (exists($vals[2+$i]))
                        {
                            if( $vals[2+$i] ne "" and $vals[2+$i] > 0)
                            {
                                $actualrunnum++;
                                $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":".$iternum.":run$actualrunnum"}=$vals[2+$i];
                            }
                        }
                    }
                    $Results{"p:".$propnum.":b:".$benchnum.":v:".$ver.":w:".$workloadnum.":".$iternum.":runnum"}=$actualrunnum;#$benchreznum;
                    #print "reznum=$benchreznum >>> @vals";
                }
                #else { print "UNKNOWN workload name:".$vals[1]."\t  >>> @vals";}
            }
            else
            {
                print "UNKNOWN benchmark name:".$vals[0]."\t  >>> @vals\n";
            }
        }
        else
        {
            #print "-";
        }

    }
    getrunprops($result_dir,$parsed_dirname);
    %uniq= ();
    close $tmpfile;
    return ""; 
}

1;
__DATA__