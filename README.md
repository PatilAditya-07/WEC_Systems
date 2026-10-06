# WEC Systems - ext2 fs parser

## Task 1: 
-> approach: https://www.nongnu.org/ext2-doc/ext2.pdf used this pdf for the entire internal layout of the ext2 structure. broadly understood the different fields (not all of them) and why they're required. 

-> used a hex editor to manually go through the .img file, for sanity checking the offsets of magic number, etc.

-> printing only a few fields which I think are "important". 

-> skipped steps like checking correctness of magic number, group count via inode AND block count, etc. cuz it's given that the file is .img, so WILL pass

-> also skipped reversing endianness, opening file in binary mode to read, etc. because not required / redundant on macOS

-> (will add screenshots of outputs with rest of the tasks at the end)

## Task 2:

-> approach: first simply just figured out how the important fields in the inode structure, printed them. was confused between the various terminologies (block group, block group descriptor, block group descriptor table, inode, inode structure, inode table, where each is located, etc.). After understanding the structure, printed some of the fields of the inode structure of inode 2 (the root directory), and verified it with e2fsprogs / debugfs / xxd against the .img file. 

-> learnt about why indirect blocks are even required in the first place, and the improvements/optimizations in the later versions (ext4 having "extents). Then read ONLY the direct blocks, and just printed the root directory contents to check if layout understanding is correct. again verified by xxd. 

-> for the actual task (directory traversal), key learning was that each block contains multiple directory entries (not directories, i was reading all the blocks and combining them into the directory entry completely forgetting that directory is a list of directory entries) and an entry starts from offset 0 inside the block, and there's no space in between two entries. Also learnt that entries cannot be in two blocks at a time, so if an entry goes beyond the block size, it's placed in the next block (contiguous mostly when sparse, else can be anywhere) and the remaining bytes are padded. Felt like the padding were a waste of space, but then, the same concept is used in multiple other places by the kernel, such as in OS page tables; aligned separate block entries are much easier to handle by the kernel, "atomic", and apparently the "internal fragmentation" (wastage of memory) is negligible. Also for the i_mode check itself, i didn't mask the permission bits with the S_IFDIR, S_IFREG values initially to check if it's directory or regular or other type of file.  

-> before actual directory traversal, confirmed understanding of the directory entry structure, specifically the rec_len part (variable length structures, so variable reads into a buffer) by printing the directory entries and verifying against xxd output. 

-> during traversal, inode value in the entry and file type are directly available, but dir->name isn't actually a null terminated string. so was using that to print name, was getting some garbage values and was printing current and parent directory ('.' and '..' which i intended to skip, so strcmp was failing). currently only implemented direct block check. 

-> can see some files like lost+found, readthis.txt, comp-dsa.pdf, rice.webp, vid.webm; printed their inode structure (of regular files), and saw that comp-dsa.pdf, vid.webm, rice.webp use singly and even doubly indirect pointers. 

-> for the indirect pointers, i didn't know how to dereference say doubly to singly indirect then direct, then (used AI) got to know that recursively making every doubly indirect block to singly, then dereferencing to direct blocks, using a "levels_left" parameter, which indicates what level of indirection is actually happening. although not required (calculated required number of blocks manually and verified by printing i_size, the largest files like the pdf and webp also didn't require more than doubly indirect pointers), i wrote code for triple indirection as well as the only change is in the levels_left argument. also had to check how many blocks are actually required to read, because it's not necessary that a file use all of the 256 (1024/4) blocks/pointers to blocks. 

## Task 3: 

-> in task 2, for recursively traversing through the filesystem directories, i had to go to the actual block pointed to by direct/indirect block pointers (i_block) on the disk, read the directory entry and move on to the next entries. here the major difference is that we have to print the contents of a file, so for the actual data, you have to read entire file size (i_size) amount of content from the disk and then redirect it to either a file, or directly to stdout (which's what i did). so to read the entire file size, you find the number of blocks the file size spans, then using that logical block numbers, you find actual physical blocks by dereferencing them as many times as required, reading each block into a buffer, till you completely read the file. so created a helper function get_actual_data which takes in inode of the file and logical block number, gives actual physical block number (i was returning block offset here (block number * block size, instead of just block number), as i did for directory traversal, was getting read errors, spent a lot of time here). 

-> the file whose content i want to print, i can either write it's name or inode number as an argument, but i used it's inode since if i used it's name, i would have to again traverse the directories and find it's inode manually. it's more user friendly to use file name, but since there's only like 5 files and i've already printed their inode structures before, i can use them directly. also this is only user space.

-> readthis.txt (inode 12) has: "This is the second file from the task" as output, rest of the files have output which are not human readable, as expected because they're in binary format. the way to see what's in them is to redirect them into a file (which i mentioned is more manual work), and then with that file, use some tool from google to see their content in human readable form. hope there's no secret in them. (will attach screenshots at the end)

-> for both task 2 and 3, need actual blocks, so thought of combining the helper functions, but no time. 

-> cleaned up the main function to only read the file image, take inputs including cli options to do any one of the task without having to remove or comment out them (which looked messy). used AI to add in cli options and clean up the code as i had no idea how it worked but wanted a way to execute any of the executed tasks and not just one of them. 

## Task 4:

-> here, realized pretty quickly that it was heavy as updating anything would mean updating the entire structure while not breaking anything. the idea (from AI) was that given an inode to update, you have to check if there's sufficient space to update it, if not, allocate more space; if less is required, deallocation. that changes bitmap of each bg, number of free blocks/inodes, etc.

-> a free block can be figured out by seeing which bit in the bitmap is not set, so for allocation (only did that), set it, update meatdata, write to a fresh block. 

-> i deliberately left the task incomplete rather than submitting code which i could not explain or verify. the first three tasks are implemented and tested, including recursive directory traversal and reading files through direct, singly indirect and doubly indirect block pointers.

## AI disclosure: AI was used during the project for conceptual clarification, debugging, and implementation guidance. i did not treat generated code as verified code; the parts of task 4 which i could not understand completely were not presented as finished functionality.
<img width="1389" height="439" alt="task3_verification" src="https://github.com/user-attachments/assets/361d1528-8510-4a82-96ac-386eea249cb8" />
<img width="1011" height="701" alt="task1" src="https://github.com/user-attachments/assets/58e16a11-ef67-4011-8c9b-b497d21f2b63" />
<img width="998" height="609" alt="root_inode" src="https://github.com/user-attachments/assets/7e0c37d7-228d-49c2-b5d5-454225d3bc2f" />
<img width="1063" height="108" alt="readthis" src="https://github.com/user-attachments/assets/5df88c65-6e00-471a-ba98-1526e7f8e87d" />
<img width="1195" height="815" alt="file_inode_content" src="https://github.com/user-attachments/assets/8f7cd1eb-7cf8-4e7b-ac24-db59cb067b5d" />
<img width="1160" height="781" alt="directory_traversal" src="https://github.com/user-attachments/assets/46b0a3aa-d90e-4a9f-83f2-5cc99931bf52" />

