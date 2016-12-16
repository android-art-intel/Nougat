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

sub getMedianRunNums ()
{
    our %Results;
    our %Config;
    my $propscount = $Results{"props:count"};
    for (my $i = 1; $i<=$propscount; $i++)
    {
	my $benchcount = $Results{"p:$i:b:num"};
	for (my $j = 1; $j<=$benchcount; $j++)
	{
	my $benchnum = $Results{"p:$i:b:list:$j"};
	my $vercount= $Results{"p:$i:b:$benchnum:v:num"};
	for (my $l = 1; $l<=$vercount; $l++)
	{

    	    if ( not defined $Results{"p:$i:b:$benchnum:v:list:$l"})
	    {
    		print "\np:$i:b:$benchnum:v:list:$l";
	    }
	    my $vernum = $Results{"p:$i:b:$benchnum:v:list:$l"};
	    my $workloadcount = $Results{"p:$i:b:$benchnum:v:$vernum:w:num"};
	    for (my $k = 1; $k<=$workloadcount; $k++)
	    {
#		my $medianrunnum ;
		my $workloadnum = $Results{"p:$i:b:$benchnum:v:$vernum:w:list:$k"};
		my $bench=$Config{"bid:".$benchnum.":name"};
		my $criteria = $Config{"b:$bench:id:".$workloadnum.":criteria"};
		my $lessisbetter = $Config{"b:$bench:id:".$workloadnum.":lessisbetter"};
		my $itercount = $Results{"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:iternum"};
		$Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} = -1;
		my @runscores;
		my %runscores;
		my $totalresnum = 0;
		for (my $m=1; $m<=$itercount; $m++)
		{
		    my $runcount = $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:runnum"};
		    for (my $n=1; $n<=$runcount; $n++)
		    {
			if ($Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} > 0)
			{
			    @runscores = ($Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} , @runscores);    
			    $runscores { $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} }=$n+$totalresnum;
			}
		    }
		    $totalresnum +=$runcount;
		}
		if ($totalresnum == 0) {next;}
		@runscores = sort {$a <=> $b } @runscores;

	##USING FILTER!!! if results number > 3
		if ($totalresnum>3)
		{
		    my $min = $runscores[0];
		    my $max = $runscores[(scalar @runscores)-1];
		    my $median = $runscores[($totalresnum)/2];

		    my $delta = ($max - $min )/2;
		    my $fmin = $median - $delta*($lessisbetter == 1?100:1);
		    my $fmax = $median + $delta*($lessisbetter == 0?100:1);
		
		    @runscores=();
		    %runscores=();
		    $totalresnum = 0;
		    for (my $m=1; $m<=$itercount; $m++)
		    {
			my $runcount = $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:runnum"};
			for (my $n=1; $n<=$runcount; $n++)
			{
			    if ($Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} < $fmin or $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} >$fmax)
			    {$Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n:filtered"} = 1; }
			    else
			    {
				@runscores = ($Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} , @runscores);
				$totalresnum++;
				$runscores { $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:$m:run$n"} }=$totalresnum;
			    }
			}
		    }
		    @runscores = sort {$a <=> $b } @runscores;
		}
		if ($criteria eq "min") { $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} = $runscores[0];}
		elsif ($criteria eq "max") { $Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} = $runscores[(scalar @runscores)-1];}
		else
		{
#		$medianrunnum = $runscores{ $runscores[($totalresnum)/2]};
		$Results {"p:$i:b:$benchnum:v:$vernum:w:$workloadnum:medianrunvalue"} = $runscores[(($totalresnum))/2];
		}
	    }
	}
	}
    }
}
1;
__DATA__


