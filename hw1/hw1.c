/*
CS 241: Homework #1
DUE DATE: On the SVN at 11:59pm, June 13, 2012 (No late submissions!)

General Instructions:
- This assignment is a SOLO assignment.
- Each question is worth equal weight.
- The assignment MUST be committed to your SVN by the due date.  For
  instructions on how to use SVN and other course information relating
  to online submission, please see:
    http://www.cs.illinois.edu/class/cs241/handin.html


[Part I]
Each question consists of a C code snippet that contains one major problem.
The problems range from compile-time errors, misuse of C functions or C
syntax, and logical errors.

In some questions, unnecessary code is eliminated and replaced with "...".

For each problem, you must do two things:
  (a): State what the code attempts to do.
  (b): Correct the code.
*/

/* ===== Problem #1 ===== */
/* Point p to the address of x. 
*/
void Problem1(){
	int x;
	...
	int *p = &x;
}


/* ===== Problem #2 ===== */
/* Let p pointer to a float value equal to 12.5.
*/
void Problem2(){
	float *p,temp=12.5;
	p = &temp;
}


/* ===== Problem #3 ===== */
/* if the value of a is in (0,1) print it in the screen.
*/
void Problem3(){
	float a;
	...
	if(a>0 && a<1)
		printf("a is a value between 0 and 1.\n");
}

/* ===== Problem #4 ===== */
/*Goal: check whether x and y are equal.
*/
void Problem4(){
	int x=5;
	int y;
	...
	if(x==y)
		printf("x and y are equal.\n");
	else
		printf("x and y are different.\n");
	
}

/* ===== Problem #5 ===== */
/*Goal: change an int value to be a float type of value, get pointers pointing to them, and return.

*/
float Problem5(int x){
	int *ip;
	float y, *fp;
	...
	ip = &x;
	y = x;
	fp = &y;
	return y;
}

/* ===== Problem #6 ===== */
/*Goal: stay in the loop until a increases to being equal to b.
*/
void Problem6(){
	int a=0, b=10;
	while (a != b){
		...
		a++;
	}
}

/* ===== Problem #7 ===== */
/*Goal: get input and turn the last character into '\0'.
*/
void Problem7(){
	char s[30];
	scanf("%30s",s);
	s[29] = '\0';
}

/* ===== Problem #8 ===== */
/*Goal: change the value of x in a function reset().
*/
void reset(int *x){
	*x=0;
}

int main(){
	...
	int x=1;
	reset(&x);
	printf("x is now 0.\n");
	return 0;
}

/* ===== Problem #9 ===== */
/*Goal: malloc space to a char pointer, given it values and free it.
*/
void Problem9(){
	char *s = (char *) malloc (50);
	...
	strcpy(s,"this is a string");
	free(s);
}

/* ===== Problem #10 ===== */
/*Goal: define an array and push values into the array.

*/
void Problem10(){
	float values[10];
	int i,n = 10;
	for(i=0;i<n;i++)
		values[i] = (float)i/n;
}

/* ===== Problem #11 ===== */
/*Goal: define an 2-dimensional array and given values to it.
*/
void Problem11(){
	int **array;
	int i,j;
	array =(int**) malloc(10*sizeof(int *));
	for(i=0;i<10;i++)
		array[i] = (int*) malloc(10*sizeof(int));
	
	for(i=0;i<10;i++)
		for(j=0;j<10;j++)
			array[i][j] = i*j;
}

/* ===== Problem #12 ===== */
/*Goal: type input to str, and give different values to x according to the string content in str.
*/
void Problem12(){
	char *str = (char *)malloc(10);
	int x;
	scanf("%9s",str);
	...
	if(strcmp(str,"blue")==0) x=1;
	else if( strcmp(str,"red")==0) x=2;
	else x=0;
}

/* ===== Problem #13 ===== */
/*Goal: type input to i, then run different functions according to the value in i.

*/
void Problem13(){
	int i;
	...
	scanf("%d",&i);
	switch(i) {
		case 1: 
			function1();
			break;
		case 2: 
			function2();
			break;
		default: 
			printf("Unknown command.\n");
			break;
	}
}

/* ===== Problem #14 ===== */
/*Goal: print the value of k.

*/
void Problem14(){
	int k = pow(2, 5);
	printf("pow(2,5)=%d\n",k);
}


/* ===== Problem #15 ===== */
/*Goal: show error and exit the program.
*/
void Problem15(){
	int x;
	...
	if(x < 0){
		printf("invalid value.\n");
		exit();
	}
}

/* ===== Problem #16 ===== */
/*Goal: append "abcd" to string str.
*/
void Problem16(){
	char str[100];
	strcat(str, "abcd");
}


/* ===== Problem #17 ===== */
/*Goal: output a double value r diveded by 360.
*/
void Problem17(){
	int r;
	...
	printf("The fractional part of the circle is: %f\n", r/360);
}

/* ===== Problem #18 ===== */
/*Goal: square the value of k.
*/
int Problem18(int k){
	k = k * k;
	return k;
}


/*
[Part II]
Each question asks you to accomplish a common C task by only using a subset of
C operators.  To receive credit for the question, you must provide working,
compilable C code to each question under the following constraints:
- You must only use the operators specified in each problem.
- You must NOT use any other operators, control statements (if, do/while, ...),
  or function calls.
- You may only use constant values from 0L to 255L (0x0L to 0xFFL).

You know only the following about the machine that is running your code:
- The machine uses 2's complement representation of integers.
- The machine performs right shift arithmetically
- The machine has unpredictable behavior when shifting a long integer by
  more than the word size.

You do not know the size of a word (32-bit, 64-bit, or even 256-bit) on
the machine running your code.  (We want generic code that works on
any machine, not a "fill all the bits of 1's and add one"-type solution.)
*/

/* ===== Problem #19 ===== */
/* Task: Return "value" where all bit positions that are set (==1) in "flag"
         and "value" are cleared (==0) in the returned result.
		 
		 Example: clear_bit(value=0xFF, flag=0x55) => 0xAA

   Allowed Operators: & ~ ^ | >> << ! + =
*/
long int clear_bit(long int value, long int flag){
	return value & (~flag);

}


/* ===== Problem #20 ===== */
/* Task: Return 1L if (x == y), otherwise return 0L.

   Allowed Operators: & ~ ^ | >> << ! + =
*/
long int is_equal(long int x, long int y){
	return !(x ^ y);

}
