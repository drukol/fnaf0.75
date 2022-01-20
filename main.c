/* Oleksii Druk */


#define _XOPEN_SOURCE
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



/* _DEF_ */



/* For convenience's sake */
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGTH 720
#define fanfreq 20 /* Frequency of fan rotation, basically FPS */
#define msens 500 /* Sensitivity of mouse input*/
#define AMPERSEC 120 /* How many IRL seconds are there in an IG hour */



/* _STATE_ */



/* To count time, since FPS isn't constant */
long int prevtime = 0, curtime = 0;
double delta;

/* Fan timer, as well as its state, overall timer, and an int to store the hour specifically */
double frame, tim;
int fs = 0;
int timam = 0;

/* Energy, obviously*/
double energy;

/* States of doors and lights */
int r_door, r_light, l_door, l_light;

/* Timers for animation of doors, tablet, power off, winning and screamers */
double rdanim, ldanim, canim, chanim, nopanim, wanim, scranim;


/* Overall state of the game */
int menu; /* Which menu am I in */
_Bool isCamera; /* Is tablet up */
int currentCamera; /* Which camera am I browsing */
_Bool isRunning; /* Is the game running */
int screamer; /* Which screamer am I experiencing */ 

/* How much energy am I using right now */
int usage;

/* State of all animatronics. Where are they, are they targeting the player, and timers until they move / until they start targeting the player */
int bonnie_pos, bonnie_isRoaming; double bonnie_timer, bonnie_move;
int freddy_pos, freddy_isRoaming; double freddy_timer, freddy_move;
int chika_pos,  chika_isRoaming;double chika_timer, chika_move;

/* Which stage is foxy in, is he going fast (which he does if you check on him), how much time till the next stage, timers for his running duration and running animation */
int foxy_pos, foxy_fast; double foxy_timer, foxy_run, fxanim;


/* "Hitboxes" for text and energy usage sprites */
SDL_Rect txt, usgr;

/* Introduction timer */
double introd;

/* Renderer initialization */
SDL_Renderer *render;

/* Defines the position of the screen relative to background */
SDL_Rect srcc;



/* _FUNC_ */



/* Set all the game values to whatever they need to be at the beginning */
void resetGame(){
	isRunning = 1;
	txt.x = 20;
	txt.y = SCREEN_HEIGTH-(usgr.h+26);
	txt.h = 23;
	SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	energy = 100;
	timam = 0; tim = 0; wanim = 0;
	currentCamera = 0; isCamera = 0; screamer = 0; r_door = r_light = l_door = l_light = 0; rdanim = ldanim = 0; scranim = 0;
	menu = 1; canim = -0.01; chanim = 8/60.0;
	introd = 5;
	foxy_pos = chika_pos = freddy_pos = bonnie_pos = 0;
	bonnie_move = 15+(drand48()*45);
	bonnie_timer = bonnie_move+15+(30*drand48());
	freddy_move = 15+(drand48()*45);
	freddy_timer = freddy_move+(15)+(30*drand48());
	chika_move = 15+(drand48()*45);
	chika_timer = chika_move+(15)+(30*drand48());
	foxy_timer = 90;
	foxy_run = -0.01; fxanim = -0.01;
	foxy_timer += drand48()*(90);
}

/* Loading optimized textures */
SDL_Texture *load_t(char const *path, SDL_Renderer *target_surface)
{
    SDL_Texture *optimized_version = NULL;
    SDL_Surface *image_surface = SDL_LoadBMP(path);

    if(!image_surface)
        return 0;

    optimized_version = SDL_CreateTextureFromSurface(target_surface, image_surface);

    SDL_FreeSurface(image_surface);
    if(!optimized_version) return 0;


    return optimized_version;
}

/* Which background specifically is to be used */
int bkg(){
	if(l_light) return  bonnie_pos== 10 ? 3 : 2;
	if(r_light) return chika_pos==11 ? 4 : 1;
	return 0;
}

/* Which of the camera states to be used */
int cimg(int cam){
	if(cam == 3) return foxy_pos; /* Foxy stages */
	return (bonnie_pos==cam)<<2 | (freddy_pos==cam)<<1 | (chika_pos==cam); /* Bitmask for different combinations, where 1XX means Bonnie, X1X means Freddie, an XX1 - Chika */
}

/* Check whether or not is the mouse in the "hitbox", and whether to relate to the background or the screen */
_Bool checkClick(int x, int y, SDL_Rect rect, int scr){
	return ((x >= (rect.x-(scr*srcc.x)) && x < ((rect.x-(scr*srcc.x)) + rect.w)) && (y >= rect.y && y < (rect.y + rect.h)));
}

/* A quick way to get a texture's size */
SDL_Point getsize(SDL_Texture *texture) {
    SDL_Point size;
    SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
    return size;
}

