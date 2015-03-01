#!/bin/bash
set -e
cd "$(dirname "$0")"
login=odroid@odroid.local
mode=Release

gitupdate() {
	ssh $login "cd work/vision2015/ && git checkout ."
	killall -9 git-daemon || echo
	git daemon &
	ssh $login "cd work/vision2015/ && git pull connor master"
	killall -9 git-daemon
	git diff-index HEAD --binary > patch
	if [ -s patch ]; then
		scp patch $login:work/vision2015/
		ssh $login "cd work/vision2015/ && git apply patch"
	fi
	rm patch
}

compile() {
	ssh $login "cd work/vision2015/build*$mode/ && make -j4"
}

run() {
	ssh $login "matches-2015/kill.sh"
	sleep 3
	ssh $login "killall -9 vision2015 start.sh || echo quit successful"
	ssh $login "cp work/vision2015/build*$mode/vision2015 matches-2015/"
	ssh $login "sudo service vncserver restart"
}

{
	echo "# git update"
	gitupdate 1>&2
	echo "$(( (1 * 100) / 3 ))"
	echo "# compile"
	compile 1>&2
	echo "$(( (2 * 100) / 3 ))"
	echo "# restart program"
	run 1>&2
	echo "$(( (3 * 100) / 3 ))"
} | zenity --progress --title "Deploy Code" --percentage 0
