# huge page

## install libhugetlbfs-dev

```
sudo apt install libhugetlbfs-dev
```

## Modify the number of huge pages

```
sudo su
echo 2 > /proc/sys/vm/nr_hugepages
```

## /proc/sys/vm/nr_hugepages

/proc/sys/vm/nr_hugepages indicates the current number of "persistent" huge
pages in the kernel's huge page pool.

## /proc/meminfo

### HugePages_Total
is the size of the pool of huge pages.

### HugePages_Free
is the number of huge pages in the pool that are not yet
allocated.

### HugePages_Rsvd
is short for "reserved," and is the number of huge pages for
which a commitment to allocate from the pool has been made,
but no allocation has yet been made.  Reserved huge pages
guarantee that an application will be able to allocate a
huge page from the pool of huge pages at fault time.

### HugePages_Surp
is short for "surplus," and is the number of huge pages in
the pool above the value in /proc/sys/vm/nr_hugepages. The
maximum number of surplus huge pages is controlled by
/proc/sys/vm/nr_overcommit_hugepages.

# example of the output

```
./hugepage
default huge page size: 2097152 B

HugePages_Total:       2
HugePages_Free:        2
HugePages_Rsvd:        0
HugePages_Surp:        0

### MMAP

HugePages_Total:       2
HugePages_Free:        2
HugePages_Rsvd:        1
HugePages_Surp:        0

### WRITE DATA

HugePages_Total:       2
HugePages_Free:        1
HugePages_Rsvd:        0
HugePages_Surp:        0

### MUNMAP

HugePages_Total:       2
HugePages_Free:        2
HugePages_Rsvd:        0
HugePages_Surp:        0
```