/* Here it comes */
int main(int argc, char * argv[]){
	


	/* _INIT_ */


	
	int i, j; /* For loops */
	
	
	/* Randomizer initialization */
	srand48(time(NULL));
	
	/* Initialize all the necessary values */
	resetGame();

	/* Initialize the window */
	SDL_Window *window;
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Video Initialization Error: %s\n", SDL_GetError());
		isRunning = 0;
	}
	window = SDL_CreateWindow(
		"Five Nights At Freddy's 3/4 - handicapped edition by Oleksii Druk", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		SCREEN_WIDTH, SCREEN_HEIGTH, 
		SDL_WINDOW_SHOWN
	);
	if(window == NULL) {
		printf("Window creation error: %s\n", SDL_GetError());
		isRunning = 0;
	}
	
	/* Event handler */
	SDL_Event event;
	
	/* Initialize the renderer */
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	
	
	/* _TEXTURES_ */
	


	/* Initializations */
	SDL_Texture *mmenu[4];
	SDL_Texture *backg[5];
	SDL_Texture *fan[3];
	SDL_Texture *b_left[4], *b_right[4];
	SDL_Texture *door_r[15], *door_l[15];
	SDL_Texture *room[10][8];
	SDL_Texture *change[8];
	SDL_Texture *tablet[10];
	SDL_Texture *usg[5];
	SDL_Texture *runfox[31];
	SDL_Texture *cicon[23];
	SDL_Texture *text[15];
	SDL_Texture *miscscreens[3];
	SDL_Texture *intr;
	
	/* Dynamically allocate for screamers' vectors, since they aren't of the same length */
	SDL_Texture **screamers[6];
	
	int scree[6]; scree[0] = -1; scree[1] = 17; scree[2] = 12; scree[3] = 30; scree[4] = 22; scree[5] = 22;
	for(i = 0; i < 6; i++){
		if(scree[i] != -1){
			screamers[i] = calloc(scree[i], sizeof(SDL_Texture*));
		}
	}
	int screp[6]; screp[0] = -1; screp[1] = 3; screp[2] = 4; screp[3] = screp[4] = screp[5] = 1;
	
	

	/* _LOAD_ */
	
	
	
	/* Screamers' animations */
	
	/* Chika */
	screamers[1][0] = load_t("screamers/65.bmp", render);
	screamers[1][1] = load_t("screamers/69.bmp", render);
	screamers[1][2] = load_t("screamers/216.bmp", render);
	screamers[1][3] = load_t("screamers/228.bmp", render);
	screamers[1][4] = load_t("screamers/229.bmp", render);
	screamers[1][5] = load_t("screamers/230.bmp", render);
	screamers[1][6] = load_t("screamers/231.bmp", render);
	screamers[1][7] = load_t("screamers/232.bmp", render);
	screamers[1][8] = load_t("screamers/233.bmp", render);
	screamers[1][9] = load_t("screamers/234.bmp", render);
	screamers[1][10] = load_t("screamers/235.bmp", render);
	screamers[1][11] = load_t("screamers/236.bmp", render);
	screamers[1][12] = load_t("screamers/237.bmp", render);
	screamers[1][13] = load_t("screamers/239.bmp", render);
	screamers[1][14] = load_t("screamers/279.bmp", render);
	screamers[1][15] = load_t("screamers/281.bmp", render);
	screamers[1][16] = load_t("img/10.bmp", render);
	
	/* Bonnie */
	screamers[2][0] = load_t("screamers/291.bmp", render);
	screamers[2][1] = load_t("screamers/293.bmp", render);
	screamers[2][2] = load_t("screamers/294.bmp", render);
	screamers[2][3] = load_t("screamers/295.bmp", render);
	screamers[2][4] = load_t("screamers/296.bmp", render);
	screamers[2][5] = load_t("screamers/297.bmp", render);
	screamers[2][6] = load_t("screamers/298.bmp", render);
	screamers[2][7] = load_t("screamers/299.bmp", render);
	screamers[2][8] = load_t("screamers/300.bmp", render);
	screamers[2][9] = load_t("screamers/301.bmp", render);
	screamers[2][10] = load_t("screamers/303.bmp", render);
	screamers[2][11] = load_t("img/10.bmp", render);
	
	/* Freddy (usual) */
	screamers[3][0] = load_t("screamers/485.bmp", render);
	screamers[3][1] = load_t("screamers/489.bmp", render);
	screamers[3][2] = load_t("screamers/490.bmp", render);
	screamers[3][3] = load_t("screamers/491.bmp", render);
	screamers[3][4] = load_t("screamers/493.bmp", render);
	screamers[3][5] = load_t("screamers/495.bmp", render);
	screamers[3][6] = load_t("screamers/496.bmp", render);
	screamers[3][7] = load_t("screamers/497.bmp", render);
	screamers[3][8] = load_t("screamers/498.bmp", render);
	screamers[3][9] = load_t("screamers/499.bmp", render);
	screamers[3][10] = load_t("screamers/500.bmp", render);
	screamers[3][11] = load_t("screamers/501.bmp", render);
	screamers[3][12] = load_t("screamers/502.bmp", render);
	screamers[3][13] = load_t("screamers/503.bmp", render);
	screamers[3][14] = load_t("screamers/504.bmp", render);
	screamers[3][15] = load_t("screamers/505.bmp", render);
	screamers[3][16] = load_t("screamers/506.bmp", render);
	screamers[3][17] = load_t("screamers/507.bmp", render);
	screamers[3][18] = load_t("screamers/508.bmp", render);
	screamers[3][19] = load_t("screamers/509.bmp", render);
	screamers[3][20] = load_t("screamers/510.bmp", render);
	screamers[3][21] = load_t("screamers/511.bmp", render);
	screamers[3][22] = load_t("screamers/512.bmp", render);
	screamers[3][23] = load_t("screamers/513.bmp", render);
	screamers[3][24] = load_t("screamers/514.bmp", render);
	screamers[3][25] = load_t("screamers/515.bmp", render);
	screamers[3][26] = load_t("screamers/516.bmp", render);
	screamers[3][27] = load_t("screamers/517.bmp", render);
	screamers[3][28] = load_t("screamers/518.bmp", render);
	screamers[3][29] = load_t("img/10.bmp", render);
	
	/* Foxy */
	screamers[4][0] = load_t("screamers/413.bmp", render);
	screamers[4][1] = load_t("screamers/415.bmp", render);
	screamers[4][2] = load_t("screamers/242.bmp", render);
	screamers[4][3] = load_t("screamers/243.bmp", render);
	screamers[4][4] = load_t("screamers/396.bmp", render);
	screamers[4][5] = load_t("screamers/397.bmp", render);
	screamers[4][6] = load_t("screamers/398.bmp", render);
	screamers[4][7] = load_t("screamers/399.bmp", render);
	screamers[4][8] = load_t("screamers/400.bmp", render);
	screamers[4][9] = load_t("screamers/401.bmp", render);
	screamers[4][10] = load_t("screamers/402.bmp", render);
	screamers[4][11] = load_t("screamers/403.bmp", render);
	screamers[4][12] = load_t("screamers/404.bmp", render);
	screamers[4][13] = load_t("screamers/405.bmp", render);
	screamers[4][14] = load_t("screamers/406.bmp", render);
	screamers[4][15] = load_t("screamers/407.bmp", render);
	screamers[4][16] = load_t("screamers/408.bmp", render);
	screamers[4][17] = load_t("screamers/409.bmp", render);
	screamers[4][18] = load_t("screamers/410.bmp", render);
	screamers[4][19] = load_t("screamers/411.bmp", render);
	screamers[4][20] = load_t("screamers/412.bmp", render);
	screamers[4][21] = load_t("img/10.bmp", render);
	
	/* Freddy (power down) */
	screamers[5][0] = load_t("screamers/326.bmp", render);
	screamers[5][1] = load_t("screamers/348.bmp", render);
	screamers[5][2] = load_t("screamers/307.bmp", render);
	screamers[5][3] = load_t("screamers/308.bmp", render);
	screamers[5][4] = load_t("screamers/309.bmp", render);
	screamers[5][5] = load_t("screamers/310.bmp", render);
	screamers[5][6] = load_t("screamers/311.bmp", render);
	screamers[5][7] = load_t("screamers/312.bmp", render);
	screamers[5][8] = load_t("screamers/313.bmp", render);
	screamers[5][9] = load_t("screamers/314.bmp", render);
	screamers[5][10] = load_t("screamers/315.bmp", render);
	screamers[5][11] = load_t("screamers/316.bmp", render);
	screamers[5][12] = load_t("screamers/317.bmp", render);
	screamers[5][13] = load_t("screamers/318.bmp", render);
	screamers[5][14] = load_t("screamers/319.bmp", render);
	screamers[5][15] = load_t("screamers/320.bmp", render);
	screamers[5][16] = load_t("screamers/321.bmp", render);
	screamers[5][17] = load_t("screamers/322.bmp", render);
	screamers[5][18] = load_t("screamers/323.bmp", render);
	screamers[5][19] = load_t("screamers/324.bmp", render);
	screamers[5][20] = load_t("screamers/325.bmp", render);
	screamers[5][21] = load_t("img/10.bmp", render);
	
	
	
	/* Main menu states*/
	mmenu[0] = load_t("img/431.bmp", render);
	mmenu[1] = load_t("img/448.bmp", render);
	mmenu[2] = load_t("img/helpbut.bmp", render);
	mmenu[3] = load_t("img/helpscr.bmp", render);
	
	
	
	/* Foxy Running Animation */
	runfox[0] = load_t("rooms/340.bmp", render);
	runfox[1] = load_t("rooms/241.bmp", render);
	runfox[2] = load_t("rooms/244.bmp", render);
	runfox[3] = load_t("rooms/245.bmp", render);
	runfox[4] = load_t("rooms/246.bmp", render);
	runfox[5] = load_t("rooms/247.bmp", render);
	runfox[6] = load_t("rooms/248.bmp", render);
	runfox[7] = load_t("rooms/250.bmp", render);
	runfox[8] = load_t("rooms/280.bmp", render);
	runfox[9] = load_t("rooms/282.bmp", render);
	runfox[10] = load_t("rooms/283.bmp", render);
	runfox[11] = load_t("rooms/284.bmp", render);
	runfox[12] = load_t("rooms/285.bmp", render);
	runfox[13] = load_t("rooms/286.bmp", render);
	runfox[14] = load_t("rooms/287.bmp", render);
	runfox[15] = load_t("rooms/288.bmp", render);
	runfox[16] = load_t("rooms/289.bmp", render);
	runfox[17] = load_t("rooms/290.bmp", render);
	runfox[18] = load_t("rooms/292.bmp", render);
	runfox[19] = load_t("rooms/302.bmp", render);
	runfox[20] = load_t("rooms/306.bmp", render);
	runfox[21] = load_t("rooms/327.bmp", render);
	runfox[22] = load_t("rooms/329.bmp", render);
	runfox[23] = load_t("rooms/330.bmp", render);
	runfox[24] = load_t("rooms/331.bmp", render);
	runfox[25] = load_t("rooms/332.bmp", render);
	runfox[26] = load_t("rooms/333.bmp", render);
	runfox[27] = load_t("rooms/334.bmp", render);
	runfox[28] = load_t("rooms/335.bmp", render);
	runfox[29] = load_t("rooms/336.bmp", render);
	runfox[30] = load_t("rooms/337.bmp", render);
	
	
	
	
	/* Rooms on Cameras */
	
	
	/* Show Stage */
	room[0][0] = load_t("rooms/484.bmp" , render);
	room[0][1] = NULL;
	room[0][2] = load_t("rooms/224.bmp" , render);
	room[0][3] = load_t("rooms/68.bmp", render);
	room[0][4] = NULL;
	room[0][5] = NULL;
	room[0][6] = load_t("rooms/223.bmp" , render);
	room[0][7] = load_t("rooms/19.bmp" , render); /* * */
	
	/* Dining Area */
	room[1][0] = load_t("rooms/48.bmp", render);
	room[1][1] = load_t("rooms/215.bmp", render);
	room[1][2] = load_t("rooms/492.bmp", render);
	room[1][3] = load_t("rooms/215.bmp", render);
	room[1][4] = load_t("rooms/90.bmp", render);
	room[1][5] = NULL;
	room[1][6] = load_t("rooms/90.bmp", render);
	room[1][7] = NULL;
	
	/* Backstage*/
	room[2][0] = load_t("rooms/83.bmp", render);
	room[2][1] = room[2][2] = room[2][3] = room[2][5] = room[2][6] = room[2][7] = NULL;
	room[2][4] = load_t("rooms/205.bmp", render);
	
	/* Pirate Cove */
	room[3][0] = load_t("rooms/66.bmp", render);
	room[3][1] = load_t("rooms/211.bmp", render);
	room[3][2] = load_t("rooms/338.bmp", render);
	room[3][3] = load_t("rooms/240.bmp", render);
	room[3][4] = room[3][5] = room[3][6] = room[3][7] = NULL;
	
	/* Restrooms */
	room[4][0] = load_t("rooms/41.bmp", render);
	room[4][1] = load_t("rooms/217.bmp", render);
	room[4][2] = load_t("rooms/494.bmp", render);
	room[4][3] = room[4][4] = room[4][5] = room[4][6] = room[4][7] = NULL;
	
	/* Supply Closet */
	room[5][0] = load_t("rooms/62.bmp", render);
	room[5][4] = load_t("rooms/190.bmp", render);
	room[5][1] = room[5][2] = room[5][3] = room[5][5] = room[5][6] = room[5][7] = NULL;
	
	/* West Hall */
	room[6][0] = load_t("rooms/44.bmp", render);
	room[6][4] = load_t("rooms/206.bmp", render);
	room[6][1] = room[6][2] = room[6][3] = room[6][5] = room[6][6] = room[6][7] = NULL;
	
	/* West Hall Corner */
	room[7][0] = load_t("rooms/0.bmp", render);
	room[7][4] = load_t("rooms/188.bmp", render);
	room[7][1] = room[7][2] = room[7][3] = room[7][5] = room[7][6] = room[7][7] = NULL;
	
	/* East Hall*/
	room[8][0] = load_t("rooms/67.bmp", render);
	room[8][1] = load_t("rooms/221.bmp", render);
	room[8][2] = load_t("rooms/487.bmp", render);
	room[8][3] = room[8][4] = room[8][5] = room[8][6] = room[8][7] = NULL;
	
	/* East Hall Corner */
	room[9][0] = load_t("rooms/49.bmp", render);
	room[9][1] = load_t("rooms/220.bmp", render);
	room[9][2] = load_t("rooms/486.bmp", render);
	room[9][3] = room[9][4] = room[9][5] = room[9][6] = room[9][7] = NULL;
	
	
	
	/* Background */
	backg[0] = load_t("img/519.bmp", render);
	backg[1] = load_t("img/127.bmp", render);
	backg[2] = load_t("img/58.bmp", render);
	backg[3] = load_t("img/225.bmp", render);
	backg[4] = load_t("img/227.bmp", render);
	
	
	
	/* Background texture */
	SDL_Texture *back;
	back = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, getsize(backg[0]).x, getsize(backg[0]).y);
	
	
	/* Fan states */
	fan[0] = load_t("img/57.bmp", render);
	fan[1] = load_t("img/59.bmp", render);
	fan[2] = load_t("img/60.bmp", render);
	
	
	
	/* Text */
	
	/* Digits */
	text[0] = load_t("img/151.bmp", render);
	text[1] = load_t("img/152.bmp", render);
	text[2] = load_t("img/153.bmp", render);
	text[3] = load_t("img/154.bmp", render);
	text[4] = load_t("img/158.bmp", render);
	text[5] = load_t("img/159.bmp", render);
	text[6] = load_t("img/178.bmp", render);
	text[7] = load_t("img/179.bmp", render);
	text[8] = load_t("img/180.bmp", render);
	text[9] = load_t("img/181.bmp", render);
	text[10] = load_t("img/208.bmp", render); /* % */
	
	/* Special text */
	text[11] = load_t("img/189.bmp", render); /* Usage: */
	text[12] = load_t("img/207.bmp", render); /* Power Left: */
	text[13] = load_t("img/420.bmp", render); /* Tablet Open */
	text[14] = load_t("img/251.bmp", render); /* AM */


	
	/* Energy usage icon */
	usg[0] = load_t("img/212.bmp", render);
	usg[1] = load_t("img/213.bmp", render);
	usg[2] = load_t("img/214.bmp", render);
	usg[3] = load_t("img/456.bmp", render);
	usg[4] = load_t("img/455.bmp", render);
	


	/* Tablet animation */
	tablet[0] = load_t("img/142.bmp", render);
	tablet[1] = load_t("img/144.bmp", render);
	tablet[2] = load_t("img/132.bmp", render);
	tablet[3] = load_t("img/133.bmp", render);
	tablet[4] = load_t("img/136.bmp", render);
	tablet[5] = load_t("img/137.bmp", render);
	tablet[6] = load_t("img/138.bmp", render);
	tablet[7] = load_t("img/139.bmp", render);
	tablet[8] = load_t("img/140.bmp", render);
	tablet[9] = load_t("img/141.bmp", render);
	
	
	
	/* White noise animation */
	change[0] = load_t("img/12.bmp", render);
	change[1] = load_t("img/13.bmp", render);
	change[2] = load_t("img/14.bmp", render);
	change[3] = load_t("img/15.bmp", render);
	change[4] = load_t("img/16.bmp", render);
	change[5] = load_t("img/17.bmp", render);
	change[6] = load_t("img/18.bmp", render);
	change[7] = load_t("img/20.bmp", render);
	
	
	
	/* Camera Icons */
	cicon[0] = load_t("img/170.bmp", render); /* CAM 1A */
	cicon[1] = load_t("img/171.bmp", render); /* CAM 1B */
	cicon[2] = load_t("img/174.bmp", render); /* CAM 5 */
	cicon[3] = load_t("img/177.bmp", render); /* CAM 1C */
	cicon[4] = load_t("img/176.bmp", render); /* CAM 7 */
	cicon[5] = load_t("img/168.bmp", render); /* CAM 3 */
	cicon[6] = load_t("img/172.bmp", render); /* CAM 2A */
	cicon[7] = load_t("img/165.bmp", render); /* CAM 2B */
	cicon[8] = load_t("img/169.bmp", render); /* CAM 4A */
	cicon[9] = load_t("img/173.bmp", render); /* CAM 4B */
	cicon[10] = load_t("img/54.bmp", render); /* SHOW STAGE */
	cicon[11] = load_t("img/72.bmp", render); /* DINING AREA*/
	cicon[12] = load_t("img/71.bmp", render); /* SUPPLY CLOSET */
	cicon[13] = load_t("img/70.bmp", render); /* PIRATE COVE */
	cicon[14] = load_t("img/77.bmp", render); /* RESTROOMS */
	cicon[15] = load_t("img/50.bmp", render); /* BACKSTAGE */
	cicon[16] = load_t("img/74.bmp", render); /* West Hall */
	cicon[17] = load_t("img/76.bmp", render); /* West Hall Corner*/
	cicon[18] = load_t("img/79.bmp", render); /* East Hall */
	cicon[19] = load_t("img/75.bmp", render); /* East Hall Corner */
	cicon[20] = load_t("img/166.bmp", render); /* Active Bruh */
	cicon[21] = load_t("img/167.bmp", render); /* Usual Bruh */
	cicon[22] = load_t("img/145.bmp", render); /* Background */
	
	
	
	/* Door animations */
	door_r[0] = load_t("img/104.bmp", render);
	door_r[1] = load_t("img/121.bmp", render);
	door_r[2] = load_t("img/106.bmp", render);
	door_r[3] = load_t("img/107.bmp", render);
	door_r[4] = load_t("img/108.bmp", render);
	door_r[5] = load_t("img/109.bmp", render);
	door_r[6] = load_t("img/110.bmp", render);
	door_r[7] = load_t("img/111.bmp", render);
	door_r[8] = load_t("img/112.bmp", render);
	door_r[9] = load_t("img/113.bmp", render);
	door_r[10] = load_t("img/114.bmp", render);
	door_r[11] = load_t("img/115.bmp", render);
	door_r[12] = load_t("img/116.bmp", render);
	door_r[13] = load_t("img/117.bmp", render);
	door_r[14] = load_t("img/118.bmp", render);
	
	door_l[0] = load_t("img/88.bmp", render);
	door_l[1] = load_t("img/105.bmp", render);
	door_l[2] = load_t("img/89.bmp", render);
	door_l[3] = load_t("img/91.bmp", render);
	door_l[4] = load_t("img/92.bmp", render);
	door_l[5] = load_t("img/93.bmp", render);
	door_l[6] = load_t("img/94.bmp", render);
	door_l[7] = load_t("img/95.bmp", render);
	door_l[8] = load_t("img/96.bmp", render);
	door_l[9] = load_t("img/97.bmp", render);
	door_l[10] = load_t("img/98.bmp", render);
	door_l[11] = load_t("img/99.bmp", render);
	door_l[12] = load_t("img/100.bmp", render);
	door_l[13] = load_t("img/101.bmp", render);
	door_l[14] = load_t("img/102.bmp", render);
	
	
	
	/* Introductory screen*/
	intr = load_t("img/574.bmp", render);
	
	
	
	/* Buttons */
	b_left[0] = load_t("img/122.bmp", render);
	b_left[1] = load_t("img/124.bmp", render);
	b_left[2] = load_t("img/125.bmp", render);
	b_left[3] = load_t("img/130.bmp", render);
	
	b_right[0] = load_t("img/134.bmp", render);
	b_right[1] = load_t("img/135.bmp", render);
	b_right[2] = load_t("img/131.bmp", render);
	b_right[3] = load_t("img/47.bmp", render);
	
	
	
	/* Other screens used in the game */
	miscscreens[0] = load_t("img/304.bmp", render);
	miscscreens[1] = load_t("img/305.bmp", render);
	miscscreens[2] = load_t("img/210.bmp", render);
	
	
	
	/* _HITBOXES_ */
	
	
	
	/* Init */
	SDL_Rect fanr, fanl, brr, blr, bdl, bdr;
	SDL_Rect newgame, helpbut, backbut, quitbut;
	SDL_Rect lbr_d, lbr_l, rbr_d, rbr_l;
	SDL_Rect CAMDEF, CAMNAM, camr[10];
	
	
	/* Main Menu*/
	
	
	/* Quit Button */
	quitbut.x = 0; quitbut.w = 160;
	quitbut.y = 650; quitbut.h = 70;
	
	/* New Game Button*/
	newgame.w = getsize(mmenu[1]).x;
	newgame.h = getsize(mmenu[1]).y;
	newgame.x = 100;
	newgame.y = 400;
	
	/* HELP button*/
	helpbut.w = 250;
	helpbut.h = 70;
	helpbut.x = 80;
	helpbut.y = 400+(newgame.h)+15;
	
	/* Back button (to return from help) */
	backbut.w = 200;
	backbut.h = 720;
	backbut.x = 0;
	backbut.y = 0;
	
	
	
	/* Text */
	
	/* Energy usage */
	usgr.w = getsize(usg[0]).x;
	usgr.h = getsize(usg[0]).y;
	usgr.x = 20;
	usgr.y = SCREEN_HEIGTH-(usgr.h+30);
	

	/* Text itself */
	txt.x = 20;
	txt.y = SCREEN_HEIGTH-(usgr.h+26);
	txt.h = 23;
	
	
	
	/* Doors */
	bdl.x = 70;
	bdl.y = 0;
	bdl.w = getsize(door_l[0]).x;
	bdl.h = getsize(door_l[0]).y;
	
	bdr.w = getsize(door_r[0]).x-5;
	bdr.h = getsize(door_r[0]).y;
	bdr.x = getsize(backg[0]).x-(bdr.w+86);
	bdr.y = 0;
	
	
	
	/* Fan */
	fanr.x = 780;
	fanr.y = 303;
	fanr.w = getsize(fan[1]).x;
	fanr.h = getsize(fan[0]).y;
	fanl.w = fanr.w;
	fanl.h = fanr.h;
	fanl.x = fanl.y = 0;
	
	
	
	/* Buttons */
	blr.x = 0;
	blr.y = 300;
	blr.w = getsize(b_left[0]).x;
	blr.h = getsize(b_left[0]).y;
	
	brr.w = getsize(b_right[0]).x;
	brr.h = getsize(b_right[0]).y;
	brr.x = getsize(backg[0]).x-brr.w;
	brr.y = 300;
	
	
	/* The screen */
	srcc.x = (getsize(backg[0]).x-SCREEN_WIDTH)/2;
	srcc.y = 0;
	srcc.w = SCREEN_WIDTH;
	srcc.h = SCREEN_HEIGTH;
	
	
	
	/* Tablet Opener */
	SDL_Rect topen;
	topen.w = 600;
	topen.h=60;
	topen.x=340;
	topen.y=650;
	int topenb = 0;
	
	
	
	/* Button press hitboxes */
	
	/* Left door */
	lbr_d.x = 30;
	lbr_d.y = 356;
	lbr_d.w = blr.w-50;
	lbr_d.h = lbr_d.w+5;
	
	/* Left light */
	lbr_l.x = 30;
	lbr_l.y = 435;
	lbr_l.w = blr.w-50;
	lbr_l.h = lbr_d.w+5;
	
	/* Right door */
	rbr_d.x = getsize(backg[0]).x-73;
	rbr_d.y = 356;
	rbr_d.w = blr.w-50;
	rbr_d.h = lbr_d.w+5;
	
	/* Right light */
	rbr_l.x = getsize(backg[0]).x-73;
	rbr_l.y = 435;
	rbr_l.w = blr.w-50;
	rbr_l.h = lbr_d.w+5;
	
	
	
	/* Camera-related */
	
	/* The map */
	CAMDEF.w = getsize(cicon[22]).x;
	CAMDEF.h = getsize(cicon[22]).y;
	CAMDEF.x = 840;
	CAMDEF.y = 300;
	
	/* Quick set for size */
	for(i = 0; i < 10; i++){
		camr[i].w = getsize(cicon[21]).x;
		camr[i].h = getsize(cicon[21]).y;
	}
	
	camr[0].x = 960;
	camr[0].y = 320;
	
	camr[1].x = 920;
	camr[1].y = 370;
	
	camr[2].x = 820;
	camr[2].y = 400;
	
	camr[3].x = 895;
	camr[3].y = 455;
	
	camr[4].x = 1160;
	camr[4].y = 400;
	
	camr[5].x = 865;
	camr[5].y = 555;
	
	camr[6].x = 950;
	camr[6].y = 570;
	
	camr[7].x = 950;
	camr[7].y = 615;
	
	camr[8].x = 1030;
	camr[8].y = 570;
	
	camr[9].x = 1030;
	camr[9].y = 615;
	
	
	/* Name of the current camera area */
	CAMNAM.y = 670;
	CAMNAM.h = getsize(cicon[10]).y;
	
	

	/* _LOOP_ */



	while(isRunning){
	
	
		/* Timer management */
		prevtime = curtime;
		curtime = SDL_GetTicks();
		delta = (curtime-prevtime)/1000.0;
		
		
		/* A little fix so that opener works properly */
		topen.h = 70;
		
		
		/* Event hanler */
		while(SDL_PollEvent(&event) != 0){
			switch(event.type){
			
				/* Quit */
				case SDL_QUIT:
					isRunning = 0;
					break;
				
				
				/* Clicks processor */
				case SDL_MOUSEBUTTONDOWN:
					/* Just left clicks, right ones aren't welcome*/
					if(event.button.button == SDL_BUTTON_LEFT) {
						
						
						/* Main Game */
						if(introd <= 0){
							
							/* Doors and lights */
							/* Left */
							if(checkClick(event.button.x, event.button.y, lbr_d, 1) && (energy > 0 && !isCamera) && (bonnie_pos != 10 || bonnie_move > 3)) 
								l_door = 1-l_door;
							if(checkClick(event.button.x, event.button.y, lbr_l, 1) && (energy > 0 && !isCamera && !r_light)) l_light = 2;
							/* Right*/
							if(checkClick(event.button.x, event.button.y, rbr_d, 1) && (energy > 0 && !isCamera) && (chika_pos != 11 || chika_move > 3))
								r_door = 1-r_door;
							if(checkClick(event.button.x, event.button.y, rbr_l, 1) && (energy > 0 && !isCamera && !l_light)) r_light = 2;
							
							/* Camera controls */
							if(isCamera && canim*60 > 7) for(i = 0; i < 10; i++) if(checkClick(event.button.x,event.button.y, camr[i], 0)) {
								chanim = -0.01;
								currentCamera = i;
							}
						}
						
						
						/* Menu */
						switch(menu) {
							case 1:
								/* The three main menu buttons*/
								if (checkClick(event.button.x, event.button.y, newgame, 0)) menu = 0; /* New Game */
								if (checkClick(event.button.x, event.button.y, helpbut, 0)) menu = 2; /* HELP */
								if (checkClick(event.button.x, event.button.y, quitbut, 0)) isRunning = 0; /* Quit */
								break;
							case 2:
								/* Return to main menu */
								if (checkClick(event.button.x, event.button.y, backbut, 0)) menu = 1;
								break;
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					
					/* So that lights need to be held, not turned like doors */
					if(event.button.button == SDL_BUTTON_LEFT){
						l_light = 0;
						r_light = 0;
					}
					break;
					
				/* Mouse motions, for tablet opener */
				case SDL_MOUSEMOTION:
					if(menu || introd > 0) break; /* No tablet in the main menu, huh? */
					if(checkClick(event.motion.x, event.motion.y, topen, 0)){
						if(!topenb) {
							isCamera = 1-isCamera;
							topenb = 1;
						}
					} else topenb = 0; /* Topen b, so that it doesn't turn off and on 1000 times a second */
					break;
			}
		}
		topen.h = 60; /* So that tablet opener sprite isn't too tall */
		
		
		/* Non-game handlers */
		
		/* Introduction timer*/
		if(!menu) introd -= delta;
		
		/* Just to be sure */
		if(!isRunning) break;
		
		/* Menu*/
		if(menu) {
			/* Own renderer */
			SDL_SetRenderTarget(render, NULL);
			SDL_RenderClear(render);
			
			/* Main Menu */
			if(menu == 1){
				SDL_RenderCopy(render, mmenu[0], NULL, NULL);
				SDL_RenderCopy(render, mmenu[1], NULL, &newgame);
				SDL_RenderCopy(render, mmenu[2], NULL, &helpbut);
			} else { /* HELP page */
				SDL_RenderCopy(render, mmenu[3], NULL, NULL);
			}
			
			/* Render and skip the rest of the handlers */
			SDL_RenderPresent(render);
			continue;
		}
		
		/* Introduction handler */
		else if(introd >= 0){
			/* Own renderer */
			SDL_SetRenderTarget(render, NULL);
			SDL_RenderClear(render);
			
			/* Introductory page, and white noise animation at the end */
			if(introd >= (8/60.0)) SDL_RenderCopy(render, intr, NULL, NULL);
			else {SDL_RenderCopy(render, change[(int) (7-(introd*60.0))], NULL, NULL);}
			
			
			/* Render and skip the rest of the handlers */
			SDL_RenderPresent(render);
			continue;
		} else introd = -0.01; /* So that it doesn't mess up anything, just to be safe */
		
		
		/* Camera movement */
		int mx, my;
		SDL_GetMouseState(&mx, &my);
		if(mx < 300 && srcc.x >= msens*delta) srcc.x -= msens*delta;
		if(mx > 980 && (1600-(srcc.x+SCREEN_WIDTH) >= msens*delta)) srcc.x += msens*delta;
		
		/* Screamer handler */
		if(screamer){
			if(scranim*30 < scree[screamer]+120 && canim <= 0) {
				/* Own renderer */
				SDL_SetRenderTarget(render, NULL);
				SDL_RenderClear(render);
				
				
				/* To ensure the screamer repeats the needed amount of times */
				if(scranim*30 >= (scree[screamer]-1) && --screp[screamer] > 0) scranim = 0;
				
				
				/* Screamer animation itself */
				SDL_RenderCopy(render, screamers[screamer][(int) (scranim*30 >= scree[screamer] ? scree[screamer]-1 : scranim*30)], screamer == 5 ? NULL : &srcc, NULL);
				
				
				/* GAME OVER screen */
				if(scranim*30 >= scree[screamer]){	
					/* "Hitbox" as well as the texture */
					SDL_Rect gameover;
					SDL_Texture *gover;
					gover = load_t("img/471.bmp", render);
					gameover.w = getsize(gover).x;
					gameover.h = getsize(gover).y;
					gameover.x = (SCREEN_WIDTH-gameover.w)/2;
					gameover.y = (SCREEN_HEIGTH-gameover.h)/2;
					
					/* Render */
					SDL_RenderCopy(render, gover, NULL, &gameover);
					
					/* Texture utilization */
					SDL_DestroyTexture(gover);
					gover = NULL;
				}
				
				/* Animation timer */
				scranim += delta;
				
				/* Render and skip the rest of the handlers */
				SDL_RenderPresent(render);
				continue;
			
			
			/* Once the duration is over, reset the game */
			} else if(scranim*30 >= scree[screamer]+120) {resetGame(); continue;}
		}
		
		
		
		/* Victory handler */
		if(timam >= 6){
			/* Own renderer */
			SDL_SetRenderTarget(render, NULL);
			SDL_RenderClear(render);
			
			
			/* Victory screen */
			if(wanim >= 13) SDL_RenderCopy(render, miscscreens[2], NULL, NULL);
			
			
			/* Victory animation */
			else {
				/* Timer */
				wanim += delta;
				
				/* "Hitboxes" of animated numbers, and the masks */
				SDL_Rect five, six, recta1, recta2;
				
				/* Hitbox for the "AM" part */
				txt.w = getsize(text[5]).x*2;
				txt.h = getsize(text[5]).y*2;
				txt.x = (SCREEN_WIDTH-(txt.w+(getsize(text[14]).x*(txt.h/(double)getsize(text[14]).y))+5))/2;
				txt.y = (SCREEN_HEIGTH-(txt.h))/2;
				
				/* Black masks */
				recta1.w = recta2.w = SCREEN_WIDTH; recta1.h = recta2.h = (SCREEN_HEIGTH-(txt.h))/2;
				recta1.x = recta2.x = 0; recta1.y = 0; recta2.y = recta2.h+txt.h;
				
				
				/* Numbers' animation */
				five.w = six.w = txt.w; five.h = six.h = txt.h; five.x = six.x = txt.x; five.y = six.y = txt.y;
				int modu = (wanim*txt.h)/(8.0) >= txt.h ? txt.h : ((wanim*txt.h)/(8.0));
				six.y -= (txt.h-modu);
				five.y += modu;
				
				
				/* AM reset */
				txt.x += txt.w + 5;
				txt.w = getsize(text[14]).x*(txt.h/(double)getsize(text[14]).y);
				
			
				/* Put the elements for render */
				SDL_RenderCopy(render, text[5], NULL, &five);
				SDL_RenderCopy(render, text[6], NULL, &six);
				SDL_RenderCopy(render, text[14], NULL, &txt);
				SDL_RenderFillRect(render, &recta1);
				SDL_RenderFillRect(render, &recta2);
				
				
				/* Render and skip the rest of the handlers */
				SDL_RenderPresent(render);
				continue;
			}
			
			/* When the duration is over, reset the game */
			if(wanim >= (23)) {
				resetGame(); continue;
			}
			
			/* Timer */
			wanim += delta;
			
			/* Render and skip the rest of the handlers */
			SDL_RenderPresent(render);
			continue;
		}
		
		
		/* Nothing works without power, right? */
		if(energy <= 0 || screamer) {
			r_door = l_door = r_light = l_light = 0;
			isCamera = 0;
		}
		/* Freddy powerdown animation */
		if(energy <= 0 && !screamer){
			/* After the duration is over, summon screamer */
			if(nopanim >= (20)) screamer = 5;
			
			/* Timers */
			nopanim += delta;
			
			/* These are so that there is a chance to still win, if the end is soon enough */
			timam = tim/(AMPERSEC); 
			tim += delta;
			
			
			/* Own renderer */
			SDL_SetRenderTarget(render, NULL);
			SDL_RenderClear(render);
			
			/* Flickering Freddy image in the darkness */
			SDL_RenderCopy(render, miscscreens[drand48() < 0.2], &srcc, NULL);
			
			
			/* Render and skip the rest of the handlers */
			SDL_RenderPresent(render);
			continue;
		};
		
		
		
		/* _PATTERNS_ */
		
		
		
		/* Animatronics behavior timers */
		foxy_timer -= delta;
		chika_move -= delta; chika_timer -= delta;
		bonnie_move -= delta; bonnie_timer -= delta;
		freddy_move -= delta; freddy_timer -= delta;
		
		
		/* To make sure Chika targets the player when the time comes */
		if(chika_timer <= 0) {
			chika_isRoaming = 0;
			chika_timer = 0;
		}
		/* 
			Chika movement pattern, which I am way too lazy to comment entirely. 
			
			No AI here, just randomizer an algorythms.
			Depending on whether or not Chika is targeting the player, the positions of other animatronics and RNG,
			the next move is decided, and then the timer till the next move is randomized.
		*/
		if(chika_move <= 0 && !screamer) {
			chika_move = 0;
			int prevpos = chika_pos;
			switch(chika_pos) {
				case 1:
					if(chika_isRoaming){
						if(drand48() < 0.5 && freddy_pos != 4) chika_pos = 4;
						else chika_pos = 13;
					} else {
						if(freddy_pos == 8 || freddy_pos == 9) break;
						chika_pos = 8;
					}
					chika_move += (5)+(drand48()*15);
					break;
				case 0:
					if(isCamera && currentCamera == 0 && bonnie_pos != 1) chanim = -0.01;
				case 13:
				case 4:
					if(bonnie_pos == 1) break;
					chika_pos = 1;
					chika_move += (5)+(drand48()*15);
					break;
				case 8:
					if(chika_isRoaming){
						if(bonnie_pos == 1) break;
						chika_pos = 1;
					} else chika_pos = 9;
					chika_move += (5)+(drand48()*15);
					break;
				case 9:
					if(chika_isRoaming){
						chika_pos = 8;
					} else {
						chika_pos = 11;
						chika_move += (5);
					}
					chika_move += (5)+(drand48()*15);
					break;
					
					
				/* Near the door behavior */
				case 11:
					if(r_door) {
						/* If the door is closed when she moves, resets her hostility for the time being, and flicks her away */
						chika_isRoaming = 1;
						chika_pos = 8;
						chika_move += (5)+(drand48()*15);
						chika_timer = chika_move+(30)+(drand48()*90);
					} else { /* Otherwise, d3ath */
						screamer = 1;
					}
					break;
			}
			
			/* So that player can't see the camera sprites change */
			if(chika_move && isCamera && (currentCamera == chika_pos || currentCamera == prevpos)) chanim = -0.01;	
		}
		
		
		/* A similar handler for Bonnie */
		if(bonnie_timer <= 0) {
			bonnie_isRoaming = 0;
			bonnie_timer = 0;
		}
		if(bonnie_move <= 0 && !screamer) {
			bonnie_move = 0;
			int prevpos = bonnie_pos;
			switch(bonnie_pos) {
				case 6:
					if(bonnie_isRoaming){
						if(drand48() < 0.5) {
							bonnie_pos = 5;
							bonnie_move += (5)+(drand48()*15);
							break;
						}
					} else {
						bonnie_pos = 7;
						bonnie_move += (5)+(drand48()*15);
						break;
					}
				case 0:
					if(isCamera && currentCamera == 0 && chika_pos != 1){
						chanim = -0.01;
					}
				case 2:
					if(chika_pos == 1) break;
					bonnie_pos = 1;
					bonnie_move += (5)+(drand48()*15);
					break;
				case 1:
					double chance = drand48();
					if(bonnie_isRoaming && (chance < 0.4)) {
						bonnie_pos = 2;
						bonnie_move += (5)+(drand48()*15);
						break;
					} else if(bonnie_isRoaming && freddy_pos == 0 && chance < 0.7) {
						bonnie_pos = 0;
						bonnie_move += (5)+(drand48()*15);
						break;
					}
				case 5:
					if(foxy_pos == 3) break;
					bonnie_pos = 6;
					bonnie_move += (5)+(drand48()*15);
					break;
				case 7:
					if(bonnie_isRoaming){
						bonnie_pos = 6;
					} else {
						bonnie_pos = 10;
						bonnie_move += (5);
					}
					bonnie_move += (5)+(drand48()*15);
					break;
				case 10:
					if(l_door) {
						bonnie_isRoaming = 1;
						bonnie_pos = 6;
						bonnie_move += (5)+(drand48()*15);
						bonnie_timer = bonnie_move+(30)+(drand48()*90);
					} else {
						screamer = 2;
					}
					break;
			}	
			if(bonnie_move && isCamera && (currentCamera == bonnie_pos || currentCamera == prevpos)) chanim = -0.01;
		}
		
		
		/* A similar handler for Freddy */
		if(freddy_timer <= 0){
			freddy_isRoaming = 0;
			freddy_timer = 0;
		}
		if(freddy_move <= 0 && !screamer) {
			freddy_move = 0;
			int prevpos = freddy_pos;
			switch(freddy_pos){
				case 0:
					if(chika_pos == 0 || bonnie_pos == 0) break;
				case 4:
				case 13:
					freddy_pos = 1;
					freddy_move += (8)+(drand48()*12);
					break;
				case 1:
					if(freddy_isRoaming){
						double chance = drand48();
						if(chance < 0.4 && chika_pos != 4) freddy_pos = 4;
						else if(chance < 0.8) freddy_pos = 13;
						else freddy_pos = 0;
					} else {
						if(chika_pos == 8 || chika_pos == 9 || chika_pos == 11) break;
						freddy_pos = 8;
					}
					freddy_move += (8)+(drand48()*12);
					break;
				case 8:
					if(freddy_isRoaming) freddy_pos = 1;
					else {
						freddy_pos = 9;
						freddy_move += (2);
					}
					freddy_move += (8)+(drand48()*12);
					break;
				case 9:
					if(r_door) freddy_pos = 8;
					else screamer = 3;
					freddy_move += (8)+(drand48()*12);
					freddy_isRoaming = 1;
					freddy_timer = freddy_move+(60)+(drand48()*90);
					break;
			}
			if(freddy_move && isCamera && (currentCamera == freddy_pos || currentCamera == prevpos)) chanim = -0.01;
		}
		
		
		/* Handler for Foxy is, obviously, different from the rest */
		if(foxy_timer <= 0 && !screamer){
			/* Just the progression of stages, with times randomized */
			foxy_timer = (15)+(drand48()*75);
			foxy_pos++;
			
			/* If the player is watching, make sure he doesn't see the sprite changes */
			if(isCamera && currentCamera == 3) chanim = -0.01;
		}
		/* This is where it's fun. Running. */
		if(foxy_pos >= 3 && !screamer){
			/* To make sure the player doesn't see sprites changing*/
			if(fxanim*30 <= 3 && isCamera && currentCamera == 6) chanim = -0.01;
			
			/* Timer, whose speed depends on whether or not have you checked on foxy */
			foxy_run += foxy_fast ? 3*delta : delta;
			if(isCamera) foxy_fast = (foxy_fast || currentCamera == 3 || currentCamera == 6); /* Here is the check if you've checked on Foxy after it started running */
			
			/* When it reaches the player */
			if(foxy_run >= (15)){
				if(l_door){ /* If the door is shut, position is reset */
					if(isCamera && (currentCamera == 3 || currentCamera == 6)) chanim = -0.01; /* Sprite change masking */
					foxy_pos = 0; foxy_run = -0.01; foxy_fast = 0;
					foxy_timer = (30)+(drand48()*60);
					fxanim = -0.01;
				} else screamer = 4; /* D3ath */
			}
		}
		/* Timer reset, just in case */
		if(fxanim > 1) fxanim = 1;
		
		
		
		/* Fan timer handler */
		if(energy > 0 && frame >= 1/(double)fanfreq){
			fs++;
			if(fs >= 3) fs = 0;
		}
		if(frame > 1/(double)fanfreq) frame = 0;
		frame += delta;
		
		
		
		/* Energy and its usage handler */
		usage = (1+(r_door)+(l_door)+(r_light/2)+(l_light/2)+(isCamera));
		if(energy > 0) energy -= usage*0.06*delta;
		else energy = 0;
		
		
		
		/* Doors animation handler */
		if(r_door && rdanim*60 < 14) rdanim += delta;
		else if(!r_door && rdanim > 0) rdanim -= delta;
		
		if(l_door && ldanim*60 < 14) ldanim += delta;
		else if(!l_door && ldanim > 0) ldanim -= delta;
		
		
		/* Tablet up/down an white noise animation */
		if(isCamera && canim < (9/60.0)) canim += delta;
		else if(!isCamera && canim >= 0) canim -= delta;
		if(isCamera && (int) (canim*60) <= 8 && (int) (canim*60) >= 6) chanim = -0.01;
		
		
		/* Safety measures, since sometimes frames can skip over usual FPS */
		if(canim > (9/60.0)) canim = (9/60.0);
		if(chanim > (8/60.0)) chanim = (8/60.0);
		if(chanim < 0) chanim = 0;
		if(rdanim < 0) rdanim = 0;
		if(ldanim < 0) ldanim = 0;
		if(rdanim*60 > 14) rdanim = 14/60.0;
		if(ldanim*60 > 14) ldanim = 14/60.0;
		
		
		
		/* _GAME_RENDER_ */
		
		
		
		/* Construct the background */
		SDL_RenderClear(render);
		
		
		SDL_SetRenderTarget(render, back);
		
		/* Office itself */
		SDL_RenderCopy(render, backg[bkg()], NULL, NULL);
		
		/* Fan and its animation */
		SDL_RenderCopy(render, fan[fs], &fanl, &fanr);
		
		/* Buttons and their states */
		SDL_RenderCopy(render, b_right[r_door+r_light], NULL, &brr);
		SDL_RenderCopy(render, b_left[l_door+l_light], NULL, &blr);
		
		/* Doors and their animations */
		SDL_RenderCopy(render, door_l[(int) (ldanim*60)], NULL, &bdl);
		SDL_RenderCopy(render, door_r[(int) (rdanim*60)], NULL, &bdr);
		
		
		
		/* Final render */
		
		
		
		/* Text hitbox reset */
		txt.x = 20;
		txt.y = SCREEN_HEIGTH-(usgr.h+26);
		txt.w = getsize(text[11]).x*(getsize(text[11]).y/(double)txt.h)*1.6;
		
		
		/* Render the previously constructed office background */
		SDL_SetRenderTarget(render, NULL);
		SDL_RenderCopy(render, back, &srcc, NULL);
		
		
		/* Render tablet */ 
		if(canim >= 0) SDL_RenderCopy(render, tablet[(int) (canim*60)], NULL, NULL);
		if(isCamera && canim*60 > 7) { /* Render tablet HUD */
		
			/* Render the room itself */
			SDL_RenderCopy(render, room[currentCamera][cimg(currentCamera)], &srcc, NULL);
			
			/* Foxy running handler, since its animation is separate from the room */
			if((foxy_pos >= 3 || fxanim > 0) && currentCamera == 6){
				if(foxy_pos >= 3 && fxanim < 0 && currentCamera == 6) fxanim = 0; /* Backup timer */
				SDL_RenderCopy(render, runfox[(int) (fxanim*30 > 30 ? 30 : fxanim*30)], &srcc, NULL); /* Animation itself */
			}
			
			/* Render the map */
			SDL_RenderCopy(render, cicon[22], NULL, &CAMDEF);
			
			/* Adjust and render the current area's name */
			CAMNAM.w = getsize(cicon[currentCamera+10]).x;
			CAMNAM.x = SCREEN_WIDTH-(CAMNAM.w+60);
			SDL_RenderCopy(render, cicon[currentCamera+10], NULL, &CAMNAM);
			
			
			/* Render the icons on the map */
			for(i = 0; i < 10; i++){
				if(currentCamera == i) SDL_RenderCopy(render, cicon[20], NULL, &camr[i]);
				else SDL_RenderCopy(render, cicon[i], NULL, &camr[i]);
			}
		}
		
		
		/* Timer */
		if(fxanim >= 0) fxanim += delta;
		
		/* White noise animaton handler (again) */
		if(isCamera) chanim += delta;
		if(isCamera && (chanim*60) < 8) {SDL_RenderCopy(render, change[(int) (chanim*60)], NULL, NULL);}
		
		
		/* Render Energy Usage & Power Left */
		
		
		/* Energy Usage Sign */
		SDL_RenderCopy(render, text[11], NULL, &txt);
		
		/* Power Left Sight */
		txt.x += txt.w;
		usgr.x = 30+txt.w;
		txt.y -= 28;
		txt.x = 20;
		txt.w = getsize(text[12]).x*(getsize(text[12]).y/(double)txt.h)*1.6;
		SDL_RenderCopy(render, text[12], NULL, &txt);
		
		/* The percentage itself */
		txt.x += txt.w + 5;
		txt.w = getsize(text[0]).x;
		int dec, num;
		dec = energy/10;
		num = ((int)energy)%10;
		SDL_RenderCopy(render, text[dec], NULL, &txt); /* Decimal place */
		txt.x += txt.w+1;
		SDL_RenderCopy(render, text[num], NULL, &txt); /* Numeral place */
		txt.x += txt.w+1;		
		SDL_RenderCopy(render, text[10], NULL, &txt); /* % sign */
		
		
		/* Clock */
		txt.w = getsize(text[0]).x;
		txt.x = SCREEN_WIDTH-(txt.w+getsize(text[14]).x+15);
		txt.y = 10;
		timam = tim/AMPERSEC;
		tim += delta;
		
		/* The number, depending on what time is it*/
		SDL_RenderCopy(render, text[timam], NULL, &txt);
		
		/* "AM" sign */
		txt.x += txt.w+5;
		txt.w = getsize(text[14]).x;
		SDL_RenderCopy(render, text[14], NULL, &txt);
		
		
		/* Render Tablet Opener */
		SDL_RenderCopy(render, text[13], NULL, &topen);

		/* Final Render */
		SDL_RenderCopy(render, usg[--usage], NULL, &usgr);
		SDL_RenderPresent(render);
	}
	
	
	
	/* _FINAL_ */
	
	
	
	/* Close the window */
	SDL_DestroyWindow(window);
	
	
	/* Utilize the textures, so that memory is nice an clean */
	for(i = 0; i < 3; i++) {
		SDL_DestroyTexture(backg[i]);
		backg[i] = NULL;
	}
	for(i = 0; i < 3; i++) {
		SDL_DestroyTexture(fan[i]);
		fan[i] = NULL;
	}
	for(i = 0; i < 4; i++) {
		SDL_DestroyTexture(mmenu[i]);
		mmenu[i] = NULL;
	}
	for(i = 0; i < 3; i++) {
		SDL_DestroyTexture(miscscreens[i]);
		miscscreens[i] = NULL;
	}
	for(i = 0; i < 4; i++) {
		SDL_DestroyTexture(b_left[i]);
		b_left[i] = NULL;
	}
	for(i = 0; i < 4; i++) {
		SDL_DestroyTexture(b_right[i]);
		b_right[i] = NULL;
	}
	for(i = 0; i < 15; i++) {
		SDL_DestroyTexture(door_r[i]);
		door_r[i] = NULL;
	}
	for(i = 0; i < 15; i++) {
		SDL_DestroyTexture(door_l[i]);
		door_l[i] = NULL;
	}
	for(i = 0; i < 10; i++) {
		SDL_DestroyTexture(tablet[i]);
		tablet[i] = NULL;
	}	
	for(i = 0; i < 8; i++) {
		SDL_DestroyTexture(change[i]);
		change[i] = NULL;
	}	
	for(i = 0; i < 23; i++) {
		SDL_DestroyTexture(cicon[i]);
		cicon[i] = NULL;
	}
	for(i = 0; i < 4; i++) {
		SDL_DestroyTexture(usg[i]);
		usg[i] = NULL;
	}
	for(i = 0; i < 13; i++) {
		SDL_DestroyTexture(text[i]);
		text[i] = NULL;
	}
	for(i = 0; i < 31; i++) {
		SDL_DestroyTexture(runfox[i]);
		runfox[i] = NULL;
	}
	for(i = 0; i < 10; i++) for(j = 0; j < 8; j++) if(room[i][j] != NULL) {
		SDL_DestroyTexture(room[i][j]);
		room[i][j] = NULL;
	};
	for(i = 0; i < 6; i++) if(scree[i] > 0) for(j = 0 ; j < scree[i]; j++){
		SDL_DestroyTexture(screamers[i][j]);
		screamers[i][j] = NULL;
	}
	SDL_DestroyTexture(intr);
	intr = NULL;
	SDL_DestroyTexture(back);
	back = NULL;
	
	
	/* Destroy the renderer and quit the program */
	SDL_DestroyRenderer(render);
	SDL_Quit();
	
	return 0;
}
