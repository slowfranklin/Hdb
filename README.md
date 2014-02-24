Hdb
===

Extendible Hashing


kv database based on Extendible Hashing database structure,
for more informations about Extendible Hashing,please visit

1. http://wenku.baidu.com/link?url=DREzY_17D0Y4X6POavkTS4ob1rYXiJH9oIaQbkqsmeGLLb6COt0ZYbijQ5qQU6eue7uPvNvxwk1Sx2g499798p4K3gexSoaDHvuS7rl0hwe
2. http://en.wikipedia.org/wiki/Extendible_hashing

HDB features

1. key's length is limited by MAX_KEY_SIZE
2. value's length have no limited
3. page size is 4*1024 byte in disk
4. LRU pagecache

now,the write speed is slowly,because when pagecache is full,page will be write to disk by call fsync function


so,in order to improve HDB write speed,will add some features in below

1. wal
2. checkpoint
3. ***
