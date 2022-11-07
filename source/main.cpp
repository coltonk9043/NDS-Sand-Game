/*
// Colton K
// 10/20/2022
*/

#include <nds.h>
#include <stdlib.h>
#include <stdio.h>

#include "Button.h"

// Texture Headers
#include "ui_brush.h"
#include "ui_eraser.h"
#include "ui_fill.h"
#include "crosshair.h"

#define SIZE_X 128
#define SIZE_Y 96

enum Type { none, stone, dirt, sand, water, lava };
enum Physics {nothing, particle, liquid, gas};
enum Tool {brush, eraser, fill};

typedef struct{
	int x = 0;
	int y = 0;
} Cursor;

typedef struct{
	Type type = none;
	Physics physics = nothing;
	unsigned short colour = 0;
}Pixel;

// World Information
Pixel pixels[SIZE_X][SIZE_Y];
Tool currentTool = brush;

// Textures
u16* brushIcon;
u16* eraserIcon;
u16* fillIcon;

u16* crosshairIcon;

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

void SpawnPixel(Cursor* cursor, Type type){
	if(pixels[cursor->x][cursor->y].type == none){
		unsigned int colour = 0;
		int variance = rand() % 2 - 1;
		Physics phys = nothing;
		switch(type){
			case none:
				return;
			case stone:
				colour = ARGB16(1, 15 - variance, 15 - variance, 15  - variance);
				break;
			case dirt:
				colour = ARGB16(1, 10 - variance, 8 - variance, 5  - variance);
				phys = particle;
				break;
			case sand:
				colour = ARGB16(1, 30 - variance, 30 - variance, 18  - variance);
				phys = particle;
				break;
			case water:
				colour = ARGB16(1, 2 - variance, 2 - variance, 30  - variance);
				phys = liquid;
				break;
			case lava:
				colour = ARGB16(1, 30, 12 - (variance * 2), 0);
				phys = liquid;
				break;
		}
		pixels[cursor->x][cursor->y].colour = colour;
		pixels[cursor->x][cursor->y].type = type;
		pixels[cursor->x][cursor->y].physics = phys;
	}
}

void DeletePixel(Cursor* cursor){
	pixels[cursor->x][cursor->y].colour =  ARGB16(0, 0, 0, 0);
	pixels[cursor->x][cursor->y].type = none;
	pixels[cursor->x][cursor->y].physics = nothing;
}

