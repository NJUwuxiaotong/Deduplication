result:main.o SHA-256.o hashtable.o rabin_polynomial.o function.o adler32.o deflate.o infblock.o infcodes.o inffast.o inflate.o inftrees.o infutil.o trees.o zd_mem.o zutil.o zdelta.o 
	gcc main.o SHA-256.o hashtable.o rabin_polynomial.o function.o adler32.o deflate.o infblock.o infcodes.o inffast.o inflate.o inftrees.o infutil.o trees.o zd_mem.o zutil.o zdelta.o -o result
adler32.o:adler32.c
	gcc -c adler32.c -o adler32.o
deflate.o:deflate.c
	gcc -c deflate.c -o deflate.o
infblock.o:infblock.c
	gcc -c infblock.c -o infblock.o
infcodes.o:infcodes.c
	gcc -c infcodes.c -o infcodes.o
inffast.o:inffast.c
	gcc -c inffast.c -o inffast.o
inflate.o:inflate.c
	gcc -c inflate.c -o inflate.o
inftrees.o:inftrees.c
	gcc -c inftrees.c -o inftrees.o
infutil.o:infutil.c
	gcc -c infutil.c -o infutil.o
trees.o:trees.c
	gcc -c trees.c -o trees.o
zd_mem.o:zd_mem.c
	gcc -c zd_mem.c -o zd_mem.o
zdelta.o:zdelta.c
	gcc -c zdelta.c -o zdelta.o
zutil.o:zutil.c
	gcc -c zutil.c -o zutil.o
function.o:function.c
	gcc -c function.c -o function.o
rabin_polynomial.o:rabin_polynomial.c
	gcc -c rabin_polynomial.c -o rabin_polynomial.o
main.o:main.c
	gcc -c main.c -o main.o
SHA-256.o:SHA-256.c
	gcc -c SHA-256.c -o SHA-256.o
hashtable.o:hashtable.c
	gcc -c hashtable.c -o hashtable.o
.PHONY:clean
clean:
	rm *.o 
.PHONY:test
test:
	./result /home/qianhuixun/source_dir2/ /home/qianhuixun/tar_dir2/
