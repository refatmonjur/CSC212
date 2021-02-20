/* compute optimal solutions for sliding block puzzle. */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <cstdlib>   /* for atexit() */
#include <algorithm>
#include <string.h>
#include <iostream>
#include <cassert>

//added{
#define FULLSCREEN_FLAG SDL_WINDOW_FULLSCREEN_DESKTOP
#define CRC32_SEED 0xFFFFFFFF // crc32 start value
#define CRC32_POLY 0xEDB88320 // crc32 polynomial
#define CRC32_CALC(crc,byte) ( ( ( crc >> 8 ) & 0x00FFFFFF ) ^ ( crc32_table[ ( crc ^ (byte) ) & 0xFF ] ) )

#define MAX_DEPTH 500
#define IC 40
#define NBLOCKS 10
#define WI 4
#define HE 5
//}

using std::swap;
using namespace std;


/* SDL reference: https://wiki.libsdl.org/CategoryAPI */

/* initial size; will be set to screen size after window creation. */
int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
int ntg;
int ntt;
int fcount = 0;
int mousestate = 0;
SDL_Point lastm = {0,0}; /* last mouse coords */
SDL_Rect bframe; /* bounding rectangle of board */
static const int ep = 2; /* epsilon offset from grid lines */

bool init(); /* setup SDL */
void initBlocks();


//added
struct field_and_crc {
	unsigned char* field;
	unsigned int crc32;
	field_and_crc* next;
};

// #define FULLSCREEN_FLAG 0

/* NOTE: ssq == "small square", lsq == "large square" */
//enum bType {hor,ver,ssq,lsq};
struct block {
	SDL_Rect R; /* screen coords + dimensions */
	int type; /* shape + orientation */
	char id;
	int x,y,w,h,p,gx,gy,gp; //added 
	block* pos[4]; //added 
	unsigned char* form; //added 
	unsigned char* dform[4]; //added 
	/* TODO: you might want to add other useful information to
	 * this struct, like where it is attached on the board.
	 * (Alternatively, you could just compute this from R.x and R.y,
	 * but it might be convenient to store it directly.) */
	void rotate() /* rotate rectangular pieces */
	{
		if (type != 2 && type != 4) return;//hor 4, ver 2
		type = (type==2)?4:2;
		swap(R.w,R.h);
	}
};
//added
struct node {
	unsigned char* field; // type field scheme
	unsigned char* field2;
	block** tiles; // all tile positions
	node* mother; // mother node
	node* next; // next sibling node
};


block B[NBLOCKS];
block* dragged = NULL;

char solve[HE][WI] =
{
	{ '.', '.', '.', '.' },
	{ '.', '.', '.', '.' },
	{ '.', '.', '.', '.' },
	{ '.', 'k', 'k', '.' },
	{ '.', 'k', 'k', '.' }
};

block* findBlock(int x, int y);
void close(); /* call this at end of main loop to free SDL resources */
SDL_Window* gWindow = 0; /* main window */
SDL_Renderer* gRenderer = 0;

//added
void err_exit( const char* msg )
{
	fprintf( stderr, "\nerror: %s\n\n", msg );
	exit( -1 );
}

//added
unsigned int* build_crc32_table( void ) {
	static unsigned int* crc32_table = (unsigned int*) calloc( 256, sizeof( int ) );
	unsigned int crc;
	int i;
	
	
	// fill crc table
	for ( i = 0; i < 256; i++ ) {
		crc = i;
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc = ( crc & 1 ) ? ( ( crc >> 1 ) ^ CRC32_POLY ) : ( crc >> 1 );
		crc32_table[ i ] = crc;
	}
	return crc32_table;
}



//added
node* build_node( void )
{
	node* nnode;
	// alloc memory, check
	nnode = (node*) malloc( sizeof( node ) );
	if ( nnode == NULL ) err_exit( "out of memory" );
	nnode->field = (unsigned char*)malloc(sizeof( unsigned char)*HE*WI);//check
	if ( nnode->field == NULL ) err_exit( "out of memory" );
	nnode->tiles = (block**) malloc( sizeof( block* )*ntt );
	if ( nnode->tiles == NULL ) err_exit( "out of memory" );
	nnode->next = NULL;
	return nnode;
}



//added
void copy_node( node* cnode, node* mnode )
{
	if(mnode -> field != NULL){
		memcpy( cnode->field, mnode->field, HE*WI * sizeof( unsigned char ) );
	}
	else{
		return ;
	}
	if(mnode -> tiles != NULL)
		memcpy( cnode->tiles, mnode->tiles, NBLOCKS * sizeof( block ) );
	else
		return ;
	cnode->mother = mnode;
	return;
}

