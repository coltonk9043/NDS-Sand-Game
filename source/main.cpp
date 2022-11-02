/*
// Colton K
// 10/20/2022
*/

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>

#define SIZE_X 128
#define SIZE_Y 96

enum Type { none, stone, sand, water };
enum Physics {nothing, particle, liquid, gas};

typedef struct{
	int x = 0;
	int y = 0;
} Cursor;

typedef struct{
	Type type = none;
	Physics physics = nothing;
	unsigned short colour = 0;
}Pixel;

Pixel pixels[SIZE_X][SIZE_Y];

void SpawnPixelAtCursor(Cursor* cursor, Type type){
	if(pixels[cursor->x][cursor->y].type == none){
		unsigned int colour = 0;
		Physics phys = nothing;
		int variance = rand() % 2 - 1;
		switch(type){
			case none:
			 	break;
			case stone:
				colour = RGB15(15 - variance, 15 - variance, 15  - variance);
				break;
			case sand:
				colour = RGB15(15 - variance, 15 - variance, 9  - variance);
				phys = particle;
				break;
			case water:
				colour = RGB15(1 - variance, 1 - variance, 15  - variance);
				phys = liquid;
				break;
		}
		pixels[cursor->x][cursor->y].colour = colour;
		pixels[cursor->x][cursor->y].type = type;
		pixels[cursor->x][cursor->y].physics = phys;
	}
}


void UpdatePixels(){
	// Updates all of the pixels
	for(int y = SIZE_Y; y >= 0; y--){
		for(int x = 0; x < SIZE_X; x++){
			Pixel* p = &pixels[x][y];
			if(p->physics == nothing) continue;
			if(p->type == none) continue;

			Pixel* below = &pixels[x][y+1];
			Pixel* toLeft = &pixels[x-1][y];
			Pixel* toRight = &pixels[x+1][y];
			Pixel* moveRight = &pixels[x+1][y+1];
			Pixel* moveLeft = &pixels[x-1][y+1];

			if(p->physics == particle){
				if(below->type == none || below->physics == liquid){
					Type tempType = below->type;
					Physics tempPhysics = below->physics;
					unsigned short tempColour = below->colour;

					below->type = p->type;
					below->colour = p->colour;
					below->physics = p->physics;
					p->type = tempType;
					p->physics = tempPhysics;
					p->colour = tempColour;
					continue;
				}

				if(toLeft->type != none && toRight->type != none) continue;

				if(toLeft->type != none){
					if(moveRight->type == none){
						moveRight->type = p->type;
						moveRight->colour = p->colour;
						moveRight->physics = p->physics;
						p->type = none;
						p->physics = nothing;
						p->colour = 0;
						continue;
					}
				}else{
					if(moveLeft->type == none){
						moveLeft->type = p->type;
						moveLeft->colour = p->colour;
						moveLeft->physics = p->physics;
						p->type = none;
						p->physics = nothing;
						p->colour = 0;
						continue;
					}
				}

				if(toRight->type != none){
					if(moveLeft->type == none){
						moveLeft->type = p->type;
						moveLeft->colour = p->colour;
						moveLeft->physics = p->physics;
						p->type = none;
						p->colour = 0;
						continue;
					}
				}else{
					if(moveRight->type == none){
						moveRight->type = p->type;
						moveRight->colour = p->colour;
						moveRight->physics = p->physics;
						p->type = none;
						p->physics = nothing;
						p->colour = 0;
						continue;
					}
				}
			}else if(p->physics == liquid){
				int direction = 0;
				if(below->type == none){
					below->type = p->type;
					below->colour = p->colour;
					below->physics = p->physics;
					p->type = none;
					p->physics = nothing;
					p->colour = 0;
					continue;
				}else{
					direction = rand() % 2 - 1;
					Pixel* moveTo = direction == 0 ? toLeft : toRight;
					if(moveTo->type != none) moveTo = direction == 1 ? toLeft : toRight;
					if(moveTo->type == none){
						Pixel* underneath;
						if(moveTo == toLeft){
							underneath = moveLeft;
						}else if (moveTo == toRight){
							underneath = moveRight;
						}

						if(underneath->type == none){
							underneath->type = p->type;
							underneath->colour = p->colour;
							underneath->physics = p->physics;
							p->type = none;
							p->physics = nothing;
							p->colour = 0;
							continue;
						}else{
							moveTo->type = p->type;
							moveTo->colour = p->colour;
							moveTo->physics = liquid;
							p->type = none;
							p->physics = nothing;
							p->colour = 0;
						}
						continue;
					}
				}
			}
		}
	}
}

void DrawPixels(){
	for(int y = 0; y < SIZE_Y; y++){
		for(int x = 0; x < SIZE_X; x++){
			int offset = (y * 512) + (x * 2);
			VRAM_A[offset] = pixels[x][y].colour;
			VRAM_A[offset+1] = pixels[x][y].colour;
			VRAM_A[offset+256] = pixels[x][y].colour;
			VRAM_A[offset+257] = pixels[x][y].colour;
		}
	}
}



int main()
{	
	//set mode FB0
	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_0_2D);

	// Initializes the default console.
	consoleDemoInit();

	// Sets our VRAM banks.
	vramSetBankA( VRAM_A_LCD );
	vramSetBankB( VRAM_B_TEXTURE );

	// Game Variables
	Cursor c;
	Type currentType = stone;

	// Game loop.
	while(1) {
		// Game Update Logic
		// Scans for keys.
		scanKeys();
		u16 keys = keysHeld();

		// Game Logic
		if((keys & KEY_UP)){
			if(c.y > 0) c.y -= 1;
		}
		if((keys & KEY_DOWN)){
			if (c.y < SIZE_Y) c.y += 1;
		}
		if((keys & KEY_RIGHT)){
			if(c.x < SIZE_X) c.x += 1;
		}
		if((keys & KEY_LEFT)){
			if(c.x > 0) c.x -= 1;
		}

		if((keys & KEY_A)){
			SpawnPixelAtCursor(&c, currentType);
		}

		if((keys & KEY_R)){
			currentType = Type((currentType + 1) % 4);
		}

		if((keys & KEY_L)){
			if(currentType == none){
				currentType = water;
			}else{
				currentType = Type(currentType - 1);
			}
		}

		UpdatePixels();

		DrawPixels();

		// Waits for a screen refresh and waits till the render engine is not busy.
		swiWaitForVBlank();
		
		// Prints out debug data and controls.
		consoleClear();
		printf("          Controls:\n\n");
		printf("D-PAD   - Move Cursor\n");
		printf("A       - Place Pixel\n\n");
		printf("Cursor Position (X,Y): %i, %i\n", c.x, c.y);
		printf("Current Tile: %i\n", currentType);
		glFlush(0);
	}

	return 0;
}
