/* rexx */
parse arg Rev1 Rev2;

address CMD 'svn diff -x -w -r '||Rev1||':'||Rev2||' >'||Rev1||'-'||Rev2||'.diff';
