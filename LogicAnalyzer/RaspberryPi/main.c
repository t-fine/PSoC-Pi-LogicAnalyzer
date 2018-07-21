//Troy Fine
//CMPE 121
//Lab Project - Logic Analyzer
//11.20.17
//Main Control Loop

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <wiringPi.h>
#include <errno.h>
#include <libusb.h>
#include <shapes.h>
#include <fontinfo.h>
#include <math.h>

int nchannels;
char* expr_file;
char* triggerD;
int mem_depth;
int sample_freq;
int xscale;
 
typedef struct{
    VGfloat x, y;
} data_point;

char channel_data[20000]; // Data samples

char bit0[20000];
char bit1[20000];
char bit2[20000];
char bit3[20000];
char bit4[20000];
char bit5[20000];
char bit6[20000];
char bit7[20000];

int trigIndex = 0;
int shift = 0;
VGfloat cX1;

char potFrame[2];

///////////////////////////////////////PARSE COMMAND-LINE ARGUMENTS/////////////////////////////////
int argCheck (int argc, char * argv[]){
if((argc > 13) || (argc % 2 == 0)){
        printf("These settings can be configured in any order: %s -m <mode> -t <trigger> -s <trigger-slope> -c <trigger-channel> -x <xscale> -y <yscale>\n", argv[0]);
        exit(1);
    }
    
    int i;
    int value;
    for(i = 1; i < argc-1;){
        if(strcmp(argv[i], "-n") == 0){
            value = atoi(argv[i+1]);
            if((value == 2) || (value == 4) || (value == 8)){
                ++i;
                nchannels = value;
                if(i < argc - 1)
                    ++i;
            }
            else{
                printf("-n <nchannels>: <nchannels> can be 2, 4, or 8.\n");
                exit(-1);
            }
            
        }

else if(strcmp(argv[i], "-t") == 0){ //file name

        if((strcmp(argv[i+1], " ") == 0) || (strcmp(argv[i+1], "\n") == 0)){
			printf("-t <expr_file>: <expr_file> must be a .txt file.\n");
                exit(-1);
            }
            else{
                expr_file = argv[++i];
                if(i < argc - 1)
                    ++i;
            }
        }

        else if(strcmp(argv[i], "-f") == 0){
            value = atoi(argv[i+1]);
            if((value == 1) || (value == 10) || (value == 50) || (value == 100)){
                ++i;
                sample_freq = value;
                if(i < argc - 1)
                    ++i;
            }
            else{
                printf("-f <sample_freq>: <sample_freq> can be 1, 10, 50, or 100 kHz.\n");
                exit(-1);
            }
            
        }
        else if(strcmp(argv[i], "-d") == 0){ //pos or neg
      
        if((strcmp(argv[i+1], "pos") == 0) || (strcmp(argv[i+1], "neg") == 0)){
                triggerD = argv[++i];
                if(i < argc - 1)
                    ++i;
            }
            else{
                printf("-d <trigger-direction>: <trigger-direction> can be either pos or neg.\n");
                exit(-1);
            }
            
        }
        else if(strcmp(argv[i], "-x") == 0){////////////////////
            value = atoi(argv[i+1]);
            if((value == 1) || (value == 5) || (value == 10) || (value == 100) || (value == 500) || (value == 1000) || (value == 2000) || (value == 5000) || (value == 10000)){
                ++i;
                xscale = value;
                if(i < argc - 1)
                    ++i;
            }
            else{
                printf("-x <xscale>: <xscale> can be 1, 5, 10, 100, 500, 1000, 2000, 5000, or 10000.\n");
                exit(-1);
            }
            
        }
        else if(strcmp(argv[i], "-m") == 0){
            value = atoi(argv[i+1]);
            if((value >= 100) && (value <= 10000)){
                ++i;
                mem_depth = value;
                if(i < argc - 1)
                    ++i;
            }
            else{
                printf("-f <mem_depth>: <mem_depth> can range from 100 to 10000\n");
                exit(-1);
            }
            
        }
        
        else {
            printf("Configurable settings include: -n <nchannels> -t <expr_file> -d <trigger-direction> -m <mem_depth> -f <sample_freq> -x <xscale>\n");
            exit(-1);
        }
    }
    
    return 0;
}