void UpdatePixels(){
	// Updates all of the pixels
	for(int y = SIZE_Y-1; y >= 0; y--){
		for(int x = 0; x < SIZE_X; x++){
			Pixel* p = &pixels[x][y];
			if(p->physics == nothing) continue;

			/* Pixel Logic Section (ie. interacting with other pixels.)*/
			if(p->type == none) continue;
			if(p->type == dirt){
				Pixel* above = &pixels[x][y-1];
				int variance = rand() % 2 - 1;
				int chance = rand() % 600 - 1;
				if(chance < 1){
					if(above->type == none){
						p->colour = ARGB16(1, 0 - variance, 20 - variance, 1 - variance);
					}else{
						p->colour = ARGB16(1, 10 - variance, 8 - variance, 5  - variance);
					}
				}
			}
			if(p->type == lava){
				int variance = rand() % 2 - 1;
				bool turnToStone = false;
				Pixel* above = &pixels[x][y-1];
				Pixel* below = &pixels[x][y+1];
				Pixel* right = &pixels[x][y+1];
				Pixel* left = &pixels[x][y-1];

				if(above->type == water){
					turnToStone = true;
					above->type = none;
					above->physics = nothing;
					above->colour = ARGB16(0, 0,0,0);
				}else if (below->type == water){
					turnToStone = true;
					below->type = none;
					below->physics = nothing;
					above->colour = ARGB16(0, 0,0,0);
				}else if(left->type == water){
					turnToStone = true;
					left->type = none;
					left->physics = nothing;
					above->colour = ARGB16(0, 0,0,0);
				}else if (right->type == water){
					turnToStone = true;
					right->type = none;
					right->physics = nothing;
					above->colour = ARGB16(0, 0,0,0);
				}

				if(turnToStone){
					p->type = stone;
					p->physics = nothing;
					p->colour = ARGB16(1, 5 - variance, 2 - variance, 5  - variance);
				}
			}


			Pixel* below = &pixels[x][y+1];
			Pixel* toLeft = &pixels[x-1][y];
			Pixel* toRight = &pixels[x+1][y];
			Pixel* moveRight = &pixels[x+1][y+1];
			Pixel* moveLeft = &pixels[x-1][y+1];

			if(p->physics == particle){
				if(y == SIZE_Y-1) continue;
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
					if(y == SIZE_Y-1) continue;
					MovePixelToLocation(p, below);
					continue;
				}else{
					direction = rand() % 2 - 1;
					Pixel* moveTo = direction == 0 ? toLeft : toRight;
					if(moveTo->type != none) moveTo = direction == 1 ? toLeft : toRight;
					if(moveTo->type == none){
						Pixel* underneath;
						if(y < SIZE_Y-1){
							if(moveTo == toLeft){
								underneath = moveLeft;
							}else if (moveTo == toRight){
								underneath = moveRight;
							}
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

void DrawPixels(u16* vram){
	for(int y = 0; y < SIZE_Y; y++){
		for(int x = 0; x < SIZE_X; x++){
			int offset = (y * 512) + (x * 2);
			vram[offset] = pixels[x][y].colour;
			vram[offset + 1] = pixels[x][y].colour;
			vram[offset + 256] = pixels[x][y].colour;
			vram[offset + 257] = pixels[x][y].colour;
		}
	}
}

int main()
{	
	touchPosition touch;

	videoSetMode(MODE_5_2D | DISPLAY_SPR_ACTIVE | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_1D_BMP);
	videoSetModeSub(MODE_5_2D);
	vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_SPRITE, VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);

    // Loads Textures
    brushIcon = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(ui_brushTiles, brushIcon, ui_brushTilesLen);
	dmaCopy(ui_brushPal, &(SPRITE_PALETTE_SUB[0]), ui_brushPalLen);

    eraserIcon = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(ui_eraserTiles, eraserIcon, ui_eraserTilesLen);
	dmaCopy(ui_eraserPal, &(SPRITE_PALETTE_SUB[16]), ui_eraserPalLen);

	fillIcon = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(ui_fillTiles, fillIcon, ui_fillTilesLen);
	dmaCopy(ui_fillPal, &(SPRITE_PALETTE_SUB[32]), ui_fillPalLen);

	crosshairIcon = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_16Color );
	dmaCopy(crosshairTiles, crosshairIcon, crosshairTilesLen);
	dmaCopy(crosshairPal, &(SPRITE_PALETTE[0]), crosshairPalLen);

	Button brushButton = {0,8,32,32, brushIcon, 0};
	Button eraserButton = {-4,40,32,32, eraserIcon, 1};
	Button fillButton = {-4,72,32,32, fillIcon, 2};

	// Intiailizes the background and gets the pointer in VRAM memory.
	int bgMain = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	int bgSub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0,0);
	u16* video_buffer_main = bgGetGfxPtr(bgMain);
	u16* video_buffer_sub = bgGetGfxPtr(bgSub);

	setBackdropColor(RGB15(16, 25, 30));
	setBackdropColorSub(RGB15(15, 15, 15));

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
		if(keys & KEY_TOUCH){
			touchRead(&touch);
			if(brushButton.Clicked(touch.px, touch.py)){
				currentTool = brush;
				brushButton.x = 0;
				eraserButton.x = -4;
				fillButton.x = -4;
			} 
			if(eraserButton.Clicked(touch.px, touch.py)){
				currentTool = eraser;
				brushButton.x = -4;
				eraserButton.x = 0;
				fillButton.x = -4;
			}
			if(fillButton.Clicked(touch.px, touch.py)){
				currentTool = fill;
				brushButton.x = -4;
				eraserButton.x = -4;
				fillButton.x = 0;
			}
		}
			
		if((keys & KEY_UP)) if(c.y > 0) c.y -= 1;
		if((keys & KEY_DOWN)) if (c.y < SIZE_Y) c.y += 1;
		if((keys & KEY_RIGHT)) if(c.x < SIZE_X) c.x += 1;
		if((keys & KEY_LEFT)) if(c.x > 0) c.x -= 1;
		if((keys & KEY_A)){
			if(currentTool == brush){
				SpawnPixel(&c, currentType);
			}else if (currentTool == eraser){
				DeletePixel(&c);
			}
		} 
		if((keys & KEY_R)) currentType = Type((currentType + 1) % 6);
		if((keys & KEY_L)){
			if(currentType == none){
				currentType = water;
			}else{
				currentType = Type(currentType - 1);
			}
		}



		UpdatePixels();

		// Draw Pixels into Background.
		DrawPixels(video_buffer_main);

		// Draw UI in Sub Engine.
		brushButton.Draw();
		eraserButton.Draw();
		fillButton.Draw();
		oamSet(&oamMain, 0, c.x*2-16, c.y*2-16, 0, 0, SpriteSize_32x32, SpriteColorFormat_16Color, crosshairIcon, 0, false, false, false, false, false); 

		// Waits for a screen refresh and waits till the render engine is not busy.
		swiWaitForVBlank();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);

		// Prints out debug data and controls.
		consoleClear();
		printf("          Controls:\n\n");
		printf("D-PAD   - Move Cursor\n");
		printf("A       - Place Pixel\n\n");
		printf("Cursor Position (X,Y): %i, %i\n", c.x, c.y);
		printf("Current Tile: %i\n", currentType);
	}

	return 0;
}
