# MEN_TextEditor
A Windows-only rudimentary GUI text editor with "vim-like commands" and multiple windows system.
This was my first personal project after the discovery of the Handmade Hero series and community.
The code base has been built from scratch and has no dependencies other than some functions in the standard library and the windows libraries accessed through the win32 API.
The source code is awful with my today's standards (although only a year has passed since then) and the program is very buggy and crashes all the time, so don't look to much into it.

# Build Instructions
To build the program you should use the batch.bat file present in the repository.

# Commands 
h, j, k, l or arrow keys -> for simple cursor movement.

w, b -> for word movement.

 "[" , "]" -> to move to the next/prev blank line.

d -> delete the line.

o -> new line

y, p -> yanking and pasting a line respectibly.

Ctrl-a, Ctrl-w, Ctrl-d, Ctrl-s -> split the current window to the left, up, right or down respectibly.

Ctrl-j, Ctrl-i, Ctrl-l, Ctrl-k -> move to the window that it's at the left, up, right, or down of the current window.

: -> your cursor moves to the minibuffer and here you whould be able to type the path of the file you want to open.

# Overview
[MEN Text Editor overview](https://www.youtube.com/watch?v=fgYtkhYv3Ws&ab_channel=MENYoutuber)
