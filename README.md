# mtgrep
Multithread grep

# Description:
AI generated task description.

# Project Description
Create a multi-threaded file search utility called mtgrep (multi-threaded grep) that searches for text patterns across multiple files simultaneously using POSIX threads (pthreads).

# Requirements
Core Functionality
Accept a search pattern and list of files/directories as command line arguments

Recursively search through directories

Use multiple threads to search files concurrently

Output matching lines with filename and line number

Implement proper thread synchronization

# Technical Specifications
Use pthreads for concurrent operations

Implement thread-safe output to prevent interleaved text

Handle both files and directories as input

Limit maximum concurrent threads (suggest 8)

Properly clean up thread resources

Advanced Challenges (Optional)
Implement a thread pool instead of creating/destroying threads

Add a work queue with proper synchronization

Include performance statistics

Support regular expressions

Sort results before output

# Expected Output
The program should display results in the format:
filename:line_number: matching_line_content

Learning Objectives
Through this project you will learn:

pthread creation and management

Passing arguments to threads

Mutex synchronization

Thread-safe I/O operations

Work distribution among threads

Proper resource cleanup

# Suggested Approach
Design data structures for thread arguments

Implement file search function for individual threads

Create directory traversal logic

Add thread management in main()

Implement synchronization for output

Handle thread cleanup

# Testing
Test your implementation with:

Single files

Multiple files

Directories (both with and without subdirectories)

Various search patterns

Mixed file/directory inputs

Would you like me to provide any additional details about specific aspects of the project?