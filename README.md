# WEC_Systems
WEC Systems OS tasks, Aditya Patil, 251IT007

Task 1: 
-> approach: https://www.nongnu.org/ext2-doc/ext2.pdf used this pdf for the entire internal layout of the ext2 structure. broadly understood the different fields (not all of them) and why they're required. 

-> used a hex editor to manually go through the .img file, for sanity checking the offsets of magic number, etc.

-> printing only a few fields which I think are "important". 

-> skipped steps like checking correctness of magic number, group count via inode AND block count, etc. cuz it's given that the file is .img, so WILL pass

-> also skipped reversing endianness, opening file in binary mode to read, etc. because not required / redundant on macOS

-> (will add screenshots of outputs with rest of the tasks at the end?)

Task 2:
-> approach: first simply just figured out how the important fields in the inode structure, printed them. was confused between the various terminologies.