//added
bool check_move( unsigned char* field, block* tile, int d ) //check block*
{
	block* ntile = tile->pos[d];
	unsigned char* dform;
	int p0, p;
	
	// check if tile can be moved
	if ( ntile == NULL ){
		return false;
	}
	p0 = ntile->p;
	dform = ntile->dform[d];
	for ( p = 0; dform[p] != 0xFF; p++ ){
		if ( dform[p] == 1 ){
			if ( field[p+p0] != 0 ){
				return false;
			}
		}
	}	
	return true;
}

//added
block* do_move( unsigned char* field, block* tile, int d ) //check block*
{
	block* ntile = tile->pos[d];
	int tp = tile->type;
	unsigned char* dform;
	int p0, p;
	
	
	// remove old tile
	p0 = tile->p; dform = tile->dform[(d+2)%4];
	for ( p = 0; dform[p] != 0xFF; p++ ) if ( dform[p] == 1 )
		field[p+p0] = 0;
		
	// insert new tile
	p0 = ntile->p; dform = ntile->dform[d];
	for ( p = 0; dform[p] != 0xFF; p++ ) if ( dform[p] == 1 )
		field[p+p0] = tp;
		
	//cout << "do_move" << endl;
	return ntile;
}

//added
bool check_node( node* cnode )
{
	static unsigned int* crc32_table = build_crc32_table();
	// static field_and_crc** fields = (field_and_crc**) calloc( 1 << 16, sizeof( field_and_crc* ) );
	static field_and_crc* fields[1<<16] = { NULL };
	unsigned char* nfield = cnode->field;
	field_and_crc* cfield;
	unsigned int crc32;
	int idx;
	int i;
	
	
	// calculate crc32
	for ( i = 0, crc32 = CRC32_SEED; i < HE*WI; i++ )
		crc32 = CRC32_CALC( crc32, nfield[i] );
	// index = first 16 crc32 bits
	idx = crc32 & 0xFFFF;
	
	// compare known states with new
	for ( cfield = fields[idx]; cfield != NULL; cfield = cfield->next )
		if ( crc32 == cfield->crc32 ) if ( memcmp( nfield, cfield->field, HE*WI*sizeof(char) ) == 0 ) break;
		
	// previously unknown state found
	if ( cfield == NULL ) {
		cfield = (field_and_crc*) calloc( 1, sizeof( field_and_crc ) );
		if ( cfield == NULL ) err_exit( "out of memory" );
		cfield->field = nfield;
		cfield->crc32 = crc32;
		cfield->next = fields[idx];
		fields[idx] = cfield;
	} else {
		return false;
	}
	return true;
}

//added
bool check_goal_condition( node* cnode )
{
	int i;
	
	
	for ( i = 0; i < NBLOCKS; i++ )
		if ( cnode->tiles[i]->p != cnode->tiles[i]->gp ) break;
	if ( i == NBLOCKS ){
		return true;
	}
	return false;
}



