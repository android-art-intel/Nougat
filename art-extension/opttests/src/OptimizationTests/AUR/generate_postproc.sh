#! /bin/sh
#how to generate postproc:

echo "declare -a testMethodsCounts=(\\"

sed 's/\(.*\)Environment of \([a-zA-Z0-9]*\) \([a-zA-Z]*\)\(@[a-zA-Z0-9]*\) \(.*\) \(no longer uses the value defined by \)\([a-zA-Z0-9]*\) \([a-zA-Z]*\)\(@[a-zA-Z0-9]*.*\)/\"runTest:Environment of \.\* \3@\.\* \6\.\* \8\" \\/' logcat_runTest | grep -v "dex2oat" | sort | uniq -c | sed 's/\(\s*\)\([0-9]*\) \"\(.*\)\" \\/\"\3==\2\" \\/'


sed 's/\(.*\)Environment of \([a-zA-Z0-9]*\) \([a-zA-Z]*\)\(@[a-zA-Z0-9]*\) \(.*\) \(still uses the value defined by \)\([a-zA-Z0-9]*\) \([a-zA-Z]*\)\(@[a-zA-Z0-9]*.*\)/\"runTest:Environment of \.\* \3@\.\* \6\.\* \8\" \\/' logcat_runTest | grep -v "dex2oat" | sort | uniq -c | sed 's/\(\s*\)\([0-9]*\) \"\(.*\)\" \\/\"\3==\2\" \\/'
sed 's/\(.*\)Removing \([a-zA-Z0-9]*\) \([a-zA-Z]*\)\(@[a-zA-Z0-9]*\) \(.*\) \(because it is no longer used by any other instruction or environment.\)/\"runTest:Removing \.\* \3@\.\* \6\" \\/' logcat_runTest | grep -v "dex2oat" | sort | uniq -c | sed 's/\(\s*\)\([0-9]*\) \"\(.*\)\" \\/\"\3==\2\" \\/'

echo ")"

lines=`cat logcat_runTest | wc -l`

echo "declare -a logcatLinesNumber=(\"runTest:${lines}\")"
