
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#ifndef PI
	#include <X11/Xlib.h>
	#include "X11/keysym.h"
#else
	extern int usleep (__useconds_t __useconds);
	#include <wiringPi.h>
	#include <stdbool.h>
	#include <wiringPiI2C.h>
	#include <stdint.h>
	#include <sys/time.h> 

	#define XK_Left 6
	#define XK_Right 24
	#define XK_Up 25
	#define XK_Down 23

	#define APPUYE LOW
	#define	BUZZER	1
	#define VIBRER 	2
	#define HT16K33_BLINK_CMD 0x80 
	#define HT16K33_BLINK_DISPLAYON 0x01 
	#define HT16K33_BLINK_OFF 0 
	#define HT16K33_BLINK_2HZ  1 
	#define HT16K33_BLINK_1HZ  2 
	#define HT16K33_BLINK_HALFHZ  3 

	#define HT16K33_CMD_BRIGHTNESS 0xE0 

	#define SEVENSEG_DIGITS 5 

	// déclaration globale 
	uint16_t nombre[4]; 

	static const uint8_t chiffre[] = { 
		0x3F, /* 0 */ 
		0x06, /* 1 */ 
		0x5B, /* 2 */ 
		0x4F, /* 3 */ 
		0x66, /* 4 */ 
		0x6D, /* 5 */ 
		0x7D, /* 6 */ 
		0x07, /* 7 */ 
		0x7F, /* 8 */ 
		0x6F, /* 9 */ 
		0xBF, /* 0.*/ 
		0x86, /* 1.*/ 
		0xDB, /* 2.*/ 
		0xCF, /* 3.*/ 
		0xE6, /* 4.*/ 
		0xED, /* 5.*/ 
		0xFD, /* 6.*/ 
		0x87, /* 7.*/ 
		0xFF, /* 8.*/ 
		0xEF, /* 9.*/ 
		0x00, // rien du tout

	};
#endif 

#define ANSI_RESET "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_PURPLE "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"
#define ANSI_ORANGE "\e[0;33m"

#ifdef DEBUG
	#define DEBUG_I(i) printf("%d\n", i);
	#define DEBUG_S(s) printf("%s\n", s);
	#define DEBUG_S1(s, p1) printf(s, p1);
	#define DEBUG_S2(s, p1, p2) printf(s, p1, p2);
	#define DEBUG_S3(s, p1, p2, p3) printf(s, p1, p2, p3);
	#define DEBUG_S4(s, p1, p2, p3, p4) printf(s, p1, p2, p3, p4);
#else
	#define DEBUG_I(i)
	#define DEBUG_S(s) (void)0;
	#define DEBUG_S1(s, p1) (void)0;
	#define DEBUG_S2(s, p1, p2) (void)0;
	#define DEBUG_S3(s, p1, p2, p3) (void)0;
	#define DEBUG_S4(s, p1, p2, p3, p4) (void)0;
#endif

#define WIDTH 1
#define HEIGHT 0.7

#ifdef PI
	#define X_PIX 250
	#define Y_PIX 100

	void setBrightness(int fd, uint8_t b) { 
		if (b > 15) b = 15; 
		wiringPiI2CWrite(fd, HT16K33_CMD_BRIGHTNESS | b); 
	} 

	void blinkRate(int fd, uint8_t b) { 
		if (b > 3) b = 0; // turn off if not sure 
		wiringPiI2CWrite(fd, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1)); 
	} 

	void begin(int fd) { 
		wiringPiI2CWrite(fd, 0x21); 
		blinkRate(fd, HT16K33_BLINK_OFF); 
		setBrightness(fd, 15); // max brightness 
	} 
#else 
	#define X_PIX 500
	#define Y_PIX 200	
#endif




typedef struct Vector{
	float x;
	float y;
	float z;
} vect;

typedef struct Vector2{
	float x;
	float y;
} vect2;

vect vect_scale(float  s, vect v) {
	vect res = {s*v.x, s*v.y, s*v.z};
	return res;
}

vect vect_add(vect v1, vect v2) {
	vect res;
	res.x = v1.x + v2.x;
	res.y = v1.y + v2.y;
	res.z = v1.z + v2.z;
	return res;
}

