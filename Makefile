default: arduino

arduino:
	gcc -shared -ansi -O2 -fPIC -I/home/thomas/dev/Pd-extended-build/pd-svn/pd-original/src arduino_message_parser.c -o arduino_message_parser.pd_linux
