<!DOCTYPE html>
<html lang="en">
<body>
<h1>Note</h1>
<p><strong>Issue:</strong>
./bench_malloc: error while loading shared libraries: libmimalloc.so.1.7: cannot open shared object file: No such file or directory
</p>
<p><strong>Solution:</strong>
shared object: need to export to env path
</p>
<pre>
$ export LD_LIBRARY_PATH=./:./lib/:$LD_LIBRARY_PATH (add to env path)
$ env | grep LIB (check LIB)
</pre>
</body>
</html>