vect vect_sub(vect v1, vect v2) {
	vect v3 = vect_scale(-1, v2);
	return vect_add(v1, v3);
}


vect vect_normalize(vect v) {
	float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	v.x /= len;
	v.y /= len;
	v.z /= len;
	return v;
}

float vect_dot(vect v1, vect v2) {
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

vect vect_cross(vect v1, vect v2) {
	return (vect) {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

void vect_print(vect v) {
	printf("x=%f, y=%f, z=%f\n", v.x, v.y, v.z);
}

vect2 vect2_add(vect2 v1, vect2 v2) {
	return (vect2) {v1.x+v2.x, v1.y+v2.y};
}

vect2 vect2_sub(vect2 v1, vect2 v2) {
	return (vect2) {v1.x-v2.x, v1.y-v2.y};
}

vect2 vect2_scale(float a, vect2 v) {
	return (vect2) {a*v.x, a*v.y};
}

float vect2_dot(vect2 v1, vect2 v2) {
	return v1.x*v2.x + v1.y*v2.y;
}

void vect2_print(vect2 v) {
	printf("%f %f\n", v.x, v.y);
}

#ifndef PI
	int key_is_pressed(KeySym ks) {
		Display *dpy = XOpenDisplay(0);
		char keys_return[32];
		XQueryKeymap(dpy, keys_return);
		KeyCode kc2 = XKeysymToKeycode(dpy, ks);
		int isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
		XCloseDisplay(dpy);
		return isPressed;
	}
#else
	int key_is_pressed(int btn){
		return (digitalRead(btn) == APPUYE);
	}
#endif


vect2 project_point(vect dir, vect point) {
	float alpha = vect_dot(point, dir) / (1 + vect_dot(point, dir));
	// project onto plane orthogonal to dir through -dir
	vect p1 = vect_add(vect_scale(-alpha, dir), vect_scale(1-alpha, point));
	// turn into 2 coordinates
	vect x = vect_normalize((vect) {-dir.y, dir.x, 0});
	vect y = vect_cross(dir, x);
	float x_coord = vect_dot(p1, x);
	float y_coord = vect_dot(p1, y);
	return (vect2) {x_coord, y_coord};
}


void put_point(vect2 projected, char c, char **picture) {
	int x_coord = (int) ((-projected.x + WIDTH)/(2*WIDTH)*X_PIX);
	int y_coord = (int) ((-projected.y + HEIGHT)/(2*HEIGHT)*Y_PIX);
	if (x_coord < 0 || x_coord >= X_PIX || y_coord < 0 || y_coord >= Y_PIX) {
		return;
	}
	else {
		picture[y_coord][x_coord] = c;
	}
}

void draw_point(vect dir, vect point, char c, char **picture) {
	// can't see points behind you
	if (vect_dot(dir, point) <= 0) {
		return;
	}
	vect2 projected = project_point(dir, point);
	put_point(projected, c, picture);
}

void draw_line(vect dir, vect v_from, vect v_to, char c, char **picture) {
	// remove the part of the line that is behind the viewer
	float scale_from = vect_dot(v_from, dir);
	float scale_to = vect_dot(v_to, dir);
	if (scale_from < 0 && scale_to < 0) {
		return;
	}
	if (scale_from < 0) {
		scale_from = -scale_from;
		v_from = vect_add(vect_scale(scale_from/(scale_from+scale_to), v_to), vect_scale(scale_to/(scale_from+scale_to), v_from));
	}
	if (scale_to < 0) {
		scale_to = -scale_to;
		v_to = vect_add(vect_scale(scale_to/(scale_to+scale_from), v_from), vect_scale(scale_from/(scale_to+scale_from), v_to));
	}
	// now draw the line
	vect2 proj_from = project_point(dir, v_from);
	vect2 proj_to = project_point(dir, v_to);
	vect2 diff = vect2_sub(proj_to, proj_from);
	int steps = (X_PIX * (fabs(diff.x)/WIDTH) + Y_PIX * (fabs(diff.y)/HEIGHT))/2 + 1;
	vect2 step = vect2_scale(1.0/steps, diff);
	for (int i = 0; i <= steps; ++i) {
		put_point(proj_from, c, picture);
		proj_from = vect2_add(proj_from, step);
	}
}


void draw_ascii(char **picture) {
	#ifndef UNDRAW
	printf("\033[0;0H");	// jump to position 0 0 to overwrite current picture
	for (int i = 0; i < Y_PIX; ++i) {
		
		for (int j = 0; j < X_PIX; ++j) {
			if (picture[i][j] == 'o')
			{
				printf("%s", ANSI_BLUE);
			}
			if (picture[i][j] == 'x')
			{
				printf("%s", ANSI_ORANGE);
			}
			if (picture[i][j] == 'I')
			{
				printf("%s", ANSI_RED);
			}
			printf("%c", picture[i][j]);
		}
		printf("\n");
		//printf("%s\n",picture[i]);
	}
	#else
		//printf("\033[0;0H");
		printf("appel de la fonction draw_ascci\n");
	#endif	
}

char **empty_picture(char empty_char) {
	char **pic;
	pic = malloc(sizeof(char *) * Y_PIX);
	for (int i = 0; i < Y_PIX; ++i) {
		pic[i] = malloc(sizeof(char *) * X_PIX);
		for (int j = 0; j < X_PIX; ++j) {
			pic[i][j] = empty_char;
		}
	}
	return pic;
}

float random_float()
{
    float r = (float)rand()/(float)RAND_MAX;
    return r;
}

int *init_obstacles(int size) {
	/*
	0 = no obstacle
	1 = right thing
	2 = left thing
	3 = down thing
	4 = up thing
	*/
	int *res = calloc(size, sizeof(int));
	for (int i = 0; i < size; ++i) {
		res[i] = rand()%5;
		if (res[i] != 0) {
			i += 2;
		}
	}
	res[0] = 0;
	res[1] = 0;
	res[size-1] = 0;
	return res;
}

int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}

#ifndef PI
	#define PATH_WIDTH 1
	#define Y_BORDER 0.7
	#define SIGHT 10	// how far you can see (roughly)
	#define GRAVITY 30
	#define JUMP_SPEED 8
	#define SPEED_INCREASE 0.1
	#define TEMPO_END 150000
	#define TEMPO_FRAME 1000000
#else
	#define PATH_WIDTH 1
	#define Y_BORDER 0.7
	#define SIGHT 10	// how far you can see (roughly)
	#define GRAVITY 30
	#define JUMP_SPEED 8
	#define SPEED_INCREASE 0.1
	#define TEMPO_END 100000
	#define TEMPO_FRAME 1000000
#endif

int main(void) {
	
	#ifndef PI
		vect dir = (vect) {1, 0, 0};
		float speed = 3;
		float tstep = 0.03;
		int turn_dist_orig = 5 + rand()%10;
		float turn_dist = turn_dist_orig;
		int next_turn_dist = 5 + rand()%10;
		// next_turn: -1 for right, 1 for left
		int next_turn = (rand()%2)*2 - 1;
		float cam_height = 1;
		float y_move_speed = 3;
		float duckspeed = 4;
		float zpos = 0;
		float ypos = 0;
		float zspeed = 0;
	#else
		vect dir = (vect) {1, 0, 0};
		float speed = 3;//3
		float tstep = 0.03;//0.03
		int turn_dist_orig = 5 + rand()%10;
		float turn_dist = turn_dist_orig;
		int next_turn_dist = 5 + rand()%10;
		// next_turn: -1 for right, 1 for left
		int next_turn = (rand()%2)*2 - 1;
		float cam_height = 1;
		float y_move_speed = 3;//3
		float duckspeed = 4;//4
		float zpos = 0;
		float ypos = 0;
		float zspeed = 0;

		//init  pin
		wiringPiSetup () ;
		pinMode (VIBRER, OUTPUT) ;
		pinMode (BUZZER, OUTPUT) ;
		pinMode (XK_Down, INPUT) ;
		pinMode (XK_Up, INPUT) ;
		pinMode (XK_Left, INPUT) ;
		pinMode (XK_Right, INPUT) ;
		pullUpDnControl (XK_Down,PUD_UP);
		pullUpDnControl (XK_Left,PUD_UP);
		pullUpDnControl (XK_Right,PUD_UP);
		pullUpDnControl (XK_Up,PUD_UP);
		
		int fda = wiringPiI2CSetup(0x70); 
		//printf("fda : %d \n", fda); 

		// begin 
		begin(fda); 

	#endif
	draw_ascii(empty_picture('X'));
	printf("Veuiller ajuster la taille de l'écrant, pour commencer apuyer sur une touche de déplacement\n");
	while(!(key_is_pressed(XK_Up) || key_is_pressed(XK_Down) || key_is_pressed(XK_Left) || key_is_pressed(XK_Right))){
		usleep(10000);
	}

	START:
	srand(time(NULL));

	int *obstacles = malloc(sizeof(int)*100);
	for (int i = 0; i < 100; ++i) {
		obstacles[i] = 0;
	}
	int *next_obstacles = init_obstacles(next_turn_dist+1);

	// main game loop
	int i = 0;
	while (1) {
		// keyboard stuff
		// move left/right
		if (key_is_pressed(XK_Right)) {
			ypos = fmax(ypos-y_move_speed*tstep, -Y_BORDER);
			DEBUG_S("Right pressed");

		}
		else if (key_is_pressed(XK_Left)) {
			ypos = fmin(ypos + y_move_speed*tstep, Y_BORDER);
			DEBUG_S("Left pressed");
		}
		else {
			if (ypos > 0) {
				ypos = fmax(0, ypos-y_move_speed*tstep);
			}
			else if (ypos < 0) {
				ypos = fmin(0, ypos + y_move_speed*tstep);
			}
		}
		// jump
		if (zpos == 0 && key_is_pressed(XK_Up)) {
			// initiate jump
			zspeed = JUMP_SPEED;
			DEBUG_S("up pressed");
		}
		zpos += zspeed*tstep;
		zspeed -= GRAVITY*tstep;
		if (zpos < 0) {
			zspeed = 0;
		}
		// duck
		if (key_is_pressed(XK_Down)) {
			zpos -= duckspeed*tstep;
			zpos = fmax(zpos, -0.5);
			DEBUG_S("Down pressed");
		}
		else if (zpos < 0) {
			zpos += duckspeed*tstep;
			if (zpos >= 0) {
				zpos = 0;
			}
		}
		// lost
		if (turn_dist < -2) {
			draw_ascii(empty_picture('-'));
			break;
		}
		// turn
		else if (turn_dist < 0) {
			if ((next_turn == -1 && key_is_pressed(XK_Right)) || (next_turn == 1 && key_is_pressed(XK_Left))) {
				DEBUG_S("Right or left pressed");
				turn_dist = next_turn_dist;
				turn_dist_orig = next_turn_dist;
				next_turn_dist = 5 + rand()%10;
				next_turn = (rand()%2)*2 - 1;
				obstacles = next_obstacles;
				next_obstacles = init_obstacles(next_turn_dist+1);
			}
		}
		char **pic = empty_picture(' ');
		for (float d = turn_dist; d > 0; d -= 1) {
			draw_line(dir, (vect){d, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){d, PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
		}

		// check for collision
		float real_pos = turn_dist_orig - turn_dist;
		int int_pos = trunc(real_pos);
		// printf("\n%f %f %d %f\n", next_turn_dist, real_pos, int_pos, fabs(int_pos-real_pos));
		if (fabs(int_pos - real_pos) < tstep*speed*5) {
			// check for collision
			if (obstacles[int_pos] == 1 && ypos < 0.1) {
				break;
			}
			else if (obstacles[int_pos] == 2 && ypos > -0.1) {
				break;
			}
			else if (obstacles[int_pos] == 3 && zpos < 0.5) {
				break;
			}
			else if (obstacles[int_pos] == 4 && zpos > -0.2) {
				break;
			}
		}

		// draw path and next path
		if (turn_dist > SIGHT) {
			draw_line(dir, (vect){0, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){SIGHT, PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
			draw_line(dir, (vect){0, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){SIGHT, -PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
		}
		else {
			if (next_turn == -1) {
				draw_line(dir, (vect){0, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist+2*PATH_WIDTH, PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir, (vect){0, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist, -PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir,  (vect){turn_dist+2*PATH_WIDTH, PATH_WIDTH-ypos, -(cam_height+zpos)}, 
								(vect){turn_dist+2*PATH_WIDTH, -SIGHT, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir, (vect){turn_dist, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist, -SIGHT, -(cam_height+zpos)}, 'x', pic);
			}
			else {	// next_turn = 1
				draw_line(dir, (vect){0, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist+2*PATH_WIDTH, -PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir, (vect){0, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist, PATH_WIDTH-ypos, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir,  (vect){turn_dist+2*PATH_WIDTH, -PATH_WIDTH-ypos, -(cam_height+zpos)}, 
								(vect){turn_dist+2*PATH_WIDTH, SIGHT, -(cam_height+zpos)}, 'x', pic);
				draw_line(dir, (vect){turn_dist, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){turn_dist, SIGHT, -(cam_height+zpos)}, 'x', pic);
			}
			for (float i = 1-next_turn*ypos; i < SIGHT; i += 1) {
				draw_line(dir,  (vect){turn_dist, i*next_turn, -(cam_height+zpos)}, 
								(vect){turn_dist+2*PATH_WIDTH, i*next_turn, -(cam_height+zpos)}, 'x', pic);
			}
		}

		// draw obstacles
		for (int i = 0; i <= turn_dist; ++i) {
			int obst = obstacles[turn_dist_orig - i];
			float pos = turn_dist - i;
			if (obst == 1) {
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -ypos, -(cam_height+zpos)}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -ypos, -(cam_height+zpos)}, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);

			}
			else if (obst == 2) {
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -ypos, -(cam_height+zpos)}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -ypos, -(cam_height+zpos)}, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, -ypos, -zpos+0.5}, 'o', pic);
			}
			else if (obst == 3) {
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, PATH_WIDTH-ypos, -zpos-0.5}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos-0.5}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.5}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, PATH_WIDTH-ypos, -zpos-0.5}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -(cam_height+zpos)}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.5}, 'o', pic);				
			}
			else if (obst == 4) {
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, PATH_WIDTH-ypos, -zpos-0.2}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos-0.2}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.2}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.2}, 'o', pic);
				draw_line(dir, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, PATH_WIDTH-ypos, -zpos-0.2}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, -PATH_WIDTH-ypos, -zpos-0.2}, 'o', pic);
				draw_line(dir, (vect){pos, PATH_WIDTH-ypos, -zpos+0.5}, (vect){pos, -PATH_WIDTH-ypos, -zpos+0.5}, 'o', pic);			
			}
		}
		// draw next obstacles
		if (turn_dist < SIGHT) {
			for (int i = 0; i <= min(SIGHT, next_turn_dist); ++i) {
				int obst = next_obstacles[i];
				float obst_y = -ypos+(i+PATH_WIDTH)*next_turn;
				float right_x = turn_dist + (next_turn+1)*PATH_WIDTH;
				float left_x = turn_dist + (next_turn*(-1)+1)*PATH_WIDTH;
				if (obst == 1) {
					draw_line(dir, (vect){right_x, obst_y, -(cam_height+zpos)}, (vect){right_x, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -(cam_height+zpos)}, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos+0.5}, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -(cam_height+zpos)},(vect){right_x, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, (vect){right_x, obst_y, -(cam_height+zpos)}, 'o', pic);

				}
				else if (obst == 2) {
					draw_line(dir, (vect){left_x, obst_y, -(cam_height+zpos)}, (vect){left_x, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -(cam_height+zpos)}, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){left_x, obst_y, -zpos+0.5}, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -(cam_height+zpos)},(vect){left_x, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){turn_dist+PATH_WIDTH, obst_y, -zpos+0.5}, (vect){left_x, obst_y, -(cam_height+zpos)}, 'o', pic);
				}
				else if (obst == 3) {
					draw_line(dir, (vect){left_x, obst_y, -(cam_height+zpos)}, (vect){left_x, obst_y, -zpos-0.5}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -(cam_height+zpos)}, (vect){right_x, obst_y, -zpos-0.5}, 'o', pic);
					draw_line(dir, (vect){left_x, obst_y, -zpos-0.5}, (vect){right_x, obst_y, -zpos-0.5}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -(cam_height+zpos)},(vect){left_x, obst_y, -zpos-0.5}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos-0.5}, (vect){left_x, obst_y, -(cam_height+zpos)}, 'o', pic);
				}
				else if (obst == 4) {
					draw_line(dir, (vect){left_x, obst_y, -zpos+0.5}, (vect){left_x, obst_y, -zpos-0.2}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos+0.5}, (vect){right_x, obst_y, -zpos-0.2}, 'o', pic);
					draw_line(dir, (vect){left_x, obst_y, -zpos-0.2}, (vect){right_x, obst_y, -zpos-0.2}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos+0.5},(vect){left_x, obst_y, -zpos-0.2}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos-0.2}, (vect){left_x, obst_y, -zpos+0.5}, 'o', pic);
					draw_line(dir, (vect){right_x, obst_y, -zpos+0.5}, (vect){left_x, obst_y, -zpos+0.5}, 'o', pic);
				}
			}
		}
		turn_dist -= tstep*speed;	
		draw_ascii(pic);
		// printf("%f %d \n", turn_dist, next_turn);
		// printf("%d\n", key_is_pressed(XK_Right));
		speed += SPEED_INCREASE*tstep;
		y_move_speed += SPEED_INCREASE*tstep*0.5;
		duckspeed += SPEED_INCREASE*tstep*0.5;
		#ifndef PI
			if(i<10000)printf("%d\n", i++);
		#else
			//i++;
			//printf("%d\n", i++);
			if(i<10000){
				i++;
				if(i%100==0){
					digitalWrite (BUZZER, HIGH) ;
				}
				if(i%100==1){
					digitalWrite (BUZZER, LOW) ;
				}
			}
			nombre[3] = i / 1000;
			nombre[2] = (i - nombre[3] * 1000) / 100;
			nombre[1] = (i - nombre[3] * 1000 - nombre[2] * 100) / 10;
			nombre[0] = (i - nombre[3] * 1000 - nombre[2] * 100 - nombre[1] * 10);	 
			wiringPiI2CWriteReg8(fda, 0x00, chiffre[nombre[3]] );  
			wiringPiI2CWriteReg8(fda, 0x02, chiffre[nombre[2]] );
			wiringPiI2CWriteReg8(fda, 0x04, 0x00 );
			wiringPiI2CWriteReg8(fda, 0x05, 0x00 );
			wiringPiI2CWriteReg8(fda, 0x06, chiffre[nombre[1]] );
			wiringPiI2CWriteReg8(fda, 0x08, chiffre[nombre[0]] );
			

		#endif
		usleep(TEMPO_FRAME*tstep);
	}

	// game finished
	#ifdef PI
		digitalWrite (VIBRER, HIGH) ;
	#endif
	for (int i = 0; i < 2; ++i) {
		draw_ascii(empty_picture(' '));
		usleep(TEMPO_END);
		draw_ascii(empty_picture('I'));
		usleep(TEMPO_END);

	}
	#ifdef PI
		digitalWrite (VIBRER, LOW) ;
		digitalWrite (BUZZER, LOW) ;
		usleep(TEMPO_END*4);
		wiringPiI2CWriteReg8(fda, 0x00, 0x00 );  
		wiringPiI2CWriteReg8(fda, 0x02, 0x00 );
		wiringPiI2CWriteReg8(fda, 0x04, 0x00 );
		wiringPiI2CWriteReg8(fda, 0x05, 0x00 );
		wiringPiI2CWriteReg8(fda, 0x06, 0x00 );
		wiringPiI2CWriteReg8(fda, 0x08, 0x00 );
		

		
	#endif
	//redemarage automatique si clé préssé
	for (int i = 0; i < 200; ++i) {
		if (key_is_pressed(XK_Up) || key_is_pressed(XK_Down) || key_is_pressed(XK_Left) || key_is_pressed(XK_Right)) {
			goto START;
		}
		usleep(10000);
	}
	draw_ascii(empty_picture(' '));
}
