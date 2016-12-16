<?php
/*
 * Copyright (C) 2015 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
?>

<HTML>
<?php
ini_set('error_reporting', E_ALL);
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);

require_once "../config.php";
try
{
    $dbh = new PDO('mysql:host='.$dbhost.';dbname='.$perfdb,$dbuser,$dbpass);
    require "../common/auth.php";
    $permissions=0;
    if ($user['group']=='user') $permissions=1;
    if ($user['group']=='poweruser') $permissions=2;
    if ($user['group']=='admin') $permissions=3;

?>
<HEAD>
    <link rel='icon' href='<?php echo $path_prefix;?>/perfdb/favicon<?php echo $perfdb_favicon_postfix;?>.png' type='image/x-icon'>
    <link rel='stylesheet' type='text/css' href='<?php echo $path_prefix;?>/css/normalize.css' />
    <!--link rel='stylesheet' type='text/css' href='<?php echo $path_prefix;?>/css/style.css' /-->
    <link rel='stylesheet/less' type='text/css' href='<?php echo $path_prefix;?>/css/_styles.less' />

    <STYLE>
.clear {
clear:both;
display:block;
height:0px;
font-size:0;
overflow:hidden;
}

.blink
    {
    display:block;
    margin-bottom:70px;
    margin-top:-50px
    }
.proplist {
    display: inline-block;
    cursor: pointer;
}
#proplistwindow{
    display: block;
    position: fixed;
    width: 50%;
    height: 50%;
    background-color: lightsteelblue;
    border: 3px double;
    left: 25%;
    top: 25%;
    text-align: center;
    opacity: 0.98;
}
#proplistwindow #header{
    height: 30px;
    background-color: #0071C5;
    border-bottom: 1px solid black;
    border-top: 1px solid white;
}
#proplistwindow #header #caption{
    text-align: left;
    padding: 7px 10px;
    float: left;
    color: white;
    font-weight: bolder;
font-size: smaller;
}

#proplistwindowcontent{
    text-align: left;
    font-size: smaller;
    overflow-y: scroll;
    height: 80%;
    padding-left: 25px;
padding-top: 20px;
}
#proplistwindowclose{
    float: right;
    margin: 3px 3px;
    background-color: lightgrey;
    padding: 2px 5px;
    border: 1px solid;
    border-radius: 2px;
}
#proplistwindowclose:hover{    background-color: darkgrey;cursor: pointer;}
#proplistwindow.hide{ display: none;}
#proplistwindowcontent table {
    margin: 0px auto;
    border: 1px solid black;
    border-collapse: separate;
    border-spacing: 3px;
}
#proplistwindowcontent div {
padding-left:20px;
}
#proplistwindowcontent table th {
    border-spacing: 2px;
    background-color: lightgrey;
    font-size: smaller;
        padding: 3px;
}
#proplistwindowcontent table td {
    font-size: smaller;
text-align: center;
    background-color: whitesmoke;
    padding: 3px;
}
.capt {
    font-size: larger;
    font-weight: bolder;
}


#proplistwindowcontent .dispnone {
border: none;
background-color: transparent;
text-align: left;
vertical-align: top;
}

    </STYLE>
    <script src="<?php echo $path_prefix;?>/js/jquery.min.js"></script>
    <script src="<?php echo $path_prefix;?>/js/less.min.js" type="text/javascript"></script>
    <SCRIPT>
//	window.open('<?php echo $path_prefix;?>/perfdb/weeklyreport.php?deviceid='+deviceid+'&branchid='+branchid+'&backendid='+backendid+'&mode='+mode+'&source='+source,'');
<?php
//if ($user{'access'}>0)
    if (1)
    {
	if ($permissions>0)
	{
?>
function getCookie(name)
{
    var cookie = " " + document.cookie;
    var search = " " + name + "=";
    var setStr = null;
    var offset = 0;
    var end = 0;
    if (cookie.length > 0)
    {
	offset = cookie.indexOf(search);
	if (offset != -1)
	{
	    offset += search.length;
	    end = cookie.indexOf(";", offset)
	    if (end == -1)
	    {
		end = cookie.length;
	    }
	    setStr = unescape(cookie.substring(offset, end));
	}
    }
    return(setStr);
}

function change(buildid,workloadid,obj,classn)
{
    caption=obj.innerHTML;
    score = obj.innerHTML;
    obj.innerHTML= 'Processing...';
    $.ajax(
    {
	url : '<?php echo $path_prefix;?>/perfdb/tools/changemedian.php',
	dataType : 'json',
	data : {buildid:buildid,workloadid:workloadid,act:classn,score:score},
	type : 'POST',
	success:function(response)
	{
	    if (!response.errcode)
	    {
if (obj.className == classn)
    {
		var elem = $("."+classn+'m');
		if (elem.length >0)
		{
		    var st = new String (elem[0].innerHTML);
		    if (st.charAt(3) == '[')
		    {
			st = st.substring(4,st.length-5);
		    }
		    elem[0].innerHTML = st;
		    elem[0].className=classn;
		}
		obj.innerHTML='<b>['+response+']</b>';
		obj.className=(classn+'m');
}
else
{

		obj.innerHTML = caption;
		var elem = $('.cl'+buildid+'s'+workloadid+'m');
		for (i=0;i<elem.length;i++)
		{
		    if (elem.length >0)
		    {
			var st = new String (elem[i].innerHTML);
			if (st.charAt(3) == '[')
			{
			    st = st.substring(4,st.length-5);
			}
			elem[i].innerHTML = st;
			elem[i].className='cl'+buildid+'s'+workloadid;
		    }
		}
		var elem = $('.cl'+buildid+'s'+workloadid);
		for (i=0;i<elem.length;i++)
		{
		    if (elem.length >0)
		    {
			var st = new String (elem[i].innerHTML);
			if (st == response)
			{
			    elem[i].className='cl'+buildid+'s'+workloadid+'m';
			    elem[i].innerHTML='<b>['+response+']</b>';
			    i=elem.length;
			}
		    }
		}
}
	    }
	    else
	    {
		consoleShowProps (response.errcode,'err');
		$("#container").toggle ();
		if (stop == 0)
		{
		    alert(response.message);
		    stop = 1;
//TODO: clear all
		}
	    }
	},
	error:function (xhr, ajaxOptions, thrownError)
	{
	    var err = $.parseJSON(xhr.responseText);
	    consoleShowProps (err.errcode,'err');
	    alert("!!!!!"+thrownError);
	},
	cache:false
    });
}

function changeold(buildid,workloadid, obj,classn)
{

if (obj.className == classn)
    {
	xhr=new XMLHttpRequest(),
	xhr.onreadystatechange=function()
	{
	    if (xhr.readyState==4 && xhr.status==200)
	    {
		var elem = $("."+classn+'m');
		if (elem.length >0)
		{
		    var st = new String (elem[0].innerHTML);
		    if (st.charAt(3) == '[')
		    {
			st = st.substring(4,st.length-5);
		    }
		    elem[0].innerHTML = st;
		    elem[0].className=classn;
		}
		obj.innerHTML='<b>['+xhr.responseText+']</b>';
		obj.className=(classn+'m');
	    }
	}
	score=obj.innerHTML;
	obj.innerHTML= 'Processing...';
	form=new FormData();
	form.append('buildid',buildid);
	form.append('score',score);
	form.append('workloadid',workloadid);
	xhr.open('post','<?php echo $path_prefix;?>/perfdb/tools/changemedian.php',true);
	xhr.send(form);
    }
    else
    {
	xhr=new XMLHttpRequest(),
	xhr.onreadystatechange=function()
	{
	    if (xhr.readyState==4 && xhr.status==200)
	    {
		obj.innerHTML = caption;
		var elem = $('.cl'+buildid+'s'+workloadid+'m');
		for (i=0;i<elem.length;i++)
		{
		    if (elem.length >0)
		    {
			var st = new String (elem[i].innerHTML);
			if (st.charAt(3) == '[')
			{
			    st = st.substring(4,st.length-5);
			}
			elem[i].innerHTML = st;
			elem[i].className='cl'+buildid+'s'+workloadid;
		    }
		}
		var elem = $('.cl'+buildid+'s'+workloadid);
		for (i=0;i<elem.length;i++)
		{
		    if (elem.length >0)
		    {
			var st = new String (elem[i].innerHTML);
			if (st == xhr.responseText)
			{
			    elem[i].className='cl'+buildid+'s'+workloadid+'m';
			    elem[i].innerHTML='<b>['+xhr.responseText+']</b>';
			    i=elem.length;
			}
		    }
		}
		
	    }
	}
	caption=obj.innerHTML;
	obj.innerHTML= 'Processing...';
	form=new FormData();
	form.append('buildid',buildid);
	form.append('score',classn);
	form.append('workloadid',workloadid);
	xhr.open('post','<?php echo $path_prefix;?>/perfdb/tools/changemedian.php',true);
	xhr.send(form);
    }
}
function switchevaluationstatus(buildid,obj)
{
    var pervcont = obj.innerHTML;
    xhr=new XMLHttpRequest(),
    xhr.onreadystatechange=function()
    {
	if (xhr.readyState==4 && xhr.status==200)
	{
	    if (xhr.responseText == '0')
	    {
		obj.className='evaluated0';
		obj.innerHTML= '<p style="margin-top: 64px;">Not evaluated</p>';
	    }
	    else if (xhr.responseText == 'ERROR')
	    {
		obj.innerHTML = prevcont;
	    }
	    else
	    {
		obj.className='evaluated1';
		obj.innerHTML= '<p style="margin-top: 64px;">Evaluated by <br>' + xhr.responseText + '</p>';
	    }
	}
    }
//    sid=getCookie('perf.sid');
    obj.innerHTML= '<p style="margin-top: 64px;">Processing...</p>';
    form=new FormData();
    form.append('buildid',buildid);
//    form.append('sid',sid);
    xhr.open('post','<?php echo $path_prefix;?>/perfdb/tools/changeevaluatestate.php',true);
    xhr.send(form);
}
<?php
	}
	if ($permissions>2)
	{
?>
function deletebuild(buildid)
{
    if (confirm('Are you sure that you want to delete build ('+buildid+')?')) 
    {
	xhr=new XMLHttpRequest(),
	xhr.onreadystatechange=function()
	{
	    if (xhr.readyState==4 && xhr.status==200)
	    {
		if (xhr.responseText == 'deleted') { window.location ='<?php echo $path_prefix;?>/perfdb/builddetails.php?buildid='+buildid;}
		else {   alert ( 'err='+xhr.responseText);}
	    }
	}
	form=new FormData();
	form.append('buildid',buildid);
	xhr.open('post','<?php echo $path_prefix;?>/perfdb/tools/deletebuild.php',true);
	xhr.send(form);
    }
}
<?php
    }
?>
    </SCRIPT>
</HEAD>
<BODY class='body'>
    <div id='proplistwindow' class='hide'><div id='header'><div id='caption'></div><div id='proplistwindowclose'>X</div></div><div id='proplistwindowcontent'></div></div>
    <!--DIV class='loader-background'><span class="loader"><span class="loader-inner"></span></span></DIV-->
    <DIV class='navbar'  id='TopMenu' ><?php require_once "navibar.php"; ?></DIV>
    <DIV id="includedContent">
<?php
	$hiddencontent=array();
	$hiddencaptions=array();
	$buildid = intval($_GET['buildid']);
	$sql= 'SELECT build,list_devices.codename,mode,list_backends.backend,source,evaluated,data_users.name,is_target
	FROM data_builds,list_devices,list_backends,data_users
	WHERE data_builds.id = '.$buildid.' and data_builds.device=list_devices.id and data_builds.backend = list_backends.id and if (isnull(evaluatedby), data_users.id=2,evaluatedby = data_users.id);';
	$query = $dbh->prepare($sql); $query->execute();
	$fv = $query->fetch(PDO::FETCH_BOTH);
	$build = $fv[0];
	$device=  $fv[1];
	$mode=  $fv[2];
	$backend = $fv[3];
	$source = $fv[4];
	if ($fv)
	{
	    print "<div id='FAQ-icon' onmouseover='\$(\"#FAQ-layer\").show();' onmouseout='\$(\"#FAQ-layer\").hide();' ></div>\n";
	    if ($permissions>0)
	    {
		print "<div class='evaluated$fv[5]' id='evaluatecheckbox' onclick='switchevaluationstatus($buildid,this);' ><p style=\"margin-top: 64px;\">".($fv[5]?'Evaluated by <br>'.$fv[6]:'Not evaluated')."</p></div>\n";
	    }
	    if ($user["group"] == "admin")
	    {
		print "<div id='builddeletebtn' onclick='deletebuild($buildid);' ></div>\n";
	    }
	    print "<DIV id='FAQ-layer' >
	<H2>Legend</H2><HR width=80%>
	<table cellspacing=5 class='center'>
	    <tr><td><b>[25270]</b></td><td> selected score</td></tr>
	    <tr><td><s>19768</s></td><td> filtered score</td></tr>
	    <tr><td><img src='/images/up16.png'></td><td> more is better</td></tr>
	    <tr><td><img src='/images/downred16.png'></td><td> less is better</td></tr>
	    <tr><td><b>Overall *</b></td><td> main workload</td></tr>
	</table></DIV>";
	    print "<h3 style='font-family: initial;'>".($fv[7]==1?"[Target] ":"")."$build on  <a href='/perfdb/devices.php' style='color: darkturquoise; text-decoration: none;'>$device</a> device in $mode mode with $backend backend. Scores were collected by $source team.</h3><hr>\n";
//20
	    $sql= 'SELECT build,backend,device,`group`,mode,if (`group` <> benchmark,concat(benchmark," - ",workload),workload),workloadid, view_allresults2.directory, hostname, group_concat(result)  as scores,buildid,
lessisbetter,criteria,group_concat(filtered),group_concat(ismedian),group_concat(`ignore`),ismain,concat("(",version,")") as "version",is_target,float_value,
data_directories.`cores_freq_min`,
data_directories.`cores_freq_max`,
data_directories.`cores_freq_avg`,
data_directories.`cpu_freq_min`,
data_directories.`cpu_freq_max`,
data_directories.`cpu_freq_avg`,
data_directories.`cpu_freq_histo`,
data_directories.`cpu_corenum`,
data_directories.`cpu_governer`,
data_directories.`cpu_temp_start`,
data_directories.`cpu_temp_end`,
data_directories.`cpu_temp_min`,
data_directories.`cpu_temp_max`,
data_directories.`cpu_temp_avg`,
data_directories.`cpu_throttle_count`,
data_directories.`env_temp`,
data_directories.`time_length`,
data_directories.`id`
	    FROM view_allresults2,data_directories
	    WHERE  view_allresults2.directoryid=data_directories.id AND buildid = '.$buildid.' GROUP BY backendid ,mode,`group`, benchmarkid , workload , device, directory ORDER BY device , backendid , mode DESC, isKPI DESC, benchmarkid, worder , workloadid , directory;';
//	    print ($sql);
	    $query = $dbh->prepare($sql); $query->execute();
	    $sql='SELECT ROUND(100.0*stddev(result)/avg(result),2) as variance
	    FROM view_allresults2 WHERE buildid =  '.$buildid.' GROUP BY backendid ,mode, benchmarkid , workload , device ORDER BY device , backendid , mode DESC, isKPI DESC, benchmarkid , workloadid ;';
	    $variancequery = $dbh->prepare($sql); $variancequery->execute ();
	    $prevworkload="";
	    $prevbenchmark="";
	    $wrecnum=0;
	    $brecnum=0;
	    $tmpstr="";
	    $tmpstr2="";
	    $text="";
	    $allscores= array ();
		print "<TABLE class='menutable w90 center' cellspacing=0>\n";

	    while ($fv = $query->fetch(PDO::FETCH_BOTH))
	    {
//		print $fv;
		$showprops=0;
		if (isset ($fv[34])) $showprops=1;
		$class = "class='newd'";
		if ( $prevworkload != $fv[6])
		{
		    if ($prevworkload  != "")
		    {
			if ( $brecnum > 0) {$tmpstr2.="<tr>";}
			if ($first) {$class = "class=''";}
			else {$class = "class='neww'";}
			$variance = $variancequery->fetch(PDO::FETCH_BOTH);
			$tmpstr2.="<td $class rowspan=$wrecnum width=200px>".($ismain?"<b>$prevworkloadname *</b>":"$prevworkloadname")."</td><td $class rowspan=$wrecnum width=100px><p style='text-align:center;'><img src='".($lessisbetter?"/images/downred16.png":"/images/up16.png")."'> - <i>$criteria</i></p></td><td $class rowspan=$wrecnum><center>".$variance[0]." %</center></td>".(($permissions>0)?"<td $class  rowspan=$wrecnum><center><a style='color:red;' onclick='change($buildid,$prevworkload,this,\"clear\");'>[CLEAR]</a><br><a style='color:green;' onclick='change($buildid,$prevworkload,this,\"max\");'>[MAX]</a><BR><a style='color:cadetblue;' onclick='change($buildid,$prevworkload,this,\"median\");'>[MEDIAN]</a><br><a style='color:blue;' onclick='change($buildid,$prevworkload,this,\"min\");'>[MIN]</a></center></td>":"").$tmpstr;
			$allscores = array();
			$tmpstr = "";
			$wrecnum=0;
		    }
		    $ismain= $fv[16];
		    $criteria = $fv[12];
		    $class = "class='neww'";
		    $first=0;
		    $lessisbetter = $fv[11];
		    $prevworkload = $fv[6];
		    $prevworkloadname = $fv[5];
		}
		if ( $prevbenchmark != $fv[3])
		{
		    $class = "class=''";
		    if ($prevbenchmark != "")
		    {
			$brecnum++;
//			print "";

//    $('html, body').animate({        scrollTop: $("#elementtoScrollToID").offset().top    }, 2000);
			print "\n\t</tr>\n<TR><TD colspan=8 class='hsplitter'><a  name='$prevbenchmark' class='blink'></a></TD></TR>\n";
			print "<tr><th >Configuration</th><th>Workload</th><th>Criteria</th><th>stdev/avg*100%</th>".(($permissions>0)?"<th>Tools</th>":"")."<th>Scores</th>".($showprops?"<th>Stats</th>":"")."<th>Hostname</th><th>Directory</th></tr>\n";
//			print "<TABLE border=1 class='menutable w90 center' cellspacing=0>\n<tr><th>Configuration</th><th>Workload</th><th>Criteria</th><th>stdev/avg*100%</th>".(($user["group"] == "admin" || $user["group"] == "user")?"<th>Tools</th>":"")."<th>Scores</th><th>Hostname</th><th>Directory</th></tr>\n";
			print "<tr><td rowspan=$brecnum width=200px><b>$prevbenchmark $prevversion</b><hr>$build<br>$backend<br>$device<br>$mode</td>".$tmpstr2."</tr>";
//			print "\n\t</tr>\n</TABLE><br>\n";
//			print "\n\t</tr>\n<TR><TD colspan=8 class='hsplitter'</TD></TR>\n";
			$wrecnum=0;
			$brecnum=0;
			$tmpstr2 = "";
			$tmpstr = "";
		    }
		    $first=1;
		    $prevbenchmark = $fv[3];
		    $prevversion = $fv[17];
		    $prevworkload = $fv[6];
		    $prevworkloadname = $fv[5];
		}
		$fv[9] = str_replace('^ ','',$fv[9]);
		if ($fv[19] == 0) {$fv[9] = str_replace('.0000','',$fv[9]);}
		if ( $wrecnum> 0) {$tmpstr.="<tr>";}
	//$"=", ";
		$scores= split (',',$fv[9]);
		$allscores = array_merge($allscores,$scores);
		$filtered= split (',',$fv[13]);
		$ismedian= split (',',$fv[14]);
		$ignored= split (',',$fv[15]);
		$class2="cl".$fv[10]."s".$fv[6];
		$suffix="";
		for ($i=0; $i < count($scores); $i++)
		{
		    if ($ismedian[$i])
		    {
			$suffix="m";
			$scores[$i]="<b>[".$scores[$i]."]</b>";
		    }
		    else
		    {
			$suffix="";
		    }
//#	    if ($filtered[$i])  {$scores[$i]="<s>".$scores[$i]."</s>";  }
//#	    if ($ignored[$i])   {$scores[$i]="<i>".$scores[$i]."</i>";    }
		    if (($permissions>0))
		    {
			$scores[$i]="<a class='".$class2.$suffix."' onclick='change($fv[10],$fv[6],this,\"$class2\")'>".$scores[$i]."</a>";
			

		    }
if ($filtered[$i])  {$scores[$i]="<s>".$scores[$i]."</s>";  }
		}
	    $propstr="";
	    if ($showprops)
	    {
		$propstr ="<td><center><span class='proplist ' onclick='showpropwindow($fv[37]);'><img src='/images/16".($fv[34]>0?"attention":"list").".png'/></span></center></td>";
		
		//$content ="<h3>Device core count: $fv[27]</h3>";
		$content ="<table class=\"dispnone\"><tr><td class=\"dispnone\"><div><span class=\"capt\">Used governor:</span> $fv[28]</div></td><td class=\"dispnone\"></td></tr>";
		$content.="<tr><td class=\"dispnone\"><div><h3>Frequencies:</h3> <table>";
		$content.="<tr><th></th>";
		for ($i=0;$i<$fv[27];$i++)
		    $content.="<th>Core $i</th>";
		$content.="<th>CPU</th></tr>";

		$freqs = json_decode($fv[20]);
		$content.="<tr><th>Min freq</th>";
		if ( is_array ($freqs) )
		{
		foreach ($freqs as $value)
		    $content.= "<td>$value</td>";
		}
		$content.= "<td>$fv[23]</td></tr>";

		$freqs = json_decode($fv[21]);
		$content.="<tr><th>Max freq</th>";
		if ($freqs)
		{
		foreach ($freqs as $value)
		    $content.= "<td>$value</td>";
		$content.="<td>$fv[24]</td></tr>";
		}
		
		$freqs = json_decode($fv[22]);
		$content.="<tr><th>AVG freq</th>";
		if ($freqs)
		{
		foreach ($freqs as $value)
		    $content.= "<td>".sprintf("%.2f",$value)."</td>";
		}
		$content.="<td>$fv[25]</td></tr></table><br></div></td>";
		
		$content.="<td class=\"dispnone\"><div><h3>Histogram:</h3><pre>";
		$freqs = json_decode($fv[26]);
		$maxnum=0;
		if ($freqs != "")
		{
		foreach ($freqs as $item)
		    foreach ($item as $key => $value)
			$maxnum+=$value;
		$symb="|";
		$symb2=" ";
		$strlen=20;
		
		foreach ($freqs as $item)
		    foreach ($item as $key => $value)
		    {
			$repeatnum = $value/$maxnum*100/5;
			$content.="$key:\t[";
			$content.= str_repeat ("$symb", $repeatnum);
			if ($strlen-$repeatnum>0)
			{
			    if ($repeatnum!=0) {$content.="$symb2";}
			    $content.=str_repeat("$symb2",($strlen-$repeatnum));
			}
			$content.="] ($value)\\n";
			
		    }
		}
		else
		{$content.= "n/a";}
		$content.="</pre></div><br></td>";

		$content.="</tr><tr>";
		$content.="<td class=\"dispnone\"><div>";
		$content.="<span class=\"capt\">Run time (sec):</span> $fv[36]<br>";
		$content.="<span class=\"capt\">Throttle count:</span> ".($fv[34]==-1?"n/a":($fv[34]>0?"<font color=red>$fv[34]</font>":$fv[34]))."<br>";
		$content.="<span class=\"capt\">Throttles/sec:</span> ".(($fv[34]==-1||$fv[36]==0)?"n/a":$fv[34]/$fv[36])."<br>";
		$content.="<span class=\"capt\">Env temperature (battery):</span> ".($fv[35]==-1?"n/a":($fv[35]/1000)."&ordm;")."<br></div></td>";

		$content.="<td class=\"dispnone\"><div>";
		$content.="<span class=\"capt\">CPU temperatures:</h3></span><br><table>";
		$content.="<tr><th>Start</th><th>Min</th><th>Max</th><th>Avg</th><th>End</th></tr><tr>";
		$content.="<td>".($fv[29]==-1?"n/a":($fv[29]/1000)."&ordm;")."</td>";
		$content.="<td>".($fv[31]==-1?"n/a":($fv[31]/1000)."&ordm;")."</td>";
		$content.="<td>".($fv[32]==-1?"n/a":($fv[32]/1000)."&ordm;")."</td>";
		$content.="<td>".($fv[33]==-1?"n/a":($fv[33]/1000)."&ordm;")."</td>";
		$content.="<td>".($fv[30]==-1?"n/a":($fv[30]/1000)."&ordm;")."</td>";
		$content.="</tr></table></div></td></tr></table>";



		
/*
    20 data_directories.`cores_freq_min`,
    21 data_directories.`cores_freq_max`,
    22 data_directories.`cores_freq_avg`,
    23 data_directories.`cpu_freq_min`,
    24 data_directories.`cpu_freq_max`,
    25 data_directories.`cpu_freq_avg`,
    26 data_directories.`cpu_freq_histo`,
    27 data_directories.`cpu_corenum`,
    28 data_directories.`cpu_governer`,
    29 data_directories.`cpu_temp_start`,
    30 data_directories.`cpu_temp_end`,
    31 data_directories.`cpu_temp_min`,
    32 data_directories.`cpu_temp_max`,
    33 data_directories.`cpu_temp_avg`,
34 data_directories.`cpu_throttle_count`,
35 data_directories.`env_temp`,
36 data_directories.`time_length`,
*/
		$hiddencontent[$fv[37]] =$content;
		$hiddencaptions[$fv[37]]=$fv[7]." (".$fv[8].")";
	    }

		$tmpstr.="<td $class width=400px style='font-family: courier;'>".implode (', ',$scores)."</td>$propstr<td $class width=200px><nobr>$fv[8]</nobr></td><td $class>$fv[7]</td></tr>\n";
		$wrecnum++;
		$brecnum++;
	    }
	    $brecnum++;
	    $wrecnum++;
	    $class = "class='neww'";
	    $variance = $variancequery->fetch(PDO::FETCH_BOTH);
	    $tmpstr2.="<td $class rowspan=$wrecnum width=200px>".($ismain?"<b>$prevworkloadname *</b>":"$prevworkloadname")."</td><td $class rowspan=$wrecnum width=100px><p style='text-align:center;'><img src='".($lessisbetter?"/images/downred16.png":"/images/up16.png")."'> - <i>$criteria</i></p></td><td $class rowspan=$wrecnum><center>".$variance[0]." %</center></td>".(($permissions>0)?"<td $class  rowspan=$wrecnum><center><a style='color:red;' onclick='change($buildid,$prevworkload,this,\"clear\");'>[CLEAR]</a><br><a style='color:green;' onclick='change($buildid,$prevworkload,this,\"max\");'>[MAX]</a><BR><a style='color:cadetblue;' onclick='change($buildid,$prevworkload,this,\"median\");'>[MEDIAN]</a><br><a style='color:blue;' onclick='change($buildid,$prevworkload,this,\"min\");'>[MIN]</a></center></td>":"").$tmpstr;
	    print "\n\t</tr>\n<TR><TD colspan=8 class='hsplitter'><a name='$prevbenchmark'></a></TD></TR>\n";
