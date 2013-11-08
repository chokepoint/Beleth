Beleth
======

Dictionary based SSH cracker

Usage: ./beleth [OPTIONS]
	-c [payload]	Execute payload on remote server once logged in
	-h				Display this help
	-l [threads]	Limit threads to given number. Default: 4
	-p [port]		Specify remote port
	-t [target]		Attempt connections to this server
	-u [user]		Attempt connection using this username
	-v				-v (Show attempts) -vv (Show debugging)
	-w [wordlist]	Use this wordlist. Defaults to wordlist.txt

Example:

$ ./beleth -l 15 -t 127.0.0.1 -u stderr -w wordlist.txt
[*] Read 71 passwords from file.
[*] Starting task manager
[*] Spawning 15 threads
[*] Starting attack on stderr@127.0.0.1:22
[!] No password matches found.

