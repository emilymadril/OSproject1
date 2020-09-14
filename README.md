Members: Nolan Dyer, Alexandar Jen, Emily Madril
Division of Labor: 
Part 2:  Nolan Dyer
Part 3:  Nolan Dyer
Part 4:  Nolan Dyer, Alexandar Jen
part 5:  Nolan Dyer
part 6:  Nolan Dyer, Alexandar Jen
part 7:  Alexandar Jen, Emily Madril
part 8:  Alexandar Jen, Emily Madril 
part 9:  Alexandar Jen
part 10: Emily Madril, Alexandar Jen, Nolan Dyer

Contents of tar archive: parser.c (source code), makefile, and README
  //add description of each file
Compile directions:

Known bugs:

  When calling a background process the output of the pipes gets put on the command line. However the next command is unaffected by it, it's just a visual bug. We tried moving 
  where we check the jobs in the code, but that did not work. 
  
  When calling exit the number of commands is equal to the number of total commands. So a pipe with 3 commands adds three to the count and a pipe with 2 adds 2. 
  
  //when does it occur (compile, link, runtime), syptoms bug is giving off, how you attempted to fix it
Unfinished portions:
Screenshot of GIT repository:
