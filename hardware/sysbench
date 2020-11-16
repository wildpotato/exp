-----------------------------------------------------------------
Sysbench General Guideline (for more details please see manual)
-----------------------------------------------------------------
#1 Benchmark CPU
#
# 每個request會讓CPU去計算直到上限的質數,並以此作為表現依據
# 這邊最在意的會是execution time
#

$ sysbench cpu --cpu-max-prime=20000 run

See exampleOutput
-----------------------------------------------------------------
#2 Benchmark File IO
#
# 用來測試File I/O, 這邊會有prepare(create files), run, 以及
# cleanup三個步驟, --file-total-size應大於RAM的許多以確保不
# 會被RAM的結果屏蔽欲測量結果(建議最少100G以上), I/O operations
# 有以下幾種常見的選擇：
# seqwr   - sequential write
# seqrewr - sequential rewrite
# seqrd   - sequential read
# rndrd   - random read
# rndwr   - random write
# rndrw   - combined random read/write
# 另外也有提供Asynchronous I/O mode file access的選項,目前僅支援
# Linux AIO,如果指定這個模式Sysbench會用一定數量的request去queue
# Linux AIO API,然後等待至少一個提交的request完成,之後再提交一系
# 列的request
# 更多檔案相關的模式請見手冊
#

$ sysbench fileio --file-total-size=15G prepare
$ sysbench fileio --file-total-size=15G --file-test-mode=rndrw run

See exampleOutput

$ sysbench fileio --file-total-size=15G cleanup
-----------------------------------------------------------------
#3 Benchmark Memory
#
# 用來測量sequential memory read/writes,預設的資料交換大小為100G
#
$ sysbench memory run

See exampleOutput
-----------------------------------------------------------------
#4 Benchmark threads
#
# 用來測試Scheduler的效能,主要適用於當scheduler有很多個thread競爭
# 一些特定mutex的情境下,Sysbench會創造指定數量的thread以及mutex(
# 預設1000 thread, 8 mutex)然後開始run request
# Request裡的工作包含對mutex進行上鎖以及讓出CPU,thread會被scheduler
# 放在run queue上,然後被重新排程後會解鎖mutex
# 每次的request會loop執行上述的工作,因此迭代多次產生的concurrency
# 也會更多
#

$ sysbench threads run

See exampleOutput
-----------------------------------------------------------------
#5 Benchmark mutex
#
# 用來模擬所有的thread在大部份情況下都是都是同步執行（concurrent)
# 且只有短暫的時間取得mutex lock,目的是要測量mutex的實踐性能
#

$ sysbench mutex run

See exampleOutput
-----------------------------------------------------------------
References:
https://imysql.com/wp-content/uploads/2014/10/sysbench-manual.pdf
-----------------------------------------------------------------
