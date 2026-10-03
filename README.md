# WEC_Systems
WEC Systems OS tasks, Aditya Patil, 251IT007

Task 1: 
-> approach: https://www.nongnu.org/ext2-doc/ext2.pdf used this pdf for the entire internal layout of the ext2 structure. broadly understood the different fields (not all of them) and why they're required. 

-> used a hex editor to manually go through the .img file, for sanity checking the offsets of magic number, etc.

-> printing only a few fields which I think are "important". 

-> skipped steps like checking correctness of magic number, group count via inode AND block count, etc. cuz it's given that the file is .img, so WILL pass

-> also skipped reversing endianness, opening file in binary mode to read, etc. because not required / redundant on macOS

-> (will add screenshots of outputs with rest of the tasks at the end)

Task 2:
-> approach: first simply just figured out how the important fields in the inode structure, printed them. was confused between the various terminologies (block group, block group descriptor, block group descriptor table, inode, inode structure, inode table, where each is located, etc.). After understanding the structure, printed some of the fields of the inode structure of inode 2 (the root directory), and verified it with e2fsprogs / debugfs / xxd against the .img file. 

-> learnt about why indirect blocks are even required in the first place, and the improvements/optimizations in the later versions (ext4 having "extents). Then read ONLY the direct blocks, and just printed the root directory contents to check if layout understanding is correct. again verified by xxd. 

-> for the actual task (directory traversal), key learning was that each block contains multiple directory entries (not directories, i was reading all the blocks and combining them into the directory entry completely forgetting that directory is a list of directory entries) and an entry starts from offset 0 inside the block, and there's no space in between two entries. Also learnt that entries cannot be in two blocks at a time, so if an entry goes beyond the block size, it's placed in the next block (contiguous mostly when sparse, else can be anywhere) and the remaining bytes are padded. Felt like the padding were a waste of space, but then, the same concept is used in multiple other places by the kernel, such as in OS page tables; aligned separate block entries are much easier to handle by the kernel, "atomic", and apparently the "internal fragmentation" (wastage of memory) is negligible. Also for the i_mode check itself, i didn't mask the permission bits with the S_IFDIR, S_IFREG values initially to check if it's directory or regular or other type of file.  

-> before actual directory traversal, confirmed understanding of the directory entry structure, specifically the rec_len part (variable length structures, so variable reads into a buffer) by printing the directory entries and verifying against xxd output. 

-> during traversal, inode value in the entry and file type are directly available, but dir->name isn't actually a null terminated string. so was using that to print name, was getting some garbage values and was printing current and parent directory ('.' and '..' which i intended to skip, so strcmp was failing). 

-> can see some files like lost+found, readthis.txt, comp-dsa.pdf, rice.webp, vid.webm. Excited to see what content they hold :D



