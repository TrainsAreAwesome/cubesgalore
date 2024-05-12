#include "cglm\include\cglm\cglm.h"

//prints a 4x4 matrix given. give the matrix itself not a pointer to it (its allready a pointer)
int logMatrix4x4(mat4 m){
    //hey not my fault it looks bad the compiler gets angry if you split string literals onto multiple lines
    printf("4x4 Matrix:\n-------------------------\n| %.2f | %.2f | %.2f | %.2f |\n-------------------------\n| %.2f | %.2f | %.2f | %.2f |\n-------------------------\n| %.2f | %.2f | %.2f | %.2f |\n-------------------------\n| %.2f | %.2f | %.2f | %.2f |\n-------------------------\n\n",
    m[0][0], m[0][1], m[0][2], m[0][3],
    m[1][0], m[1][1], m[1][2], m[1][3],
    m[2][0], m[2][1], m[2][2], m[2][3],
    m[3][0], m[3][1], m[3][2], m[3][3]);
}