//Troy Fine
//tfine
//CE 121 - Microprocessor System Design
//HW1 - #1

#include <stdio.h>
#include <string.h>

int evaluate(char* postFix, int h, int g, int f, int e, int d, int c, int b, int a){
    int result;
    int op[100];
    //int A, B, C, D, E, F, G, H;
    int j = 0;
    for(int i = 0; i < strlen(postFix); i++){ //looking at each char of expr
        if(postFix[i] == 'A' || postFix[i] == 'B' || postFix[i] == 'C' || postFix[i] == 'D' || postFix[i] == 'E' || postFix[i] == 'F' || postFix[i] == 'G' || postFix[i] == 'H'){
            if(postFix[i] == 'A'){
                op[++j] = a;
                
            }
            else if(postFix[i] == 'B'){
                op[++j] = b;
                
            }
            else if(postFix[i] == 'C'){
                op[++j] = c;
                
            }
            else if(postFix[i] == 'D'){
                op[++j] = d;
                
            }
            else if(postFix[i] == 'E'){
                op[++j] = e;
                
            }
            else if(postFix[i] == 'F'){
                op[++j] = f;
                
            }
            else if(postFix[i] == 'G'){
                op[++j] = g;
                
            }
            else if(postFix[i] == 'H'){
                op[++j] = h;
                
            }
        }
        else if(postFix[i] == '&' || postFix[i] == '|'){
            if(postFix[i] == '&'){
                result = op[j-1] & op[j];
                --j;
                op[j] = result;
            }
            else if(postFix[i] == '|'){
                result = op[j-1] | op[j];
                --j;
                op[j] = result;
            }
        }
        else if(postFix[i] == '~'){
            result = !op[j];
            op[j] = result;
        }
        
        
    }
    result = op[j];
    return result;
}


void truthTable (char * expr, char * postFix){
    int i;
    int count = 0;
    int operators = 0; //number of elements in the array
    int operands = 0;
    char logic[100];
    //char postFix[100] = {};
    char* expression;
    
    /////////////////////////Get expr to PostFix Form/////////////////////////////////
    
    for(i = 0; i < strlen(expr); i++){ //looking at each char of expr
        
        if(expr[i] == 'A' || expr[i] == 'B' || expr[i] == 'C' || expr[i] == 'D' || expr[i] == 'E' || expr[i] == 'F' || expr[i] == 'G' || expr[i] == 'H'){
            ++count;
            postFix[operands] = expr[i];
            ++operands;
        }
        else{
            if(expr[i] == '('){ //push
                logic[operators] = expr[i];
                ++operators;
            }
            if(expr[i] == ')'){ //pop and copy to postFix[] until you reach '('
                --operators;
                while(operators >= 0 && logic[operators] != '('){
                    postFix[operands] = logic[operators];
                    ++operands;
                    
                    logic[operators] = '\0';
                    --operators;
                }
                
                /*
                
                if(logic[operators] != '\0'){
                    postFix[operands] = logic[operators];
                    
                    logic[operators] = '\0';
                    --operators;
                }
                
                logic[operators] = '\0';
                if(operators > 0){
                    --operators;
                }
                
                */
            }
            if(expr[i] == '&' || expr[i] == '|' || expr[i] == '~'){
                logic[operators] = expr[i];
                ++operators;
            }
        }
    }
    
    while(operators >= 0){ //do this if there are leftover operators in array logic[]
        if(logic[operators] == '('){
            logic[operators] = '\0';
            --operators;
        }
        else if(logic[operators] == '&' || logic[operators] == '|' || logic[operators] == '~'){
            postFix[operands] = logic[operators];
            ++operands;
            logic[operators] = '\0';
            --operators;
        }
        else{
            logic[operators] = '\0';
            --operators;
        }
    }
    //printf("%s\n", postFix);
    
    ////////////////////////////Solving the PostFix expr//////////////////////////////////
    
    /////////////////CALL EVALUATE WITH A-H AND GET SOLUTION EXPRESSION//////////////////
    
    
    
    
    
}

int main(){
    
    char expr[100];
    char postFix[100] = {};
    int trigger[256] = {};
    
    int channels = 8;
    int iterations = 1;
    
    for(int i = 1; i <= 8; ++i){
        iterations *= 2;
    }
    
    int A=0, B=0, C=0, D=0, E=0, F=0, G=0, H=0;
    
    printf("Enter a logic expression: \n");
    fgets(expr, 100, stdin);
    
    
    truthTable(expr, postFix);
    
    printf("%s\n", postFix);
    
    
    for(int i = 0; i < 256; i++){
        if(i % 2 < 1)
            A = 0;
        else A = 1;
        if(i % 4 < 2)
            B = 0;
        else B = 1;
        if(i % 8 < 4)
            C = 0;
        else C = 1;
        if(i % 16 < 8)
            D = 0;
        else D = 1;
        if(i % 32 < 16)
            E = 0;
        else E = 1;
        if(i % 64 < 32)
            F = 0;
        else F = 1;
        if(i % 128 < 64)
            G = 0;
        else G = 1;
        if(i % 256 < 128)
            H = 0;
        else H = 1;
        
        trigger[i] = evaluate(postFix, H, G, F, E, D, C, B, A);
    }
 
    for(int i = 0; i < 256; ++i){
        printf("%d ", trigger[i]);
    }
    
    
    
    
    return 0;
}

