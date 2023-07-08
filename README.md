# `ttytris`

<p align=center>
Tetris in your TTY<br/><br/>
<img src="https://github.com/m-bartlett/ttytris/assets/85039141/10fe03a0-b905-4677-8ee5-a175d7e728d1)">

</p>


>**Note**
><br/>For best visuals, you may want to configure your terminal to have a wide character width or otherwise use a font
>with such, such that the character dimensions are nearly square.
<br/>



## Test

To run the custom test suite which was used to validate the core game logic one may run `make test` as a sanity check. All tests should pass.

## Build

Requires ncurses. Run `make`.

## Run

After building, execute the resulting `ttytris` binary.

Controls can be found and modified here: https://github.com/m-bartlett/ttytris/blob/503e758b865575942b6d238a0551fbfce9b47de2/src/engine.c#L63-L76

## About

This implementation represents the block layouts for each tetromino as a 4x4 grid which is the minimum size needed to contain each piece type. This grid is overlayed on the playfield as part of the graphics drawing and collision checks.

The 4x4 grid is encoded as an unsigned 16-bit integer which can be thought of as a serialized bitfield for each cell in the grid. In other words, the top left of the grid is the most-significant bit of the `uint16_t` and the bottom right of the right will be the least-significant bit of the `uint16_t`. `1` means the block in the grid is populated for the given tetromino type and the given rotation. Here is a diagram of each tetromino type in each of the 4 possible rotations and how it is serialized into an unsigned 16-bit integer value:

![image](https://github.com/m-bartlett/ttytris/assets/85039141/20145cf6-b207-403d-97c4-1aa5289f226a)

This encoding has the consequence of very efficient collision checks. The game logic needs only to extract the 4x4 target area of the playfield as an unsigned 16-bit integer in the same way and perform a boolean-and operation (`&`) between both integer values. If the result of the boolean-and is nonzero, then there is a collision and the tetromino cannot be placed in the target location.

For example, here is a diagram of a toy playfield scenario where the game is evaluating the legality of placing an "S" tetromino rotated 90 degrees at the location of the green dashed square. Since all tetrominos orientations are stored in a 4x4 grid, the location of the grid is given by the coordinates of its bottom right corner cell (denoted by the extra outlines around the bottom right cell).


![image](https://github.com/m-bartlett/ttytris/assets/85039141/945fa39e-9d1c-4f3d-a159-a7011ce151a2)

To the right of the playfield shows how the 4x4 window of the state of the playfield is encoded into an unsigned 16-bit integer in the same serialization order as the tetrominos (top-to-bottom, left-to-right) with color-coding to match the cell as it appears in the playfield. The 16-bit integer representing the state of the playfield at the given location is boolean-anded with the 16-bit integer which represents the block layout of the tetromino. In this case, the result of the boolean-and is 0, meaning it is legal to move the S tetromino into this position in the playfield.


The abstract method for the boolean-and check in the code is here:

https://github.com/m-bartlett/ttytris/blob/503e758b865575942b6d238a0551fbfce9b47de2/src/playfield.c#L44-L49