bool init()
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init failed.  Error: %s\n", SDL_GetError());
		return false;
	}
	/* NOTE: take this out if you have issues, say in a virtualized
	 * environment: */
	if(!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) {
		printf("Warning: vsync hint didn't work.\n");
	}
	/* create main window */
	gWindow = SDL_CreateWindow("Sliding block puzzle solver",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								SCREEN_WIDTH, SCREEN_HEIGHT,
								SDL_WINDOW_SHOWN|FULLSCREEN_FLAG);
	if(!gWindow) {
		printf("Failed to create main window. SDL Error: %s\n", SDL_GetError());
		return false;
	}
	/* set width and height */
	SDL_GetWindowSize(gWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	/* setup renderer with frame-sync'd drawing: */
	gRenderer = SDL_CreateRenderer(gWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(!gRenderer) {
		printf("Failed to create renderer. SDL Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawBlendMode(gRenderer,SDL_BLENDMODE_BLEND);
	initBlocks();
	return true;
}

/* TODO: you'll probably want a function that takes a state / configuration
 * and arranges the blocks in accord.  This will be useful for stepping
 * through a solution.  Be careful to ensure your underlying representation
 * stays in sync with what's drawn on the screen... */

void initBlocks()
{
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT; // (SCREEN_HEIGHT*3/4)/5-2*ep
	int h = H*3/4;
	int w = 4*h/5;
	int u = h/5-2*ep;
	int mw = (W-w)/2;
	int mh = (H-h)/2;

	/* setup bounding rectangle of the board: */
	bframe.x = (W-w)/2;
	bframe.y = (H-h)/2;
	//cout << bframe.x << "/" << bframe.y << endl;
	bframe.w = w;
	bframe.h = h;

	/* NOTE: there is a tacit assumption that should probably be
	 * made explicit: blocks 0--4 are the rectangles, 5-8 are small
	 * squares, and 9 is the big square.  This is assumed by the
	 * drawBlocks function below. */

	for (size_t i = 0; i < 5; i++) {
		B[i].R.x = (mw-2*u)/2;
		B[i].R.y = mh + (i+1)*(u/5) + i*u;
		B[i].R.w = 2*(u+ep);
		B[i].R.h = u;
		B[i].type = 4;
	}
	B[4].R.x = mw+ep;
	B[4].R.y = mh+ep;
	B[4].R.w = 2*(u+ep);
	B[4].R.h = u;
	B[4].type = 4;
	/* small squares */
	for (size_t i = 0; i < 4; i++) {
		B[i+5].R.x = (W+w)/2 + (mw-2*u)/2 + (i%2)*(u+u/5);
		B[i+5].R.y = mh + ((i/2)+1)*(u/5) + (i/2)*u;
		B[i+5].R.w = u;
		B[i+5].R.h = u;
		B[i+5].type = 3;
	}
	B[9].R.x = B[5].R.x + u/10;
	B[9].R.y = B[7].R.y + u + 2*u/5;
	B[9].R.w = 2*(u+ep);
	B[9].R.h = 2*(u+ep);
	B[9].type = 1;
}

void drawBlocks()
{
	/* rectangles */
	SDL_SetRenderDrawColor(gRenderer, 0x43, 0x4c, 0x5e, 0xff);
	for (size_t i = 0; i < 5; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	/* small squares */
	SDL_SetRenderDrawColor(gRenderer, 0x5e, 0x81, 0xac, 0xff);
	for (size_t i = 5; i < 9; i++) {
		SDL_RenderFillRect(gRenderer,&B[i].R);
	}
	/* large square */
	SDL_SetRenderDrawColor(gRenderer, 0xa3, 0xbe, 0x8c, 0xff);
	SDL_RenderFillRect(gRenderer,&B[9].R);
	//cout << "drawBlocks" << endl;
}

// return a block containing (x,y), or NULL if none exists. 
block* findBlock(int x, int y)
{
	// NOTE: we go backwards to be compatible with z-order 
	for (int i = NBLOCKS-1; i >= 0; i--) {
		
		if (B[i].R.x <= x && x <= B[i].R.x + B[i].R.w && B[i].R.y <= y && y <= B[i].R.y + B[i].R.h){
			return (B+i);
		}
	}
	//cout << "findBlock" << endl;
	return NULL;
}

void close()
{
	SDL_DestroyRenderer(gRenderer); gRenderer = NULL;
	SDL_DestroyWindow(gWindow); gWindow = NULL;
	SDL_Quit();
}

void render()
{
	/* draw entire screen to be black: */
	SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderClear(gRenderer);

	/* first, draw the frame: */
	int& W = SCREEN_WIDTH;
	int& H = SCREEN_HEIGHT;
	int w = bframe.w;
	int h = bframe.h;
	SDL_SetRenderDrawColor(gRenderer, 0x39, 0x39, 0x39, 0xff);
	SDL_RenderDrawRect(gRenderer, &bframe);
	/* make a double frame */
	SDL_Rect rframe(bframe);
	int e = 3;
	rframe.x -= e; 
	rframe.y -= e;
	rframe.w += 2*e;
	rframe.h += 2*e;
	SDL_RenderDrawRect(gRenderer, &rframe);

	/* draw some grid lines: */
	SDL_Point p1,p2;
	SDL_SetRenderDrawColor(gRenderer, 0x19, 0x19, 0x1a, 0xff);
	/* vertical */
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x;
	p2.y = p1.y + h;
	for (size_t i = 1; i < 4; i++) {
		p1.x += w/4;
		p2.x += w/4;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	/* horizontal */
	p1.x = (W-w)/2;
	p1.y = (H-h)/2;
	p2.x = p1.x + w;
	p2.y = p1.y;
	for (size_t i = 1; i < 5; i++) {
		p1.y += h/5;
		p2.y += h/5;
		SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
	}
	SDL_SetRenderDrawColor(gRenderer, 0xd8, 0xde, 0xe9, 0x7f);
	SDL_Rect goal = {bframe.x + w/4 + ep, bframe.y + 3*h/5 + ep,
	                 w/2 - 2*ep, 2*h/5 - 2*ep};
	SDL_RenderDrawRect(gRenderer,&goal);
	//cout << "build_node" << endl;
	/* now iterate through and draw the blocks */
	drawBlocks();
	/* finally render contents on screen, which should happen once every
	 * vsync for the display */
	SDL_RenderPresent(gRenderer);
	//cout << "render" << endl;
}

void snap(block* b)
{
	/* TODO: once you have established a representation for configurations,
	 * you should update this function to make sure the configuration is
	 * updated when blocks are placed on the board, or taken off.  */
	assert(b != NULL);
	/* upper left of grid element (i,j) will be at
	 * bframe.{x,y} + (j*bframe.w/4,i*bframe.h/5) */
	/* translate the corner of the bounding box of the board to (0,0). */
	int x = b->R.x - bframe.x;
	int y = b->R.y - bframe.y;
	int uw = bframe.w/4;
	int uh = bframe.h/5;
	/* NOTE: in a perfect world, the above would be equal. */
	int i = (y+uh/2)/uh; /* row */
	int j = (x+uw/2)/uw; /* col */
	if (0 <= i && i < 5 && 0 <= j && j < 4) {
		b->R.x = bframe.x + j*uw + ep;
		b->R.y = bframe.y + i*uh + ep;
	}
	//cout << "snap" << endl;
}

//added
void output_image( node* cnode, int s ) {
	unsigned char image[IC*IC*HE*WI] = { 0 };
	unsigned char grid[HE][WI];
	unsigned char c;
	block* tile; //block*
	FILE* fp;
	char fn[256];
	int px, py;
	int x, y;
	int p, i;
	
	
	// build grid
	memset( grid, 0x00, HE*WI*sizeof(char) );
	for ( y = 0, p = 0; y < HE; y++ ) for ( x = 0; x < WI; x++, p++ )
		grid[y][x] = cnode->field[p];
	for ( i = 0; i < NBLOCKS; i++ ) {
		tile = cnode->tiles[i];
		for ( y = tile->y, x = tile->x, p = 0; y < HE; y++, x = 0 ) for ( ; x < WI; x++, p++ )
			if ( tile->form[p] == 1 ) grid[y][x] = i+1;
	}
	
	// translate grid to image
	for ( py = 0; py < HE; py++ ) for ( px = 0; px < WI; px++ ) {
		/*c = ( grid[py][px] == 0x00 ) ? 0x00 :
			( grid[py][px] == 0xFF ) ? 0x40 :
			( grid[py][px] <=  ntg ) ? 0xC0 : 0x80;*/
		c = ( grid[py][px] == 0x00 ) ? 0x20 :
			( grid[py][px] == 0xFF ) ? 0x00 :
			( grid[py][px] <=  ntg ) ? 0xC0 : 0x80;
		// paint inner square
		for ( y = py * IC + 1; y < (py+1) * IC - 1; y++ ) 
			for ( x = px * IC + 1; x < (px+1) * IC - 1; x++ )
				image[ y * WI * IC + x ] = c;
		// paint connections
		if ( py > 0 ) if ( grid[py][px] == grid[py-1][px] ) // top
			for ( x = px * IC + 1, y = py * IC; x < (px+1) * IC - 1; x++ )
				image[ y * WI * IC + x ] = c;
		if ( px > 0 ) if ( grid[py][px] == grid[py][px-1] ) // left
			for ( y = py * IC + 1, x = px * IC; y < (py+1) * IC - 1; y++ )
				image[ y * WI * IC + x ] = c;
		if ( py < HE-1 ) if ( grid[py][px] == grid[py+1][px] ) // bottom
			for ( x = px * IC + 1, y = (py+1) * IC - 1; x < (px+1) * IC - 1; x++ )
				image[ y * WI * IC + x ] = c;
		if ( px < WI-1 ) if ( grid[py][px] == grid[py][px+1] ) // right
			for ( y = py * IC + 1, x = (px+1) * IC - 1; y < (py+1) * IC - 1; y++ )
				image[ y * WI * IC + x ] = c;
		// inner connections
		if ( ( py < HE-1 ) && ( px < WI-1 ) ) {
			if ( ( grid[py][px] == grid[py+1][px] ) &&
				 ( grid[py][px] == grid[py][px+1] ) &&
				 ( grid[py][px] == grid[py+1][px+1] ) ) {
				image[ ((py+1)*IC-1)*WI*IC + ((px+1)*IC-1) ] = c;
				image[ ((py+1)*IC-1)*WI*IC + ((px+1)*IC-0) ] = c;
				image[ ((py+1)*IC-0)*WI*IC + ((px+1)*IC-1) ] = c;
				image[ ((py+1)*IC-0)*WI*IC + ((px+1)*IC-0) ] = c;
			}
		}
	}
	
	// image done, write to file
	sprintf( fn, "bmove_solution_%03i.pgm", s );
	fp = fopen( fn, "wb" );
	if ( fp == NULL ) err_exit( "couldn't write image" );
	fprintf( fp, "P5\n%i %i\n255\n", WI * IC, HE * IC );
	fwrite( image, sizeof( char ), IC*IC*WI*HE, fp );
	fclose( fp );
	
	//cout << "output_image" << endl;
	return;
}

//added

node* convert_puzzle( void )
{
	unsigned char* field;
	unsigned char* field2;
	block* tgrid[HE][WI];
	block** tiles;
	block* swap;
	node* node0;
	int ntp;
	int x, y;
	int p, d;
	int i;
	
	
	// alloc memory
	node0 = (node*) calloc( 1, sizeof( node ) );
	field = (unsigned char*) calloc( HE*WI, sizeof( char ) );
	field2 = (unsigned char*) calloc( HE*WI, sizeof( char ) );
	tiles = (block**) calloc( 1, sizeof( block* ) );
	
	// preset known node variables
	node0->field = field;
	node0->field2 = field2;
	node0->mother = NULL;
	node0->next = NULL;
	int tile_width;

	for(x=0;x<NBLOCKS;x++){
		B[x].id = 'a' + (x+1);
	}
	
	for(x = 0; x < WI; x++ ){
		for(y = 0; y < HE; y++ ) {
			tile_width = (SCREEN_HEIGHT*3/4)/5-2*ep;
			if(findBlock(bframe.x + x*tile_width + 24, bframe.y + y*tile_width + 24) != NULL)
				node0->field2[y*WI+x] = findBlock(bframe.x + x*tile_width + 24, bframe.y + y*tile_width + 24) -> id ;
			else{
				node0->field2[y*WI+x] = '.';
			}
		}
	}/*
	for(x = 0; x < WI; x++ ){
		for(y = 0; y < HE; y++ ) {
			cout << node0->field2[y*WI+x] << endl;
		}
		cout << "\n";
	}*/
	//}

	// seek and catalogue puzzle tiles, build base field form
	for ( x = 0, ntt = 0; x < WI; x++ ) for ( y = 0; y < HE; y++ ) {
		if ( ( node0->field2[y*WI+x] != '.' ) && ( node0->field2[y*WI+x] != ' ' ) ) {
			for ( i = 0; i < ntt; i++ ) // found one, check if new
				if ( tiles[i]->id == node0->field2[y*WI+x] ) break;
			if ( i == ntt ) { // previously unknown tile
				ntt++; // increment tile count
				if ( ntt > 1 ) tiles = (block**) realloc( tiles, ntt * sizeof( block* ) );
				tiles[ntt-1] = (block*) calloc( 1, sizeof( block ) );
				tiles[ntt-1]->form = (unsigned char*) calloc( HE*WI+1, sizeof( char ) );//check if needed or comment
				for ( d = 0; d < 4; d++ ) tiles[ntt-1]->dform[d] =
					(unsigned char*) calloc( HE*WI+1, sizeof( char ) );//check if needed or comment
				tiles[ntt-1]->id =  node0->field2[y*WI+x];
				tiles[ntt-1]->gx = -1; // check this later
				tiles[ntt-1]->gy = -1; // check this later
				tiles[ntt-1]->x = -1; // find this later
				tiles[ntt-1]->y = -1; // find this later
			}
		}
		else{
			node0->field2[y*WI+x] = 0xFF;
		}
	}
	// count and move goal tiles to front
	for ( x = 0, ntg = 0; x < WI; x++ ){
		for ( y = 0; y < HE; y++ ) {
			if ( ( solve[y][x] != '.' ) && ( solve[y][x] != ' ' ) ) {
				for ( i = ntt - 1; i >= 0; i-- ){ // seek id in known tiles
					if ( tiles[i]->id == solve[y][x] ) 
						break;
				}
				if ( i == -1 )
					err_exit( "goal tile not present in puzzle" );
				else if ( i >= ntg ){ // bubble sort unmoved tiles to front
					for ( ; i > ntg; i-- ) {
						swap = tiles[i];
						tiles[i] = tiles[i-1];
						tiles[i-1] = swap;
					}
					ntg++; // increment goal tile counter
				}
			}
		}
	}
	// analyze tiles
	for(x=0;x<NBLOCKS;x++){
		//cout << "1.2" << endl;
		tiles[x]->R.x = B[x].R.x;
		//cout << tiles[x]->R.x << endl;
		tiles[x]->R.y = B[x].R.y;
		tiles[x]->R.w = B[x].R.w;
		tiles[x]->R.h = B[x].R.h;
		//cout << "1.3" << endl;
	}
	
	for ( i = 0, ntp = 0; i < ntt; i++ ) {
		// -- x and y position --
		
		for ( x = 0; x < WI; x++ ) {
			for ( y = 0; y < HE; y++ ) if ( node0->field2[y*WI+x] == tiles[i]->id ) break;
			if ( y < HE ) { // x position
				tiles[i]->x = x;
				break;
			}
		}
		for ( y = 0; y < HE; y++ ) {
			for ( x = 0; x < WI; x++ ) if ( node0->field2[y*WI+x] == tiles[i]->id ) break;
			if ( x < WI ) { // y position
				tiles[i]->y = y;
				break;
			}
		}		
		// -- width and height --
		for ( x = WI-1; x >= 0; x-- ) {
			for ( y = 0; y < HE; y++ ) if ( node0->field2[y*WI+x] == tiles[i]->id ) break;
			if ( y < HE ) { // width
				tiles[i]->w = x - tiles[i]->x + 1;
				break;
			}
		}
		for ( y = HE-1; y >= 0; y-- ) {
			for ( x = 0; x < WI; x++ ) if ( node0->field2[y*WI+x] == tiles[i]->id ) break;
			if ( x < WI ) { // height
				tiles[i]->h = y - tiles[i]->y + 1;
				break;
			}
		}
		
		//FOUND: tile position and tile dimensions known
		// -- tile form ---
		//cout << " tile form check " << endl;
		for ( y = tiles[i]->y, x = tiles[i]->x, p = 0; y < HE; y++, x = 0 ){ 
			
			for ( ; x < WI; x++, p++ ){
				//cout << node0->field2[y*WI+x] << " form check " << tiles[i]->id << endl;
				if ( node0->field2[y*WI+x] == tiles[i]->id ){
					//cout << "form check" << endl;
					tiles[i]->form[p] = 1; // form
				}
			}
		}
		for ( p = HE*WI - 1; p >= 0; p-- )
			if ( tiles[i]->form[p] == 1 )
				break;
		memset( tiles[i]->form+p+1, 0xFF, HE*WI-p );
		// -- tile types --
		if ( i < ntg )
			tiles[i]->type = ++ntp; // goal tiles: always unique
		else { // other tiles: compare with previous forms 
			for ( p = i-1; p >= ntg; p-- )
				if ( memcmp( tiles[i]->form, tiles[p]->form, HE*WI*sizeof( char ) ) == 0 )
					break;
			if ( p < ntg )
				tiles[i]->type = ++ntp; // new unique type
			else
				tiles[i]->type = tiles[p]->type; // known type
		}
		// -- tile diff forms --
		if ( tiles[i]->h < HE ) { // up/down
			for ( y = 0, p = 0; y < HE; y++ ){
					for ( x = 0; x < WI; x++, p++ ) {
						if ( y == 0 )
							tiles[i]->dform[0][p] = tiles[i]->form[p];
						else if ( ( tiles[i]->form[p] == 1 ) && ( tiles[i]->form[p-WI] != 1 ) )
							tiles[i]->dform[0][p] = 1; // up
						if ( y == HE-1 )
							tiles[i]->dform[2][p] = tiles[i]->form[p];
						else if ( ( tiles[i]->form[p] == 1 ) && ( tiles[i]->form[p+WI] != 1 ) )
							tiles[i]->dform[2][p] = 1; // down
					}
			}
		}
		if ( tiles[i]->w < WI ) { // left/right
			for ( y = 0, p = 0; y < HE; y++ ) for ( x = 0; x < WI; x++, p++ ) {
				if ( x == 0 )
					tiles[i]->dform[1][p] = tiles[i]->form[p];
				else if ( ( tiles[i]->form[p] == 1 ) && ( tiles[i]->form[p-1] != 1 ) )
					tiles[i]->dform[1][p] = 1; // left
				if ( x == WI-1 )
					tiles[i]->dform[3][p] = tiles[i]->form[p];
				else if ( ( tiles[i]->form[p] == 1 ) && ( tiles[i]->form[p+1] != 1 ) )
					tiles[i]->dform[3][p] = 1; // right
			}
		}
		for ( d = 0; d < 4; d++ ) {
			for ( p = HE*WI-1; p >= 0; p-- )
				if ( tiles[i]->dform[d][p] == 1 )
					break;
			memset( tiles[i]->dform[d]+p+1, 0xFF, HE*WI-p );
		}/*
		//fprintf( stderr, "\n\nt%i: base",i );
		for ( p = 0; p < HE*WI; p++ ) {
			if ( p % WI == 0 )
				fprintf( stderr, "\n" );
			fprintf( stderr, "%02X ", tiles[i]->form[p] );
		}
		cout <<"form dform gap" << endl;
		for ( d = 0; d < 4; d++ ){
			fprintf( stderr, "\n\nt%i: %s", i, (d==0)?"up":(d==1)?"left":(d==2)?"down":(d==3)?"right":"" );
			for ( p = 0; p < HE*WI; p++ ) {
				if ( p % WI == 0 )
					fprintf( stderr, "\n" );
				fprintf( stderr, "%02X ", tiles[i]->dform[d][p] );
			}
		}*/
	}
	// additional info and safety checks for goal tiles
	if ( ntg == 0 ) err_exit( "no goal tiles found, nothing to do" );
	for ( i = 0; i < ntg; i++ ) {
		// --- find goal x and y position ---
		for ( x = 0; x < WI; x++ ) {
			for ( y = 0; y < HE; y++ ) if ( solve[y][x] == tiles[i]->id ) break;
			if ( y < HE ) { // goal x position
				tiles[i]->gx = x;
				break;
			}
		}
		for ( y = 0; y < HE; y++ ) {
			for ( x = 0; x < WI; x++ ) if ( solve[y][x] == tiles[i]->id ) break;
			if ( x < WI ) { // goal y position
				tiles[i]->gy = y;
				break;
			}
		}
		tiles[i]->gp = tiles[i]->gy * WI + tiles[i]->gx;
		// --- compare form ---
		for ( y = tiles[i]->gy, x = tiles[i]->gx, p = 0; y < HE; y++, x = 0 ) for ( ; x < WI; x++, p++ ) {
			if ( solve[y][x] == tiles[i]->id ) {
				if ( tiles[i]->form[p] == 0 ) err_exit( "goal tile forms don't match" );
			} else if ( tiles[i]->form[p] == 1 ) err_exit( "goal tile forms don't match" );
		}
		for ( ; p < HE*WI; p++ ) if ( tiles[i]->form[p] == 1 )
			err_exit( "goal tile forms don't match" );
	}
	// build all possible tiledef positions
	for ( i = 0; i < ntt; i++ ) {
		for ( y = 0; y <= HE - tiles[i]->h; y++ ) for ( x = 0; x <= WI - tiles[i]->w; x++ ) {
			if ( ( x != tiles[i]->x ) || ( y != tiles[i]->y ) ) {
				tgrid[y][x] = (block*) calloc( 1, sizeof( block ) );
				tgrid[y][x]->R = tiles[i]->R ;
				tgrid[y][x]->type = tiles[i]->type ;
				tgrid[y][x]->id = tiles[i]->id ;
				tgrid[y][x]->w = tiles[i]->w ;
				tgrid[y][x]->h = tiles[i]->h ;
				tgrid[y][x]->gx = tiles[i]->gx ;
				tgrid[y][x]->gy = tiles[i]->gy ;
				tgrid[y][x]->gp = tiles[i]->gp ;
				tgrid[y][x]->form = tiles[i]->form ;
				tgrid[y][x]->dform[0] = tiles[i]->dform[0] ;
				tgrid[y][x]->dform[1] = tiles[i]->dform[1] ;
				tgrid[y][x]->dform[2] = tiles[i]->dform[2] ;
				tgrid[y][x]->dform[3] = tiles[i]->dform[3] ;
				tgrid[y][x]->x = x;
				tgrid[y][x]->y = y;
			} else tgrid[y][x] = tiles[i];
			// add 1D position
			tgrid[y][x]->p = y*WI + x;
			// default: no connections
			tgrid[y][x]->pos[0] = NULL;
			tgrid[y][x]->pos[1] = NULL;
			tgrid[y][x]->pos[2] = NULL;
			tgrid[y][x]->pos[3] = NULL;
			// build connections
			if ( y > 0 ) { // vertical (up/down)
				tgrid[y][x]->pos[0] = tgrid[y-1][x];
				tgrid[y-1][x]->pos[2] = tgrid[y][x];
			}
			if ( x > 0 ) { // horizontal (left/right)
				tgrid[y][x]->pos[1] = tgrid[y][x-1];
				tgrid[y][x-1]->pos[3] = tgrid[y][x];
			}
		}
	}
	// insert tiles
	
	node0->tiles = tiles;
	for ( i = 0; i < ntt; i++ ) { 
		
		for ( p = 0; tiles[i]->form[p] != 0xFF; p++ ) if ( tiles[i]->form[p] == 1 ) {
			field[p+tiles[i]->p] = tiles[i]->type;
		}
	}
	if ( !check_node( node0 ) ) err_exit( "something went terribly wrong" );
	
	//cout << "convert_puzzle" << endl;
	// all done, ready to go!
	return node0;
}


//added
void solution(){
	block* ntile;
	node* nodes[MAX_DEPTH+1];
	node* mnode;
	node* cnode;
	node* snode;
	node* fnode;
	int nn;
	int s, t, d;
	//int x,y;
	/*
	for(x=0;x<NBLOCKS;x++){
		B[x].id = 'a' + (x+1);
	}

	//typed{
	nodes[0] = build_node();
	for(x = 0; x < WI; x++ ){
		for(y = 0; y < HE; y++ ) {
			nodes[0]->field[y*WI+x] = findBlock(x, y) -> id ;
		}
	}
	for(x = 0; x < WI; x++ ){
		for(y = 0; y < HE; y++ ) {
			fprintf( stderr, " %i\r", nodes[0]->field[y*WI+x] );
		}
		fprintf( stderr, "\r" );
	}
	//}
	*/
	nodes[0] = convert_puzzle();
	//for(int yu=0;yu<16;yu++)
		//cout << "nodes[0] -> field" << (int)(nodes[0] -> field[yu]) << endl;
	for ( s = 1, nn = 1,fnode = NULL, cnode = build_node(); s <= MAX_DEPTH; s++ ) {		
		for ( mnode = nodes[s-1], snode = NULL; mnode != NULL; mnode = mnode->next ) {
			copy_node( cnode, mnode ); // copy mother to child
			for ( t = 0; (t < ntt) && (fnode == NULL); t++ ) { // tile counter CHECK ntt
				for ( d = 0; (d < 4) && (fnode == NULL); d++ ) { // try move tile t in direction d
					if(cnode->field != NULL and cnode->tiles[t]!=NULL){
					if ( !check_move( cnode->field, cnode->tiles[t], d ) ) 
						continue; // check move
					}
					ntile = do_move( cnode->field, cnode->tiles[t], d ); // move tile
					if ( check_node( cnode ) ) { // check if new
						cnode->tiles[t] = ntile;
						if ( snode == NULL ){
							nodes[s] = cnode;
						}
						else
							snode->next = cnode;
						snode = cnode;
						cnode = build_node();
						copy_node( cnode, mnode );
						// cnode = child_node( mnode );
						// announce progress
						fprintf( stderr, "current step: %i / states found: %i\r", s, ++nn );
						// goal conditions met?
						if ( check_goal_condition( snode ) ) {
							fnode = snode;
							break;
						}
					} else
						do_move( cnode->field, ntile, (d+2)%4 ); // move tile back
				}
			}
			if ( fnode != NULL ) break;
		} if ( fnode != NULL ) break;
	}
	// summary
	if ( fnode == NULL ) fprintf( stderr, "\n->finished, no solutions found!\n" );
	else {
		fprintf( stderr, "\n-> finished, best solution has %i steps\n", s );
		for ( int i = s; fnode != NULL; fnode = fnode->mother, i-- ) {
			fprintf( stderr, "-> dumping solution: %i of %i\r", s-i, s );
			output_image( fnode, i ); 
		}
		fprintf( stderr, "\n\n" );
	}
	
}


int main(int argc, char *argv[])
{
	// TODO: add option to specify starting state from cmd line? 
	// start SDL; create window and such: 
	
	if(!init()) {
		printf( "Failed to initialize from main().\n" );
		return 1;
	}
	
	atexit(close);
	bool quit = false; // set this to exit main loop.
	SDL_Event e;
	// main loop: 
	while(!quit) {
		// handle events 
		while(SDL_PollEvent(&e) != 0) {
			// meta-q in i3, for example: 
			if(e.type == SDL_MOUSEMOTION) {
				if (mousestate == 1 && dragged) {
					int dx = e.button.x - lastm.x;
					int dy = e.button.y - lastm.y;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged->R.x += dx;
					dragged->R.y += dy;
				}
			} else if (e.type == SDL_MOUSEBUTTONDOWN) {
				if (e.button.button == SDL_BUTTON_RIGHT) {
					block* b = findBlock(e.button.x,e.button.y);
					if (b) b->rotate();
				} else {
					mousestate = 1;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					dragged = findBlock(e.button.x,e.button.y);
				}
				// XXX happens if during a drag, someone presses yet
				 // another mouse button??  Probably we should ignore it. 
			} else if (e.type == SDL_MOUSEBUTTONUP) {
				if (e.button.button == SDL_BUTTON_LEFT) {
					mousestate = 0;
					lastm.x = e.button.x;
					lastm.y = e.button.y;
					if (dragged) {
						// snap to grid if nearest location is empty. 
						snap(dragged);
					}
					dragged = NULL;
				}
			} else if (e.type == SDL_QUIT) {
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
					case SDLK_q:
						quit = true;
						break;
					case SDLK_LEFT:
						// TODO: show previous step of solution 
						break;
					case SDLK_RIGHT:
						// TODO: show next step of solution 
						break;
					case SDLK_p:
						// TODO: print the state to stdout
						 //(maybe for debugging purposes...) 
						break;
					case SDLK_s:
						// TODO: try to find a solution
						solution(); 
						break;
					default:
						break;
				}
			}
		}
		fcount++;
		render();
	}
	
	printf("total frames rendered: %i\n",fcount);
	
	return 0;
}


//  g++    -O2 -Wall -Wformat=2 -march=native -DNDEBUG -D_REENTRANT -c solver.cpp -o solver.o
// make
//  ./solver