///////////////////////////////////////////SOLVING FOR TRIGGER////////////////////////////////////////////
int evaluate(char* postFix, int h, int g, int f, int e, int d, int c, int b, int a){
	int result;
    int op[100];
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

int level(char operator){
	if(operator == '~'){
		return 2;
	}
	else if(operator == '&' || operator == '|'){
		return 1;
	}
	else return 0;
}

///////////////////////////////////////////Get expr to PostFix Form///////////////////////////////////////////////
void truthTable (char * expr, char * postFix){
    int i, j;
    int count = 0;
    int operators = 0; //number of elements in the array
    int operands = 0;
    char logic[100];
    //char postFix[100] = {};
    char* expression;
	int order;     

    for(i = 0; i < strlen(expr); i++){ //looking at each char of expr
        
        if(expr[i] == 'A' || expr[i] == 'B' || expr[i] == 'C' || expr[i] == 'D' || expr[i] == 'E' || expr[i] == 'F' || expr[i] == 'G' || expr[i] == 'H'){
            ++count;
            postFix[operands] = expr[i];
            ++operands;
        }
        else if(expr[i] == '&' || expr[i] == '|' || expr[i] == '~' || expr[i] == '(' || expr[i] == ')'){
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
            }
            if(expr[i] == '&' || expr[i] == '|' || expr[i] == '~'){
				if(operators > 0){
					if(level(expr[i]) >= level(logic[operators-1])){
						//postFix[operands++] = expr[i];
						logic[operators++] = expr[i];
						//++operators;
						//++i;
				
					}
					else{
						postFix[operands++] = logic[--operators];
						logic[operators] = '\0';
                        logic[operators++] = expr[i];						
					}
				}
				else{
                    if(level(expr[i]) >= level(logic[operators])){
                        //postFix[operands++] = expr[i];
                        logic[operators++] = expr[i];
                        //++operators;
                        //++i;

                    }
					else{
						postFix[operands++] = logic[operators];
						logic[operators] = '\0';
						logic[operators++] = expr[i];
					} 
				}               
//++operators;	
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
    
}

///////////////////////////////////////////////////DRAW GRIDLINES//////////////////////////////////////////////////
void grid(VGfloat x, VGfloat y, // Coordinates of lower left corner
          int nx, int ny, // Number of x and y divisions
          int w, int h) // screen width and height

{
    VGfloat ix, iy;
    Stroke(128, 128, 128, 0.5); // Set color
    StrokeWidth(2); // Set width of lines 
    for (ix = x; ix <= x + w; ix += w/nx) {
        Line(ix, y, ix, y + h);
    }

    for (iy = y; iy <= y + h; iy += h/ny) {
        Line(x, iy, x + w, iy);
    }
}

////////////////////////////////////Draw the background for the oscilloscope screen//////////////////////
void drawBackground(int w, int h, // width and height of screen
                    int xdiv, int ydiv,// Number of x and y divisions
                    int margin){ // Margin around the image
    VGfloat x1 = margin;
    VGfloat y1 = margin;
    VGfloat x2 = w - 2*margin;
    VGfloat y2 = h - 2*margin;

    Background(128, 128, 128);

    Stroke(204, 204, 204, 1);
    StrokeWidth(1);

    Fill(0, 0, 0, 1);
    Rect(10, 10, w-20, h-20); // Draw framing rectangle

    grid(x1, y1, xdiv, ydiv, x2, y2); // Draw grid lines
}

/////////////////////////////////////////////PROCESS SAMPLES////////////////////////////////////////////////
void processSamples(char *data, // sample data
                    int nsamples, // Number of samples
                    int xstart, // starting x position of wave
                    int xfinish, // Ending x position of wave
                    float yscale, // y scale in pixels per volt
         	           data_point *point_array){
    VGfloat x1, y1;
    data_point p;

    for (int i=0; i< nsamples+shift; i++){
        x1 = xstart + (xfinish-xstart)*i/nsamples;
        y1 = data[i+shift] * 5 * yscale/256;
        p.x = x1;
        p.y = y1;
        point_array[i+shift] = p;
    }
}

/////////////////////////////////////////////////////PLOT WAVEFORM//////////////////////////////////////////////////
void plotWave(data_point *data, // sample data
              int nsamples, // Number of samples
              int yoffset, // y offset from bottom of screen
              VGfloat linecolor[4] // Color for the wave
){

    data_point p;
    VGfloat x1, y1, x2, y2;

    Stroke(linecolor[0], linecolor[1], linecolor[2], linecolor[3]);
    StrokeWidth(4);

    p = data[shift];
    x1 = p.x;
    y1 = p.y + yoffset;

    for (int i=0; i< nsamples+shift; i++){
		p = data[i+shift];
		
        x2 = p.x;
        y2 = p.y + yoffset;

		if(i+shift == trigIndex){ //this is the index the trigger occurred
            Stroke(75, 200, 0, 0.5); // Set color
            StrokeWidth(5); // Set width of lines           
            Line(x2,0,x2,1200);
        }

		Stroke(linecolor[0], linecolor[1], linecolor[2], linecolor[3]);
	    StrokeWidth(4);

        Line(x1, y1, x2, y2);
        x1 = x2;
        y1 = y2;


		Stroke(200, 2, 0, 0.5); // Set color
        StrokeWidth(5); // Set width of lines
        //draw cursor line
        Line(10+((potFrame[1]*1900)/255),0,10+((potFrame[1]*1900)/255),1200);


    }
}

///////////////////////////////////////////MAIN CONTROL LOOP////////////////////////////////////////
int main (int argc, char * argv[]){
    
	nchannels = 8;
	expr_file = "test.txt";
	triggerD = "pos";;
	mem_depth = 10000;
	sample_freq = 10;
	int xscale= 100;
	int yscale = 8;
	int rcvd_bytes;
	int return_val;
	FILE *in;

    char expr[100];
    char postFix[100] = {};
    int trigger[256] = {};

    int A=0, B=0, C=0, D=0, E=0, F=0, G=0, H=0;

	argCheck(argc, argv); //check and set the command-line arguments

    in = fopen(expr_file, "r");

    fgets(expr, 100, in);

	printf("expression: %s\n", expr);
    
//////////////////////////////////SOLVING LOGIC EQUATION AND GETTING TRUTH TABLE/////////////////////////////
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

	printf("truth table: \n");
    for(int i = 0; i < 256; ++i){
		if(i%16==0)
			printf("\n");
        printf("%d", trigger[i]);
    }
	printf("\n");
	
////////////////////////////////////////////////

    libusb_device_handle* dev; // Pointer to data structure representing USB device

    libusb_init(NULL); // Initialize the LIBUSB library

    // Open the USB device (the Cypress device has
    // Vendor ID = 0x04B4 and Product ID = 0x8051)
    dev = libusb_open_device_with_vid_pid(NULL, 0x04B4, 0x8051);

    if (dev == NULL){
        perror("device not found\n");
    }

    // Reset the USB device.
    // This step is not always needed, but clears any residual state from
    // previous invocations of the program.
    if (libusb_reset_device(dev) != 0){
        perror("Device reset failed\n");
    }

    // Set configuration of USB device
    if (libusb_set_configuration(dev, 1) != 0){
        perror("Set configuration failed\n");
    }


    // Claim the interface.  This step is needed before any I/Os can be
    // issued to the USB device.
    if (libusb_claim_interface(dev, 0) !=0){
        perror("Cannot claim interface");
    }


	/////////////////////////Set up and draw the display/////////////////
    int width, height; // Width and height of screen in pixels
    int margin = 10; // Margin spacing around screen
    int xdivisions = 10; // Number of x-axis divisions
    int ydivisions = 8; // Number of y-axis divisions
    char str[100];

    VGfloat textcolor[4] = {100, 2, 100, 0.5}; // Color for displaying text
    VGfloat wave1color[4] = {200, 2, 0, 1}; // Color for displaying Channel 1 data
    VGfloat wave2color[4] = {200, 3, 100, 1}; // Color for displaying Channel 2 data
    VGfloat wave3color[4] = {50, 200, 0, 1}; // Color for displaying Channel 2 data
    VGfloat wave4color[4] = {150, 50, 0, 1}; // Color for displaying Channel 2 data
    VGfloat wave5color[4] = {75, 200, 200, 1}; // Color for displaying Channel 2 data
    VGfloat wave6color[4] = {10, 2, 100, 1}; // Color for displaying Channel 2 data
    VGfloat wave7color[4] = {200, 100, 0, 1}; // Color for displaying Channel 2 data
    VGfloat wave8color[4] = {200, 200, 200, 1}; // Color for displaying Channel 2 data

    data_point channel0_points[20000];
    data_point channel1_points[20000];
	data_point channel2_points[20000];
    data_point channel3_points[20000];
	data_point channel4_points[20000];
    data_point channel5_points[20000];
	data_point channel6_points[20000];
    data_point channel7_points[20000];

    int xstart = margin;
    int xlimit = width - 2*margin;
    int ystart = margin;
    int ylimit = height - 2*margin;


    int pixels_per_volt = (ylimit-ystart)/(yscale);

    init(&width, &height); // Initialize display and get width and height
    Start(width, height);

    /////////////////////////////////READ IN DATA FROM PSoC////////////////////////////////
	return_val = libusb_bulk_transfer(dev, // Handle for the USB device
                                    (0x01 | 0x80), // Address of the Endpoint in USB device
                                    // MS bit nust be 1 for IN transfers
                                    channel_data, // address of receive data buffer
                                    20000, // Size of data buffer
                                    &rcvd_bytes, // Number of bytes actually received
                                    0 // Timeout in milliseconds (0 to disable timeout)
                                    );

    //////////////////////////////////SEARCH FOR TRIGGER/////////////////////////////////
	int i = mem_depth/2;

	if(strcmp("pos", triggerD) == 0){
		for(; i < 20000; ++i){
			if(trigger[channel_data[i + 1]] == 1 && trigger[channel_data[i]] == 0){
				trigIndex = i;
				shift = i - 500;
				printf("trigger point: %d\n", trigIndex);
				break;
			}
		}
	}
	else if(strcmp("neg", triggerD) == 0){
        for(; i < 20000; ++i){
            if(trigger[channel_data[i + 1]] == 0 && trigger[channel_data[i]] == 1){
                trigIndex = i;
                shift = i - 500;
                printf("trigger point: %d\n", trigIndex);
                break;
            }
        }
    }

	cX1 = margin;
	
    while(1){
		i = 0;
		for(; i < 20000; ++i){
			bit0[i] = (channel_data[i] & (1 << 0));
			bit1[i] = (channel_data[i] & (1 << 1));			
			bit2[i] = (channel_data[i] & (1 << 2));
			bit3[i] = (channel_data[i] & (1 << 3));
			bit4[i] = (channel_data[i] & (1 << 4));
			bit5[i] = (channel_data[i] & (1 << 5));
			bit6[i] = (channel_data[i] & (1 << 6));
			bit7[i] = (channel_data[i] & (1 << 7));
		}		
        //////////////////////////////////SET UP DISPLAY/////////////////////////////////
		WindowClear();
        Start(width, height);
        drawBackground(width, height, xdivisions, ydivisions, margin);
        
		if(nchannels == 8){
							     //nsamples			//xstart //endx //yscale
			processSamples(bit7, (xscale*xdivisions), margin, width, 30, channel0_points);
			processSamples(bit6, (xscale*xdivisions), margin, width, 50, channel1_points);
			processSamples(bit5, (xscale*xdivisions), margin, width, 100, channel2_points);
			processSamples(bit4, (xscale*xdivisions), margin, width, 200, channel3_points);
			processSamples(bit3, (xscale*xdivisions), margin, width, 400, channel4_points);
			processSamples(bit2, (xscale*xdivisions), margin, width, 600, channel5_points);
			processSamples(bit1, (xscale*xdivisions), margin, width, 800, channel6_points);
			processSamples(bit0, (xscale*xdivisions), margin, width, 1000, channel7_points);

			plotWave(channel0_points, (xscale*xdivisions), 1000, wave1color);
			plotWave(channel1_points, (xscale*xdivisions), 800, wave2color);
			plotWave(channel2_points, (xscale*xdivisions), 600, wave3color);
			plotWave(channel3_points, (xscale*xdivisions), 400, wave4color);
			plotWave(channel4_points, (xscale*xdivisions), 300, wave5color);
			plotWave(channel5_points, (xscale*xdivisions), 200, wave6color);
			plotWave(channel6_points, (xscale*xdivisions), 100, wave7color);
			plotWave(channel7_points, (xscale*xdivisions), 25, wave8color);
		}
		else if(nchannels == 4){
			processSamples(bit3, (xscale*xdivisions), margin, width, 400, channel4_points);
            processSamples(bit2, (xscale*xdivisions), margin, width, 600, channel5_points);
            processSamples(bit1, (xscale*xdivisions), margin, width, 800, channel6_points);
            processSamples(bit0, (xscale*xdivisions), margin, width, 1000, channel7_points);

			plotWave(channel4_points, (xscale*xdivisions), 300, wave5color);
            plotWave(channel5_points, (xscale*xdivisions), 200, wave6color);
            plotWave(channel6_points, (xscale*xdivisions), 100, wave7color);
            plotWave(channel7_points, (xscale*xdivisions), 25, wave8color);

		}
		else if(nchannels == 2){
			processSamples(bit1, (xscale*xdivisions), margin, width, 800, channel6_points);
            processSamples(bit0, (xscale*xdivisions), margin, width, 1000, channel7_points);

			plotWave(channel6_points, (xscale*xdivisions), 100, wave7color);
            plotWave(channel7_points, (xscale*xdivisions), 25, wave8color);
		}
		
		End();
        
        ///////////////////////////READ IN POT VALUES FROM PSoC//////////////////////////////////
		return_val = libusb_bulk_transfer(dev, // Handle for the USB device
                                    (0x03 | 0x80), // Address of the Endpoint in USB device
                                    // MS bit nust be 1 for IN transfers
                                    potFrame, // address of receive data buffer
                                    2, // Size of data buffer
                                    &rcvd_bytes, // Number of bytes actually received
                                    0 // Timeout in milliseconds (0 to disable timeout)
                                    );

        //////////////////////////////SOLVE FOR FRAME SHIFT////////////////////////////
        printf("potFrame: %d\n", potFrame[0]);
		shift = trigIndex+(mem_depth/2)*((potFrame[0]/(float)128)-(float)1);

		printf("potFrame 2: %d\n", potFrame[1]);
			
    }

    libusb_close(dev);

    return 0;
}
