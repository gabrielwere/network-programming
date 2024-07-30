## FTP-CLIENT
Basic ftp client built for the `pyftpdlib` server.<br/>
You need to have python and pyftpdlib installed.<br/>

#### Compilation
 ```
 gcc -g ftp.c -o ftp

 ```
#### Usage

* On a new terminal,start the ftp server on port 21
	`sudo python3 -m pyftpdlib -p 21`

* Start the ftp client
	`./ftp`
 
