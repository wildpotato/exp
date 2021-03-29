<!DOCTYPE html>
<html lang="en">
<body>
  <h1>Note</h1>
  <p><strong>Issue:</strong>
  ./bench_malloc: error while loading shared libraries: libmimalloc.so.1.7: cannot open shared object file: No such file or directory</p>
  <p><strong>Solution:</strong>
  shared object: need to export to env path</p>
  <pre>
  $ export LD_LIBRARY_PATH=./:./lib/:$LD_LIBRARY_PATH (add to env path)
  $ env | grep LIB (check LIB)
  </pre>
</body>
<footer>
<p><strong>Third Party Libraries:</strong></p>
  <nav>
    <li><p><a href="https://github.com/gperftools/gperftools">gperftools (tcmalloc)</a></p></li>
    <li><p><a href="https://github.com/microsoft/mimalloc">mimalloc</a></p></li>
    <li><p><a href="https://github.com/lava/matplotlib-cpp">matplotlib-cpp</a></p></li>
  </nav>
</footer>
</html>
