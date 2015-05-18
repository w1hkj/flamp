To Obtain or Build the Documentation:

Documents are not provided by the normal program distribution. In order to
obtain the documents, there are two options.

1. Download from W1HKJ's website.
2. Create from Source by downloading the source code from SourceForge.

Requirements for Building the Documents from source:
	HTML - DOxygen Version 1.8.9.
	PDF  - TeXLive Version 2013.

Other versions might work with the exception of TeX Live 2014, which failed
to create the Documents at the time this readme file was created.

Links to Documents:

W1HKJ's Website: www.w1hkj.com/download

In order to download the documents source, the program GIT is required.

git clone git://git.code.sf.net/p/fldigi/flamp flamp-master

To create from source after cloning a copy from SourceForge:

'cd' into the doxygen directory in the source code top directory.

Issue this command:
    ./make_docs.sh <user> and/or <prog>

Example: ./make_docs.sh user prog

If there are no error, the documentation will be place in the doc/
directory in the root of the source code location.

Issues? I can be reached at:

kk5vd@yahoo.com


Robert, KK5VD
FLDIGI Development Team
