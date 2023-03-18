# memory-page-replacement
A program that simulates memory system using a variety of paging schemes

This is just a simulation of the page table, so no actual reading or writing data from/to the disk. When a simulated disk read or write occurs, simply increment a counter to keep track of disk reads and writes, respectively.

## Implemented Page Replacement Algorithms
- FIFO: Replace the page that has been resident in memory longest.
- LRU: Replace the page that has been resident in memory oldest.
- ARB: Use multiple reference bits to approximate LRU. Your implementation should use an a-bit shift register and the given regular interval b. See textbook section 9.4.5.1.

  ** If two page’s ARB are equal, you should use FIFO (longest) to replace the frame.

- WSARB-1: Combine the ARB page replacement algorithm with the working set model that keeps track of the page reference frequencies over a given window size (page references) for each process (see textbook section 9.6.2). It works as follows:

  **Associate each page with a reference bits as the shift register (R) to track the reference pattern over the recent time intervals (for a given time interval length), and an integer counter (C) of 8 bits to record the reference frequency of pages (in the memory frames) in the current working set window. R and C are initialized to 0.

  **Page replacement is done by first selecting the victim page of the smallest reference frequency (the smallest value of C), and then selecting the page that has the smallest reference pattern value in the ARB shift register (the smallest value of R) if there are multiple pages with the smallest reference frequency.

- WSARB-2: Same as WSARB-1 in combining ARB with the working set model, but se- lecting a victim for page replacement is done in the reverse order:

  **First select the page having the smallest reference patten value in R, and then that of the smallest reference frequency value in C if there are multiple pages with the smallest reference patten value.
Note that for WSARB-1 and WSARB-2, same as ARB, if there are multiple pages with the same smallest values of R and C, victim will be chosen according to FIFO among them.

## Memory Traces
Each trace is a series of lines, each listing a hexadecimal memory address preceded by R or W to indicate a read or a write. There are also lines throughout the trace starting with a # followed by a process ID and command. For example, a trace file for gcc might start like this:

```
# gcc
R 0041f7a0
R 13f5e2c0
R 05e78900 R 004758a0 W 31348900
```

The lines in the trace file beginning with #.

