ver0.1   on Jan. 9, 2020

# fsXT
A tool for filesystem eXtreme Testing on linux.

This tool consists of two parts, a core part and an a organizing part.
* The core part is used to generate random content, write to files, calculate md5 digests to valid writing reliability.
* The orginaizing part is used to 
> * orchestrate different number of OS processes to write, size of each write (record), number of records per file and number of files; 
> * collect data of timestamps on file open, close, mv, write, read, etc. ; 
> * analysize and summary collected data, and output report eventually.


## The core part --- Torturefs
It is written by GNU C, abbr. is ttfs, source is in src/ttfs.
You need install openssl development library before you build it.


## The orginazing part
This part is all in a jupyter notebook file fsXT.ipynb.
### Concept definition
* file system -- the tested posix file system, including distributed ones, such as glusterfs, and single-host ones, such as xfs.
* file -- the file written and read on file system.
* record -- the entity written to file which has a fix length for one file.
* worker -- a client of file system, technically is a process occupying a core of CPU.
* combination -- combines arguments of length of record, number of record, files for per worker and number of workers.
* epoch -- a turn of testing file system with an unique combination.

### Aim to archieve
* evaluate the reliability of file system.
* find combinations of record length, record number, files, workers for the best whole effiency of host and the best process efficency.

### Steps to use this fsXT
* Before do the following steps, you should build (make) the source, the executive file fsXT will be generated and saved to fsXT/src/fsXT directory where this notebook file is in.
* Step 1: Please adjust as your wish if the following default values of arguments are inapproporiate.
* Step 2: Generate test scripts.
* Step 3: Run test and wait until all epochs finished.
* Step 4: Collect data from log files of epochs.
* Step 5: Wrange data.
* Step 6: Evaluate reliablity by comparing md5 digest with writing and md5 digest with reading for each file.
* Step 7: Analysize correlationship based on epoch speed, file write speed and file read speed
* Step 8: Score combinations on epoch speed 70%, file write speed 20%, file read speed 10%, and show top 10.
* Step 9: Draw I/O graphs of top 10.
