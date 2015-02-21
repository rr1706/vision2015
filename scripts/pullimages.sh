#!/bin/bash
set -e
cd "$(dirname "$0")"
login=odroid@odroid.local

ssh $login ./matches-2015/kill.sh
echo "cd matches-2015
lcd ../matches
ls
get -r 2015*" > batchfile
sftp -b batchfile $login
ssh $login "rm -r matches-2015/2015*"
ssh $login "sudo service vncserver restart"
