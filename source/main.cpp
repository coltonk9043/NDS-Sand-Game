/*
// Colton K
// 10/20/2022
*/

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>

// Texture Headers
#include "ui_brush.h"
#include "ui_eraser.h"
#include "ui_fill.h"

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

// Debug
bool debug = false;

// World Information
Pixel pixels[SIZE_X][SIZE_Y];

// Textures
u16* brushIcon;
u16* eraserIcon;
u16* fillIcon;

void MovePixelToLocation(Pixel* source, Pixel* destination){
	Type tempType = destination->type;
	Physics tempPhysics = destination->physics;
	unsigned short tempColour = destination->colour;

	destination->type = source->type;
	destination->colour = source->colour;
	destination->physics = source->physics;
	source->type = tempType;
	source->physics = tempPhysics;
	source->colour = tempColour;
}

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
					MovePixelToLocation(p, below);
					continue;
				}
				if(toLeft->type == none){
					if(moveLeft->type == none){
						MovePixelToLocation(p, moveLeft);
						continue;
					}
				}
				if(toRight->type == none){
					if(moveRight->type == none){
						MovePixelToLocation(p, moveRight);
						continue;
					}
				}
			}else if(p->physics == liquid){
				int direction = 0;
				if(below->type == none){
					MovePixelToLocation(p, below);
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
							MovePixelToLocation(p, underneath);
							continue;
						}else{
							MovePixelToLocation(p, moveTo);
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

void DrawUI(Cursor* c){
	int offset((c->y * 512) + (c->x * 2));
	VRAM_A[offset] = RGB15(15, 0, 0);
	VRAM_A[offset+1] = RGB15(15, 0, 0);
	VRAM_A[offset+256] = RGB15(15, 0, 0);
	VRAM_A[offset+257] = RGB15(15, 0, 0);
	oamSet(&oamSub, 0, 0, 8, 0, 0, SpriteSize_32x32, SpriteColorFormat_16Color , brushIcon, 0, false, false, false, false, false); 
}

int main()
{	
	//set mode FB0
	videoSetMode(MODE_FB0);

    if(debug){
        // Initializes the default console.
        videoSetModeSub(MODE_0_2D);
	    consoleDemoInit();
    }else{
        videoSetModeSub(MODE_5_2D | DISPLAY_SPR_ACTIVE | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
		oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
    }

	// Sets our VRAM banks.
	vramSetBankA( VRAM_A_LCD );
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000 );

    // Loads Textures
    brushIcon = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(ui_brushTiles, brushIcon, ui_brushTilesLen);
	dmaCopy(ui_brushPal, &SPRITE_PALETTE[0], ui_brushPalLen);

    eraserIcon = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(ui_eraserTiles, eraserIcon, ui_eraserTilesLen);
	dmaCopy(ui_eraserPal, &SPRITE_PALETTE[16], ui_eraserPalLen);

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
		if(!debug){
			DrawUI(&c);
		}
		
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