//	    print "<a name='$prevbenchmark' style='display:block;margin-bottom:70px; margin-top:-50px'></a>";
	    print "<tr><th>Configuration</th><th>Workload</th><th>Criteria</th><th>stdev/avg*100%</th>".(($permissions>0)?"<th>Tools</th>":"")."<th>Scores</th>".($showprops?"<th>Props</th>":"")."<th>Hostname</th><th>Directory</th></tr>\n";
	    print "<tr><td rowspan=$brecnum width=200px><b>$prevbenchmark $prevversion</b><hr>$build<br>$backend<br>$device<br>$mode</td>".$tmpstr2."</tr>";
	    print "\n\t</tr>\n</TABLE><br>\n\n$text\n";
	    $content="<SCRIPT>\npropsdata= new Object (null);propscaptions= new Object (null);";
	    
	    foreach($hiddencontent as $key=>$value)
	    {
	    	//$mykey = key($array);
		$content.="\npropsdata[$key]='$value';";
	    }
	    foreach($hiddencaptions as $key=>$value)
	    {
	    	//$mykey = key($array);
		$content.="\npropscaptions[$key]='$value';";
	    }


	    $content.="\n</SCRIPT>";
	    print "$content";
	}
	else
	{
	    print "<h2>No such build. :-(</h2>";
	}

    }
    else
    {
    $err{'errcode'}=403;
    $err{'message'}='403 Forbidden you have no access to this tool...';
    print json_encode($err);
    }
    $dbh = null;
}
catch (PDOException $e)
{
    print "Error!: " . $e->getMessage() . "<br/>";
    die();
}
?>
<SCRIPT>
    $('#proplistwindowclose').on ('click',function ()
    {
	$('#proplistwindow').addClass('hide');
    });

    $('*').on ('keyup',function (e)
    {
    if (e.which==27){ $('#proplistwindow').addClass('hide');}
    });

function showpropwindow (propnum)
{
$("#proplistwindow #header #caption").empty()[0].innerHTML=propscaptions[propnum];
$("#proplistwindowcontent").empty().append(propsdata[propnum]).parent().removeClass("hide");
}
</SCRIPT>
    </DIV>
</BODY>
</HTML>
