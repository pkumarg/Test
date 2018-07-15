/*
Problem Statement
    
Alice Margatroid has n dolls sitting on the plane. You are given their coordinates in the vector <int>s x and y. For each valid i, there
is a doll sitting at (x[i], y[i]).  Each doll can emit a laser beam along a half-line. The laser beam must have an angle of exactly 30
degrees with the negative y axis. There are two possibilities for such a laser beam: either it points "towards bottom left", or "towards bottom right".  Formally, if there is a doll at (x0,y0), it can emit either one of the following two laser beams:
The ray consisting of all points (x,y) such that (y - y0) = +sqrt(3) * (x - x0) and y <= y0.
The ray consisting of all points (x,y) such that (y - y0) = -sqrt(3) * (x - x0) and y <= y0.
These n rays will divide the plane into multiple regions. Let f be the number of regions that have a finite area.  Alice can choose which of the two rays each doll emits. She would like to do that in such a way that f will be as large as possible. Please calculate and return the largest possible value of f.
Definition
    
Class:
MaidensBunraku
Method:
maximal
Parameters:
vector <int>, vector <int>
Returns:
int
Method signature:
int maximal(vector <int> x, vector <int> y)
(be sure your method is public)
Limits
    
Time limit (s):
2.000
Memory limit (MB):
256
Stack limit (MB):
256
Constraints
-
x will contain between 1 and 2,000, elements, inclusive.
-
x and y will contain the same number of elements.
-
Each element in x will be between -1,000,000 and 1,000,000, inclusive.
-
Each element in y will be between -1,000,000 and 1,000,000, inclusive.
-
Points (x[i], y[i]) will be distinct.
Examples
0)

    
{-5,-5,-5,5,5,5}
{4,5,6,4,5,6}
Returns: 4
Alice should set the first three dolls to emit their rays towards the right and the last three dolls to emit their rays towards the left. The rays will then look as follows:
 2            5
  \          /
 1 \        / 4
  \ \      / /
 0 \ \    / / 3
  \ \ \  / / /
   \ \ \/ / /
    \ \/\/ /
     \/\/\/
     /\/\/\
    / /\/\ \
   / / /\ \ \
  / / /  \ \ \
 . . .    . . .
. . .      . . .   
In the middle of the figure we can see four finite regions.
1)

    
{-1,-1,1,1}
{4,0,4,0}
Returns: 0
Whatever directions Alice chooses for the laser beams, there will never be any finite regions. For example, if she chooses "right" for the first two dolls and "left" for the last two dolls, we will get the following situation:
    0  2
     \/
     /\
    /  \
   /    \
  / 1  3 \
 .   \/   . 
.    /\    .
    /  \
   /    \
  .      .
 .        .
Which don't have a closed area.
2)

    
{0,1,2,3,4,0,1,2,3,4}
{0,0,0,0,0,1,1,1,1,1}
Returns: 15

3)

    
{0,1,2,3,4,5,6,7,8,9}
{0,1,2,3,4,5,6,7,8,9}
Returns: 16

4)

    
{1000000}
{-1000000}
Returns: 0

This problem statement is the exclusive and proprietary property of TopCoder, Inc. Any unauthorized use or reproduction of this information without the prior written consent of TopCoder, Inc. is strictly prohibited. (c)2003, TopCoder, Inc. All rights reserved.
*/
