sudo mount -t cifs\
	-o username=eero,password=eero,uid=$(id -u),gid=$(id -g)\
	//192.168.0.16/sharethis /home/eero/all/chesss/windows-share